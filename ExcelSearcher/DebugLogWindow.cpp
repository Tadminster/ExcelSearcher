#include "DebugLogWindow.h"
#include "SystemUtils.h" // WStringToUTF8()

#include <imgui.h>
#include <cstdarg>
#include <vector>

//============================================================================
// 정적 상태 정의
//============================================================================
std::deque<DebugLogWindow::Entry> DebugLogWindow::s_Entries;
size_t                      DebugLogWindow::s_MaxEntries = 5000;
std::mutex                  DebugLogWindow::s_Mutex;

bool DebugLogWindow::s_AutoScroll       = true;
bool DebugLogWindow::s_Paused           = false;

bool DebugLogWindow::s_FilterInfo       = true;
bool DebugLogWindow::s_FilterWarning    = true;
bool DebugLogWindow::s_FilterError      = true;
bool DebugLogWindow::s_FilterFile       = true;
bool DebugLogWindow::s_FilterSheet      = true;
bool DebugLogWindow::s_FilterSeparator  = true;
char DebugLogWindow::s_TextFilter[256]  = "";

// 초기화: 최대 보관 개수 지정 및 버퍼 클리어
void DebugLogWindow::Init(size_t maxEntries)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_MaxEntries = maxEntries;
    s_Entries.clear();
}

// 로그 추가 (스레드 세이프 / 일시정지 시 무시)
void DebugLogWindow::Add(const std::wstring& msg, LogLevel level)
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    if (s_Paused) return;

    s_Entries.emplace_back(Entry{ std::chrono::system_clock::now(), level, msg });
    if (s_Entries.size() > s_MaxEntries)
        s_Entries.pop_front();
}

// 포맷 기반 로그 추가
//- 가변 인자 → 안전한 버퍼 크기 산출 → 출력 → Add()
void DebugLogWindow::AddFmt(LogLevel level, const wchar_t* fmt, ...)
{
    if (!fmt) return;

    va_list args;
    va_start(args, fmt);

    // 가변 버퍼 길이 산출
    va_list args2;
    va_copy(args2, args);
    int len = _vscwprintf(fmt, args2); // null 제외한 출력 문자 수
    va_end(args2);

    std::wstring out;
    if (len > 0)
    {
        // null 포함 길이로 버퍼 확보 > 실제 출력 길이에 맞춰 shrink
        out.resize(static_cast<size_t>(len) + 1);
        int written = vswprintf_s(out.data(), out.size(), fmt, args); // null 포함 크기 전달
        if (written >= 0)
        {
            // written에는 null 제외 길이가 들어오므로 그 길이에 맞춰 리사이즈
            out.resize(static_cast<size_t>(written));
        }
        else
        {
            out.clear();
        }
    }
    va_end(args);

    if (!out.empty())
        Add(out, level);
}

// 전체 로그 제거
void DebugLogWindow::Clear()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_Entries.clear();
}

// 상태 토글/조회
void DebugLogWindow::SetAutoScroll(bool enabled) { s_AutoScroll = enabled; }
void DebugLogWindow::SetPaused(bool paused) { s_Paused = paused; }
bool DebugLogWindow::IsPaused() { return s_Paused; }
bool DebugLogWindow::IsAutoScroll() { return s_AutoScroll; }

// 레벨 문자열 (테이블/클립보드 표시용)
const char* DebugLogWindow::LevelToShortString(LogLevel lv)
{
    switch (lv)
    {
    case LogLevel::Info:        return "Info";
    case LogLevel::Warning:     return "Warning";
    case LogLevel::Error:       return "Error";
    case LogLevel::File:        return "File";
    case LogLevel::Sheet:       return "Sheet";
    case LogLevel::Separator:   return "Separator";
    default: return "?";
    }
}

// 레벨 컬러 (UI 가독성)
ImU32 DebugLogWindow::LevelColor(LogLevel lv)
{
    switch (lv)
    {
    case LogLevel::Info:        return IM_COL32(200, 200, 200, 255);
    case LogLevel::Warning:     return IM_COL32(255, 200, 0, 255);
    case LogLevel::Error:       return IM_COL32(255, 80, 80, 255);
    case LogLevel::File:        return IM_COL32(100, 255, 100, 255);
    case LogLevel::Sheet:       return IM_COL32(100, 200, 255, 255);
    case LogLevel::Separator:   return IM_COL32(160, 160, 160, 255);
    default: return IM_COL32_WHITE;
    }
}

// 타임스탬프 포맷 "HH:MM:SS.mmm" (로컬타임)
std::string DebugLogWindow::FormatTime(const std::chrono::system_clock::time_point& tp)
{
    using namespace std::chrono;
    auto t = system_clock::to_time_t(tp);
    auto ms = duration_cast<milliseconds>(tp.time_since_epoch()) % 1000;

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d",
        tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(ms.count()));
    return std::string(buf);
}

//============================================================================
// ImGui 창 렌더링
//============================================================================
void DebugLogWindow::DrawWindow(bool* p_open)
{
    // 제목/플래그: Collapse 비활성 + 메뉴바 활성
    if (!ImGui::Begin(u8"디버그 오버레이", p_open,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
    {
        ImGui::End();
        return;
    }

    //------------------------------------------------------------------------
    // 메뉴바
    //------------------------------------------------------------------------
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Log"))
        {
            if (ImGui::MenuItem("Clear", nullptr, false, true))
                Clear();

            bool paused = s_Paused;
            if (ImGui::MenuItem("Pause", nullptr, &paused))
                s_Paused = paused;

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Auto Scroll", nullptr, &s_AutoScroll);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    //------------------------------------------------------------------------
    // 필터 바: 텍스트 필터 + 레벨 토글 + 일괄 복사
    //------------------------------------------------------------------------
    ImGui::SetNextItemWidth(220.f);
    ImGui::InputTextWithHint("##filter", u8"텍스트 필터...", s_TextFilter, sizeof(s_TextFilter));
    ImGui::SameLine();
    ImGui::Checkbox("Info", &s_FilterInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &s_FilterWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &s_FilterError);
    ImGui::SameLine();
    ImGui::Checkbox("File", &s_FilterFile);
    ImGui::SameLine();
    ImGui::Checkbox("Sheet", &s_FilterSheet);
    ImGui::SameLine();
    ImGui::Checkbox("Separator", &s_FilterSeparator);

    ImGui::SameLine();
    if (ImGui::Button("Copy All"))
    {
        // 필터가 적용된 상태 그대로 UTF-8로 클립보드에 복사
        std::string bulk;
        {
            std::lock_guard<std::mutex> lock(s_Mutex);
            bulk.reserve(s_Entries.size() * 64);

            for (const auto& e : s_Entries)
            {
                // 레벨 필터
                if ((e.Level == LogLevel::Info && !s_FilterInfo) ||
                    (e.Level == LogLevel::Warning && !s_FilterWarning) ||
                    (e.Level == LogLevel::Error && !s_FilterError) ||
                    (e.Level == LogLevel::File && !s_FilterFile) ||
                    (e.Level == LogLevel::Sheet && !s_FilterSheet) ||
                    (e.Level == LogLevel::Separator && !s_FilterSeparator))
                    continue;

                // 텍스트 필터
                if (s_TextFilter[0] != '\0')
                {
                    std::string u8 = SystemUtils::WStringToUTF8(e.Message);
                    if (u8.find(s_TextFilter) == std::string::npos)
                        continue;

                    bulk += FormatTime(e.Ts);
                    bulk += " [";
                    bulk += LevelToShortString(e.Level);
                    bulk += "] ";
                    bulk += u8;
                    bulk += "\n";
                }
                else
                {
                    bulk += FormatTime(e.Ts);
                    bulk += " [";
                    bulk += LevelToShortString(e.Level);
                    bulk += "] ";
                    bulk += SystemUtils::WStringToUTF8(e.Message);
                    bulk += "\n";
                }
            }
        }
        ImGui::SetClipboardText(bulk.c_str());
    }

    //------------------------------------------------------------------------
    // 로그 테이블 (가로 리사이즈/세로 스크롤)
    //------------------------------------------------------------------------
    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_BordersInnerV;

    // 가용 높이 모두 사용
    if (ImGui::BeginTable("LogTable", 3, tableFlags, ImVec2(0, 0)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 120.f);
        ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 60.f);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // 대용량 최적화 (잠금 범위 최소 + 리스트 클리퍼)
        std::lock_guard<std::mutex> lock(s_Mutex);
        const int total = static_cast<int>(s_Entries.size());

        ImGuiListClipper clipper;
        clipper.Begin(total);
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                const Entry& e = s_Entries[i];

                // 레벨 필터
                if ((e.Level == LogLevel::Info && !s_FilterInfo) ||
                    (e.Level == LogLevel::Warning && !s_FilterWarning) ||
                    (e.Level == LogLevel::Error && !s_FilterError) ||
                    (e.Level == LogLevel::File && !s_FilterFile) ||
                    (e.Level == LogLevel::Sheet && !s_FilterSheet) ||
                    (e.Level == LogLevel::Separator && !s_FilterSeparator))
                    continue;

                // 텍스트 필터
                if (s_TextFilter[0] != '\0')
                {
                    std::string u8 = SystemUtils::WStringToUTF8(e.Message);
                    if (u8.find(s_TextFilter) == std::string::npos)
                        continue;
                }

                ImGui::TableNextRow();

                // Time
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(FormatTime(e.Ts).c_str());

                // Level
                ImGui::TableSetColumnIndex(1);
                ImU32 color = LevelColor(e.Level);
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(LevelToShortString(e.Level));
                ImGui::PopStyleColor();

                // Message
                ImGui::TableSetColumnIndex(2);
                std::string u8 = SystemUtils::WStringToUTF8(e.Message);
                ImGui::TextUnformatted(u8.c_str());
            }
        }

        // 자동 스크롤: 현재 바닥에 있을 때만 바닥 유지
        if (s_AutoScroll)
        {
            float y = ImGui::GetScrollY();
            float maxY = ImGui::GetScrollMaxY();
            if (y >= maxY - 1.0f)
                ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
