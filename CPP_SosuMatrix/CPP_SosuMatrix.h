#pragma once
#include "stdafx.h"
#include "resource.h"

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

    int __M;
    int __N;
    int __SIZE_X;
    int __SIZE_Y;

public:
    MatrixArea(int M, int N, int sizeX, int sizeY)
        : __M(M), __N(N), __SIZE_X(sizeX), __SIZE_Y(sizeY), _NaNumbers(M * N)
    {
        for (int i = 0; i < M * N; ++i)
            _NaNumbers[i] = NaturalNumber(i + 1);
    }

    MatrixArea() : __M(0), __N(0), __SIZE_X(0), __SIZE_Y(0) {}

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

    int SIZE_X() const { return __SIZE_X; }
    int SIZE_Y() const { return __SIZE_Y; }
};

extern std::unordered_map<COLORREF, HBRUSH> g_brushCache;
extern HFONT g_font;

HBRUSH GetBrush(COLORREF c);
void ClearBrushCache();
std::wstring MakeWorstCaseDigits(int digits);
int NumDigits(int v);

int FindBestFontHeight(HDC hdc, int cellW, int cellH, int digits);
