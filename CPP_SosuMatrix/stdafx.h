#pragma once
// GridNumbersWin32.cpp
// Build (MSVC example):
//   cl /std:c++17 /DUNICODE /D_UNICODE GridNumbersWin32.cpp user32.lib gdi32.lib
#define NOMINMAX // 先に定義しておくことが重要！
#include <windows.h>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
