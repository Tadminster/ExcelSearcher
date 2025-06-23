#pragma once

// 시스템 헤더
#include <d3d11.h>
#include <string>
#include <map>
#include <vector>

// 외부 라이브러리 헤더
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <OpenXLSX.hpp>

// 프로젝트 내부 헤더
#include "Types.h"
#include "ExcelSearchResult.h"


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


// 멤버 변수
public:
    const char* filters{ "엑셀 파일 (*.xlsx){.xlsx},모든 파일{.*}" }; // 파일 다이얼로그 설정

    // 외부 접근이 필요한 데이터
    std::map<std::string, std::string> selectedFiles;        // 파일명, 경로
    std::vector<ExcelSearchResult> searchResults;            // 검색 결과 리스트

private:
    std::string currentKeyword; // 현재 검색중인 키워드
    std::vector<std::pair<std::string, std::string>> searchQueue; // 검색 대기(큐)

// 상태 체크용
public:
    // main에서 isWindowOpen을 리턴받아 종료시점을 알기 위한 함수
    bool IsDone() const;

private:
    // 내부 상태
    bool show_demo_window{ false }; // 데모 윈도우를 보여줄지 여부
    bool isWindowOpen{ true }; // 윈도우창이 켜져 되어있는지 여부
    bool isSearching{ false }; // 현재 검색이 진행중인지 여부



//====================================================================================
//  엑셀 파일 검색 처리
//====================================================================================
private:
    bool StartSearch(const std::string& keyword);
    void SearchInExcelFile(const std::string& keyword, const std::pair<std::string, std::string>& filePair);  // 선택된 엑셀 파일에서 검색
    std::string CopyExcelFile(const std::string& originalPath); // 엑셀 파일 복사
    bool ProcessCell(OpenXLSX::XLWorksheet& sheet, const std::string& fileName, const std::string& sheetName, const std::string& keyword, uint16_t row, uint16_t col);

//====================================================================================
//  Progress Bar
//====================================================================================
private:
    bool showProgressBar{ false };  // 진행바 표시 여부
    int currentFileIndex{ 0 };      // 현재 처리 중인 파일 인덱스
    int totalFilesCount{ 0 };       // 총 파일 수
    float progressValue{ 0.0f };    // 0.0f ~ 1.0f 범위

};
