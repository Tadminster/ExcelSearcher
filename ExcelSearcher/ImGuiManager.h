#pragma once

// 시스템 헤더
#include <d3d11.h>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <filesystem>

// 외부 라이브러리 헤더
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <OpenXLSX.hpp>

// 프로젝트 내부 헤더
#include "Types.h"
#include "ExcelSearchResult.h"
#include "SearchOptionsWindow.h"
#include "DebugLogWindow.h"


struct FViewState
{
    bool bEnablePreview = true;
    bool bShowFilePath = true;
    bool bShowSheetName = true;
    bool bShowCellAddress = true;
    bool bShowSnippet = true;
    bool bUseCustomMeta = false;

    int  ColumnCount = 0;        // 위 플래그 합계
};


class ImGuiManager : public Singleton<ImGuiManager>, public Scene
{
    //====================================================================================
    //  ImGui 관련
    //====================================================================================
public:
    void Init() override;
    void Release() override;
    void Update() override;
    void LateUpdate() override;
    void Render() override;
    void ResizeScreen() override;

    // ImGui 초기화
    void SetupImGuiContext(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    void SetupWindowIconsForViewports();   // 뷰포트 생성 시 아이콘 설정용 콜백 등록

    // 스타일 및 설정
    void SetupStyle();

private:
    void DrawResultsTable();    // 검색 결과 테이블


    //====================================================================================
    //  멤버 변수들
    //====================================================================================
public:
    const char* filters{ "엑셀 파일 (*.xlsx){.xlsx},모든 파일{.*}" }; // 파일 다이얼로그 설정

    // 외부 접근이 필요한 데이터
    std::map<std::string, std::string> selectedFiles;        // 선택된 파일명, 경로
    std::vector<ExcelSearchResult> searchResults;            // 검색 결과 리스트

private:
    std::string currentKeyword; // 현재 검색중인 키워드
    std::vector<std::pair<std::string, std::string>> searchQueue; // 검색 대기(큐)
    class SearchOptionsWindow SearchOptions; // 검색 옵션 UI
    class DebugLogWindow DebugLog;       // 디버그 로그 UI

    // 상태 체크용
public:
    // main에서 bIsWindowOpen을 리턴받아 종료시점을 알기 위한 함수
    bool IsDone() const;

private:
    // 내부 상태
    bool bShowDemoWindow{ false }; // 데모 윈도우를 보여줄지 여부
    bool bIsWindowOpen{ true }; // 윈도우창이 켜져 되어있는지 여부
    bool bIsSearching{ false }; // 현재 검색이 진행중인지 여부
    bool bHasSearched{ false }; // 검색한 적이 있는지 여부
    std::vector<std::string> CustomFilters; // 사용자 정의 필터 목록

//====================================================================================
//  Cached View
//====================================================================================
public:
    struct FViewState CachedView; // 뷰 상태 캐시 (옵션 창에서 변경될 때마다 갱신)

private:
    void UpdateCachedView(class SearchOptionsWindow& InSearchOptions); // CachedView 갱신

//====================================================================================
//  엑셀 파일 검색 처리
//====================================================================================
private:
    bool StartSearch(const std::string& keyword);
    void SearchInExcelFile(const std::string& keyword, const std::pair<std::string, std::string>& filePair);  // 선택된 엑셀 파일에서 검색
    std::string CopyExcelFile(const std::string& originalPath); // 엑셀 파일 복사
    bool ProcessCell(OpenXLSX::XLWorksheet& sheet, const std::string& fileName, const std::string& sheetName, const std::string& keyword, uint16_t row, uint16_t col);
    bool IsValueMatch(const std::string& cellValue, const std::string& keyword); // 셀 값이 키워드와 일치하는지 여부
    std::string GetCellText(OpenXLSX::XLWorksheet& sheet, OpenXLSX::XLCellReference); // 셀의 텍스트 값을 문자열로 변환
    std::vector<std::string> GetFullRowData(OpenXLSX::XLWorksheet& sheet, uint16_t col, uint16_t maxRow = 100); // 특정 행의 전체 데이터 가져오기

    static bool ContainsIgnoreCase(const std::string& cellValue, const std::string& keyword); // 대소문자 구분 없이 포함 여부 확인

    FSheetMetaState BuildMetaState(const FSearchOptions& Opt); // 커스텀 메타데이터 상태 구축
    inline void TryResolveMetaCol(const std::string& cellText, uint32_t col, FSheetMetaState& st); // 커스텀 메타데이터 열 확인 및 저장


//====================================================================================
//  엑셀 파일 내보내기
//====================================================================================
private:
    bool                                bExportDialogRequested = false;
    std::vector<ExcelSearchResult>      ExportCachedResults;  // 버튼 클릭 시점의 결과 스냅샷
    std::string                         ExportFileName;    // "Result_YYYYMMDD_HHMM.xlsx"
    const char*                         ExportDialogKey = "SaveResultsDlg";
    const char*                         ExportDialogTitle = u8"저장할 폴더 선택";
    const char*                         ExportFilters = "Excel (*.xlsx){.xlsx}";

    double s_LastBeat = 0.0;     // 마지막 파일 이름 버퍼 갱신 시각
    const double beatInterval = 0.5;    // 파일 이름 버퍼 갱신 주기

    // 검색 결과 내보내기(저장)
    void ExportSearchResultsOpenDialog(const std::vector<ExcelSearchResult>& results);
    void ExportFolderSeleterModalDraw();
    bool SaveSearchResultsToExcel(const std::vector<ExcelSearchResult>& Results, const std::string& UesrInputPath);


//====================================================================================
//  Progress Bar
//====================================================================================
private:
    bool showProgressBar{ false };  // 진행바 표시 여부
    float progressValue{ 0.0f };    // 0.0f ~ 1.0f 범위
    int currentFileIndex{ 0 };      // 현재 처리 중인 파일 인덱스
    int totalFilesCount{ 0 };       // 총 파일 수

};
