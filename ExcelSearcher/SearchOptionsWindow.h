#pragma once
#include <functional>
#include <imgui.h>

// 검색 옵션을 한 곳에 모아두는 구조체
struct FSearchOptions
{
    // 기본 검색 동작
    bool bCaseSensitive = false;        // 대/소문자 구분
    bool bWholeWord = false;            // 단어 전체 일치
    bool bUseCustomFillter = false;     // 커스텀 필터 사용

    // 디버그
    bool bShowDebugOverlay = false;     // 디버그 오버레이(히트박스/시간 등)

    // 추가 정보(메타데이터) 선택
    bool bEnablePreview     = true;     // 미리보기 패널 사용
    bool bShowFilePath      = true;     // 파일 경로 표시
    bool bShowSheetName     = true;     // 시트 이름 표시 (엑셀 등)
    bool bShowCellAddress   = true;     // 셀 주소 표시 (예: A1)
    bool bShowSnippet       = true;     // 스니펫(문맥) 표시
};


class SearchOptionsWindow
{
public:
    // 옵션 변경이 적용될 때 외부에 알려줄 콜백
    using FOnApply = std::function<void(const FSearchOptions&)>;

    SearchOptionsWindow() = default;

    // ====== 수명/제어 ======
    void Open();
    void Close();
    bool IsOpen() const { return bOpen; }

    // 매 프레임 호출하여 창을 그립니다.
    void Draw();
   

    // 외부에서 현재 옵션을 읽고 싶을 때
    const FSearchOptions& Get() const { return Options; }

    // 외부에서 옵션을 갱신하고 싶을 때(예: 저장 불러오기)
    void Set(const FSearchOptions& In) { Options = In; }

    // ‘적용’ 버튼을 눌렀을 때 호출할 콜백 등록
    void SetOnApply(FOnApply In) { OnApply = std::move(In); }

private:
    // 실제 UI 구성
    void DrawContents();

private:
    const char* Title = "검색 옵션";
    bool bOpen = false;             // 창 열림 여부
    bool bModal = false;            // 모달 팝업 여부
    FSearchOptions Options;         // 현재 옵션 값
    FSearchOptions DraftOptions;    // 임시 저장용 옵션
    FOnApply OnApply;               // 적용 콜백
};

