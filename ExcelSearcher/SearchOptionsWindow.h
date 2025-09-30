#pragma once
#include <string>
#include <functional>
#include <vector>
#include <imgui.h>

// 검색 옵션을 한 곳에 모아두는 구조체
struct FSearchOptions
{
    // 기본 검색 동작
    bool bCaseSensitive     = false;    // 대/소문자 구분
    bool bWholeWord         = false;    // 단어 전체 일치
    bool bUseRegex          = false;    // 정규식 사용

    // 추가 정보(메타데이터) 선택
    bool bEnablePreview     = true;     // 미리보기 패널 사용
    bool bShowFileName      = true;     // 파일 이름 표시
    bool bShowSheetName     = true;     // 시트 이름 표시
    bool bShowCellAddress   = true;     // 셀 주소 표시 (예: A1)
    bool bShowSnippet       = true;     // 스니펫(문맥) 표시
    bool bUseCustomFillter  = false;    // 커스텀 필터 사용

    // 디버그
    bool bShowDebugOverlay = false;     // 디버그 오버레이(히트박스/시간 등)

    // 커스텀 메타데이터
    std::vector<std::string> CustomMetadatas; // 사용자 정의 메타데이터 목록
};


class SearchOptionsWindow
{
public:
    SearchOptionsWindow();

    // 옵션 변경이 적용될 때 외부에 알려줄 콜백
    using FOnApply = std::function<void(const FSearchOptions&)>;

    // ============================================================================
    // 수명 제어
    // ============================================================================
public:
    void Open();
    void Close();
    bool IsOpen() const { return bOpen; }
    void Draw();

private:
    // 실제 UI 구성
    void DrawContents();

    // ============================================================================
    // 외부 인터페이스
    // ============================================================================
public:
    // 외부에서 현재 옵션을 읽고 싶을 때
    const FSearchOptions& Get() const { return Options; }

    // 외부에서 옵션을 갱신용 (저장/불러오기)
    void Set(const FSearchOptions& In);

    // 변경 사항 적용 콜백
    void SetOnApply(FOnApply In) { OnApply = std::move(In); }

    // 메타데이터 열 개수 (결과 리스트 헤더 설정용)
    int GetMetaColumnCount() const { return MetaColumnCount; }

    // ============================================================================
    // 내부 로직용 함수
    // ============================================================================
private:
    void SetMetaColumnCount(); // 메타데이터 열 개수 계산
    void SanitizeCustomMetadatas(std::vector<std::string>& metas); // 커스텀 메타데이터 정리 (중복/공백 제거)


    // ============================================================================
    // 멤버 변수
    // ============================================================================
private:
    const char* Title = u8"검색 옵션";
    bool bOpen = false;             // 창 열림 여부
    bool bModal = false;            // 모달 팝업 여부
    FSearchOptions Options;         // 현재 옵션 값
    FSearchOptions DraftOptions;    // 임시 저장용 옵션
    FOnApply OnApply;               // 적용 콜백

    int MetaColumnCount;            // 메타데이터 열 개수 (결과 리스트 헤더 설정용)
};

