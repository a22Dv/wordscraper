/**
 * screenshot.cpp
 *
 * Implementation for screenshot.hpp
 */

#include "core/screenshot.hpp"  // Master header file.

#include <system_error>

#include "core/pch.hpp"
#include "core/types.hpp"
#include "utils/utils.hpp"

namespace ch = std::chrono;

namespace
{

WSC_ASSERTSTR screenNegative[] = "Negative screen dimensions received.";
WSC_EXCEPTIONSTR screenDimensionsFail[] = "Invalid screen dimensions received.";
WSC_EXCEPTIONSTR getDcNull[] = "Could not retrieve device context.";
WSC_EXCEPTIONSTR createDcNull[] = "Could not create device context.";
WSC_EXCEPTIONSTR createBitmapNull[] = "Could not create bitmap.";
WSC_EXCEPTIONSTR selectObjNull[] = "Could not transfer bitmap.";
WSC_EXCEPTIONSTR diBitsFail[] = "Could not copy screen bits.";
WSC_EXCEPTIONSTR bitBltFail[] = "Could not transfer screen bits.";
WSC_EXCEPTIONSTR threadFail[] = "Thread encountered an unexpected failure.";

struct GDIData
{
  HDC screenDc = nullptr;
  HDC memoryDc = nullptr;
  HBITMAP bitmap = nullptr;
  HBITMAP exBitmap = nullptr;
  BITMAPINFOHEADER bitmapInfo = {};

  GDIData(int screenX, int screenY)
  {
    wsc::utils::runtimeRequire(screenX > 0 && screenY > 0, WSC_EXCEPT_FORMAT(screenNegative));

    screenDc = GetDC(nullptr);
    wsc::utils::windowsRequire(screenDc != nullptr, WSC_EXCEPT_FORMAT(getDcNull));

    memoryDc = CreateCompatibleDC(screenDc);
    wsc::utils::windowsRequire(memoryDc != nullptr, WSC_EXCEPT_FORMAT(createDcNull));

    bitmap = CreateCompatibleBitmap(screenDc, screenX, screenY);
    wsc::utils::windowsRequire(bitmap != nullptr, WSC_EXCEPT_FORMAT(createBitmapNull));

    exBitmap = static_cast<HBITMAP>(SelectObject(memoryDc, bitmap));
    wsc::utils::windowsRequire(exBitmap != nullptr, WSC_EXCEPT_FORMAT(selectObjNull));

    bitmapInfo.biBitCount = sizeof(wsc::RGBA) * CHAR_BIT;
    bitmapInfo.biCompression = BI_RGB;
    bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.biPlanes = 1;
    bitmapInfo.biWidth = screenX;
    bitmapInfo.biHeight = -screenY;
  }
  GDIData(const GDIData &) = delete;
  GDIData &operator=(const GDIData &) = delete;
  GDIData(GDIData &&) = delete;
  GDIData &operator=(GDIData &&) = delete;
  ~GDIData()
  {
    SelectObject(memoryDc, exBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDc);
    ReleaseDC(nullptr, screenDc);
  }
};

}  // namespace

namespace wsc
{

void Screenshot::threadExec_() noexcept
{
  try
  {
    GDIData gdiData(screenX_, screenY_);
    const int offsetX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int offsetY = GetSystemMetrics(SM_YVIRTUALSCREEN);

    while (!terminate_.load())
    {
      WSC_PROFILESCOPE();
      std::unique_lock lock(cycleMutex_);

      BOOL winRt = FALSE;
      {
        WSC_PROFILESCOPEN("BitBlt");
        winRt = BitBlt(
            gdiData.memoryDc, 0, 0, screenX_, screenY_,  // Destination offset &  dimensions.
            gdiData.screenDc, offsetX, offsetY,          // Source offset.
            SRCCOPY                                      // Mode.
        );
        utils::windowsRequire(winRt, bitBltFail);
      }
      {
        WSC_PROFILESCOPEN("GetDIBits");
        winRt = GetDIBits(
            gdiData.memoryDc, gdiData.bitmap, 0, screenY_, buffer_.data(),
            reinterpret_cast<LPBITMAPINFO>(&gdiData.bitmapInfo), DIB_RGB_COLORS
        );
        utils::windowsRequire(winRt, diBitsFail);
      }
      acknowledged_.store(true);
      cycle_.notify_all();  // Notify before sleeping.
      cycle_.wait_for(lock, interval_.load(), [this] { return terminate_.load(); });
    }
  }
  catch (const std::system_error &sysE)
  {
    threadStatus_.store(sysE.code().value());
    cycle_.notify_all();
  }
  catch (...)  // Catch-all for std::* exceptions.
  {
    threadStatus_.store(ERROR_BAD_ENVIRONMENT);
    cycle_.notify_all();
  }
}

Screenshot::Screenshot(ch::milliseconds interval)
{
  screenX_ = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  utils::windowsRequire(screenX_ > 0, screenDimensionsFail);

  screenY_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  utils::windowsRequire(screenY_ > 0, screenDimensionsFail);

  interval_.store(interval);
  buffer_.resize(screenX_ * screenY_);

  {
    std::unique_lock lock(cycleMutex_);
    acknowledged_.store(false);
    thread_ = std::thread([this] { threadExec_(); });
    cycle_.wait(lock, [this] { return threadStatus_.load() || acknowledged_.load(); });
  }
  const DWORD status = threadStatus_.load();
  if (status != ERROR_SUCCESS)
  {
    throw std::system_error(status, std::system_category(), WSC_EXCEPT_FORMAT(threadFail));
  }
}

Screenshot::~Screenshot() {
  terminate_.store(true);
  if (thread_.joinable()) {
    thread_.join();
  }
}


ch::milliseconds Screenshot::interval() const noexcept { return interval_.load(); }
int Screenshot::screenX() const noexcept { return screenX_; }
int Screenshot::screenY() const noexcept { return screenY_; }

void Screenshot::setInterval(ch::milliseconds interval) noexcept { interval_.store(interval); }

void Screenshot::take(std::vector<RGBA>& buffer) const
{
  buffer.resize(screenX_ * screenY_);
  const DWORD status = threadStatus_.load();
  if (status != ERROR_SUCCESS)
  {
    throw std::system_error(status, std::system_category(), WSC_EXCEPT_FORMAT(threadFail));
  }
  {
    std::unique_lock lock(cycleMutex_);
    std::copy(buffer_.begin(), buffer_.end(), buffer.begin());
  }
}

std::vector<RGBA> Screenshot::take() const
{
  auto buffer = std::vector<RGBA>(screenX_ * screenY_);
  take(buffer);
  return buffer;
}

}  // namespace wsc