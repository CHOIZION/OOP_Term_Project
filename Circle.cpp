// Circle.cpp
#include "Circle.h"
#include <cmath>
#include <string>
#include <unordered_map>

void DrawPieChart(HDC hdc, const std::unordered_map<std::wstring, int>& counts) {
    int total = 0;
    for (const auto& entry : counts) {
        total += entry.second;
    }

    int startAngle = 0;
    int radius = 150;
    int centerX = 300, centerY = 300;  // ���� �߽� ��ġ

    for (const auto& entry : counts) {
        int sweepAngle = static_cast<int>((entry.second / static_cast<double>(total)) * 360.0);

        // ���� ���� �׸���
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // ��輱 ����
        HBRUSH hBrush = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256)); // ���� ����
        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);

        Pie(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius,
            static_cast<int>(centerX + radius * cos(startAngle * 3.141592 / 180)),
            static_cast<int>(centerY - radius * sin(startAngle * 3.141592 / 180)),
            static_cast<int>(centerX + radius * cos((startAngle + sweepAngle) * 3.141592 / 180)),
            static_cast<int>(centerY - radius * sin((startAngle + sweepAngle) * 3.141592 / 180)));

        // �� ���� ������ �߾ӿ� �ؽ�Ʈ ǥ��
        double midAngle = (startAngle + sweepAngle / 2.0) * 3.141592 / 180.0;
        int textX = static_cast<int>(centerX + (radius / 1.5) * cos(midAngle));
        int textY = static_cast<int>(centerY - (radius / 1.5) * sin(midAngle));

        double percentage = (entry.second / static_cast<double>(total)) * 100.0;
        std::wstring label = entry.first + L" (" + std::to_wstring(static_cast<int>(percentage)) + L"%)";
        TextOut(hdc, textX, textY, label.c_str(), static_cast<int>(label.size()));

        // ���� ���� ����
        startAngle += sweepAngle;

        DeleteObject(hBrush);
        DeleteObject(hPen);
    }
}
