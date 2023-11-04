#ifndef TCENV_CONTROL_FLOW_HPP
#define TCENV_CONTROL_FLOW_HPP

#include <gsl/util>

struct finally_receiver
{
    auto operator->*(auto&& fn)
    {
        return gsl::finally(std::forward<decltype(fn)>(fn));
    }
};

#define TCENV_STRING_CAT(a, b) a##b
#define TCENV_MAKE_VAR_NAME(L) TCENV_STRING_CAT(tcenv_exit_, L)
#define FINALLY                auto TCENV_MAKE_VAR_NAME(__LINE__) = finally_receiver{}->*[&]

#define TRY(...)                                                                          \
    if (auto TCENV_MAKE_VAR_NAME(__LINE__) = __VA_ARGS__; !TCENV_MAKE_VAR_NAME(__LINE__)) \
        return TCENV_MAKE_VAR_NAME(__LINE__);

#endif    // TCENV_CONTROL_FLOW_HPP
