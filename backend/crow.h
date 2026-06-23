// This file is a placeholder for local builds.
// When using CMake (recommended), Crow is fetched automatically via FetchContent.
// For manual builds, download Crow from https://github.com/CrowCpp/Crow
// and place crow_all.h here, then replace this file with:
//   #include "crow_all.h"
//
// CMake build (preferred):
//   mkdir build && cd build && cmake .. && make
//
// After building, run:
//   ./bin/startup_validator YOUR_GEMINI_API_KEY
//   OR: export GEMINI_API_KEY=your_key && ./bin/startup_validator

// For CMake builds, Crow include path is set automatically.
// This file only exists as documentation for non-CMake users.
#pragma once
#ifdef CROW_ALL_INCLUDE_PRESENT
#include "crow_all.h"
#else
// CMake will provide the correct include path
#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/routing.h>
#endif
