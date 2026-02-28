// GridNumbersWin32.cpp
// Build (MSVC example):
//   cl /std:c++17 /DUNICODE /D_UNICODE GridNumbersWin32.cpp user32.lib gdi32.lib
#define NOMINMAX // 先に定義しておくことが重要！
//#include <windows.h>
//#include <string>
//#include <algorithm>
//#include <unordered_set>
//#include <unordered_map>

#include "stdafx.h"
#include "CPP_SosuMatrix.h"

#include "Resource.h" // リソース定義（メニュー等）

MatrixArea _MtxArea(20,24, 1600,1200); // グローバルなMatrixAreaインスタンス

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg) 
    {
        //ウィンドウ初期化 
        case WM_CREATE:
        {
            // メニューの作成
            HMENU hMenu = LoadMenuW(GetModuleHandle(nullptr), MAKEINTRESOURCEW(IDR_MAINFRAME));
            //　メニューの表示

            if (hMenu)
            {
                SetMenu(hwnd, hMenu);
                // メニューのチェック状態を更新
                //CheckMenuItem(hMenu, IDM_HIGHLIGHT, g_highlightQ ? MF_CHECKED : MF_UNCHECKED);
            }
            return 0;
        }break;

        case WM_SIZE: 
        {
            // サイズが変わったらフォントを作り直す（client sizeに依存）
            HDC hdc = GetDC(hwnd);
            RECT rc{};
            GetClientRect(hwnd, &rc);
            _MtxArea.RebuildFont(hwnd, hdc, (int)(rc.right - rc.left), (int)(rc.bottom - rc.top));
            ReleaseDC(hwnd, hdc);

            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        case WM_PAINT: 
        {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd, &ps);
            _MtxArea.PaintGrid(hwnd, hdc);
            
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            const int x = static_cast<short>(LOWORD(lParam));
            const int y = static_cast<short>(HIWORD(lParam));

            int q = 0;
            if (_MtxArea.TryGetQFromPoint(hwnd, x, y, &q))
            {
                if (_MtxArea.IsCellHighlighted(q))
                    _MtxArea.RemoveCellHighlight(hwnd, q);
                else
                    _MtxArea.SetCellHighlight(hwnd, q, RGB(255, 220, 80), RGB(0, 0, 0));
            }
            return 0;
        }


        case WM_DESTROY:
        {
            ClearBrushCache();

            if (g_font) { 
                DeleteObject(g_font); g_font = nullptr; 
            }
            PostQuitMessage(0);
            return 0;
        }

        case WM_COMMAND: 
        {
            int wmId = LOWORD(wParam);
            switch (wmId) 
            {
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    return 0;
                case IDM_HIGHLIGHT:
                {
                    // ハイライトのトグル
                    if (!_MtxArea.HasAnyHighlights()) 
                    {
                        _MtxArea.SetCellHighlight(hwnd, 50, RGB(255, 220, 80), RGB(0, 0, 0));
                        _MtxArea.SetCellHighlight(hwnd, 77, RGB(80, 220, 255), RGB(0, 0, 0));
                    }
                    else {
                        _MtxArea.ClearAllCellHighlights(hwnd);
                    }
                    return 0;
                }
                default:
                    return DefWindowProcW(hwnd, msg, wParam, lParam);
            }
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t* kClassName = L"GridNumbersWin32Class";

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // 実際の塗りはWM_PAINTで行う

    wc.lpszMenuName = MAKEINTRESOURCE(IDC_CPPSOSUMATRIX); //メニューの登録

    RegisterClassW(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW;

    RECT rc{ 0, 0, _MtxArea.SIZE_X(), _MtxArea.SIZE_Y()};
    AdjustWindowRect(&rc, style, FALSE);

    HWND hwnd = CreateWindowW(
        kClassName,
        //L"Grid Numbers (Win32/GDI)",
        L"素数で遊ぼう",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInst, nullptr
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 初回フォント生成（client size確定後）
    {
        HDC hdc = GetDC(hwnd);
        RECT c{};
        GetClientRect(hwnd, &c);
        _MtxArea.RebuildFont(hwnd, hdc, (int)(c.right - c.left), (int)(c.bottom - c.top));
        ReleaseDC(hwnd, hdc);
    }

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}