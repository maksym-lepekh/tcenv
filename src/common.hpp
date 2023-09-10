//
// Created by maxl on 11.08.23.
//

#ifndef TCENV_COMMON_HPP
#define TCENV_COMMON_HPP

#include <functional>
#include <iostream>


namespace common
{
    struct scope_end_sentinel
    {
        std::function<void()> job;
        ~scope_end_sentinel()
        {
            if (job) job();
        }
    };
}

#define TCENV_STRING_CAT(a, b) a##b
#define TCENV_MAKE_VAR_NAME(L) TCENV_STRING_CAT(tcenv_exit_, L)
#define AT_SCOPE_EXIT(X) auto TCENV_MAKE_VAR_NAME(__LINE__) = common::scope_end_sentinel([&](){X;})

#endif //TCENV_COMMON_HPP
