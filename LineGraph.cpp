// LineGraph.cpp
#include "LineGraph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>  // 추가된 헤더

// main.cpp에서 선언된 전역 변수 사용
extern std::vector<std::pair<RECT, std::wstring>> lineGraphPoints;

void DrawLineGraph(HDC hdc, const std::unordered_map<std::wstring, int>& counts) {
    // 이전 데이터 포인트 위치 초기화
    lineGraphPoints.clear();

    int width = 800;
    int height = 400;
    int margin = 50;

    int maxCount = 0;
    for (const auto& entry : counts) {
        if (entry.second > maxCount) {
            maxCount = entry.second;
        }
    }

    int x = margin;
    int y = height - margin;

    // 학생 이름들을 정렬하여 일정한 x 간격으로 배치
    std::vector<std::pair<std::wstring, int>> sortedCounts(counts.begin(), counts.end());
    std::sort(sortedCounts.begin(), sortedCounts.end());

    int pointCount = static_cast<int>(sortedCounts.size());
    int xStep = (width - 2 * margin) / (pointCount - 1);

    POINT* points = new POINT[pointCount];
    int index = 0;

    for (const auto& entry : sortedCounts) {
        int pointX = x + index * xStep;
        int pointY = y - static_cast<int>((entry.second / static_cast<double>(maxCount)) * (height - 2 * margin));

        points[index] = { pointX, pointY };

        // 데이터 포인트 그리기
        Ellipse(hdc, pointX - 5, pointY - 5, pointX + 5, pointY + 5);

        // 학생 이름 저장을 위한 영역 저장
        RECT rect = { pointX - 5, pointY - 5, pointX + 5, pointY + 5 };
        lineGraphPoints.push_back({ rect, entry.first });

        // 학생 이름 표시
        TextOut(hdc, pointX - 10, pointY - 20, entry.first.c_str(), static_cast<int>(entry.first.size()));

        index++;
    }

    // 꺾은선 그리기
    Polyline(hdc, points, pointCount);

    delete[] points;
}
