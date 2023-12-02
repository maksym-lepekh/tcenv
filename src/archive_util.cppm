module;
#include "control_flow.hpp"

#include <archive.h>
#include <archive_entry.h>
#include <gsl/pointers>

export module archive_util;
import std;
import error;
import logger;
import c_api;

export namespace archive_util
{
    using std::filesystem::path;
    auto extract(const path& input, const path& dest) -> result<void>;
}

module :private;
namespace
{
    auto copy_data(const gsl::not_null<archive*> a_reader, const gsl::not_null<archive*> a_writer) -> result<void>
    {
        for (;;)
        {
            const void* buff = {};
            size_t size      = {};
            int64_t offset   = {};

            const auto res = archive_read_data_block(a_reader, &buff, &size, &offset);
            if (res == ARCHIVE_EOF)
            {
                return {};
            }
            if (res != ARCHIVE_OK)
            {
                return std::unexpected(error_t{archive_error_string(a_reader)});
            }

            if (const auto ret = archive_write_data_block(a_writer, buff, size, offset); ret != ARCHIVE_OK)
            {
                return std::unexpected(error_t{archive_error_string(a_writer)});
            }
        }
    }
}    // namespace

namespace archive_util
{
    auto extract(const path& input, const path& dest) -> result<void>
    {
        logger::debug("input = ", input);
        logger::debug("dest = ", dest);

        auto reader = c_api::opaque(archive_read_new, archive_read_free);

        archive_read_support_format_all(reader);
        archive_read_support_filter_all(reader);

        constexpr auto chunk_size = 10'240;
        auto ret                  = archive_read_open_filename(reader, input.c_str(), chunk_size);
        if (ret != ARCHIVE_OK)
        {
            return std::unexpected(error_t{archive_error_string(reader)});
        }

        FINALLY
        {
            archive_read_close(reader);
        };

        auto writer = c_api::opaque(archive_write_disk_new, archive_write_free);
        archive_write_disk_set_options(writer, ARCHIVE_EXTRACT_TIME);

        for (;;)
        {
            archive_entry* entry = nullptr;
            ret                  = archive_read_next_header(reader, &entry);
            if (ret == ARCHIVE_EOF)
            {
                break;
            }

            if (ret != ARCHIVE_OK)
            {
                return std::unexpected(error_t{archive_error_string(reader)});
            }

            auto dest_pathname = dest / archive_entry_pathname(entry);
            archive_entry_set_pathname(entry, dest_pathname.c_str());

            ret = archive_write_header(writer, entry);
            if (ret != ARCHIVE_OK)
            {
                return std::unexpected(error_t{archive_error_string(writer)});
            }

            TRY(copy_data(reader, writer));
            ret = archive_write_finish_entry(writer);
            if (ret != ARCHIVE_OK)
            {
                return std::unexpected(error_t{archive_error_string(writer)});
            }
        }

        return {};
    }
}    // namespace archive_util
