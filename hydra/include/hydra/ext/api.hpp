#pragma once

#ifdef _WIN32
#ifdef HYDRA_EXPORTS
#define HYDRA_API __declspec(dllexport)
#else
#define HYDRA_API __declspec(dllimport)
#endif
#else
#define HYDRA_API
#endif