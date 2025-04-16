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
public:
    // ImGuiManager 라이프 사이클
    void Init() override;
    void Release() override;
    void Update() override;
    void LateUpdate() override;
    void Render() override;
    void ResizeScreen() override;

    // ImGui 초기화
    void SetupImGuiContext(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    void SetupWindowIconsForViewports();   // 뷰포트 생성 시 아이콘 설정용 콜백 등록

    // 상태
    bool IsDone() const;

public:
    // 파일 다이얼로그 설정
    const char* filters{ "엑셀 파일 (*.xlsx){.xlsx},모든 파일 {.*}" };

    // 외부 접근이 필요한 데이터
    std::map<std::string, std::string> selectedFiles;        // 선택된 파일 경로
    std::vector<ExcelSearchResult> searchResults;            // 검색 결과 리스트

private:
    // 내부 상태
    bool show_demo_window{ false };
    bool isWindowOpen{ true };

    // 스타일 및 설정
    void SetupStyle();

    // 엑셀 파일 검색 처리
    void SearchInSelectedFiles(const std::string& keyword);  // 선택된 파일들에서 검색
    std::string CopyExcelFile(const std::string& originalPath); // 엑셀 파일 복사
    bool ProcessCell(OpenXLSX::XLWorksheet& sheet, const std::string& fileName, const std::string& sheetName, const std::string& keyword, uint64_t row, uint16_t col);
};
