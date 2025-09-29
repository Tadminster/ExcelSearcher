#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <chrono>
#include <cstdint>

// 로그 레벨 정의 (표시/필터/색상에 사용)
enum class LogLevel : uint8_t
{
    Info,
    Warning,
    Error,
    File,
    Sheet,
    Separator
};


class DebugLogWindow
{
public:
    //=========================================================================
    // 수명 제어
    //=========================================================================
    // 최대 보관 개수 지정 및 초기화
    static void Init(size_t maxEntries = 5000);

    //=========================================================================
    // 외부 인터페이스 (로그 입력)
    //=========================================================================
    // 메시지 추가 (wide string)
    static void Add(const std::wstring& msg, LogLevel level = LogLevel::Info);

    // 포맷 기반 메세지추가 (printf 계열, wide 포맷)
    static void AddFmt(LogLevel level, const wchar_t* fmt, ...);

    // 모든 로그 제거
    static void Clear();

    //=========================================================================
    // UI
    //=========================================================================
    static void DrawWindow(bool* p_open);

    // 편의 토글
    static void SetAutoScroll(bool enabled);
    static void SetPaused(bool paused);

    // 상태 조회
    static bool IsPaused();
    static bool IsAutoScroll();

private:
    //한 줄 로그 엔트리(타임스탬프/레벨/메시지)
    struct Entry
    {
        std::chrono::system_clock::time_point Ts;
        LogLevel Level;
        std::wstring Message;
    };

    //=========================================================================
    // 내부 저장소 (스레드 세이프)
    //=========================================================================
    static std::deque<Entry> s_Entries;  // FIFO 링버퍼 스타일
    static size_t            s_MaxEntries;
    static std::mutex        s_Mutex;

    // 동작 상태
    static bool              s_AutoScroll;   // 스크롤 맨 아래 유지
    static bool              s_Paused;       // true면 Add/ AddFmt 무시

    // UI 상태 (레벨 필터/텍스트 필터)
    static bool              s_FilterInfo;
    static bool              s_FilterWarning;
    static bool              s_FilterError;
    static bool              s_FilterFile;
    static bool              s_FilterSheet;
    static bool              s_FilterSeparator;
    static char              s_TextFilter[256];

    //=========================================================================
    // 내부 유틸
    //=========================================================================
    static const char* LevelToShortString(LogLevel lv);         // 레벨 문자열
    static uint32_t    LevelColor(LogLevel lv);                 // 레벨 컬러
    static std::string FormatTime(const std::chrono::system_clock::time_point& tp); // "HH:MM:SS.mmm"
};
