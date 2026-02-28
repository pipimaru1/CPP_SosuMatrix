#include "stdafx.h"
#include "CPP_SosuMatrix.h"


HFONT g_font = nullptr;
std::unordered_map<COLORREF, HBRUSH> g_brushCache;

HBRUSH GetBrush(COLORREF c)
{
    auto it = g_brushCache.find(c);
    if (it != g_brushCache.end())
        return it->second;

    HBRUSH b = CreateSolidBrush(c);
    g_brushCache.emplace(c, b);
    return b;
}

void ClearBrushCache()
{
    for (auto& kv : g_brushCache)
        DeleteObject(kv.second);
    g_brushCache.clear();
}

std::wstring MakeWorstCaseDigits(int digits)
{
    if (digits <= 0)
        digits = 1;
    return std::wstring(digits, L'8');
}

int NumDigits(int v)
{
    int d = 1;
    while (v >= 10) {
        v /= 10;
        ++d;
    }
    return d;
}

int FindBestFontHeight(HDC hdc, int cellW, int cellH, int digits)
{
    const int padX = std::max(2, cellW / 16);
    const int padY = std::max(2, cellH / 16);

    const int availW = std::max(1, cellW - 2 * padX);
    const int availH = std::max(1, cellH - 2 * padY);

    int lo = 1;
    int hi = std::max(1, availH);

    std::wstring sample = MakeWorstCaseDigits(digits);

    auto fits = [&](int heightPx) -> bool {
        HFONT f = CreateFontW(
            -heightPx, 0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI");
        if (!f)
            return false;

        HGDIOBJ old = SelectObject(hdc, f);
        SIZE sz{};
        BOOL ok = GetTextExtentPoint32W(hdc, sample.c_str(), (int)sample.size(), &sz);
        SelectObject(hdc, old);
        DeleteObject(f);

        if (!ok)
            return false;
        return (sz.cx <= availW) && (sz.cy <= availH);
    };

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

int MatrixArea::SetSize(int M, int N)
{
	__M = M;
	__N = N;
    _hoverQ = 0;

	_NaNumbers.resize(M * N);

    for (int i = 0; i < M * N; ++i)
        _NaNumbers[i] = NaturalNumber(i + 1);

    return 0;
}

void MatrixArea::RebuildFont(HWND hwnd, HDC hdc, int clientW, int clientH)
{
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
        L"Segoe UI");

    if (newFont) {
        if (g_font)
            DeleteObject(g_font);
        g_font = newFont;
    }

    (void)hwnd;
}


void MatrixArea::SetHoverCell(HWND hwnd, int q)
{
    if (!IsValidQ(q) || _hoverQ == q)
        return;

    const int previousHover = _hoverQ;
    _hoverQ = q;

    if (IsValidQ(previousHover))
        InvalidateCellByQ(hwnd, previousHover);
    InvalidateCellByQ(hwnd, _hoverQ);
}

void MatrixArea::ClearHoverCell(HWND hwnd)
{
    if (!IsValidQ(_hoverQ))
        return;

    const int previousHover = _hoverQ;
    _hoverQ = 0;
    InvalidateCellByQ(hwnd, previousHover);
}

#define HILIGHT_HOVERED_WIDTH 5

void MatrixArea::PaintGrid(HWND hwnd, HDC hdc)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    const int W = std::max(1L, rc.right - rc.left);
    const int H = std::max(1L, rc.bottom - rc.top);

    HBRUSH bgBrush = CreateSolidBrush(COLOR_BG);
    FillRect(hdc, &rc, bgBrush);
    DeleteObject(bgBrush);

    auto xAt = [&](int c) -> int { return (int)((long long)W * c / __M); };
    auto yAt = [&](int r) -> int { return (int)((long long)H * r / __N); };

    HPEN gridPen    = CreatePen(PS_SOLID, 1, COLOR_GRID);
    HPEN hoverPen   = CreatePen(PS_SOLID, HILIGHT_HOVERED_WIDTH, COLOR_HOVER_BORDER);
    HPEN unhoverPen = CreatePen(PS_SOLID, HILIGHT_HOVERED_WIDTH, COLOR_BG); //太くした枠を消去するためのペン。背景色と同じ色で、太さはhoverPenと同じにする。

    HGDIOBJ oldPen = SelectObject(hdc, gridPen);

    HGDIOBJ oldFont = nullptr;
    if (g_font)
        oldFont = SelectObject(hdc, g_font);

    SetBkMode(hdc, TRANSPARENT);

    int val = 1;

# pragma omp parallel for
    for (int r = 0; r < __N; ++r) {
        int y0 = yAt(r);
        int y1 = yAt(r + 1);
# pragma omp parallel for
        for (int c = 0; c < __M; ++c) {
            int x0 = xAt(c);
            int x1 = xAt(c + 1);
            RECT cell{ x0, y0, x1, y1 };

            const NaturalNumber& number = _NaNumbers[val - 1];
            const bool isHovered = (val == _hoverQ);

            HBRUSH b = GetBrush(number.cellColor);
            HGDIOBJ oldB = SelectObject(hdc, b);
            HGDIOBJ cellPen;

            if (isHovered)
            {
				int x0_inner = std::max(0, x0 + HILIGHT_HOVERED_WIDTH / 2);
				int y0_inner = std::max(0, y0 + HILIGHT_HOVERED_WIDTH / 2);
				int x1_inner = std::min(W, x1 - HILIGHT_HOVERED_WIDTH / 2);
				int y1_inner = std::min(H, y1 - HILIGHT_HOVERED_WIDTH / 2);

                cellPen = SelectObject(hdc, hoverPen);
                Rectangle(hdc, x0_inner, y0_inner, x1_inner, y1_inner);
                SelectObject(hdc, cellPen);
                SelectObject(hdc, oldB);
            }
            else
            {
                //int x0_inner = std::max(0, x0 + HILIGHT_HOVERED_WIDTH / 2);
                //int y0_inner = std::max(0, y0 + HILIGHT_HOVERED_WIDTH / 2);
                //int x1_inner = std::min(W, x1 - HILIGHT_HOVERED_WIDTH / 2);
                //int y1_inner = std::min(H, y1 - HILIGHT_HOVERED_WIDTH / 2);

                //cellPen = SelectObject(hdc, unhoverPen);
                //Rectangle(hdc, x0_inner, y0_inner, x1_inner, y1_inner);
                cellPen = SelectObject(hdc, gridPen);
                Rectangle(hdc, x0, y0, x1, y1);
                SelectObject(hdc, cellPen);
                SelectObject(hdc, oldB);
            }
            SetTextColor(hdc, number.textColor);
            std::wstring s = std::to_wstring(number.value);
            DrawTextW(hdc, s.c_str(), (int)s.size(), &cell, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            ++val;
        }
    }

    if (oldFont)
        SelectObject(hdc, oldFont);
    SelectObject(hdc, oldPen);
    DeleteObject(gridPen);
    DeleteObject(hoverPen);
}

bool GetCellRectByQ(HWND hwnd, int _Q, RECT* out, int _M, int _N)
{
    if (!out)
        return false;
    if (_Q < 1 || _Q > _N * _M)
        return false;

    RECT rc{};
    GetClientRect(hwnd, &rc);
    const int W = std::max(1L, rc.right - rc.left);
    const int H = std::max(1L, rc.bottom - rc.top);

    auto xAt = [&](int c) -> int { return (int)((long long)W * c / _M); };
    auto yAt = [&](int r) -> int { return (int)((long long)H * r / _N); };

    int idx = _Q - 1;
    int r = idx / _M;
    int c = idx % _M;

    out->left = xAt(c);
    out->top = yAt(r);
    out->right = xAt(c + 1);
    out->bottom = yAt(r + 1);
    return true;
}

///////////////////////////////////////////////
// qが有効なセル番号かどうかをチェックする関数。
// 1からM*Nまでの範囲内であれば有効とみなす。
bool MatrixArea::IsValidQ(int q) const
{
    return (1 <= q && q <= __N * __M);
}

///////////////////////////////////////////////
// x, yの座標からセルの矩形を取得する関数。
// qからセルの矩形を取得する関数。
// qが無効な場合はfalseを返す。
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
        InvalidateRect(hwnd, &r, TRUE);
}

void MatrixArea::SetCellHighlight(HWND hwnd, int q, COLORREF cellColor, COLORREF textColor)
{
    if (!IsValidQ(q))
        return;

    NaturalNumber& number = _NaNumbers[q - 1];
    number.cellColor = cellColor;
    number.textColor = textColor;
    InvalidateCellByQ(hwnd, q);
}

void MatrixArea::RemoveCellHighlight(HWND hwnd, int q)
{
    if (!IsValidQ(q))
        return;

    NaturalNumber& number = _NaNumbers[q - 1];
    number.cellColor = COLOR_CELL;
    number.textColor = COLOR_TEXT;
    InvalidateCellByQ(hwnd, q);
}

void MatrixArea::ClearAllCellHighlights(HWND hwnd)
{
    for (auto& number : _NaNumbers) {
        number.cellColor = COLOR_CELL;
        number.textColor = COLOR_TEXT;
    }
    InvalidateRect(hwnd, nullptr, TRUE);
}

void MatrixArea::ApplyMultiplesHighlight(HWND hwnd, int q)
{
    if (!IsValidQ(q))
        return;

    const NaturalNumber& selected = _NaNumbers[q - 1];
    const int step = selected.value;
    const int maxQ = __M * __N;

    for (int multiple = step * 2; multiple <= maxQ; multiple += step)
    {
        NaturalNumber& target = _NaNumbers[multiple - 1];
        //if (target.cellColor == COLOR_CLICKED)
        //    continue;

        target.cellColor = COLOR_MULTIPL;
        target.textColor = COLOR_TEXT;
        InvalidateCellByQ(hwnd, multiple);
    }
}

bool MatrixArea::IsCellHighlighted(int q) const
{
    if (!IsValidQ(q))
        return false;

    const NaturalNumber& number = _NaNumbers[q - 1];
    return number.cellColor != COLOR_CELL || number.textColor != COLOR_TEXT;
}

bool MatrixArea::HasAnyHighlights() const
{
    for (const auto& number : _NaNumbers) {
        if (number.cellColor != COLOR_CELL || number.textColor != COLOR_TEXT)
            return true;
    }
    return false;
}
