/**
 * Defines a API macro to export/import symbols on Windows
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once

#ifdef _WIN32
#ifdef HYDRA_BASE_EXPORTS
#define HYDRA_BASE_API __declspec(dllexport)
#define HYDRA_BASE_SET_NULLPTR = nullptr
#else
#define HYDRA_BASE_API __declspec(dllimport)
#define HYDRA_BASE_SET_NULLPTR
#endif

#ifdef HYDRA_GRAPHICS_EXPORTS
#define HYDRA_GRAPHICS_API __declspec(dllexport)
#define HYDRA_GRAPHICS_SET_NULLPTR = nullptr
#else
#define HYDRA_GRAPHICS_API __declspec(dllimport)
#define HYDRA_GRAPHICS_SET_NULLPTR
#endif

#ifdef HYDRA_NETWORK_EXPORTS
#define HYDRA_NETWORK_API __declspec(dllexport)
#define HYDRA_NETWORK_SET_NULLPTR = nullptr
#else
#define HYDRA_NETWORK_API __declspec(dllimport)
#define HYDRA_NETWORK_SET_NULLPTR
#endif

#ifdef HYDRA_PHYSICS_EXPORTS
#define HYDRA_PHYSICS_API __declspec(dllexport)
#define HYDRA_PHYSICS_SET_NULLPTR = nullptr
#else
#define HYDRA_PHYSICS_API __declspec(dllimport)
#define HYDRA_PHYSICS_SET_NULLPTR
#endif

#ifdef HYDRA_SOUND_EXPORTS
#define HYDRA_SOUND_API __declspec(dllexport)
#define HYDRA_SOUND_SET_NULLPTR = nullptr
#else
#define HYDRA_SOUND_API __declspec(dllimport)
#define HYDRA_SOUND_SET_NULLPTR
#endif

#else
#define HYDRA_BASE_API
#define HYDRA_GRAPHICS_API
#define HYDRA_NETWORK_API
#define HYDRA_PHYSICS_API
#define HYDRA_SOUND_API

#define HYDRA_BASE_SET_NULLPTR = nullptr
#define HYDRA_GRAPHICS_SET_NULLPTR = nullptr
#define HYDRA_NETWORK_SET_NULLPTR = nullptr
#define HYDRA_PHYSICS_SET_NULLPTR = nullptr
#define HYDRA_SOUND_SET_NULLPTR = nullptr
#endif

