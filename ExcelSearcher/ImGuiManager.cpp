#include "ImGuiManager.h"

#include <locale>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>

#include <ImGuiFileDialog.h>
#include <OpenXLSX.hpp>
#include <filesystem>

#include "SystemUtils.h"

std::unordered_map<std::string, FSheetMetaState> MetaStatePerSheet; // key: file|sheet

// ImGui 매니저 초기화 함수
void ImGuiManager::Init()
{
    // 콘솔 출력 로케일 설정 (한글 지원)
    std::wcout.imbue(std::locale(""));
    std::wcerr.imbue(std::locale(""));

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
    if (places_ptr)
    {
        // 시스템 경로 가져오기
        std::string downloadsPath = SystemUtils::GetDownloads();
        std::string documentsPath = SystemUtils::GetDocuments();
        std::string desktopPath = SystemUtils::GetDesktop();

        // 자주 접근할 폴더 바로가기 추가
        if (!desktopPath.empty())
        {
            std::string placeName = std::string(ICON_FA_PERSON_THROUGH_WINDOW) + "  바탕화면";
            places_ptr->AddPlace(placeName.c_str(), desktopPath, false, IGFD::FileStyle(ImVec4(1.0f, 0.6f, 0.2f, 1.0f)));
        }

        if (!downloadsPath.empty())
        {
            std::string placeName = std::string(ICON_FA_DOWNLOAD) + "   다운로드";
            places_ptr->AddPlace(placeName.c_str(), downloadsPath, false, IGFD::FileStyle(ImVec4(0.1f, 0.8f, 0.2f, 1.0f)));
        }

        if (!documentsPath.empty())
        {
            std::string placeName = std::string(ICON_FA_NEWSPAPER) + "   내 문서";
            places_ptr->AddPlace(placeName.c_str(), documentsPath, false, IGFD::FileStyle(ImVec4(0.2f, 0.7f, 1.0f, 1.0f)));
        }

        places_ptr->AddPlaceSeparator(2.0f); // 구분선 추가
    }

    // 즐겨찾기 불러오기
    std::ifstream ifs("bookmarks.dat", std::ios::binary);
    if (ifs)
    {
        std::stringstream data;
        data << ifs.rdbuf();
        ImGuiFileDialog::Instance()->DeserializePlaces(data.str());
    }

    // 검색 옵션 불러오기
    if (SearchOptions.LoadFromFile())
    {
        // 로드 성공 시 현재 뷰 상태 캐시 갱신
        UpdateCachedView(SearchOptions);
    }
    else
    {
        // 로드 실패 시 기본값으로 초기화
        CachedView = FViewState();
    }


    // 특정 파일 확장자 강조색
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
    bIsWindowOpen = true;


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
    return !bIsWindowOpen;
}

void ImGuiManager::Release()
{
    // 즐겨찾기 저장
    std::ofstream ofs("bookmarks.dat", std::ios::binary);
    if (ofs)
    {
        std::string serialized = ImGuiFileDialog::Instance()->SerializePlaces();

        ofs.write(serialized.c_str(), serialized.size());
    }

    // 검색 옵션 저장
    if (!SearchOptions.SaveToFile())
    {

    }

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
    if (bShowDemoWindow)
        ImGui::ShowDemoWindow(&bShowDemoWindow);

    // 검색 중이고 아직 처리할 파일이 남아 있다면 업데이트마다 검색되도록 처리
    if (bIsSearching)
    {
        // searchQueue에서 이번에 검색할 파일명과 경로를 가져옴
        const auto& filePair = searchQueue[currentFileIndex - 1];
        // 해당 엑셀 파일 내에서 키워드를 검색
        SearchInExcelFile(currentKeyword, filePair);

        // 처리된 파일 수 증가
        currentFileIndex++;
        // 진행도 계산 (0.0f ~ 1.0f 사이)
        progressValue = static_cast<float>(currentFileIndex) / totalFilesCount;
        progressValue = std::clamp(progressValue, 0.0f, 1.0f);

        // 모든 파일 검색 완료 시 검색 종료
        if (currentFileIndex > totalFilesCount)
        {
            bIsSearching = false;
            bHasSearched = true;
        }
    }

    static char keywordBuffer[128];  // 검색어 입력 버퍼
    // 메인 창 시작
    ImGui::Begin(u8"Excel Searcher", &bIsWindowOpen);
    {
        //------------------------------------------------------------------------
        // file open button & dialog
        //------------------------------------------------------------------------
        if (ImGui::Button(u8"파일 열기"))
        {
            // 파일 선택기 설정
            IGFD::FileDialogConfig config;
            config.path = ".";              // 기본 경로
            config.countSelectionMax = 0;   // 다중 선택
            ImGuiFileDialog::Instance()->OpenDialog("FileOpenDialog", u8"파일 선택", filters, config);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(u8"파일 열기\n검색할 엑셀 파일을 선택하려면 이 버튼을 누르세요.");
        }

        // 파일 선택기 표시
        if (ImGuiFileDialog::Instance()->Display("FileOpenDialog"))
        {
            // 파일 선택기에서 선택된 파일 목록 가져오기
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                // 현재 선택된 모든 파일
                auto allSelected = ImGuiFileDialog::Instance()->GetSelection();

                // 기존에 선택된 파일 목록 초기화
                selectedFiles.clear();

                // 현재 필터 확인
                std::wstring currentFilter = SystemUtils::UTF8ToWString(ImGuiFileDialog::Instance()->GetCurrentFilter());

                // 사용자가 모든 파일(.*) 필터를 선택한 경우
                if (currentFilter == L"모든 파일")
                {
                    // 선택된 파일 중 엑셀 파일만 필터링
                    for (const auto& pair : allSelected)
                    {
                        const std::string& filePath = pair.second;
                        if (filePath.size() >= 5)
                        {
                            std::string extension = filePath.substr(filePath.size() - 5);
                            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                            if (extension == ".xlsx")
                            {
                                selectedFiles.insert(pair);
                            }
                        }
                    }
                }
                // 그 외엔 모든 파일을 선택
                else
                {
                    selectedFiles = allSelected;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        //------------------------------------------------------------------------
        // selected files debug info
        //------------------------------------------------------------------------
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

        ImGui::Separator();

        //------------------------------------------------------------------------
        // search field erase button
        //------------------------------------------------------------------------
        if (ImGui::Button(ICON_FA_ERASER))
        {
            keywordBuffer[0] = '\0'; // 검색어 초기화
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(u8"검색어 초기화\n입력된 모든 검색어를 지우려면 이 버튼을 누르세요.");
        }
        ImGui::SameLine();

        //------------------------------------------------------------------------
        // search field ui & hint text
        //------------------------------------------------------------------------
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
                u8" 검색어를 입력하세요."
            );
        }
        ImGui::PopID();

        ImGui::SameLine();

        //------------------------------------------------------------------------
        // search button & options
        //------------------------------------------------------------------------
        if (ImGui::Button(u8"검색"))
        {
            // 검색 옵션 캐시 갱신
            UpdateCachedView(SearchOptions);

            // 선택된 파일 개수 초기화
            totalFilesCount = selectedFiles.size();
            if (totalFilesCount < 1)
            {
                showProgressBar = false;
                totalFilesCount = 0;
                currentFileIndex = 0;
            }
            else if (StartSearch(keywordBuffer))
            {
                // 디버그용 (검색시작 알림, 키워드 버퍼 알림)
                std::wstring wKeyword = SystemUtils::UTF8ToWString(keywordBuffer);
                if (SearchOptions.Get().bShowDebugOverlay)
                {
                    DebugLog.Add(L"검색을 시작합니다.", LogLevel::Info);
                    DebugLog.AddFmt(LogLevel::Info, L"검색어: %s", wKeyword.c_str());

                    // Options 정보 출력
                    DebugLog.AddFmt(LogLevel::Info, L"기본 검색 동작: 대소문자 구분(%s), 정규식(%s), 전체 단어(%s), 미리보기(%s)",
                        SearchOptions.Get().bCaseSensitive ? L"T" : L"F",
                        SearchOptions.Get().bUseRegex ? L"T" : L"F",
                        SearchOptions.Get().bWholeWord ? L"T" : L"F",
                        SearchOptions.Get().bEnablePreview ? L"T" : L"F");

                    DebugLog.AddFmt(LogLevel::Info, L"추가 정보: 미리보기(%s), 파일 이름(%s), 시트 이름(%s), 셀 주소(%s), 스니펫(%s)",
                        SearchOptions.Get().bEnablePreview ? L"T" : L"F",
                        SearchOptions.Get().bShowFileName ? L"T" : L"F",
                        SearchOptions.Get().bShowSheetName ? L"T" : L"F",
                        SearchOptions.Get().bShowCellAddress ? L"T" : L"F",
                        SearchOptions.Get().bShowSnippet ? L"T" : L"F");

                    if (SearchOptions.Get().bUseCustomMeta)
                    {
                        std::wstring metas = L"";
                        for (const auto& meta : SearchOptions.Get().CustomMetadatas)
                        {
                            metas += SystemUtils::UTF8ToWString(meta) + L", ";
                        }
                        if (!metas.empty())
                        {
                            metas = metas.substr(0, metas.size() - 2); // 마지막 ", " 제거
                        }
                        DebugLog.AddFmt(LogLevel::Info, L"커스텀 메타데이터: T(%s)", metas.c_str());
                    }
                    else
                    {
                        DebugLog.AddFmt(LogLevel::Info, L"커스텀 메타데이터: F");
                    }
                }
            }
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(u8"검색하기\n입력된 단어를 검색하려면 이 버튼을 누르세요.");
        }

        // 검색옵션
        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_GEAR))
        {
            SearchOptions.Open();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(u8"검색 옵션 설정\n검색 옵션을 설정하려면 이 버튼을 누르세요.");
        }

        SearchOptions.Draw();

        //------------------------------------------------------------------------
        // progress bar
        //------------------------------------------------------------------------
        if (showProgressBar)
        {
            ImGui::SameLine();
            ImGui::Text("%d / %d", currentFileIndex - 1, totalFilesCount);
            ImGui::SameLine();
            ImGui::ProgressBar(progressValue, ImVec2(-1, 0));
        }

        ImGui::Separator();

        //------------------------------------------------------------------------
        // search result table
        //------------------------------------------------------------------------
        // 검색 결과가 있으면
        if (!searchResults.empty())
        {
            // 결과 테이블 그리기
            DrawResultsTable();
        }
        // 검색 결과가 없고, 검색이 끝났다면 
        else if (bHasSearched)
        {
            // 결과 없음 메시지
            ImGui::Separator();
            ImGui::Text(u8"검색 결과: 0개 일치");
        }


    }
    ImGui::End();

    //------------------------------------------------------------------------
    // debug log overlay
    //------------------------------------------------------------------------
    bool show = SearchOptions.Get().bShowDebugOverlay;
    if (show)
    {
        // 디버그 로그 창 그리기
        DebugLog.DrawWindow(&show);

        // x 버튼이 눌려서 창을 닫았을 때
        if (show != SearchOptions.Get().bShowDebugOverlay)
        {
            auto cur = SearchOptions.Get();   // 현재 옵션 복사
            cur.bShowDebugOverlay = show;       // false로 내려옴(창에서 X를 눌렀을 때 등)
            SearchOptions.Set(cur);           // ✅ 단일 진입로로 적용(캐시/동기화 포함)
        }
    }
}

void ImGuiManager::LateUpdate()
{

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

}

void ImGuiManager::SetupStyle()
{
    // 스타일 설정
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGuiStyle& style = ImGui::GetStyle();

    // 커스텀 컬러 설정
    ImVec4* colors = style.Colors;

    // 기본
    /*기본*/colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);         // 어두운 회색
    /*호버*/colors[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.50f, 0.90f, 1.0f);  // 밝은 파랑
    /*활성*/colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.40f, 0.85f, 1.0f);   // 진한 파랑
    // 포커스 잃은 탭들
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.30f, 0.55f, 1.0f);

    // 테두리/라운딩으로 시각적 구분 더 강화
    style.TabRounding = 6.0f;   // 탭 코너 둥글게
    style.TabBorderSize = 1.0f;   // 탭 경계선 두께
    style.FrameBorderSize = 1.0f;   // 프레임 경계선(탭 바 아래 라인 포함)
    //colors[ImGuiCol_Border] = ImVec4(0.18f, 0.45f, 0.90f, 0.80f); // 탭 테두리 색


    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

void ImGuiManager::DrawResultsTable()
{
    ImGui::Separator();
    ImGui::Text(u8"검색 결과: %d개 일치", searchResults.size());

    // 같은 라인에서 오른쪽 정렬
    float buttonWidth = 40.0f;
    float rightX = ImGui::GetContentRegionAvail().x - buttonWidth;
    ImGui::SameLine(rightX > 0 ? rightX : 0);

    // 검색이 모두 완료되었다면
    if (!bIsSearching && bHasSearched)
    {
        // 결과 내보내기 버튼
        if (ImGui::Button(ICON_FA_FLOPPY_DISK, ImVec2(buttonWidth, 0)))
        {
            ExportSearchResultsOpenDialog(searchResults);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(u8"검색 결과 내보내기\n검색 결과를 Excel 파일로 저장합니다.");
        }
    }


    // 컬럼이 하나도 없으면 메시지 출력 후 종료
    if (CachedView.ColumnCount <= 0)
    {
        ImGui::TextUnformatted(u8"표시할 컬럼이 없습니다. 검색 옵션에서 최소 1개 이상 활성화하세요.");

        return;
    }

    // 검색 옵션 Cache
    const auto& Opt = SearchOptions.Get();

    // 표 크기 지정
    ImVec2 resultSize(0, ImGui::GetTextLineHeightWithSpacing() * 15);
    ImGui::BeginChild("SearchResults", resultSize, true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    ImGuiTableFlags TableFlags =
        ImGuiTableFlags_ScrollY |          // 세로 스크롤
        ImGuiTableFlags_RowBg |            // 짝/홀수 줄 배경
        ImGuiTableFlags_Borders |          // 전체 경계선
        ImGuiTableFlags_Resizable |        // 컬럼 리사이즈
        ImGuiTableFlags_SizingStretchProp; // 컬럼 비율 배치

    // 열 수(column count) 계산
    int BaseColumnCount = CachedView.ColumnCount;
    int MetaColumnCount = (CachedView.bUseCustomMeta ? (int)Opt.CustomMetadatas.size() : 0);
    int TotalColumnCount = BaseColumnCount + MetaColumnCount;

    // 표 시작
    if (ImGui::BeginTable("ResultTable", TotalColumnCount, TableFlags))
    {
        // 헤더 고정(스크롤 시 상단에 고정)
        ImGui::TableSetupScrollFreeze(0, 1);

        // 헤더 정의 (활성화된 옵션에 따라 컬럼 추가)
        if (CachedView.bEnablePreview) ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        if (CachedView.bShowFilePath) ImGui::TableSetupColumn(u8"파일명", ImGuiTableColumnFlags_WidthStretch);
        if (CachedView.bShowSheetName) ImGui::TableSetupColumn(u8"시트", ImGuiTableColumnFlags_WidthFixed);
        if (CachedView.bShowCellAddress) ImGui::TableSetupColumn(u8"위치", ImGuiTableColumnFlags_WidthFixed);
        if (CachedView.bShowSnippet) ImGui::TableSetupColumn(u8"내용", ImGuiTableColumnFlags_WidthStretch);

        // 커스텀 메타데이터 컬럼 추가
        if (CachedView.bUseCustomMeta)
        {
            for (const auto& label : Opt.CustomMetadatas)
            {
                ImGui::TableSetupColumn(label.c_str(), ImGuiTableColumnFlags_WidthFixed);
            }
        }

        ImGui::TableHeadersRow();

        // ----------------------------------- Cripper 시작 -----------------------------------
        // 많은 행이 있을 때 성능 최적화
        const int rowCount = (int)searchResults.size();
        ImGuiListClipper clipper;
        clipper.Begin(rowCount);

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                const auto& result = searchResults[row];

                // (행 높이 힌트: 내용이 대부분 1줄이면 켜면 좋음)
                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeightWithSpacing());

                // 1열: 미리보기
                if (CachedView.bEnablePreview)
                {
                    ImGui::TableNextColumn();

                    ImGui::PushID(&result); // 행 고유 ID

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 2));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.80f, 1));
                    const bool open = ImGui::SmallButton(ICON_FA_EYE);
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();

                    if (open) ImGui::OpenPopup("row_preview");

                    if (ImGui::BeginPopup("row_preview"))
                    {
                        // 메타 요약
                        ImGui::Text("%s | %s | %s",
                            result.FileName.c_str(),
                            result.SheetName.c_str(),
                            result.CellAddress.c_str());
                        ImGui::Separator();

                        // 스크롤 가능한 본문
                        ImGui::BeginChild("row_tip", ImVec2(800, 100), true,
                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_HorizontalScrollbar);

                        if (result.FullRowData.empty())
                        {
                            ImGui::TextUnformatted(u8"(표시할 값이 없습니다)");
                        }
                        else
                        {
                            // 값 하나당 최대 가로폭
                            static const float maxCellWidth = 400.0f;

                            bool first = true;
                            for (const auto& text : result.FullRowData)
                            {
                                if (text.empty()) continue;

                                if (!first)
                                {
                                    ImGui::SameLine(0.0f, 8.0f);
                                    ImGui::TextDisabled("|");
                                    ImGui::SameLine(0.0f, 8.0f);
                                }
                                first = false;

                                const float startX = ImGui::GetCursorPosX();
                                ImGui::PushTextWrapPos(startX + maxCellWidth);
                                ImGui::TextUnformatted(text.c_str());
                                ImGui::PopTextWrapPos();
                            }
                        }

                        ImGui::EndChild();

                        // 복사 버튼
                        if (ImGui::Button(u8"복사"))
                        {
                            std::string buf; buf.reserve(4096);
                            for (const auto& v : result.FullRowData)
                            {
                                if (!v.empty())
                                {
                                    buf += v;
                                    buf += '\n';
                                }
                            }
                            ImGui::SetClipboardText(buf.c_str());
                        }
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip(u8"클립보드에 복사합니다.");

                        ImGui::SameLine();

                        // 닫기 버튼
                        if (ImGui::Button(u8"닫기"))
                            ImGui::CloseCurrentPopup();
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip(u8"팝업 닫기");

                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                }

                // 2열: 파일명
                if (CachedView.bShowFilePath)
                {
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(result.FileName.c_str());
                }

                // 3열: 시트명
                if (CachedView.bShowSheetName)
                {
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(result.SheetName.c_str());
                }

                // 4열: 위치(셀 주소)
                if (CachedView.bShowCellAddress)
                {
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(result.CellAddress.c_str());
                }

                // 5열: 내용
                if (CachedView.bShowSnippet)
                {
                    ImGui::TableNextColumn();
                    // 래핑을 쓰면 행 높이가 가변
                    ImGui::TextWrapped("%s", result.CellValue.c_str());
                }

                // 커스텀 메타데이터 컬럼
                if (MetaColumnCount > 0)
                {
                    // 라벨 불일치(대소문자/공백 차이) 대비: 느슨한 조회 람다
                    auto findMeta = [&](const std::string& wantLabel) -> const char*
                        {
                            // 1) 완전 일치 우선
                            auto it = result.CustomMetadata.find(wantLabel);
                            if (it != result.CustomMetadata.end()) return it->second.c_str();

                            // 2) 대소문자만 무시(Trim 금지 정책 유지)
                            const std::string wantLower = SystemUtils::ToLower(wantLabel);
                            for (const auto& kv : result.CustomMetadata)
                            {
                                if (SystemUtils::ToLower(kv.first) == wantLower)
                                    return kv.second.c_str();
                            }
                            return "";
                        };

                    for (const auto& label : Opt.CustomMetadatas)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(findMeta(label));
                    }
                }
            }
        }
        // ----------------------------------- Cripper 종료 -----------------------------------

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ExportSearchResultsDrawModal();
}

void ImGuiManager::UpdateCachedView(SearchOptionsWindow& InSearchOptions)
{
    const auto& Opt = InSearchOptions.Get();

    CachedView.bEnablePreview = Opt.bEnablePreview;
    CachedView.bShowFilePath = Opt.bShowFileName;
    CachedView.bShowSheetName = Opt.bShowSheetName;
    CachedView.bShowCellAddress = Opt.bShowCellAddress;
    CachedView.bShowSnippet = Opt.bShowSnippet;
    CachedView.bUseCustomMeta = Opt.bUseCustomMeta;

    CachedView.ColumnCount = SearchOptions.GetMetaColumnCount();
}

bool ImGuiManager::StartSearch(const std::string& keyword)
{
    // 키워드가 없거나, 선택된 파일이 없으면 return
    if (keyword.empty() || selectedFiles.empty())
        return false;

    searchResults.clear();      // 이전 검색 결과 초기화
    MetaStatePerSheet.clear();  // 이전 커스텀 메타데이터 상태 초기화
    totalFilesCount = selectedFiles.size();
    currentFileIndex = 1;
    progressValue = 0;
    currentKeyword = keyword;
    showProgressBar = true;
    bIsSearching = true;

    // 이전 검색큐를 비우고
    searchQueue.clear();
    // 선택된 파일을 순차 접근 가능한 벡터에 복사
    for (const auto& pair : selectedFiles)
        searchQueue.emplace_back(pair);

    return true;
}

// ==========================================================================
// 엑셀파일 내 특정 키워드를 검색하는 메인 처리 함수
// ==========================================================================
void ImGuiManager::SearchInExcelFile(const std::string& keyword, const std::pair<std::string, std::string>& filePair)
{
    // 검색어가 없으면 리턴
    if (keyword.empty())
        return;

    // 파일 이름과 경로 가져오기
    const std::string& fileName = filePair.first;
    const std::string& filePath = filePair.second;

    // 파일 이름과 경로를 wstring으로 변환 (debug 출력용)
    std::wstring wfileName = SystemUtils::UTF8ToWString(fileName);
    std::wstring wfilePath = SystemUtils::UTF8ToWString(filePath);

    // 디버그용 검색옵션 bool 가져오기
    bool bShowDebugLog = SearchOptions.Get().bShowDebugOverlay;

    if (bShowDebugLog)
    {
        // File Info
        DebugLog.Add(L"===========================================", LogLevel::Separator);
        DebugLog.Add(L"파일 이름: " + wfileName, LogLevel::File);
        DebugLog.Add(L"전체 경로: " + wfilePath, LogLevel::File);
    }

    // 엑셀파일을 임시 디렉토리에 복사
    std::string safePath = CopyExcelFile(filePath);

    try
    {
        OpenXLSX::XLDocument doc;

        // 엑셀 파일 열기
        doc.open(safePath);

        // 시트 개수 출력
        if (bShowDebugLog)
        {
            int sheetcnt = doc.workbook().sheetCount();
            DebugLog.AddFmt(LogLevel::File, L"시트 개수: %d", sheetcnt);
        }

        // 엑셀 문서 내의 모든 시트 반복
        for (const auto& sheetName : doc.workbook().worksheetNames())
        {
            // 시트 객체 생성
            OpenXLSX::XLWorksheet sheet = doc.workbook().worksheet(sheetName);
            std::wstring wSheetName = SystemUtils::UTF8ToWString(sheetName);

            // 커스텀 메타데이터 상태 초기화
            auto& st = MetaStatePerSheet[SheetKey(fileName, sheetName)];
            if (st.Pending.empty() && st.Found.empty())
            {
                st = BuildMetaState(SearchOptions.Get());
            }

            // 시트 이름/행열 개수 출력
            if (bShowDebugLog)
            {
                DebugLog.AddFmt(LogLevel::Sheet, L"시트: %ls  [rows %d] * [cols %d]",
                    wSheetName.c_str(), sheet.rowCount(), sheet.columnCount());
            }

            try
            {
                // 비어 있는 시트는 건너뜀
                if (sheet.rowCount() == 0 || sheet.columnCount() == 0)
                {
                    if (bShowDebugLog)
                    {
                        DebugLog.Add(L"해당 시트는 비어 있어 건너뜁니다: " + wSheetName, LogLevel::Error);
                    }

                    continue;
                }

                // 셀의 개수가 비정상적으로 많으면 예외 발생 > fallback으로 대체
                if (sheet.rowCount() > 2000 || sheet.columnCount() > 2000)
                {
                    if (bShowDebugLog)
                    {
                        DebugLog.Add(L"해당 시트는 셀의 개수가 비정상적으로 많아 건너뜁니다: " + wSheetName, LogLevel::Error);
                    }

                    //throw std::runtime_error("too large for range");
                    continue;
                }

                // 시트 전체 범위를 가져오기
                OpenXLSX::XLCellRange range = sheet.range();
                auto first = range.topLeft();
                auto last = range.bottomRight();

                // 셀 순회
                for (uint16_t row = first.row(); row <= last.row(); ++row)
                {
                    for (uint16_t col = first.column(); col <= last.column(); ++col)
                    {
                        ProcessCell(sheet, fileName, sheetName, keyword, row, col);
                    }
                }
            }
            catch (const std::exception& ex)
            {
                // 예외 발생 알림
                std::wcout << L"range() 실패: " << SystemUtils::UTF8ToWString(ex.what()) << std::endl;

                // 시트가 연속적으로 비어있을 때 스킵할 플래그 변수
                bool bSkipSheet = false;

                // fallback 처리
                // 20 * 30 셀을 순회하며 검색, 100개 연속 빈 셀 탐지 시 스킵
                const uint16_t maxRows = 20;
                const uint16_t maxCols = 30;
                const int maxConsecutiveEmptyCells = 100;

                // 빈 셀 카운트를 저장할 변수
                int emptyCellStack = 0;

                // 셀 순회
                for (uint16_t col = 1; col <= maxCols && !bSkipSheet; ++col)
                {
                    for (uint16_t row = 1; row <= maxRows; ++row)
                    {
                        bool hasValue = ProcessCell(sheet, fileName, sheetName, keyword, row, col);

                        // 빈 셀이면
                        if (!hasValue)
                        {
                            // 빈 셀 카운트 증가
                            emptyCellStack++;

                            // 연속 빈 셀 카운트가 최대값에 도달하면 스킵
                            if (emptyCellStack >= maxConsecutiveEmptyCells)
                            {
                                std::wcout << L"연속으로 "
                                    << maxConsecutiveEmptyCells
                                    << L"개의 빈 셀이 탐지되어 시트를 스킵합니다." << std::endl;

                                bSkipSheet = true; // 바깥쪽 반복문 종료
                                break; // 안쪽 반복문 종료
                            }
                        }
                        else
                        {
                            // 유효셀을 찾았으면 빈 셀 카운트 초기화
                            emptyCellStack = 0;
                        }
                    }
                }
            }
        }

        if (bShowDebugLog)
        {
            DebugLog.Add(L"검색 완료. 문서를 닫습니다.", LogLevel::Info);
        }

        // 엑셀 문서 닫기
        doc.close();
    }
    catch (const std::exception& ex)
    {
        // 예외 발생 시 알림
        if (bShowDebugLog)
        {
            DebugLog.Add(L"엑셀 파일을 여는 도중 오류가 발생했습니다.", LogLevel::Error);
            DebugLog.Add(L"파일: " + wfilePath, LogLevel::Error);

            std::wstring wErr = SystemUtils::UTF8ToWString(ex.what());
            DebugLog.Add(L"사유: " + wErr, LogLevel::Error);
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
    uint16_t row, uint16_t col)
{
    // 셀 참조 생성
    OpenXLSX::XLCellReference cellRef(row, col);

    // 셀 텍스트 가져오기
    std::string cellText = GetCellText(sheet, cellRef);

    // ★ 커스텀 메타 해석: 텍스트만 전달
    auto& st = MetaStatePerSheet[SheetKey(fileName, sheetName)];
    TryResolveMetaCol(cellText, col, st);

    // 키워드 포함 여부 확인
    if (IsValueMatch(cellText, keyword))
    {
        // 검색 결과 저장
        ExcelSearchResult result;
        result.FileName = fileName;
        result.SheetName = sheetName;
        result.CellAddress = cellRef.address();
        result.CellValue = cellText;


        // 같은 행의 메타 열 값들 부착
        for (const auto& kv : st.Found)
        {
            uint32_t metaCol = kv.first;
            const std::string& label = kv.second; // 라벨은 원문 그대로

            auto metaCell = sheet.cell(OpenXLSX::XLCellReference(row, metaCol));
            std::string metaVal;

            // 히트난 셀이 메타 열과 같은 열이면, 이미 읽은 값을 재사용
            if (metaCol == col)
            {
                metaVal = cellText;
            }
            // 그 외에는 메타 열의 셀 값을 읽어옴
            else
            {
                const OpenXLSX::XLCellReference metaRef(row, metaCol);
                metaVal = GetCellText(sheet, metaRef);
            }

            // 결과에 저장
            result.CustomMetadata[label] = std::move(metaVal);
        }

        // 해당 셀이 포함된 row 전체 내용 저장
        result.FullRowData = GetFullRowData(sheet, row);

        // 결과 벡터에 추가
        searchResults.emplace_back(result);

        return true;  // 유효한 값 있는 셀
    }
    // 빈 셀 또는 키워드 미포함
    else return false;
}

bool ImGuiManager::IsValueMatch(const std::string& cellValue, const std::string& keyword)
{
    if (keyword.empty()) return false;

    const auto& Opt = SearchOptions.Get();

    try
    {
        // 정규식 검색
        if (Opt.bUseRegex)
        {
            std::regex_constants::syntax_option_type flags = std::regex::ECMAScript;
            if (!Opt.bCaseSensitive) flags |= std::regex::icase;

            std::regex re(keyword, flags);
            return std::regex_search(cellValue, re);
        }
        // 완전 일치 검색
        else if (Opt.bWholeWord)
        {
            // 대소문자 구분 있는 완전 일치 검색
            if (Opt.bCaseSensitive)
            {
                return (cellValue == keyword);
            }
            // 대소문자 구분 없는 완전 일치 검색
            else
            {
                return (SystemUtils::ToLower(cellValue) == SystemUtils::ToLower(keyword));
            }
        }
        // 부분 문자열 검색
        else
        {
            // 대소문자 구분 있는 부분 문자열 검색
            if (Opt.bCaseSensitive)
            {
                return (cellValue.find(keyword) != std::string::npos);
            }
            // 대소문자 구분 없는 부분 문자열 검색
            else
            {
                return ContainsIgnoreCase(cellValue, keyword);
            }
        }
    }
    catch (const std::regex_error& e)
    {
        // 잘못된 정규식 등 오류 시, 디버그 오버레이에 알림
        if (SearchOptions.Get().bShowDebugOverlay)
        {
            std::wstring wMsg = L"정규식 오류: ";
            wMsg += SystemUtils::UTF8ToWString(e.what());
            DebugLog.Add(wMsg, LogLevel::Error);
        }
        return false;
    }
}

// ==========================================================================
// 특정 셀을 문자열로 변환하여 반환. 예외/공백은 빈 문자열("")을 반환함.
// ==========================================================================
std::string ImGuiManager::GetCellText(OpenXLSX::XLWorksheet& sheet, OpenXLSX::XLCellReference cellRef)
{
    try
    {
        // 셀 참조로부터 셀 가져오기
        auto cell = sheet.cell(cellRef);
        // 셀 비었으면 빈문자열 리턴
        if (cell.empty()) return "";

        // 셀 값 가져오기
        OpenXLSX::XLCellValue value = cell.value();
        // 셀 value가 비어있으면 빈문자열 리턴
        if (value.type() == OpenXLSX::XLValueType::Empty) return "";

        std::string cellText;
        // 셀 값이 문자열이면
        if (value.type() == OpenXLSX::XLValueType::String)
        {
            // 안정적으로 문자열 가져옴
            cellText = value.get<std::string>();
        }
        // 그 외의 type은
        else
        {
            // 문자열로 변환하여 반환
            std::ostringstream oss;
            oss << value;
            cellText = oss.str();
        }

        // 앞뒤 공백 제거
        cellText = SystemUtils::Trim(std::move(cellText));

        return cellText;
    }
    catch (...)
    {
        return "";
    }

}


std::vector<std::string> ImGuiManager::GetFullRowData(OpenXLSX::XLWorksheet& sheet, uint16_t col, uint16_t maxRow /*=100*/)
{
    // 데이터를 담고 return할 벡터
    std::vector<std::string> outVector;
    outVector.reserve(maxRow);

    int emptyCellStack = 0; // 빈 셀 카운트
    int maxEmptyCells = 10; // 연속 빈 셀 최대 개수

    for (int rowIndex = 1; rowIndex <= maxRow; rowIndex++)
    {
        std::string cellText = GetCellText(sheet, OpenXLSX::XLCellReference(col, rowIndex));

        // 빈 셀이라면
        if (cellText.empty())
        {
            // 빈 셀 카운트 증가 후 continue
            emptyCellStack++;

            if (emptyCellStack >= maxEmptyCells) break; // 연속 빈 셀 최대 개수 도달 시 종료
            else continue;
        }

        // 유효 셀을 찾았으면 빈 셀 카운트 초기화
        emptyCellStack = 0;

        // 벡터에 추가
        outVector.emplace_back(std::move(cellText));
    }

    return outVector;
}

FSheetMetaState ImGuiManager::BuildMetaState(const FSearchOptions& Opt)
{
    FSheetMetaState st;
    if (!Opt.bUseCustomMeta) return st;


    for (auto& raw : Opt.CustomMetadatas)
    {
        // 빈 문자열이 아니라면
        if (!raw.empty())
        {
            // 앞뒤 공백 제거 후 소문자로 변환하여 삽입
            st.Pending.insert(SystemUtils::ToLower(raw));
        }
    }

    return st;
}

inline void ImGuiManager::TryResolveMetaCol(const std::string& cellText, uint32_t col, FSheetMetaState& st)
{
    if (st.Pending.empty()) return;             // 남은 타겟 없음
    if (st.Found.find(col) != st.Found.end()) return; // 이 열 이미 확정

    if (cellText.empty()) return;

    const std::string lowered = SystemUtils::ToLower(cellText);
    auto it = st.Pending.find(lowered);
    if (it == st.Pending.end()) return;         // 대상 아님

    // 매치 → 이 열을 메타열로 확정(라벨은 원문 그대로)
    st.Found.emplace(col, cellText);
    st.Pending.erase(it);

    if (SearchOptions.Get().bShowDebugOverlay)
    {
        DebugLog.AddFmt(LogLevel::Info, L"[Meta-Match] col=%u  label=\"%ls\"  (Pending->Found, 남은=%d)",
            (unsigned)col,
            SystemUtils::UTF8ToWString(cellText).c_str(),
            (int)(st.Pending.size()));
    }
}

void ImGuiManager::ExportSearchResultsOpenDialog(const std::vector<ExcelSearchResult>& results)
{
    // 검색 결과가 없으면 리턴
    if (results.empty())
    {
        return;
    }

    // 결과를 캐시
    g_ExportCachedResults = results;
    // 기본 파일명 생성
    g_ExportDefaultFile = SystemUtils::MakeDefaultResultFileName();

    // 저장 다이얼로그 열기
    IGFD::FileDialogConfig cfg;
    cfg.path = ".";                     // 탐색 경로를 기본으로 설정(현재 디렉토리)
    cfg.fileName = g_ExportDefaultFile.c_str();
    cfg.countSelectionMax = 1;          // 단일 파일 선택만 허용
    cfg.flags = ImGuiFileDialogFlags_ConfirmOverwrite;  // 덮어쓰기 경고

    ImGuiFileDialog::Instance()->OpenDialog(
        kExportDialogKey,
        kExportDialogTitle,
        kFilters,
        cfg);

    // 다음 프레임에서 display 처리하도록 플래그 설정
    g_ExportDialogRequested = true;
}

void ImGuiManager::ExportSearchResultsDrawModal()
{
    // 저장 불가 알림 팝업
    if (ImGui::BeginPopupModal("NoResultsToExport", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(u8"내보낼 검색 결과가 없습니다.");
        if (ImGui::Button(u8"확인", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // 다이얼로그 표시
    if (g_ExportDialogRequested &&
        ImGuiFileDialog::Instance()->Display(kExportDialogKey,
            ImGuiWindowFlags_NoCollapse, ImVec2(720, 480)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            // 선택된 경로 획득
            std::string UserInputPath = ImGuiFileDialog::Instance()->GetFilePathName();

            // 확장자 검사 및 보정 (.xlsx)
            UserInputPath = EnsureXlsxExtension(std::move(UserInputPath));

            // 엑셀 파일로 저장
            const bool ok = SaveSearchResultsToExcel(g_ExportCachedResults, UserInputPath);

            // 결과 팝업
            if (ok)  ImGui::OpenPopup("SaveSuccess");
            else     ImGui::OpenPopup("SaveFailed");
        }

        // 다이얼로그 닫기 및 플래그 초기화
        ImGuiFileDialog::Instance()->Close();
        g_ExportDialogRequested = false;
        g_ExportCachedResults.clear();
    }

    // 저장 성공/실패 피드백 팝업
    if (ImGui::BeginPopupModal("SaveSuccess", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(u8"검색 결과가 Excel 파일로 저장되었습니다.");
        if (ImGui::Button(u8"확인", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("SaveFailed", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(u8"저장 중 오류가 발생했습니다. 콘솔 로그를 확인하세요.");
        if (ImGui::Button(u8"확인", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

bool ImGuiManager::SaveSearchResultsToExcel(const std::vector<ExcelSearchResult>& Results, const std::string& UesrInputPath)
{
    // 필요한 변수들 get
    bool bShowDebugLog = SearchOptions.Get().bShowDebugOverlay; // 디버그 로그 출력 여부
    const auto& Opt = SearchOptions.Get();                      // 커스텀 메타데이터 라벨 참조용

    if (Results.empty())
    {
        if (bShowDebugLog)
        {
            DebugLog.Add(L"저장할 검색 결과가 없습니다.", LogLevel::Warning);
        }

        return false;
    }

    try
    {
        // 디버그 로그
        if (bShowDebugLog)
        {
            DebugLog.Add(L"===========================================", LogLevel::Separator);
            DebugLog.Add(L"검색 결과를 엑셀 파일로 저장합니다.", LogLevel::Info);
            DebugLog.AddFmt(LogLevel::Info, L"전체 경로: %ls", SystemUtils::UTF8ToWString(UesrInputPath).c_str());
        }

        // 유저 입력 경로에서 부모 디렉토리 추출
        std::string ParentDir = SystemUtils::GetParentDirectory(UesrInputPath);
        // 임시 엑셀 파일 경로 생성
        std::string TempFilePath = ParentDir + "\\" + EnsureXlsxExtension(g_ExportDefaultFile);


        // ----------------------------------- 엑셀 파일 생성 -----------------------------------
        OpenXLSX::XLDocument doc;
        doc.create(TempFilePath);                   // 새 엑셀 파일 생성
        auto wb = doc.workbook();                   // 워크북 가져오기
        auto ws = wb.worksheet("Sheet1");           // 기본 시트 가져오기
        ws.setName("Results");                      // 시트 이름 변경


        // ----------------------------------- 헤더 구성 -----------------------------------
        // 헤더를 담을 벡터
        std::vector<std::string> headers;
        // 헤더 크기 예약 (기본 4개 + 커스텀 메타데이터 개수)
        headers.reserve(4 + SearchOptions.Get().CustomMetadatas.size());

        // 기본 헤더 추가
        headers.emplace_back(u8"파일");
        headers.emplace_back(u8"시트");
        headers.emplace_back(u8"위치");
        headers.emplace_back(u8"내용");

        // 메타 데이터에 따라 헤더 추가
        if (CachedView.bUseCustomMeta)
        {
            for (const auto& label : Opt.CustomMetadatas)
            {
                headers.emplace_back(label);
            }
        }

        // ----------------------------------- 데이터 작성 -----------------------------------
        // 헤더 작성 (1행)
        for (int i = 0; i < (int)headers.size(); ++i)
        {
            ws.cell(OpenXLSX::XLCellReference(/*row*/1, /*col*/ i + 1)).value() = headers[i];
        }

        // 셀 값 앞에 특수문자가 오면 엑셀에서 수식으로 인식하는 문제 방지용 람다
        auto sanitize = [](const std::string& s) -> std::string
            {
                if (s.empty()) return s;
                const char ch = s[0];
                if (ch == '=' || ch == '+' || ch == '-' || ch == '@')
                {
                    return std::string(1, '\'') + s; // 안전하게 문자열 생성
                }
                return s;
            };


        // 결과 데이터 작성 (2행부터)
        int row = 2;

        // 미리 메타 라벨 목록을 보존 (라벨 순서대로 값을 꺼내기 위함)
        std::vector<std::string> metaLabels;
        if (Opt.bUseCustomMeta)
        {
            metaLabels = Opt.CustomMetadatas;
        }

        for (const auto& r : Results)
        {
            int col = 1;

            // 기본 컬럼 작성
            ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = sanitize(r.FileName);      // 파일명
            ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = sanitize(r.SheetName);     // 시트명
            ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = sanitize(r.CellAddress);   // 셀 주소
            ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = sanitize(r.CellValue);     // 셀 내용

            // 커스텀 메타데이터
            if (!metaLabels.empty())
            {
                // 메타데이터 라벨을 순회하며
                for (const auto& label : metaLabels)
                {
                    std::string val;
                    // 라벨과 완전 일치하는 키가 있으면 해당 값을 사용
                    if (auto it = r.CustomMetadata.find(label); it != r.CustomMetadata.end())
                    {
                        val = it->second;
                    }
                    // 없으면
                    else
                    {
                        // 매치된 값이 있는지 확인용 플래그
                        bool bFound = false;

                        // 대소문자 구분 없이 재탐색
                        const auto wantLower = SystemUtils::ToLower(label);
                        for (const auto& kv : r.CustomMetadata) {
                            if (SystemUtils::ToLower(kv.first) == wantLower)
                            {
                                val = kv.second;
                                bFound = true;
                                break;
                            }
                        }

                        // 매치된 값이 없으면 빈문자열
                        if (!bFound)
                        {
                            val = "";
                        }
                    }

                    // 메타데이터 값 작성
                    ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = sanitize(val);
                }
            }

            ++row;
        }

        // 저장/닫기
        doc.save();
        doc.close();

        // 디버그 로그
        if (bShowDebugLog)
        {
            DebugLog.Add(L"엑셀 저장 완료.", LogLevel::Info);
        }

        // 임시 파일 → 최종 유저 파일명으로 변경
        std::wstring wTemp = SystemUtils::UTF8ToWString(TempFilePath);
        std::wstring wFinal = SystemUtils::UTF8ToWString(UesrInputPath);

        // 같은 드라이브라면 빠른 rename, 이미 존재하면 덮어쓰기
        if (MoveFileExW(wTemp.c_str(), wFinal.c_str(), MOVEFILE_REPLACE_EXISTING))
        {
        //    if (bShowDebugLog)
        //    {
        //        DebugLog.AddFmt(LogLevel::Info, L"파일 이름 변경 완료: %ls", wFinal.c_str());
        //    }
        }
        else
        {
            DWORD err = GetLastError();
            if (bShowDebugLog)
            {
                DebugLog.AddFmt(LogLevel::Error, L"파일 이름 변경 실패 (WinErr=%lu)", err);
            }
            return false;
        }

        return true;
    }
    catch (const std::exception& e)
    {
        // 예외 발생 시 디버그 로그
        if (bShowDebugLog)
        {
            DebugLog.AddFmt(LogLevel::Error, L"엑셀 저장 실패: %ls", SystemUtils::UTF8ToWString(e.what()).c_str());
        }

        return false;
    }
}

std::string ImGuiManager::EnsureXlsxExtension(std::string path)
{
    try
    {
        // UTF-8 문자열을 wide 문자열으로 변환
        std::wstring wPath = SystemUtils::UTF8ToWString(path);

        // wide 기반으로 확장자 확인/수정
        std::filesystem::path p(wPath);
        if (p.extension() != L".xlsx")
            p.replace_extension(L".xlsx");

        // 다시 UTF-8 문자열으로 반환
        return SystemUtils::WStringToUTF8(p.wstring());
    }
    catch (...)
    {
        // 파일명에 한글이 있어도 죽지 않게 예외 흡수
        // 단순 문자열로 ".xlsx"만 보장
        std::string safe = path;
        auto ends_with = [](const std::string& s, const std::string& suf)
            {
                return s.size() >= suf.size() &&
                    std::equal(suf.rbegin(), suf.rend(), s.rbegin(),
                        [](char a, char b) { return std::tolower(a) == std::tolower(b); });
            };
        if (!ends_with(safe, ".xlsx"))
            safe += ".xlsx";
        return safe;
    }
}

bool ImGuiManager::ContainsIgnoreCase(const std::string& cellValue, const std::string& keyword)
{
    if (keyword.empty()) return false;
    const std::string t = SystemUtils::ToLower(cellValue);
    const std::string k = SystemUtils::ToLower(keyword);

    return (t.find(k) != std::string::npos);
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
