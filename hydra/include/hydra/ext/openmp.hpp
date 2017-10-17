#pragma once

namespace {
#include <cstdint>
}

#ifdef _WIN32
typedef intptr_t int_openmp_t;
#else
typedef size_t int_openmp_t;
#endif