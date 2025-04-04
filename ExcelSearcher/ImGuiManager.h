#pragma once

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <d3d11.h>
#include <string>
#include <map>
#include <vector>

#include <OpenXLSX.hpp>

#include "Types.h"
#include "ExcelSearchResult.h"

class ImGuiManager : public Singleton<ImGuiManager>, public Scene
{
public:
    void Init() override;
    void Release() override;
    void Update() override;
    void LateUpdate() override;
    void Render() override;
    void ResizeScreen() override;

private:
    bool show_demo_window{ false };
    bool isWindowOpen{ true };

public:
    void SetupImGuiContext(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

    bool IsDone() const;

private:
    void SetupStyle();  // 스타일 설정 함수
    void SearchInSelectedFiles(const std::string& keyword);  // 선택된 파일들에서 검색
    std::string CopyExcelFile(const std::string& originalPath); // 엑셀 파일 복사
    bool ProcessCell(OpenXLSX::XLWorksheet& sheet, const std::string& fileName, const std::string& sheetName, const std::string& keyword, uint64_t row, uint16_t col);

public:
    const char* filters{ "엑셀 파일 (*.xlsx){.xlsx},모든 파일 {.*}" };  // 파일 다이얼로그 필터
    std::map<std::string, std::string> selectedFiles;  // 선택된 파일들을 저장할 맵

    std::vector<ExcelSearchResult> searchResults;  // 검색 결과 목록
};
