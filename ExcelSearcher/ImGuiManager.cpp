#include "ImGuiManager.h"

#include <ImGuiFileDialog.h>
#include <OpenXLSX.hpp>

#include "SystemUtils.h"
#include <locale>
#include <sstream>



// ImGui 매니저 초기화 함수
void ImGuiManager::Init()
{
    // ImGui 버전 체크 및 컨텍스트 생성
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // IO 설정 객체 가져오기
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // ImGui 설정
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // 키보드 네비게이션 기능 활성화
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // 윈도우 도킹 기능 활성화 (창을 다른 창에 붙이거나 탭으로 병합 가능)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // 다중 뷰포트 기능 활성화 (ImGui 윈도우를 OS의 독립적인 창으로 분리 가능)

    // 기타 인터페이스 설정
    io.ConfigInputTextCursorBlink = true;         // 텍스트 필드에서 커서 깜빡임
    io.ConfigInputTextEnterKeepActive = true;     // Enter 누른 후에도 텍스트 박스 활성 상태로 유지
    io.ConfigViewportsNoAutoMerge = true;         // 도킹 시 자동으로 뷰포트가 병합되지 않도록 설정
    io.ConfigViewportsNoDefaultParent = true;     // 부모 없는 뷰포트 허용
    io.ConfigDockingTransparentPayload = false;   // 도킹 시 투명 드래그 미사용


    // 한글 폰트 로드
    //io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesKorean());
    io.Fonts->AddFontFromFileTTF("..\\Contents\\Fonts\\NotoSansKR-Medium.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesKorean());

    // 아이콘 폰트 병합 및 로드
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;    // 기존 폰트와 병합하여 사용
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("..\\Contents\\Fonts\\FontAwesome-solid-900.otf", 18.0f, &icons_config, icons_ranges);


    // 스타일 설정
    SetupStyle();

    // 파일 다이얼로그 내 Places 그룹 생성 및 설정
    ImGuiFileDialog::Instance()->AddPlacesGroup(placesBookmarksGroupName, 0.5f, true, true);

    // 사용자 정의 경로 그룹 생성
    auto places_ptr = ImGuiFileDialog::Instance()->GetPlacesGroupPtr(placesBookmarksGroupName);
    if (places_ptr != nullptr)
    {
        // 시스템 경로 가져오기
        std::string downloadsPath = SystemUtils::GetDownloads();
        std::string documentsPath = SystemUtils::GetDocuments();
        std::string desktopPath = SystemUtils::GetDesktop();

        // 자주 접근할 폴더 바로가기 추가 (자동 경로 사용)
        if (!desktopPath.empty())
        {
            std::string placeName = std::string(ICON_FA_PERSON_THROUGH_WINDOW) + "  바탕화면";
            places_ptr->AddPlace(placeName.c_str(), desktopPath, true, IGFD::FileStyle(ImVec4(1.0f, 0.6f, 0.2f, 1.0f)));
        }

        if (!downloadsPath.empty())
        {
            std::string placeName = std::string(ICON_FA_DOWNLOAD) + "   다운로드";
            places_ptr->AddPlace(placeName.c_str(), downloadsPath, true, IGFD::FileStyle(ImVec4(0.1f, 0.8f, 0.2f, 1.0f)));
        }

        if (!documentsPath.empty())
        {
            std::string placeName = std::string(ICON_FA_NEWSPAPER) + "   내 문서";
            places_ptr->AddPlace(placeName.c_str(), documentsPath, true, IGFD::FileStyle(ImVec4(0.2f, 0.7f, 1.0f, 1.0f)));
        }


        // 구분선 추가
        places_ptr->AddPlaceSeparator(2.0f);
    }


    // 특정 파일 확장자에 강조색
    {
        // 엑셀
        //ImVec4 ExcelColor(0.1f, 0.8f, 0.2f, 1.0f); // 초록
        ImVec4 ExcelColor(0.5f, 0.9f, 0.5f, 1.0f); // 초록
        ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".xlsx", ExcelColor);

        // 디렉토리 
        ImVec4 DirColor(1.0f, 1.0f, 0.6f, 1.0f); // 노랑
        ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, DirColor);
    }

    // 기본 창 상태 true로 설정
    isWindowOpen = true;

    // 콘솔 출력 로케일 설정 (한글 지원)
    std::wcout.imbue(std::locale(""));
    std::wcerr.imbue(std::locale(""));
}

void ImGuiManager::SetupImGuiContext(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    // 플랫폼/렌더러 백엔드 초기화
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, deviceContext);
}


bool ImGuiManager::IsDone() const
{
    return !isWindowOpen;
}

void ImGuiManager::Release()
{
    // ImGui 셧다운 및 리소스 해제
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiManager::Update()
{
    // 새로운 프레임 시작
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 첫 실행 시 창 크기 설정
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    // 데모 창 보기 (테스트용)
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    static char keywordBuffer[128];  // 검색어 입력 버퍼
    // 메인 창 시작
    ImGui::Begin("ImGui Window", &isWindowOpen);
    {
        // 파일 열기 버튼
        if (ImGui::Button(u8"파일 열기"))
        {
            // 파일 선택기 설정
            IGFD::FileDialogConfig config;
            config.path = ".";              // 기본 경로
            config.countSelectionMax = 0;   // 다중 선택
            ImGuiFileDialog::Instance()->OpenDialog("FileOpenDialog", u8"파일 선택", filters, config);
        }

        // 파일 선택기 표시
        if (ImGuiFileDialog::Instance()->Display("FileOpenDialog"))
        {
            // 파일 선택기에서 선택된 파일 목록 가져오기
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                selectedFiles = ImGuiFileDialog::Instance()->GetSelection();
            }
            ImGuiFileDialog::Instance()->Close();
        }

        // 선택된 파일 디버깅
        if (!selectedFiles.empty())
        {
            ImGui::SameLine();
            ImGui::Text(u8"선택된 파일 개수: %d", selectedFiles.size());

            // 스크롤 가능한 영역 시작
            float lineHeight = ImGui::GetTextLineHeightWithSpacing();
            ImVec2 childSize = ImVec2(0, lineHeight * 10); // 세로 길이


            ImGui::BeginChild("SelectedFilesList", childSize, true, ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& file : selectedFiles)
            {
                const std::string& fileName = file.first;
                const std::string& filePathName = file.second;

                ImGui::BulletText(u8"파일 이름: %s", fileName.c_str());
                ImGui::Text(u8"전체 경로: %s", filePathName.c_str());
                ImGui::Separator();
            }
            ImGui::EndChild(); // 스크롤 영역 종료
        }
        else
        {
            ImGui::SameLine();
            ImGui::Text(u8"현재 선택 파일이 없습니다. 파일을 선택하세요.");
        }

        // 검색어 입력 UI 영역 -----------------------------
        ImGui::Separator();
        // 초기화 버튼
        if (ImGui::Button(ICON_FA_ERASER))
        {
            keywordBuffer[0] = '\0'; // 검색어 초기화
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(u8"검색어 초기화\n입력된 모든 검색어를 지우려면 이 버튼을 누르세요.");
        }
        ImGui::SameLine();

        // 검색어 입력 필드와 힌트 텍스트 처리
        ImGui::PushID("SearchBox");
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##SearchKeyword", keywordBuffer, IM_ARRAYSIZE(keywordBuffer));
        if (keywordBuffer[0] == '\0' && !ImGui::IsItemActive())
        {
            ImGui::GetWindowDrawList()->AddText
            (
                cursorPos,
                ImColor(150, 150, 150, 255),
                u8"검색어를 입력하세요."
            );
        }
        ImGui::PopID();

        // 검색 버튼
        ImGui::SameLine();
        if (ImGui::Button(u8"검색"))
        {
            SearchInSelectedFiles(keywordBuffer);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(u8"검색하기\n입력된 단어를 검색하려면 이 버튼을 누르세요.");
        }
        // -------------------------------------------------

        ImGui::Separator();

        if (!searchResults.empty())
        {
            ImGui::Separator();
            ImGui::Text(u8"검색 결과: %d개 일치", searchResults.size());

            // 표 크기 지정
            ImVec2 resultSize(0, ImGui::GetTextLineHeightWithSpacing() * 15);
            ImGui::BeginChild("SearchResults", resultSize, true, ImGuiWindowFlags_HorizontalScrollbar);

            // ✅ 표 시작 (4열)
            if (ImGui::BeginTable("ResultTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
            {
                // ✅ 헤더 행
// 열 비율 설정
                ImGui::TableSetupColumn(u8"파일명", ImGuiTableColumnFlags_WidthStretch, 3.0f);  // 3/10
                ImGui::TableSetupColumn(u8"시트", ImGuiTableColumnFlags_WidthStretch, 1.5f);  // 1.5/10
                ImGui::TableSetupColumn(u8"위치", ImGuiTableColumnFlags_WidthStretch, 1.5f);  // 1.5/10
                ImGui::TableSetupColumn(u8"내용", ImGuiTableColumnFlags_WidthStretch, 4.0f);  // 4/10
                ImGui::TableHeadersRow();

                // ✅ 결과 반복 출력
                for (const auto& result : searchResults)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", result.fileName.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", result.sheetName.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", result.cellAddress.c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextWrapped("%s", result.cellValue.c_str());
                }

                ImGui::EndTable();
            }

            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void ImGuiManager::LateUpdate()
{
    // 만약 추가적인 업데이트가 필요한 경우, LateUpdate에서 처리
}

void ImGuiManager::Render()
{
    // ImGui의 렌더링 처리
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // 추가 플랫폼 창 렌더링
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void ImGuiManager::ResizeScreen()
{
    // 스크린 크기 변경에 따라 필요한 작업을 처리
}

void ImGuiManager::SetupStyle()
{
    // 스타일 설정
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

void ImGuiManager::SearchInSelectedFiles(const std::string& keyword)
{
    if (keyword.empty())
        return;

    searchResults.clear();
    std::wstring wKeyword = SystemUtils::UTF8ToWString(keyword);
    std::wcout << L"검색어: " << wKeyword << std::endl;

    for (const auto& filePair : selectedFiles)
    {
        const std::string& fileName = filePair.first;
        const std::string& filePath = filePair.second;

        std::wstring wfileName = SystemUtils::UTF8ToWString(fileName);
        std::wstring wfilePath = SystemUtils::UTF8ToWString(filePath);

        std::wcout << L"===========================================" << std::endl;
        std::wcout << L"파일 이름: " << wfileName << std::endl;
        std::wcout << L"전체 경로: " << wfilePath << std::endl;

        std::string safePath = CopyExcelFile(filePath);

        try
        {
            OpenXLSX::XLDocument doc;
            doc.open(safePath);

            int sheetcnt = doc.workbook().sheetCount();
            std::wcout << L"시트 개수: " << sheetcnt << std::endl;

            for (const auto& sheetName : doc.workbook().worksheetNames())
            {
                std::wstring wSheetName = SystemUtils::UTF8ToWString(sheetName);
                std::wcout << L"시트 이름: " << wSheetName;

                OpenXLSX::XLWorksheet sheet = doc.workbook().worksheet(sheetName);
                // rowCount, columnCount() 출력

                // 시트의 행과 열 개수 출력
                std::wcout << L" (" << sheet.rowCount() << L" * " << sheet.columnCount() << L")" << std::endl;

                try
                {

                    if (sheet.rowCount() == 0 || sheet.columnCount() == 0)
                    {
                        std::wcout << L"시트가 비어 있어 건너뜁니다: " << wSheetName << std::endl;
                        break;
                    }

                    OpenXLSX::XLCellRange range = sheet.range();
                    auto first = range.topLeft();
                    auto last = range.bottomRight();

                    for (uint64_t row = first.row(); row <= last.row(); ++row)
                    {
                        for (uint16_t col = first.column(); col <= last.column(); ++col)
                        {
                            ProcessCell(sheet, fileName, sheetName, keyword, row, col);
                        }
                    }
                }
                catch (const std::exception& ex)
                {
                    std::wcout << L"⚠️ range() 실패: " << SystemUtils::UTF8ToWString(ex.what()) << std::endl;
                    std::wcout << L"→ 반복문 fallback (20x30 셀, 연속 100개 비면 스킵)" << std::endl;

                    const uint64_t maxRows = 20;
                    const uint16_t maxCols = 30;
                    const int maxConsecutiveEmptyCells = 100;

                    int emptyCellStreak = 0;

                    for (uint64_t col = 1; col <= maxCols; ++col)
                    {
                        for (uint16_t row = 1; row <= maxRows; ++row)
                        {
                            bool hasValue = ProcessCell(sheet, fileName, sheetName, keyword, row, col);

                            if (!hasValue)
                            {
                                emptyCellStreak++;

                                if (emptyCellStreak >= maxConsecutiveEmptyCells)
                                {
                                    std::wcout << L"📭 연속으로 "
                                        << maxConsecutiveEmptyCells
                                        << L"개의 빈 셀이 탐지되어 시트를 스킵합니다." << std::endl;
                                    goto SkipSheet;
                                }
                            }
                            else
                            {
                                emptyCellStreak = 0;
                            }
                        }
                    }

                SkipSheet:
                    continue;
                }
            }

            doc.close();
        }
        catch (const std::exception& ex)
        {
            std::wcout << L"\n파일: " << wfilePath << std::endl;
            std::wstring wErr = SystemUtils::UTF8ToWString(ex.what());
            std::wcout << L"사유: " << wErr << std::endl;
        }
    }
}




std::string ImGuiManager::CopyExcelFile(const std::string& originalPath)
{
    std::wstring wOriginal = SystemUtils::UTF8ToWString(originalPath);

    wchar_t tempDir[MAX_PATH];
    GetTempPathW(MAX_PATH, tempDir);

    // 임시 파일명 생성 (고유성 부여 가능)
    std::wstring wTempPath = std::wstring(tempDir) + L"temp_excel.xlsx";

    // 복사
    if (!CopyFileW(wOriginal.c_str(), wTempPath.c_str(), FALSE))
    {
        // 복사 실패 시 에러 메시지 출력
        std::wcerr << L"파일 복사 실패: " << wOriginal << std::endl;
        std::wcerr << L"사유: " << GetLastError() << std::endl;

    }

    return SystemUtils::WStringToUTF8(wTempPath);
}

bool ImGuiManager::ProcessCell(OpenXLSX::XLWorksheet& sheet,
    const std::string& fileName,
    const std::string& sheetName,
    const std::string& keyword,
    uint64_t row, uint16_t col)
{
    try
    {
        OpenXLSX::XLCellReference cellRef(row, col);
        auto cell = sheet.cell(cellRef);

        if (cell.empty())
            return false;

        OpenXLSX::XLCellValue value = cell.value();

        if (value.type() == OpenXLSX::XLValueType::Empty)
            return false;

        std::string cellText;

        if (value.type() == OpenXLSX::XLValueType::String)
            cellText = value.get<std::string>();
        else
        {
            std::ostringstream oss;
            oss << value;
            cellText = oss.str();
        }

        if (cellText.find(keyword) != std::string::npos)
        {
            ExcelSearchResult result;
            result.fileName = fileName;
            result.sheetName = sheetName;
            result.cellAddress = cellRef.address();
            result.cellValue = cellText;
            searchResults.push_back(result);
        }

        return true;  // 유효한 값 있는 셀
    }
    catch (...)
    {
        return false;  // 예외 발생 시 무시
    }

    return false;
}
