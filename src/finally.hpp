//
// Created by maxl on 11.08.23.
//

#ifndef TCENV_FINALLY_HPP
#define TCENV_FINALLY_HPP

#include <gsl/util>

#define TCENV_STRING_CAT(a, b) a##b
#define TCENV_MAKE_VAR_NAME(L) TCENV_STRING_CAT(tcenv_exit_, L)
#define FINALLY(X)             auto TCENV_MAKE_VAR_NAME(__LINE__) = gsl::finally([&]() { X; })

#endif    // TCENV_FINALLY_HPP
