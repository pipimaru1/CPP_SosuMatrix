#pragma once
#include "stdafx.h"
#include "resource.h"

constexpr COLORREF COLOR_CLICKED = RGB(255, 220, 80);
constexpr COLORREF COLOR_MULTIPL = RGB(80, 220, 255);
constexpr COLORREF COLOR_BG = RGB(0x20, 0x20, 0x20);
constexpr COLORREF COLOR_GRID = RGB(0x80, 0x80, 0x80);
constexpr COLORREF COLOR_CELL = RGB(0x30, 0x30, 0x30);
constexpr COLORREF COLOR_TEXT = RGB(0xFF, 0xFF, 0xFF);
constexpr COLORREF COLOR_HOVER_BORDER = RGB(0xFF, 0xD0, 0x40);
constexpr COLORREF COLOR_HOVER = RGB(0xFF, 0xFF, 0x00);

struct NaturalNumber {
    int value;
    COLORREF cellColor;
    COLORREF textColor;
    bool calculated;
    bool isPrime;

    NaturalNumber()
        : value(0), cellColor(RGB(0x30, 0x30, 0x30)), textColor(RGB(0xFF, 0xFF, 0xFF)), calculated(false), isPrime(false) {
    }

    NaturalNumber(int v)
        : value(v), cellColor(RGB(0x30, 0x30, 0x30)), textColor(RGB(0xFF, 0xFF, 0xFF)), calculated(false), isPrime(false) {
    }
};

class MatrixArea
{
    std::vector<NaturalNumber> _NaNumbers;
    int _hoverQ;

    int __M;
    int __N;
    int __SIZE_X;
    int __SIZE_Y;

public:

    MatrixArea(int M, int N, int sizeX, int sizeY)
        : _hoverQ(0)
    {
        __SIZE_X = sizeX;
        __SIZE_Y = sizeY;
        SetSize(M, N);
    }

    MatrixArea() : _hoverQ(0), __M(0), __N(0), __SIZE_X(0), __SIZE_Y(0) {}

    int SetSize(int M, int N);


    void RebuildFont(HWND hwnd, HDC hdc, int clientW, int clientH);
    void PaintGrid(HWND hwnd, HDC hdc);
    void SetCellHighlight(HWND hwnd, int q, COLORREF cellColor, COLORREF textColor);
    void InvalidateCellByQ(HWND hwnd, int q);
    void RemoveCellHighlight(HWND hwnd, int q);
    void ClearAllCellHighlights(HWND hwnd);
    bool TryGetQFromPoint(HWND hwnd, int x, int y, int* outQ) const;

    bool IsValidQ(int q) const;
    bool IsCellHighlighted(int q) const;
    bool HasAnyHighlights() const;
    void SetHoverCell(HWND hwnd, int q);
    void ClearHoverCell(HWND hwnd);

    int SIZE_X() const { return __SIZE_X; }
    int SIZE_Y() const { return __SIZE_Y; }
	int GET_M() const { return __M; }
	int GET_N() const { return __N; }
};

extern std::unordered_map<COLORREF, HBRUSH> g_brushCache;
extern HFONT g_font;

HBRUSH GetBrush(COLORREF c);
void ClearBrushCache();
std::wstring MakeWorstCaseDigits(int digits);
int NumDigits(int v);

int FindBestFontHeight(HDC hdc, int cellW, int cellH, int digits);
