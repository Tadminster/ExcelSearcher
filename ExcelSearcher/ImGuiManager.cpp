#include "ImGuiManager.h"
#include "../backends/ImGuiFileDialog.h"

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

    auto places_ptr = ImGuiFileDialog::Instance()->GetPlacesGroupPtr("CustomPlaces");
    if (places_ptr != nullptr)
    {
        // 자주 접근할 폴더 바로가기 추가
        places_ptr->AddPlace("엑셀 데이터", "C:\\Users\\Dev\\Downloads", true, IGFD::FileStyle(ImVec4(0.1f, 0.8f, 0.2f, 1.0f)));

        // 경로 구분선 추가
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

    // 파일 열기용 메인 UI 창
    ImGui::Begin("ImGui Window", &isWindowOpen);
    {
        // 파일 열기 버튼 클릭 시 파일 다이얼로그 열기
        if (ImGui::Button("파일 열기"))
        {
            IGFD::FileDialogConfig config;
            config.path = "."; // 현재 경로
            config.countSelectionMax = 0; // 다중 선택 허용
            ImGuiFileDialog::Instance()->OpenDialog("FileOpenDialog", "파일 선택", filters, config);
        }

        // 파일 선택 다이얼로그 표시
        if (ImGuiFileDialog::Instance()->Display("FileOpenDialog"))
        {
            // 확인 버튼 눌렸는지 확인
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                // 선택한 파일 정보를 저장
                selectedFiles = ImGuiFileDialog::Instance()->GetSelection();
            }
            ImGuiFileDialog::Instance()->Close(); // 다이얼로그 닫기
        }
    }
    ImGui::End();

    // 파일 선택 결과 출력 창 (디버깅 용도)
    ImGui::Begin("파일 디버깅");
    if (!selectedFiles.empty())
    {
        ImGui::Text("선택된 파일 수: %d", selectedFiles.size());

        // 선택된 각 파일 정보 출력
        for (const auto& file : selectedFiles)
        {
            const std::string& fileName = file.first;       // 파일명
            const std::string& filePathName = file.second;  // 전체 경로

            ImGui::Text("파일 이름: %s", fileName.c_str());
            ImGui::Text("전체 경로: %s", filePathName.c_str());
            ImGui::Separator(); // 구분선
        }

    }
    else
    {
        ImGui::Text("선택된 파일 없음");
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
