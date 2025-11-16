/**
 * pch.hpp
 *
 * Project-wide pre-compiled header.
 */

#pragma once

/* ----------------------------------- STL ---------------------------------- */
#include <crtdbg.h>            // IWYU pragma: export

#include <algorithm>           // IWYU pragma: export
#include <array>               // IWYU pragma: export
#include <atomic>              // IWYU pragma: export
#include <chrono>              // IWYU pragma: export
#include <climits>             // IWYU pragma: export
#include <condition_variable>  // IWYU pragma: export
#include <cstddef>             // IWYU pragma: export
#include <cstdint>             // IWYU pragma: export
#include <cstdlib>             // IWYU pragma: export
#include <filesystem>          // IWYU pragma: epxort
#include <format>              // IWYU pragma: export
#include <mutex>               // IWYU pragma: export
#include <string>              // IWYU pragma: export
#include <string_view>         // IWYU pragma: export
#include <thread>              // IWYU pragma: export
#include <vector>              // IWYU pragma: export


/* --------------------------------- OpenCV --------------------------------- */
#include <opencv2/opencv.hpp>  // IWYU pragma: export

/* --------------------------------- Windows -------------------------------- */
#ifdef _WIN64
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>  // IWYU pragma: export
#else
#error Requires a Windows(x64) compilation target.
#endif

/* ---------------------------------- Tracy --------------------------------- */
#include <tracy/Tracy.hpp>  // IWYU pragma: export