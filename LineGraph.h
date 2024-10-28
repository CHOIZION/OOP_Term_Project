// LineGraph.h
#ifndef LINEGRAPH_H
#define LINEGRAPH_H

#include <windows.h>
#include <unordered_map>
#include <string>

void DrawLineGraph(HDC hdc, const std::unordered_map<std::wstring, int>& counts);

#endif // LINEGRAPH_H
