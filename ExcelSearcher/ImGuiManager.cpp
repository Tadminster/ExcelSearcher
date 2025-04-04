#include "ImGuiManager.h"

#include <ImGuiFileDialog.h>
#include <OpenXLSX.hpp>

#include "SystemPaths.h"
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
        std::string downloadsPath = SystemPaths::GetDownloads();
        std::string documentsPath = SystemPaths::GetDocuments();
        std::string desktopPath = SystemPaths::GetDesktop();

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

            ImVec2 resultSize(0, ImGui::GetTextLineHeightWithSpacing() * 12);
            ImGui::BeginChild("SearchResults", resultSize, true, ImGuiWindowFlags_HorizontalScrollbar);

            for (const auto& result : searchResults)
            {
                ImGui::Text(u8"파일: %s", result.fileName.c_str());
                ImGui::Text(u8"시트: %s", result.sheetName.c_str());
                ImGui::Text(u8"셀 위치: %s", result.cellAddress.c_str());
                ImGui::Text(u8"내용: %s", result.cellValue.c_str());
                ImGui::Separator();
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

// 선택된 파일들에서 keyword를 검색하는 함수
void ImGuiManager::SearchInSelectedFiles(const std::string& keyword)
{
    // 검색어가 비어 있다면 아무 작업도 하지 않음
    if (keyword.empty())
        return;

    // 이전 검색 결과를 초기화
    searchResults.clear();

    // 검색어 출력
    std::wcout << L"검색어: " << std::wstring(keyword.begin(), keyword.end()) << std::endl;

    // 선택된 모든 파일을 순회
    for (const auto& filePair : selectedFiles)
    {
        const std::string& fileName = filePair.first;   // 파일 이름
        const std::string& filePath = filePair.second;  // 전체 경로

        // 파일 경로와 이름 출력
        std::wcout << L"파일 이름: " << std::wstring(fileName.begin(), fileName.end()) << std::endl;
        std::wcout << L"전체 경로: " << std::wstring(filePath.begin(), filePath.end()) << std::endl;

        try
        {
            // OpenXLSX를 사용하여 엑셀 파일 열기
            OpenXLSX::XLDocument doc;
            doc.open(filePath);

            // 시트 개수 출력
            int sheetcnt = doc.workbook().sheetCount();
            std::wcout << L"시트 개수: " << sheetcnt << std::endl;

            // 워크북에 존재하는 모든 시트를 순회
            for (const auto& sheetName : doc.workbook().worksheetNames())
            {
                // 시트 이름 출력
                std::wcout << L"시트 이름: " << std::wstring(sheetName.begin(), sheetName.end()) << std::endl;

                // 해당 시트를 가져옴
                OpenXLSX::XLWorksheet sheet = doc.workbook().worksheet(sheetName);

                // 시트 전체 범위를 가져옴
                OpenXLSX::XLCellRange range = sheet.range();
                OpenXLSX::XLCellReference firstCell = range.topLeft();      // 시작 셀 (예: A1)
                OpenXLSX::XLCellReference lastCell = range.bottomRight();   // 끝 셀 (예: E20)

                // 모든 행(row)을 순회
                for (uint64_t row = firstCell.row(); row <= lastCell.row(); ++row)
                {
                    // 모든 열(column)을 순회
                    for (uint16_t col = firstCell.column(); col <= lastCell.column(); ++col)
                    {
                        // 셀 참조를 생성 (예: A1, B2 등)
                        OpenXLSX::XLCellReference cellRef(row, col);

                        // 해당 셀을 가져옴
                        auto cell = sheet.cell(cellRef);

                        // 셀에 값이 있는 경우
                        if (!cell.empty())
                        {
                            // 셀의 값을 가져옴
                            OpenXLSX::XLCellValue value = cell.value();

                            // 셀의 값 타입에 따라 처리
                            switch (value.type())
                            {
                                // 셀의 값이 문자열인 경우
                            case OpenXLSX::XLValueType::String:
                            {
                                std::string cellText = value.get<std::string>();
                                // 검색어 포함 검사
                                if (cellText.find(keyword) != std::string::npos)
                                {
                                    ExcelSearchResult result;
                                    result.fileName = fileName;
                                    result.sheetName = sheetName;
                                    result.cellAddress = cellRef.address();
                                    result.cellValue = cellText;
                                    searchResults.push_back(result);
                                }
                            }
                            break;

                            // 셀의 값이 숫자, 부동소수점, 불리언인 경우
                            case OpenXLSX::XLValueType::Integer:
                            case OpenXLSX::XLValueType::Float:
                            case OpenXLSX::XLValueType::Boolean:
                            {
                                std::ostringstream oss;
                                oss << value;  // OpenXLSX는 ostream 연산자 << 를 오버로드했음

                                std::string cellText = oss.str();
                                if (cellText.find(keyword) != std::string::npos)
                                {
                                    ExcelSearchResult result;
                                    result.fileName = fileName;
                                    result.sheetName = sheetName;
                                    result.cellAddress = cellRef.address();
                                    result.cellValue = cellText;
                                    searchResults.push_back(result);
                                }
                            }
                            case OpenXLSX::XLValueType::Empty:
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }
            }

            // 엑셀 문서 닫기
            doc.close();
        }
        catch (const std::exception& ex)
        {
            // 파일 처리 중 에러 발생 시 콘솔에 출력
            std::wcout << L"파일 처리 오류: " << std::wstring(filePath.begin(), filePath.end()) << std::endl;
            std::wcout << L"사유: " << std::wstring(ex.what(), ex.what() + strlen(ex.what())) << std::endl;
        }
    }
}
