//
// Created by maxl on 11.08.23.
//

#ifndef TCENV_ARCHIVE_UTIL_HPP
#define TCENV_ARCHIVE_UTIL_HPP


#include <boost/filesystem.hpp>

namespace archive_util
{
    namespace fs = boost::filesystem;

    bool extract(const fs::path& input, const fs::path& dest);
}

#endif //TCENV_ARCHIVE_UTIL_HPP
