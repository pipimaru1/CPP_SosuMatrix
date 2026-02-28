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
	bool _sosu_fast = false; // 素数を高速に表示するかどうか

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
                //CheckMenuItem(hMenu, IDM_SOSU, g_highlightQ ? MF_CHECKED : MF_UNCHECKED);
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

        case WM_MOUSEMOVE:
        {
            //マウスオーバーのセルを表示するのは80x100以下に限定（描画負荷対策）
			if (_MtxArea.GET_M() < 80 && _MtxArea.GET_N()<80) 
            {
                TRACKMOUSEEVENT tme{};
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);

                const int x = static_cast<short>(LOWORD(lParam));
                const int y = static_cast<short>(HIWORD(lParam));

                int q = 0;
                if (_MtxArea.TryGetQFromPoint(hwnd, x, y, &q))
                    _MtxArea.SetHoverCell(hwnd, q);
                else
                    _MtxArea.ClearHoverCell(hwnd);
            }

            return 0;
        }

        case WM_MOUSELEAVE:
            _MtxArea.ClearHoverCell(hwnd);
            return 0;

        case WM_LBUTTONDOWN:
        {
            const int x = static_cast<short>(LOWORD(lParam));
            const int y = static_cast<short>(HIWORD(lParam));

            int q = 0;
            if (_MtxArea.TryGetQFromPoint(hwnd, x, y, &q))
            {
                if (q == 1 || _MtxArea.IsCellHighlighted(q))
                    return 0;

                _MtxArea.SetCellHighlight(hwnd, q, COLOR_CLICKED, RGB(0, 0, 0));
                _MtxArea.ApplyMultiplesHighlight(hwnd, q);
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
				case IDM_SIZE_10x12://格子のサイズ変更
                {
                    _MtxArea.SetSize(10, 12);
					PostMessage(hwnd, WM_SIZE, 0, 0); // フォントサイズの再計算と再描画を促すためにWM_SIZEを送る

                    // サイズが変わったらフォントを作り直す（client sizeに依存）
                    //HDC hdc = GetDC(hwnd);
                    //RECT rc{};
                    //GetClientRect(hwnd, &rc);
                    //_MtxArea.RebuildFont(hwnd, hdc, (int)(rc.right - rc.left), (int)(rc.bottom - rc.top));
                    //ReleaseDC(hwnd, hdc);
                    ////再描画
                    //InvalidateRect(hwnd, nullptr, TRUE);
                    return 0;

                }break;

                // 格子のサイズ変更
                // フォントサイズの再計算と再描画を促すためにWM_SIZEを送る
                case IDM_SIZE_20x24:_MtxArea.SetSize(20, 24); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_30x36:_MtxArea.SetSize(30, 36); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_40x48:_MtxArea.SetSize(40, 48); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_15x18:_MtxArea.SetSize(15, 18); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_25x30:_MtxArea.SetSize(25, 30); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_35x42:_MtxArea.SetSize(35, 42); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_45x54:_MtxArea.SetSize(45, 54); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_80x100:_MtxArea.SetSize(80, 100); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_120x160:_MtxArea.SetSize(120, 160); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_180x200:_MtxArea.SetSize(180, 200); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_220x240:_MtxArea.SetSize(220, 240); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;

				case IDM_SIZE_13x16:_MtxArea.SetSize(13, 16); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_23x28:_MtxArea.SetSize(23, 28); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_37x44:_MtxArea.SetSize(37, 44); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_47x56:_MtxArea.SetSize(47, 56); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_17x23:_MtxArea.SetSize(17, 23); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_29x37:_MtxArea.SetSize(29, 37); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_37x47:_MtxArea.SetSize(37, 47); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_47x57:_MtxArea.SetSize(47, 57); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_57x71:_MtxArea.SetSize(57, 71); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;

                case IDM_SIZE_X10: _MtxArea.SetSize(10,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X13: _MtxArea.SetSize(13,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X23: _MtxArea.SetSize(23,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X29: _MtxArea.SetSize(29,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X37: _MtxArea.SetSize(37,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X49: _MtxArea.SetSize(49,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X59: _MtxArea.SetSize(59,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X67: _MtxArea.SetSize(67,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X79: _MtxArea.SetSize(79,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X89: _MtxArea.SetSize(89,  _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X101:_MtxArea.SetSize(101, _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X127:_MtxArea.SetSize(127, _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X157:_MtxArea.SetSize(157, _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X179:_MtxArea.SetSize(179, _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
                case IDM_SIZE_X211:_MtxArea.SetSize(211, _MtxArea.GET_N()); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;

                case IDM_SIZE_Y10: _MtxArea.SetSize(_MtxArea.GET_M(), 10); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y13: _MtxArea.SetSize(_MtxArea.GET_M(), 13); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y23: _MtxArea.SetSize(_MtxArea.GET_M(), 23); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y29: _MtxArea.SetSize(_MtxArea.GET_M(), 29); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y37: _MtxArea.SetSize(_MtxArea.GET_M(), 37); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y49: _MtxArea.SetSize(_MtxArea.GET_M(), 49); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y59: _MtxArea.SetSize(_MtxArea.GET_M(), 59); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y67: _MtxArea.SetSize(_MtxArea.GET_M(), 67); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y79: _MtxArea.SetSize(_MtxArea.GET_M(), 79); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y89: _MtxArea.SetSize(_MtxArea.GET_M(), 89); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y101:_MtxArea.SetSize(_MtxArea.GET_M(), 101); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y127:_MtxArea.SetSize(_MtxArea.GET_M(), 127); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y157:_MtxArea.SetSize(_MtxArea.GET_M(), 157); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y179:_MtxArea.SetSize(_MtxArea.GET_M(), 179); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;
				case IDM_SIZE_Y211:_MtxArea.SetSize(_MtxArea.GET_M(), 211); PostMessage(hwnd, WM_SIZE, 0, 0); return 0;

                case IDM_RESET:
                {
                    _MtxArea.ClearAllCellHighlights(hwnd);
                    return 0;
                }

                case IDM_SOSUFAST:
					_sosu_fast = true;
                case IDM_SOSU:
                {
					for (int q = 2; q <= _MtxArea.GET_M()* _MtxArea.GET_N(); ++q)
                    {
                        if (!(q == 1 || _MtxArea.IsCellHighlighted(q)))
                        {
                            _MtxArea.SetCellHighlight(hwnd, q, COLOR_CLICKED, RGB(0, 0, 0));
                            _MtxArea.ApplyMultiplesHighlight(hwnd, q);
	
                            if (!_sosu_fast)
                                SendMessageW(hwnd, WM_PAINT, 0, 0); // 描画更新を促す
                        }
                    }
                    _sosu_fast = false;
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