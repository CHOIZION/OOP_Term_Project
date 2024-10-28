// Circle.h
#ifndef CIRCLE_H
#define CIRCLE_H

#include <windows.h>
#include <unordered_map>
#include <string>

void DrawPieChart(HDC hdc, const std::unordered_map<std::wstring, int>& counts);

#endif // CIRCLE_H
