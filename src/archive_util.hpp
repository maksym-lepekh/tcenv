//
// Created by maxl on 11.08.23.
//

#ifndef TCENV_ARCHIVE_UTIL_HPP
#define TCENV_ARCHIVE_UTIL_HPP


#include <boost/filesystem.hpp>
#include <archive.h>


namespace archive_util
{
    namespace fs = boost::filesystem;

    bool extract(fs::path input, fs::path dest);
}

#endif //TCENV_ARCHIVE_UTIL_HPP
