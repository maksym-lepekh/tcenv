#include "download_util.hpp"
#include "c_api.hpp"
#include "common.hpp"
#include "log.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/url.hpp>
#include <openssl/ssl.h>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = boost::filesystem;


auto download_util::download(const std::string& url, const fs::path& dest)
    -> std::expected<fs::path, boost::beast::error_code>
{
    auto parsed_url = boost::urls::parse_uri(url);
    auto host = std::string(parsed_url->encoded_host());
    auto resource = std::string (parsed_url->encoded_resource());
    auto fname = std::string(parsed_url->encoded_path());
    if (fname.empty()) {
        fname = "file";
    }
    if (fname.contains('/')) {
        fname = fname.substr(fname.find_last_of('/'));
    }
    auto dest_file = dest / fname;

    log::debug("host =", host);
    log::debug("resource =", resource);
    log::debug("dest_file =", dest_file);

    auto ioc = net::io_context{};
    auto ctx = net::ssl::context{net::ssl::context::tlsv12_client};
    ctx.set_default_verify_paths();

    auto stream = beast::ssl_stream<beast::tcp_stream>{ioc, ctx};

    stream.set_verify_mode(net::ssl::verify_none);
    stream.set_verify_callback([](bool preverified, net::ssl::verify_context& ctx) {
        return true; // Accept any certificate
    });

    // Enable SNI
    if(!SSL_set_tlsext_host_name(stream.native_handle(), host.data())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    // Connect to the HTTPS server
    auto resolver = net::ip::tcp::resolver{ioc};
    get_lowest_layer(stream).connect(resolver.resolve({host, "443"}));
    get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Construct request
    auto req = http::request<http::empty_body>{http::verb::get, resource , 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the request
    stream.handshake(net::ssl::stream_base::client);
    http::write(stream, req);


    // Receive the response
    beast::flat_buffer buffer;
    auto res = http::response<http::file_body>{};
    auto ec = boost::beast::error_code{};
    res.body().open(dest_file.c_str(), beast::file_mode::write, ec);
    if (ec)
    {
        throw beast::system_error{ec};
    }

    http::read(stream, buffer, res);

    // Cleanup
    stream.shutdown(ec);
    if (ec == net::error::eof) {
        ec = {};
    }
    if (ec) {
        throw beast::system_error{ec};
    }

    return dest_file;
}
