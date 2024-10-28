// LineGraph.cpp
#include "LineGraph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>  // �߰��� ���

// main.cpp���� ����� ���� ���� ���
extern std::vector<std::pair<RECT, std::wstring>> lineGraphPoints;

void DrawLineGraph(HDC hdc, const std::unordered_map<std::wstring, int>& counts) {
    // ���� ������ ����Ʈ ��ġ �ʱ�ȭ
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

    // �л� �̸����� �����Ͽ� ������ x �������� ��ġ
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

        // ������ ����Ʈ �׸���
        Ellipse(hdc, pointX - 5, pointY - 5, pointX + 5, pointY + 5);

        // �л� �̸� ������ ���� ���� ����
        RECT rect = { pointX - 5, pointY - 5, pointX + 5, pointY + 5 };
        lineGraphPoints.push_back({ rect, entry.first });

        // �л� �̸� ǥ��
        TextOut(hdc, pointX - 10, pointY - 20, entry.first.c_str(), static_cast<int>(entry.first.size()));

        index++;
    }

    // ������ �׸���
    Polyline(hdc, points, pointCount);

    delete[] points;
}
