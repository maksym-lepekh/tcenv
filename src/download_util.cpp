#include "download_util.hpp"

#include "logger.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/url.hpp>
#include <openssl/ssl.h>

using namespace std::literals;
namespace net   = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
namespace fs    = std::filesystem;

namespace
{
    auto to_msg(const beast::error_code& ecode) -> std::string
    {
        return ecode.category().name() + ":"s + ecode.message();
    }
}    // namespace

namespace download_util
{
    auto download(const std::string& url, const path& dest) -> result<path>
    {
        auto parsed_url = boost::urls::parse_uri(url);
        auto host       = std::string(parsed_url->encoded_host());
        auto resource   = std::string(parsed_url->encoded_resource());
        auto file_name  = std::string(parsed_url->encoded_path());
        if (file_name.empty())
        {
            file_name = "file";
        }
        if (file_name.contains('/'))
        {
            file_name = file_name.substr(file_name.find_last_of('/') + 1);
        }
        auto dest_file = dest / file_name;

        logger::debug("host =", host);
        logger::debug("resource =", resource);
        logger::debug("dest_file =", dest_file);

        auto ioc = net::io_context{};
        auto ctx = net::ssl::context{net::ssl::context::tlsv12_client};
        ctx.set_default_verify_paths();

        auto stream = beast::ssl_stream<beast::tcp_stream>{ioc, ctx};

        stream.set_verify_mode(net::ssl::verify_none);
        stream.set_verify_callback(
            [](bool, net::ssl::verify_context&)
            {
                return true;    // Accept any certificate
            });

        // Enable SNI
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
        {
            beast::error_code error_c{static_cast<int>(ERR_get_error()), net::error::get_ssl_category()};
            return std::unexpected(error_t{to_msg(error_c)});
        }

        // Connect to the HTTPS server
        constexpr auto connect_timeout = std::chrono::seconds(30);
        auto resolver                  = net::ip::tcp::resolver{ioc};
        get_lowest_layer(stream).connect(resolver.resolve({host, "443"}));
        get_lowest_layer(stream).expires_after(connect_timeout);

        // Construct request
        constexpr auto http_1_1_ver = 11;
        auto req                    = http::request<http::empty_body>{http::verb::get, resource, http_1_1_ver};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the request
        stream.handshake(net::ssl::stream_base::client);
        http::write(stream, req);

        // Receive the response
        beast::flat_buffer buffer;
        auto res   = http::response<http::file_body>{};
        auto err_c = beast::error_code{};
        res.body().open(dest_file.c_str(), beast::file_mode::write, err_c);
        if (err_c)
        {
            return std::unexpected(error_t{to_msg(err_c)});
        }

        http::read(stream, buffer, res);

        // Cleanup
        stream.shutdown(err_c);
        if (err_c == net::error::eof)
        {
            err_c = {};
        }
        if (err_c)
        {
            return std::unexpected(error_t{to_msg(err_c)});
        }

        return dest_file;
    }
}    // namespace download_util
