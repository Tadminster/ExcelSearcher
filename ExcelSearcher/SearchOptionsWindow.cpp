#include "SearchOptionsWindow.h"
#include "../ExcelSearcher/IconsFontAwesome6.h"

SearchOptionsWindow::SearchOptionsWindow()
{
    SetMetaColumnCount();
}

void SearchOptionsWindow::Open()
{
    // 모달 팝업을 열 준비
    bOpen = true;               // 상태 플래그 set
    DraftOptions = Options;     // 임시 옵션에 현재 옵션을 저장
    ImGui::OpenPopup(Title);    // ImGui 팝업 오픈
}

void SearchOptionsWindow::Close()
{
    // 모달 팝업 닫기
    ImGui::CloseCurrentPopup(); // ImGui 팝업 닫기
    bOpen = false;              // 상태 플래그 clear
}

void SearchOptionsWindow::Draw()
{
    // 외부에서 Open() 호출된 경우에만 그림
    if (!bOpen) return;

    // ---------------------------------------------------------------------------
    // SearchOptions 창 크기 설정
    // ---------------------------------------------------------------------------
    constexpr float minWidth = 280.0f;   // 최소 너비
    constexpr float minHeight = 360.0f;   // 최소 높이

    ImVec2 sizeMin = ImVec2(minWidth, minHeight);                               // 창의 최소 크기
    ImVec2 sizeMax = ImVec2(FLT_MAX/*가로 무제한*/, minHeight/*세로 고정*/);     // 창의 최대 크기

    // 창 크기 제약 설정
    ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

    // 창이 처음 나타날 때 크기 설정
    ImGui::SetNextWindowSize(ImVec2(minWidth, minHeight), ImGuiCond_Appearing);


    // ---------------------------------------------------------------------------
    // 모달 팝업 시작
    // ---------------------------------------------------------------------------
    if (ImGui::BeginPopupModal(Title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        DrawContents();     // 실제 UI 구성
        ImGui::EndPopup();  // 모달 팝업 종료
    }
    else
    {
        // 모달이 닫힌 경우 상태 플래그 clear
        bOpen = false;
    }
}

// ============================================================================
// 모달 내부 컨텐츠(탭/체크박스/버튼) 구성
// - 탭: 기본 / 메타데이터 / 디버그
// - 각 탭의 옵션들은 FSearchOptions 구조체에 바인딩
// - 하단 버튼: 초기화, 취소, 적용(OnApply 콜백 호출)
// ============================================================================
void SearchOptionsWindow::DrawContents()
{
    // 스크롤 가능한 내부 컨테이너 높이
    // 내부 컨테이너 높이를 넘으면 Child에서 스크롤 처리
    constexpr float contentHeight = 280.0f;

    // ------------------------------------------------------------------------
    // 내부 컨테이너(Child)
    // ------------------------------------------------------------------------
    if (ImGui::BeginChild("OptionsContent", ImVec2(0, contentHeight), true,
        ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_AlwaysVerticalScrollbar))
    {
        // TabBar 컨테이너 (TabItem의 상위)
        if (ImGui::BeginTabBar("OptionsTabs"))
        {
            // ----------------------------------- 기본 -----------------------------------
            // 검색 동작에 직접적인 영향을 주는 범용 설정
            if (ImGui::BeginTabItem("기본"))
            {
                if (ImGui::BeginTable("BasicTable", 2, ImGuiTableFlags_SizingStretchSame))
                {
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 180.0f);
                    ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                    // 대/소문자 구분
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 대/소문자 구분", &DraftOptions.bCaseSensitive);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"예: 'Test'와 'test'를 서로 다른 단어로 구분합니다.");

                    ImGui::Dummy(ImVec2(0, 1)); // 항목 간 간격용 더미

                    // 단어 전체 일치
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 단어 전체 일치", &DraftOptions.bWholeWord);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"예: 'cat'를 찾을 때 'concatenate'는 제외하고 정확히 'cat'만 찾습니다.");

                    ImGui::Dummy(ImVec2(0, 1));

                    // 커스텀 필터 사용 (향후 확장용 스위치)
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 커스텀 필터 사용", &DraftOptions.bUseCustomFillter);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"고급 사용자 정의 필터 파이프라인을 활성화합니다. (향후 확장)");

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            // ----------------------------------- 메타데이터 -----------------------------------
            // 결과 리스트에 무엇을 표시할지에 대한 시각적 옵션
            if (ImGui::BeginTabItem("메타데이터"))
            {
                ImGui::SeparatorText("표시할 추가 정보");
                if (ImGui::BeginTable("MetaTable", 2, ImGuiTableFlags_SizingStretchSame))
                {
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 180.0f);
                    ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                    // 미리보기(행 전체 프리뷰 팝업)
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 미리보기", &DraftOptions.bEnablePreview);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"결과 행의 주변 맥락(같은 행의 다른 셀들)을 팝업으로 보여줍니다.");
                    ImGui::Dummy(ImVec2(0, 1));

                    // 파일 경로
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 파일 경로", &DraftOptions.bShowFileName);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"결과 표에 전체 파일 경로를 함께 표시합니다.");
                    ImGui::Dummy(ImVec2(0, 1));

                    // 시트 이름
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 시트 이름", &DraftOptions.bShowSheetName);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"각 결과가 어느 시트에서 발견되었는지 표시합니다.");
                    ImGui::Dummy(ImVec2(0, 1));

                    // 셀 주소
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 셀 주소", &DraftOptions.bShowCellAddress);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"예: A1, C12 같은 셀 좌표를 결과 표에 표시합니다.");
                    ImGui::Dummy(ImVec2(0, 1));

                    // 스니펫(문맥)
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 스니펫(문맥)", &DraftOptions.bShowSnippet);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"검색된 문맥을 함께 보여줍니다.");

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            // ----------------------------------- 디버그 -----------------------------------
            // 개발/테스트/사용자 지원용 디버그 옵션
            if (ImGui::BeginTabItem("디버그"))
            {
                if (ImGui::BeginTable("DebugTable", 2, ImGuiTableFlags_SizingFixedFit))
                {
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 180.0f);
                    ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                    // 오버레이
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox("디버그 오버레이 활성화", &DraftOptions.bShowDebugOverlay);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"개발용 디버그 오버레이를 켭니다.");


                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            // 탭 종료
            ImGui::EndTabBar();
        }

        // 내부 컨테이너 종료
        ImGui::EndChild(); 
    }


    // ------------------------------------------------------------------------
    // 하단 버튼 영역
    // ------------------------------------------------------------------------
    ImGui::Separator();
    ImGui::SameLine(); ImGui::Dummy(ImVec2(0, 2));

    // 초기화 버튼 (모든 옵션 기본값으로 되돌림)
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT))
    {
        DraftOptions = FSearchOptions{};
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(u8"초기화\n모든 설정을 기본값으로 재설정하려면 이 버튼을 누르세요.");
    }

    // 버튼 간격 조정
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(1, 0));
    ImGui::SameLine();

    // 버튼 크기
    ImVec2 btnSize(ImGui::GetFontSize() * 5.0f, 0);

    // 취소: 변경 사항을 버리고 모달 닫기
    if (ImGui::Button("취소", btnSize))
    {
        Close();
    }


    ImGui::SameLine();

    // 적용: 콜백이 지정되어 있다면 현재 Options를 전달 후 닫기
    if (ImGui::Button("적용", btnSize))
    {
        // 메타데이터 열 개수 계산
        SetMetaColumnCount();

        // 실제 옵션에 반영
        Options = DraftOptions;

        // 외부 콜백 호출
        if (OnApply)
            OnApply(Options);

        Close();
    }
}

void SearchOptionsWindow::SetMetaColumnCount()
{
    MetaColumnCount = 0;
    if (DraftOptions.bEnablePreview)   MetaColumnCount++;
    if (DraftOptions.bShowFileName)    MetaColumnCount++;
    if (DraftOptions.bShowSheetName)   MetaColumnCount++;
    if (DraftOptions.bShowCellAddress) MetaColumnCount++;
    if (DraftOptions.bShowSnippet)     MetaColumnCount++;
}

