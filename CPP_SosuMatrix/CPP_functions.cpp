#include "stdafx.h"
#include "CPP_SosuMatrix.h"

//int __M = 10;          // cols
//int __N = 12;          // rows
//int __SIZE_X = 1600;    // initial window client-ish (actual client depends on styles)
//int __SIZE_Y = 1200;

constexpr COLORREF COLOR_BG = RGB(0x20, 0x20, 0x20);
constexpr COLORREF COLOR_GRID = RGB(0x80, 0x80, 0x80);
constexpr COLORREF COLOR_CELL = RGB(0x30, 0x30, 0x30);
constexpr COLORREF COLOR_TEXT = RGB(0xFF, 0xFF, 0xFF);

HFONT g_font = nullptr;

std::unordered_map<int, CellStyle> g_highlightMap; // key: Q
std::unordered_map<COLORREF, HBRUSH> g_brushCache;

// ハイライト用
//static int      g_highlightQ = 0;                 // 0: 無効, 1..N*M: 対象
//static COLORREF g_hlCellColor = RGB(255, 220, 80);
//static COLORREF g_hlTextColor = RGB(0, 0, 0);

HBRUSH GetBrush(COLORREF c)
{
    auto it = g_brushCache.find(c);
    if (it != g_brushCache.end()) return it->second;
    HBRUSH b = CreateSolidBrush(c);
    g_brushCache.emplace(c, b);
    return b;
}

void ClearBrushCache() {
    for (auto& kv : g_brushCache) DeleteObject(kv.second);
    g_brushCache.clear();
}

// 桁数（maxVal）の文字列として、幅の厳しさを見るため "888...8" を作る（最も幅が出やすいケースに寄せる）
std::wstring MakeWorstCaseDigits(int digits) {
    if (digits <= 0) digits = 1;
    return std::wstring(digits, L'8');
}

int NumDigits(int v) {
    int d = 1;
    while (v >= 10) { v /= 10; ++d; }
    return d;
}

// セル内に収まる最大フォント高さ（ピクセル）を探索して返す。
// - 幅/高さともに padding を引いた領域に収まること
int FindBestFontHeight(HDC hdc, int cellW, int cellH, int digits) {
    const int padX = std::max(2, cellW / 16);
    const int padY = std::max(2, cellH / 16);

    const int availW = std::max(1, cellW - 2 * padX);
    const int availH = std::max(1, cellH - 2 * padY);

    // フォント高さ探索範囲（GDIのCreateFontは負値で「文字高さ」指定）
    int lo = 1;
    int hi = std::max(1, availH); // 高さ方向上限

    std::wstring sample = MakeWorstCaseDigits(digits);

    auto fits = [&](int heightPx) -> bool {
        HFONT f = CreateFontW(
            -heightPx, 0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
        if (!f) 
            return false;

        HGDIOBJ old = SelectObject(hdc, f);
        SIZE sz{};
        BOOL ok = GetTextExtentPoint32W(hdc, sample.c_str(), (int)sample.size(), &sz);
        SelectObject(hdc, old);
        DeleteObject(f);

        if (!ok) 
            return false;

        // 高さはフォント高さそのものより text extent の cy を優先
        return (sz.cx <= availW) && (sz.cy <= availH);
        };

    // 二分探索
    int best = 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (fits(mid)) {
            best = mid;
            lo = mid + 1;
        }
        else {
            hi = mid - 1;
        }
    }
    return best;
}
void MatrixArea::RebuildFont(HWND hwnd, HDC hdc, int clientW, int clientH)
{
    // セルサイズ
    const int cellW = std::max(1, clientW / __M);
    const int cellH = std::max(1, clientH / __N);

    const int maxVal = __N * __M;
    const int digits = NumDigits(maxVal);

    int bestH = FindBestFontHeight(hdc, cellW, cellH, digits);

    HFONT newFont = CreateFontW(
        -bestH, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );

    if (newFont) {
        if (g_font) DeleteObject(g_font);
        g_font = newFont;
    }

    (void)hwnd;
}
void MatrixArea::PaintGrid(HWND hwnd, HDC hdc)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    const int W = std::max(1L, rc.right - rc.left);
    const int H = std::max(1L, rc.bottom - rc.top);

    // 背景塗り
    HBRUSH bgBrush = CreateSolidBrush(COLOR_BG);
    FillRect(hdc, &rc, bgBrush);
    DeleteObject(bgBrush);

    // セル寸法（端は割り切れないので「境界座標を都度計算」してズレを最小化）
    auto xAt = [&](int c) -> int { return (int)((long long)W * c / __M); };
    auto yAt = [&](int r) -> int { return (int)((long long)H * r / __N); };

    // ペン/ブラシ
    HPEN gridPen = CreatePen(PS_SOLID, 1, COLOR_GRID);
    //HBRUSH normalBrush = CreateSolidBrush(COLOR_CELL);
    //HBRUSH hlBrush = CreateSolidBrush(g_hlCellColor); //追加

    HGDIOBJ oldPen = SelectObject(hdc, gridPen);
    //HGDIOBJ oldBrush = SelectObject(hdc, normalBrush);

    // フォント
    HGDIOBJ oldFont = nullptr;
    if (g_font) oldFont = SelectObject(hdc, g_font);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_TEXT);

    int val = 1;
    for (int r = 0; r < __N; ++r) {
        int y0 = yAt(r);
        int y1 = yAt(r + 1);
        for (int c = 0; c < __M; ++c) {
            int x0 = xAt(c);
            int x1 = xAt(c + 1);

            RECT cell{ x0, y0, x1, y1 };

            //bool isHL = (g_highlightSet.find(val) != g_highlightSet.end());
            auto it = g_highlightMap.find(val);
            bool isHL = (it != g_highlightMap.end());

            //COLORREF cellColor = isHL ? it->second.cell : COLOR_CELL;
            //COLORREF textColor = isHL ? it->second.text : COLOR_TEXT;

            COLORREF cellColor = COLOR_CELL;
            COLORREF textColor = COLOR_TEXT;
            //bool isHL = false;

            //auto it = g_highlightMap.find(val);
            if (it != g_highlightMap.end()) {
                isHL = true;
                cellColor = it->second.cell;
                textColor = it->second.text;
            }

            //SelectObject(hdc, isHL ? hlBrush : normalBrush);
            SetTextColor(hdc, isHL ? textColor : COLOR_TEXT);

            // セルの塗り
            //SelectObject(hdc, isHL ? hlBrush : normalBrush);
            //Rectangle(hdc, x0, y0, x1, y1);

            HBRUSH b = GetBrush(cellColor);
            HGDIOBJ oldB = SelectObject(hdc, b);
            Rectangle(hdc, x0, y0, x1, y1);
            SelectObject(hdc, oldB);

            // 数字
            std::wstring s = std::to_wstring(val);
            DrawTextW(hdc, s.c_str(), (int)s.size(), &cell,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            ++val;
        }
    }

    // 後始末
    if (oldFont)
        SelectObject(hdc, oldFont);

    //DeleteObject(hlBrush); //追加
    //DeleteObject(normalBrush);
    DeleteObject(gridPen);
}

bool GetCellRectByQ(HWND hwnd, int _Q, RECT* out, int _M, int _N) {
    if (!out)
        return false;
    if (_Q < 1 || _Q > _N * _M)
        return false;

    RECT rc{};
    GetClientRect(hwnd, &rc);
    const int W = std::max(1L, rc.right - rc.left);
    const int H = std::max(1L, rc.bottom - rc.top);

    // ラムダ式
    // クエリ番号は1から始まるので、0ベースのインデックスに変換
    auto xAt = [&](int c) -> int {return (int)((long long)W * c / _M); };
    auto yAt = [&](int r) -> int {return (int)((long long)H * r / _N); };

    int idx = _Q - 1;
    int r = idx / _M;
    int c = idx % _M;

    out->left = xAt(c);
    out->top = yAt(r);
    out->right = xAt(c + 1);
    out->bottom = yAt(r + 1);
    return true;
}

bool MatrixArea::IsValidQ(int q) const
{
    return (1 <= q && q <= __N * __M);
}

bool MatrixArea::TryGetQFromPoint(HWND hwnd, int x, int y, int* outQ) const
{
    if (!outQ)
        return false;

    RECT rc{};
    GetClientRect(hwnd, &rc);

    const int width = std::max(1L, rc.right - rc.left);
    const int height = std::max(1L, rc.bottom - rc.top);

    if (x < 0 || y < 0 || x >= width || y >= height)
        return false;

    const int col = std::min(__M - 1, (x * __M) / width);
    const int row = std::min(__N - 1, (y * __N) / height);

    *outQ = row * __M + col + 1;
    return IsValidQ(*outQ);
}
void MatrixArea::InvalidateCellByQ(HWND hwnd, int q)
{
    RECT r{};
    if (GetCellRectByQ(hwnd, q, &r, __M, __N))
    {
        InvalidateRect(hwnd, &r, TRUE);
    }
}
void MatrixArea::SetCellHighlight(HWND hwnd, int q, COLORREF cellColor, COLORREF textColor)
{
    if (!IsValidQ(q))
        return;
    g_highlightMap[q] = CellStyle{ cellColor, textColor };
    InvalidateCellByQ(hwnd, q);
}
void MatrixArea::RemoveCellHighlight(HWND hwnd, int q)
{
    if (!IsValidQ(q))
        return;
    if (g_highlightMap.erase(q) > 0)
        InvalidateCellByQ(hwnd, q);
}

void ClearAllCellHighlights(HWND hwnd)
{
    if (!g_highlightMap.empty()) {
        g_highlightMap.clear();
        InvalidateRect(hwnd, nullptr, TRUE);
    }
}