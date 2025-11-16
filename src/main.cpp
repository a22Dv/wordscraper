/**
 * main.cpp
 *
 * Program entry point.
 */

#ifndef _WIN64
#error Requires a Windows(x64) compilation target.
#endif

#include "core/config.hpp"
#include "core/pch.hpp"  // IWYU pragma: export
#include "core/screenshot.hpp"
#include "utils/utils.hpp"


namespace cvlog = cv::utils::logging;

#if WSC_TESTCNT == 0

int main()  // Main entry point.
{
  WSC_EXCEPTIONSTR cpErr = "Failed to set console code point to UTF-8.";

#ifndef WSC_NOCATCH
  try
  {
#endif

    BOOL setConsoleRt = FALSE;
    setConsoleRt = SetConsoleCP(CP_UTF8);
    wsc::utils::windowsRequire(setConsoleRt, cpErr);

    setConsoleRt = SetConsoleOutputCP(CP_UTF8);
    wsc::utils::windowsRequire(setConsoleRt, cpErr);

    cvlog::setLogLevel(cvlog::LOG_LEVEL_ERROR);

#ifndef WSC_NOCATCH
  }
  catch (const std::system_error &sysE)
  {
    std::cerr << "FATAL EXCEPTION:\n" << sysE.what() << std::flush;
  }
#endif
}

#endif

/* --------------------------------- Testing Harnesses -------------------------------- */

#if defined(WSC_TEST1)  // Screenshot class.
int main()
{
  cvlog::setLogLevel(cvlog::LOG_LEVEL_ERROR);

  const wsc::Screenshot ss = {};
  std::vector<wsc::RGBA> screenshot = ss.take();

  const auto screenshotMat = cv::Mat(ss.screenY(), ss.screenX(), CV_8UC4, screenshot.data());
  WSC_SHOWMAT(screenshotMat);
}
#elif defined(WSC_TEST2)
#elif defined(WSC_TEST3)
#elif defined(WSC_TEST4)
#elif defined(WSC_TEST5)
#endif
