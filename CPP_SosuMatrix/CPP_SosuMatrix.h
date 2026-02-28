#pragma once
#include "stdafx.h"
#include "resource.h"


//自然数を表す構造体、数値、色、計算済みかどうか、素数かどうか などを持つ
struct NaturalNumber {
	int value;          // 数値
	COLORREF cellColor; // セルの背景色
	COLORREF textColor; // セルの背景色とテキストの色
	bool calculated;    // 計算済みかどうかのフラグ
	bool isPrime;       // 素数かどうかのフラグ

    NaturalNumber() : value(0), cellColor(RGB(255, 255, 255)), textColor(RGB(0, 0, 0)), calculated(false), isPrime(false) {}
    NaturalNumber(int v) : value(v), cellColor(RGB(255, 255, 255)), textColor(RGB(0, 0, 0)), calculated(false), isPrime(false) {}
};

// セルごとに色を変える（std::unordered_map）
//static std::unordered_set<int> g_highlightSet; // 1..N*M
struct CellStyle {
    COLORREF cell;
    COLORREF text;
};

class MatrixArea
{
	std::vector<NaturalNumber> _NaNumbers; // 1..N*M の自然数を持つ配列、セルのスタイルもここで管理する（背景色、テキストの色）

    int __M;          // cols
    int __N;          // rows
    int __SIZE_X;    // initial window client-ish (actual client depends on styles)
    int __SIZE_Y;

public:
    MatrixArea(int M, int N, int sizeX, int sizeY) : 
        __M(M), __N(N), __SIZE_X(sizeX), __SIZE_Y(sizeY) 
    {}
	MatrixArea() : __M(0), __N(0), __SIZE_X(0), __SIZE_Y(0) {}

    void RebuildFont(HWND hwnd, HDC hdc, int clientW, int clientH);
    void PaintGrid(HWND hwnd, HDC hdc);
    void SetCellHighlight(HWND hwnd, int q, COLORREF cellColor, COLORREF textColor);
    void InvalidateCellByQ(HWND hwnd, int q);
    void RemoveCellHighlight(HWND hwnd, int q);
    bool TryGetQFromPoint(HWND hwnd, int x, int y, int* outQ) const;

    bool IsValidQ(int q) const;

 //   int M() const { return __M; }
 //   int N() const { return __N; }
      int SIZE_X() const { return __SIZE_X; }
	  int SIZE_Y() const { return __SIZE_Y; }
};

extern std::unordered_map<int, CellStyle> g_highlightMap; // key: Q
extern std::unordered_map<COLORREF, HBRUSH> g_brushCache;
extern HFONT g_font;

HBRUSH GetBrush(COLORREF c);
void ClearBrushCache();
std::wstring MakeWorstCaseDigits(int digits);   // 桁数（maxVal）の文字列として、幅の厳しさを見るため "888...8" を作る（最も幅が出やすいケースに寄せる）
int NumDigits(int v);

int FindBestFontHeight(HDC hdc, int cellW, int cellH, int digits);
void ClearAllCellHighlights(HWND hwnd);
//void RebuildFont(HWND hwnd, HDC hdc, int clientW, int clientH, int _M, int _N);
//void PaintGrid(HWND hwnd, HDC hdc, int _M, int _N);
//bool GetCellRectByQ(HWND hwnd, int _Q, RECT* out, int _M, int _N);
//bool IsValidQ(int q, int _M, int _N);
//void InvalidateCellByQ(HWND hwnd, int q, int _M, int _N);
//void SetCellHighlight(HWND hwnd, int q, COLORREF cellColor, COLORREF textColor, int _M, int _N);
//void RemoveCellHighlight(HWND hwnd, int q, int _M, int _N);
