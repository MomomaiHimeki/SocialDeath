#include <windows.h>
#include <gdiplus.h>

#include <memory>

using namespace Gdiplus;

namespace {
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr wchar_t kWindowClassName[] = L"SocialDeathWindow";
constexpr wchar_t kWindowTitle[] = L"Social Death";
constexpr wchar_t kBackgroundPath[] = L"D:\\Project\\Social Death\\Resource\\a16199a2b316199f.jpg";
constexpr wchar_t kIconPath[] = L"D:\\Project\\Social Death\\Resource\\distorted-face_1faea.png";

std::unique_ptr<Image> g_background;
HICON g_windowIcon = nullptr;

void DrawRoundedOverlay(Graphics& graphics) {
    constexpr int x = 50;
    constexpr int y = 470;
    constexpr int width = 1180;
    constexpr int height = 200;
    constexpr int radius = 30;

    GraphicsPath path;
    const float diameter = static_cast<float>(radius * 2);
    path.AddArc(static_cast<REAL>(x), static_cast<REAL>(y), diameter, diameter, 180.0f, 90.0f);
    path.AddArc(static_cast<REAL>(x + width - radius * 2), static_cast<REAL>(y), diameter, diameter, 270.0f, 90.0f);
    path.AddArc(static_cast<REAL>(x + width - radius * 2), static_cast<REAL>(y + height - radius * 2), diameter, diameter, 0.0f, 90.0f);
    path.AddArc(static_cast<REAL>(x), static_cast<REAL>(y + height - radius * 2), diameter, diameter, 90.0f, 90.0f);
    path.CloseFigure();

    // #607C8E with 50% opacity.
    SolidBrush brush(Color(128, 0x60, 0x7C, 0x8E));
    graphics.FillPath(&brush, &path);
}

void PaintScene(HWND window, HDC deviceContext) {
    RECT clientRect{};
    GetClientRect(window, &clientRect);

    Graphics graphics(deviceContext);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.Clear(Color(18, 18, 24));

    if (!g_background || g_background->GetLastStatus() != Ok) {
        return;
    }

    const UINT width = g_background->GetWidth();
    const UINT height = g_background->GetHeight();
    if (width == 0 || height == 0) {
        return;
    }

    const float scaleX = static_cast<float>(clientRect.right) / width;
    const float scaleY = static_cast<float>(clientRect.bottom) / height;
    const float scale = (scaleX > scaleY) ? scaleX : scaleY;
    const int drawWidth = static_cast<int>(width * scale);
    const int drawHeight = static_cast<int>(height * scale);
    const int x = (clientRect.right - drawWidth) / 2;
    const int y = (clientRect.bottom - drawHeight) / 2;

    graphics.DrawImage(g_background.get(), x, y, drawWidth, drawHeight);
    DrawRoundedOverlay(graphics);
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC deviceContext = BeginPaint(window, &paint);
        PaintScene(window, deviceContext);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}
} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand) {
    GdiplusStartupInput gdiplusStartupInput{};
    ULONG_PTR gdiplusToken = 0;
    if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Ok) {
        MessageBoxW(nullptr, L"GDI+ 初始化失败。", kWindowTitle, MB_ICONERROR);
        return 1;
    }

    g_background = std::make_unique<Image>(kBackgroundPath);
    Bitmap iconImage(kIconPath);
    if (iconImage.GetLastStatus() == Ok) {
        iconImage.GetHICON(&g_windowIcon);
    }

    WNDCLASSW windowClass{};
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.lpszClassName = kWindowClassName;
    // MinGW 下 IDC_ARROW 宏可能展开为窄字符资源标识，
    // 显式使用宽字符资源标识以匹配 LoadCursorW。
    windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    windowClass.hIcon = g_windowIcon;
    windowClass.hbrBackground = nullptr;
    RegisterClassW(&windowClass);

    RECT desiredRect{0, 0, kWindowWidth, kWindowHeight};
    AdjustWindowRect(&desiredRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND window = CreateWindowExW(
        0,
        kWindowClassName,
        kWindowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        desiredRect.right - desiredRect.left,
        desiredRect.bottom - desiredRect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!window) {
        GdiplusShutdown(gdiplusToken);
        MessageBoxW(nullptr, L"窗口创建失败。", kWindowTitle, MB_ICONERROR);
        return 1;
    }

    if (g_windowIcon != nullptr) {
        SendMessageW(window, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(g_windowIcon));
        SendMessageW(window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(g_windowIcon));
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    g_background.reset();
    if (g_windowIcon != nullptr) {
        DestroyIcon(g_windowIcon);
        g_windowIcon = nullptr;
    }
    GdiplusShutdown(gdiplusToken);
    return static_cast<int>(message.wParam);
}
