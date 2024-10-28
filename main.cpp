// main.cpp
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <locale>
#include <algorithm>
#include <iomanip>
#include <codecvt>   // 추가된 헤더
#include "Circle.h"      // 원그래프 관련 함수 포함
#include "LineGraph.h"   // 꺾은선 그래프 관련 함수 포함

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct ParticipationRecord {
    std::wstring dateTime;
    std::wstring message;
};

std::unordered_map<std::wstring, std::vector<ParticipationRecord>> analyzeParticipation(const std::vector<std::wstring>& filePaths);
void DrawBarGraph(HDC hdc, const std::unordered_map<std::wstring, std::vector<ParticipationRecord>>& data);
void ShowPercentage(HWND hwnd, const std::unordered_map<std::wstring, std::vector<ParticipationRecord>>& data);
void ShowStudentDetails(HWND hwnd, const std::wstring& studentName);
std::unordered_map<std::wstring, int> calculateParticipationCounts(const std::unordered_map<std::wstring, std::vector<ParticipationRecord>>& data);

std::unordered_map<std::wstring, std::vector<ParticipationRecord>> participationData;
bool dataReady = false;
bool showPieChart = false;      // 원그래프 표시 여부 플래그
bool showLineGraph = false;     // 꺾은선 그래프 표시 여부 플래그
int scrollPos = 0;              // 수평 스크롤 위치 저장
std::vector<std::wstring> selectedFiles; // 추가된 파일 목록

// 학생 막대 위치와 이름을 저장하는 구조체
std::vector<std::pair<RECT, std::wstring>> studentRects;

// 꺾은선 그래프 데이터 포인트 위치와 이름을 저장하는 구조체
std::vector<std::pair<RECT, std::wstring>> lineGraphPoints;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Participation Analyzer";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        TEXT("학생 참여도 분석기"),
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 800,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void OpenFileAndAnalyze(HWND hwnd) {
    selectedFiles.clear();  // 기존 파일 목록 초기화
    OPENFILENAME ofn;
    wchar_t fileName[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = TEXT("Text Files\0*.txt\0All Files\0*.*\0");
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        selectedFiles.push_back(fileName); // 파일 추가
        participationData = analyzeParticipation(selectedFiles);
        dataReady = false;
        showPieChart = false;
        showLineGraph = false;
        scrollPos = 0;

        // 스크롤 범위 설정
        int totalWidth = static_cast<int>(participationData.size()) * (40 + 60) + 50; // 막대와 간격 합계
        SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE, 0, totalWidth, 800 };
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void AddFile(HWND hwnd) {
    OPENFILENAME ofn;
    wchar_t fileName[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = TEXT("Text Files\0*.txt\0All Files\0*.*\0");
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        selectedFiles.push_back(fileName); // 파일 목록에 추가
        participationData = analyzeParticipation(selectedFiles);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

std::unordered_map<std::wstring, std::vector<ParticipationRecord>> analyzeParticipation(const std::vector<std::wstring>& filePaths) {
    std::unordered_map<std::wstring, std::vector<ParticipationRecord>> data;

    for (const auto& filePath : filePaths) {
        std::wifstream file(filePath);
        file.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));  // UTF-8 인코딩 설정

        if (!file.is_open()) {
            MessageBox(NULL, TEXT("파일을 열 수 없습니다."), TEXT("오류"), MB_OK | MB_ICONERROR);
            continue;
        }

        std::wstring line;
        // 정규 표현식 수정: 시간, 이름 추출
        std::wregex pattern(LR"(^(\d{2}:\d{2}:\d{2}) From (.*?) to (.*?):$)");
        std::wsmatch match;
        while (std::getline(file, line)) {
            if (std::regex_match(line, match, pattern)) {
                ParticipationRecord record;
                record.dateTime = match[1].str();
                std::wstring name = match[2].str();
                // 다음 줄에서 메시지 읽기
                if (std::getline(file, record.message)) {
                    // 메시지 앞의 공백 제거
                    record.message.erase(0, record.message.find_first_not_of(L" \t"));
                    data[name].push_back(record);
                }
            }
        }
    }
    return data;
}

void DrawFileList(HDC hdc) {
    int yPos = 100;  // 파일 목록 표시 위치를 아래로 이동
    TextOut(hdc, 50, yPos, TEXT("추가된 파일 목록:"), lstrlen(TEXT("추가된 파일 목록:")));
    yPos += 20;

    for (const auto& file : selectedFiles) {
        TextOut(hdc, 50, yPos, file.c_str(), static_cast<int>(file.length()));
        yPos += 20;
    }
}

void DrawBarGraph(HDC hdc, const std::unordered_map<std::wstring, std::vector<ParticipationRecord>>& data) {
    int x = 50 - scrollPos;
    int max_height = 300;
    int bar_width = 40;
    int spacing = 60;

    // 참여 횟수를 계산
    std::unordered_map<std::wstring, int> counts;
    int max_value = 0;
    for (const auto& entry : data) {
        int count = static_cast<int>(entry.second.size());
        counts[entry.first] = count;
        if (count > max_value) {
            max_value = count;
        }
    }

    // 막대 그래프 그리기
    studentRects.clear(); // 학생 막대 위치 정보 초기화
    for (const auto& entry : counts) {
        int bar_height = static_cast<int>((entry.second / static_cast<double>(max_value)) * max_height);
        int bar_x = x;
        Rectangle(hdc, bar_x, 500 - bar_height, bar_x + bar_width, 500);

        std::wstring text = entry.first + L" (" + std::to_wstring(entry.second) + L")";
        SIZE textSize;
        GetTextExtentPoint32(hdc, text.c_str(), static_cast<int>(text.size()), &textSize);
        int text_x = bar_x + (bar_width - textSize.cx) / 2;

        TextOut(hdc, text_x, 500 - bar_height - 20, text.c_str(), static_cast<int>(text.size()));

        // 학생 막대 위치 저장
        RECT rect = { bar_x, 500 - bar_height, bar_x + bar_width, 500 };
        studentRects.push_back({ rect, entry.first });

        x += bar_width + spacing;
    }
}

std::unordered_map<std::wstring, int> calculateParticipationCounts(const std::unordered_map<std::wstring, std::vector<ParticipationRecord>>& data) {
    std::unordered_map<std::wstring, int> counts;
    for (const auto& entry : data) {
        counts[entry.first] = static_cast<int>(entry.second.size());
    }
    return counts;
}

void ShowPercentage(HWND hwnd, const std::unordered_map<std::wstring, std::vector<ParticipationRecord>>& data) {
    if (data.empty()) {
        MessageBox(hwnd, TEXT("파일을 먼저 선택해주세요."), TEXT("오류"), MB_OK | MB_ICONWARNING);
        return;
    }

    int totalCount = 0;
    std::unordered_map<std::wstring, int> counts;
    for (const auto& entry : data) {
        int count = static_cast<int>(entry.second.size());
        counts[entry.first] = count;
        totalCount += count;
    }

    std::vector<std::pair<std::wstring, double>> percentages;
    for (const auto& entry : counts) {
        double percent = (entry.second / static_cast<double>(totalCount)) * 100.0;
        percentages.push_back({ entry.first, percent });
    }

    std::sort(percentages.begin(), percentages.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    std::wstringstream result;
    result << L"참여도 순위\n\n";
    for (const auto& entry : percentages) {
        result << entry.first << L": " << std::fixed << std::setprecision(2) << entry.second << L"%\n";
    }

    MessageBox(hwnd, result.str().c_str(), TEXT("참여도 퍼센트"), MB_OK);
}

void ShowStudentDetails(HWND hwnd, const std::wstring& studentName) {
    auto it = participationData.find(studentName);
    if (it == participationData.end()) {
        MessageBox(hwnd, TEXT("학생 정보를 찾을 수 없습니다."), TEXT("오류"), MB_OK | MB_ICONERROR);
        return;
    }

    const auto& records = it->second;
    std::wstringstream ss;
    ss << studentName << L"의 참여 세부 정보:\n\n";
    for (const auto& record : records) {
        ss << L"[" << record.dateTime << L"] " << record.message << L"\n";
    }

    MessageBox(hwnd, ss.str().c_str(), studentName.c_str(), MB_OK);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND openFileButton;
    static HWND addFileButton;
    static HWND analyzeButton;
    static HWND percentageButton;
    static HWND pieChartButton;
    static HWND lineGraphButton;

    switch (uMsg) {
    case WM_CREATE: {
        openFileButton = CreateWindow(TEXT("BUTTON"), TEXT("파일 열기"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 20, 100, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        addFileButton = CreateWindow(TEXT("BUTTON"), TEXT("파일 추가하기"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            200, 20, 100, 30, hwnd, (HMENU)6, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        analyzeButton = CreateWindow(TEXT("BUTTON"), TEXT("참여도 분석"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            350, 20, 100, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        percentageButton = CreateWindow(TEXT("BUTTON"), TEXT("퍼센트 보기"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            500, 20, 100, 30, hwnd, (HMENU)3, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        pieChartButton = CreateWindow(TEXT("BUTTON"), TEXT("원그래프 보기"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            650, 20, 120, 30, hwnd, (HMENU)4, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        lineGraphButton = CreateWindow(TEXT("BUTTON"), TEXT("꺾은선 그래프"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            800, 20, 120, 30, hwnd, (HMENU)5, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        ShowWindow(openFileButton, SW_SHOW);
        ShowWindow(addFileButton, SW_SHOW);
        ShowWindow(analyzeButton, SW_SHOW);
        ShowWindow(percentageButton, SW_SHOW);
        ShowWindow(pieChartButton, SW_SHOW);
        ShowWindow(lineGraphButton, SW_SHOW);

        InvalidateRect(hwnd, NULL, TRUE);
    } break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            OpenFileAndAnalyze(hwnd);
        }
        else if (LOWORD(wParam) == 6) {
            AddFile(hwnd);
        }
        else if (LOWORD(wParam) == 2) {
            if (participationData.empty()) {
                MessageBox(hwnd, TEXT("파일을 먼저 선택해주세요."), TEXT("오류"), MB_OK | MB_ICONWARNING);
            }
            else {
                dataReady = true;
                showPieChart = false;
                showLineGraph = false;
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        else if (LOWORD(wParam) == 3) {
            if (participationData.empty()) {
                MessageBox(hwnd, TEXT("파일을 먼저 선택해주세요."), TEXT("오류"), MB_OK | MB_ICONWARNING);
            }
            else {
                ShowPercentage(hwnd, participationData);
            }
        }
        else if (LOWORD(wParam) == 4) {
            if (participationData.empty()) {
                MessageBox(hwnd, TEXT("파일을 먼저 선택해주세요."), TEXT("오류"), MB_OK | MB_ICONWARNING);
            }
            else {
                dataReady = true;  // dataReady 설정
                showPieChart = true;
                showLineGraph = false;
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        else if (LOWORD(wParam) == 5) {
            if (participationData.empty()) {
                MessageBox(hwnd, TEXT("파일을 먼저 선택해주세요."), TEXT("오류"), MB_OK | MB_ICONWARNING);
            }
            else {
                dataReady = true;  // dataReady 설정
                showLineGraph = true;
                showPieChart = false;
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        break;

    case WM_LBUTTONDOWN: {
        POINT pt;
        pt.x = LOWORD(lParam) + scrollPos; // 스크롤 위치를 고려
        pt.y = HIWORD(lParam);

        if (dataReady && !participationData.empty()) {
            if (showPieChart) {
                // 원 그래프 클릭 처리 (현재는 구현하지 않음)
            }
            else if (showLineGraph) {
                // 꺾은선 그래프 클릭 처리
                for (const auto& entry : lineGraphPoints) {
                    if (PtInRect(&entry.first, pt)) {
                        ShowStudentDetails(hwnd, entry.second);
                        break;
                    }
                }
            }
            else {
                // 막대 그래프 클릭 처리
                for (const auto& entry : studentRects) {
                    if (PtInRect(&entry.first, pt)) {
                        ShowStudentDetails(hwnd, entry.second);
                        break;
                    }
                }
            }
        }
    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (!selectedFiles.empty()) {
            DrawFileList(hdc);
        }

        if (dataReady && !participationData.empty()) {
            // 참여 횟수 맵 생성
            auto counts = calculateParticipationCounts(participationData);

            if (showPieChart) {
                DrawPieChart(hdc, counts);
            }
            else if (showLineGraph) {
                DrawLineGraph(hdc, counts);
            }
            else {
                DrawBarGraph(hdc, participationData);
            }
        }

        EndPaint(hwnd, &ps);
    } break;

    case WM_HSCROLL: {
        int oldScrollPos = scrollPos;
        switch (LOWORD(wParam)) {
        case SB_LINELEFT: scrollPos -= 20; break;
        case SB_LINERIGHT: scrollPos += 20; break;
        case SB_PAGELEFT: scrollPos -= 50; break;
        case SB_PAGERIGHT: scrollPos += 50; break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: scrollPos = HIWORD(wParam); break;
        }

        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE | SIF_PAGE;
        GetScrollInfo(hwnd, SB_HORZ, &si);
        scrollPos = max(0, min(scrollPos, si.nMax - si.nPage));
        if (scrollPos != oldScrollPos) {
            SetScrollPos(hwnd, SB_HORZ, scrollPos, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    } break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
