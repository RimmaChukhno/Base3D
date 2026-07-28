#include <Windows.h>

#include "EngineAPI.hpp"

#include <filesystem>
#include <string>

static const wchar_t* kWindowClassName = L"TronGameWindowClass";

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  auto* engine = reinterpret_cast<EngineHandle*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (engine)
  {
    Engine_OnWin32Message(engine, msg, static_cast<uintptr_t>(wParam), static_cast<intptr_t>(lParam));
  }

  switch (msg)
  {
  case WM_SIZE:
  {
    if (wParam == SIZE_MINIMIZED) return 0;
    if (!engine) return 0;
    const int width = LOWORD(lParam);
    const int height = HIWORD(lParam);
    if (width > 0 && height > 0)
    {
      Engine_Resize(engine, width, height);
    }
    return 0;
  }
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  default:
    return DefWindowProcW(hwnd, msg, wParam, lParam);
  }
}

static HWND createMainWindow(HINSTANCE hInstance, int width, int height)
{
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.lpszClassName = kWindowClassName;

  RegisterClassExW(&wc);

  DWORD style = WS_OVERLAPPEDWINDOW;
  RECT rect{ 0, 0, width, height };
  AdjustWindowRect(&rect, style, FALSE);

  HWND hwnd = CreateWindowExW(
    0,
    kWindowClassName,
    L"Tron - Step 2 (Triangle + Resize)",
    style,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    rect.right - rect.left,
    rect.bottom - rect.top,
    nullptr,
    nullptr,
    hInstance,
    nullptr
  );

  if (hwnd)
  {
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
  }

  return hwnd;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
  constexpr int kWidth = 1280;
  constexpr int kHeight = 720;

  HWND hwnd = createMainWindow(hInstance, kWidth, kHeight);
  if (!hwnd)
  {
    MessageBoxW(nullptr, L"Failed to create Win32 window.", L"Tron", MB_ICONERROR | MB_OK);
    return -1;
  }

  EngineConfig cfg{};
  cfg.windowHandle = hwnd;
  cfg.width = kWidth;
  cfg.height = kHeight;
#if defined(_DEBUG)
  cfg.enableValidation = true;
#else
  cfg.enableValidation = false;
#endif

  wchar_t exePath[MAX_PATH]{};
  GetModuleFileNameW(nullptr, exePath, MAX_PATH);
  const std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
  const std::wstring assetsRoot = (exeDir / L"Assets").wstring();
  cfg.assetsRoot = assetsRoot.c_str();

  EngineHandle* engine = Engine_Create(&cfg);
  if (!engine)
  {
    MessageBoxW(nullptr, L"Engine_Create failed (D3D11 init).", L"Tron", MB_ICONERROR | MB_OK);
    return -2;
  }

  SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(engine));

  MSG msg{};
  bool running = true;
  float lastTitleUpdate = 0.0f;
  while (running)
  {
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
      {
        running = false;
        break;
      }
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    if (!running) break;
    Engine_Tick(engine);

    EngineFrameStats stats{};
    Engine_GetFrameStats(engine, &stats);
    if (stats.totalSeconds - lastTitleUpdate >= 0.25f)
    {
      lastTitleUpdate = stats.totalSeconds;
      wchar_t title[256]{};
      swprintf_s(
        title,
        L"Tron - Step 5 (RenderSystem) | FPS: %.1f | dt: %.3f ms | Entities: %d | Moving: %d | Draws: %d | Mouse: %d,%d (d %d,%d) | Wheel: %d",
        stats.fps,
        stats.deltaSeconds * 1000.0f,
        stats.entityCount,
        stats.movingCount,
        stats.drawCount,
        stats.mouseX,
        stats.mouseY,
        stats.mouseDeltaX,
        stats.mouseDeltaY,
        stats.wheelDelta
      );
      SetWindowTextW(hwnd, title);
    }
  }

  Engine_Destroy(engine);
  return 0;
}

