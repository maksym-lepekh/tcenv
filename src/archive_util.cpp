#include "archive_util.hpp"
#include "common.hpp"
#include "c_api.hpp"
#include <archive.h>
#include <archive_entry.h>

namespace
{
    int copy_data(archive *ar, archive *aw)
    {
        for (;;) {
            const void *buff;
            size_t size;
            int64_t offset;

            auto r = archive_read_data_block(ar, &buff, &size, &offset);
            if (r == ARCHIVE_EOF)
                return (ARCHIVE_OK);
            if (r != ARCHIVE_OK)
                return r;
            auto ret = archive_write_data_block(aw, buff, size, offset);
            if (ret != ARCHIVE_OK) {
                log("archive_write_data_block() ", archive_error_string(aw));
                return ret;
            }
        }
    }
}

bool archive_util::extract(const fs::path& input, const fs::path& dest)
{
    log("input = ", input);
    log("dest = ", dest);

    auto reader = c_api::opaque(archive_read_new, archive_read_free);

    archive_read_support_format_all(reader);
    archive_read_support_filter_all(reader);

    auto ret = archive_read_open_filename(reader, input.c_str(), 10240);
    if (ret != 0)
    {
        log("archive_read_open_filename() ", ret, " ", archive_error_string(reader));
        return false;
    }
    AT_SCOPE_EXIT(archive_read_close(reader));

    auto writer = c_api::opaque(archive_write_disk_new, archive_write_free);

    for(;;)
    {
        struct archive_entry *entry;
        ret = archive_read_next_header(reader, &entry);
        if (ret == ARCHIVE_EOF)
            break;

        if (ret != ARCHIVE_OK)
        {
            log("archive_read_next_header() ", archive_error_string(reader));
            return false;
        }

        auto dest_pathname = dest / archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, dest_pathname.c_str());

        ret = archive_write_header(writer, entry);
        if (ret != ARCHIVE_OK)
        {
            log("archive_write_header() ", archive_error_string(writer));
            return false;
        }
        else {
            copy_data(reader, writer);
            ret = archive_write_finish_entry(writer);
            if (ret != ARCHIVE_OK)
            {
                log("archive_write_finish_entry() ", archive_error_string(writer));
                return false;
            }
        }
    }

    return true;
}
