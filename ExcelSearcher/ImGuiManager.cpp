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
    //io.Fonts->AddFontFromFileTTF("..\\Contents\\Fonts\\NotoSansKR-Medium.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesKorean());
    io.Fonts->AddFontFromFileTTF("Resource/NotoSansKR-Medium.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesKorean());

    // 아이콘 폰트 병합 및 로드
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;    // 기존 폰트와 병합하여 사용
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("Resource/FontAwesome-solid-900.otf", 18.0f, &icons_config, icons_ranges);


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

    // 뷰포트에 아이콘 설정
    SetupWindowIconsForViewports(); 
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
    ImGui::Begin(u8"Excel Searcher", &isWindowOpen);
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

            // 표 시작 (4열)
            if (ImGui::BeginTable("ResultTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
            {
                // 헤더 행
                ImGui::TableSetupColumn(u8"파일명", ImGuiTableColumnFlags_WidthStretch, 3.0f);  // 3/10
                ImGui::TableSetupColumn(u8"시트", ImGuiTableColumnFlags_WidthStretch, 1.5f);  // 1.5/10
                ImGui::TableSetupColumn(u8"위치", ImGuiTableColumnFlags_WidthStretch, 1.5f);  // 1.5/10
                ImGui::TableSetupColumn(u8"내용", ImGuiTableColumnFlags_WidthStretch, 4.0f);  // 4/10
                ImGui::TableHeadersRow();

                // 결과 반복 출력
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

// ==========================================================================
// 선택된 엑셀 파일들에서 특정 키워드를 검색하는 메인 처리 함수
// ==========================================================================
void ImGuiManager::SearchInSelectedFiles(const std::string& keyword)
{
    // 검색어가 없으면 리턴
    if (keyword.empty())
        return;

    // 기존 검색 결과 초기화
    searchResults.clear();

    // 선택된 모든 파일 반복
    std::wstring wKeyword = SystemUtils::UTF8ToWString(keyword);
    std::wcout << L"검색어: " << wKeyword << std::endl;

    for (const auto& filePair : selectedFiles)
    {
        // 파일 이름과 경로 가져오기
        const std::string& fileName = filePair.first;
        const std::string& filePath = filePair.second;

        // 파일 이름과 경로 출력
        std::wstring wfileName = SystemUtils::UTF8ToWString(fileName);
        std::wstring wfilePath = SystemUtils::UTF8ToWString(filePath);
        std::wcout << L"===========================================" << std::endl;
        std::wcout << L"파일 이름: " << wfileName << std::endl;
        std::wcout << L"전체 경로: " << wfilePath << std::endl;

        // 엑셀파일을 임시 디렉토리에 복사
        std::string safePath = CopyExcelFile(filePath);

        try
        {
            OpenXLSX::XLDocument doc;

            // 엑셀 파일 열기
            doc.open(safePath);

            // 시트 개수 출력
            int sheetcnt = doc.workbook().sheetCount();
            std::wcout << L"시트 개수: " << sheetcnt << std::endl;

            // 엑셀 문서 내의 모든 시트 반복
            for (const auto& sheetName : doc.workbook().worksheetNames())
            {
                // 시트 이름 출력
                std::wstring wSheetName = SystemUtils::UTF8ToWString(sheetName);
                std::wcout << L"시트 이름: " << wSheetName;

                // 시트 객체 생성
                OpenXLSX::XLWorksheet sheet = doc.workbook().worksheet(sheetName);

                // 시트의 행과 열의 개수 출력
                std::wcout << L" (" << sheet.rowCount() << L" * " << sheet.columnCount() << L")" << std::endl;

                try
                {
                    // 비어 있는 시트는 건너뜀
                    if (sheet.rowCount() == 0 || sheet.columnCount() == 0)
                    {
                        std::wcout << L"시트가 비어 있어 건너뜁니다: " << wSheetName << std::endl;
                        break;
                    }

                    // 셀의 개수가 비정상적으로 많으면 예외 발생 > fallback으로 대체
                    if (sheet.rowCount() > 1000 || sheet.columnCount() > 1000)
                    {
                        std::wcout << L"셀의 개수가 비정상적으로 많습니다." << std::endl;
                        throw std::runtime_error("too large for range");
                    }

                    // 시트 전체 범위를 가져오기
                    OpenXLSX::XLCellRange range = sheet.range();
                    auto first = range.topLeft();
                    auto last = range.bottomRight();

                    // 셀 순회
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
                    // 예외 발생 시 알림
                    std::wcout << L"range() 실패: " << SystemUtils::UTF8ToWString(ex.what()) << std::endl;

                    // fallback 처리
                    // 20 * 30 셀을 순회하며 검색, 100개 연속 빈 셀 탐지 시 스킵
                    const uint64_t maxRows = 20;
                    const uint16_t maxCols = 30;
                    const int maxConsecutiveEmptyCells = 100;

                    // 빈 셀 카운트를 저장할 변수
                    int emptyCellStreak = 0;

                    // 셀 순회
                    for (uint64_t col = 1; col <= maxCols; ++col)
                    {
                        for (uint16_t row = 1; row <= maxRows; ++row)
                        {
                            bool hasValue = ProcessCell(sheet, fileName, sheetName, keyword, row, col);

                            // 빈 셀이면
                            if (!hasValue)
                            {
                                // 빈 셀 카운트 증가
                                emptyCellStreak++;

                                // 연속 빈 셀 카운트가 최대값에 도달하면 스킵
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
                                // 유효셀을 찾았으면 빈 셀 카운트 초기화
                                emptyCellStreak = 0;
                            }
                        }
                    }

                SkipSheet:
                    continue;
                }
            }

            // 엑셀 문서 닫기
            doc.close();
        }
        catch (const std::exception& ex)
        {
            // 예외 발생 시 알림
            std::wcout << L"\n파일: " << wfilePath << std::endl;
            std::wstring wErr = SystemUtils::UTF8ToWString(ex.what());
            std::wcout << L"사유: " << wErr << std::endl;
        }

    }
}

// ==========================================================================
// 특정 시트에서 한 개의 셀(row, col)을 검사하고, 유효하면 keyword 포함 여부 확인
// ==========================================================================
bool ImGuiManager::ProcessCell(OpenXLSX::XLWorksheet& sheet,
    const std::string& fileName,
    const std::string& sheetName,
    const std::string& keyword,
    uint64_t row, uint16_t col)
{
    try
    {
        OpenXLSX::XLCellReference cellRef(row, col);
        // 셀 참조 가져오기
        auto cell = sheet.cell(cellRef);

        // 셀 비었으면 false 리턴
        if (cell.empty())
            return false;

        // 셀 값 가져오기
        OpenXLSX::XLCellValue value = cell.value();

        // 셀 value가 비어있으면 false 리턴
        if (value.type() == OpenXLSX::XLValueType::Empty)
            return false;

        std::string cellText;
        // 셀 값이 문자열이면
        if (value.type() == OpenXLSX::XLValueType::String)
        {
            // 안정적으로 문자열 가져옴
            cellText = value.get<std::string>();
        }
        else // 그 외의 type은
        {
            // 문자열로 변환하여 저장
            std::ostringstream oss;
            oss << value;
            cellText = oss.str();
        }

        // 키워드 포함 여부 확인
        if (cellText.find(keyword) != std::string::npos)
        {
            // 검색 결과 저장
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


// ==========================================================================
// 다중 뷰포트(독립 OS 창) 생성 시, 각 창에 아이콘을 설정하기 위한 콜백 함수 등록
// ImGui가 창을 생성할 때 Platform_CreateWindow를 호출하므로, 이를 오버라이드함
// ==========================================================================
void ImGuiManager::SetupWindowIconsForViewports()
{
    static auto original_create_window = ImGui::GetPlatformIO().Platform_CreateWindow;

    ImGui::GetPlatformIO().Platform_CreateWindow = [](ImGuiViewport* viewport)
        {
            // 기존 창 생성 콜백 호출
            if (original_create_window)
            {
                original_create_window(viewport);
            }

            // 생성된 창 핸들에서 아이콘 설정
            if (viewport->PlatformHandleRaw)
            {
                HWND hwnd = (HWND)viewport->PlatformHandleRaw;

                // 아이콘 파일을 로드하여 창에 설정
                HICON hIcon = (HICON)LoadImageW
                (
                    nullptr,
                    L"Resource/icon.ico",
                    IMAGE_ICON,
                    0, 0,
                    LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED
                );

                if (hIcon)
                {
                    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
                    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
                }
                else
                {
                    DWORD err = GetLastError();

                    std::wcerr << L"아이콘 로드 실패. 오류 코드: " << err << std::endl;
                }
            }
        };
}

// ==========================================================================
// OpenXLSX는 UTF-8 한글 경로를 직접 지원하지 않기 때문에,
// 엑셀 파일을 임시 경로에 복사한 후, 해당 파일을 열도록 우회 처리
// ==========================================================================
std::string ImGuiManager::CopyExcelFile(const std::string& originalPath)
{
    std::wstring wOriginal = SystemUtils::UTF8ToWString(originalPath);

    // 임시 디렉토리 경로 가져오기
    wchar_t tempDir[MAX_PATH];
    GetTempPathW(MAX_PATH, tempDir);

    // 임시 파일명 생성
    std::wstring wTempPath = std::wstring(tempDir) + L"temp_excel.xlsx";

    // 파일 복사 시도
    if (!CopyFileW(wOriginal.c_str(), wTempPath.c_str(), FALSE))
    {
        //실패 시 에러 메시지 출력
        std::wcerr << L"파일 복사 실패: " << wOriginal << std::endl;
        std::wcerr << L"사유: " << GetLastError() << std::endl;
    }

    return SystemUtils::WStringToUTF8(wTempPath);
}
