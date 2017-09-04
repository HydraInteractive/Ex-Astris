/**
 * Defines a API macro to export/import symbols on Windows
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
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
