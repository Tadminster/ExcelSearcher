#include "SearchOptionsWindow.h"
#include "../ExcelSearcher/IconsFontAwesome6.h"

#include <unordered_set>

// @brief InputText 의 CallbackResize 처리
static int InputTextCallback_Resize(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* str = reinterpret_cast<std::string*>(data->UserData);
        str->resize(data->BufTextLen);  // 문자열 크기를 현재 입력 길이에 맞춤
        data->Buf = str->data();        // 최신 버퍼 주소를 다시 알려줌
    }
    return 0;
}

// @brief std::string 기반 InputText 위젯
static bool InputTextResizable(const char* label, std::string* str, ImGuiInputTextFlags flags = 0)
{
    // CallbackResize 를 항상 켜서 std::string 과 연동
    flags |= ImGuiInputTextFlags_CallbackResize;

    // 초기 capacity 가 0일 경우 버퍼를 넉넉히 확보
    if (str->capacity() == 0) str->reserve(64);

    // 실제 입력 처리
    bool changed = ImGui::InputText(label,
        str->data(),
        str->capacity() + 1,
        flags,
        InputTextCallback_Resize,
        str);

    return changed;
}



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
    constexpr float minWidth = 300.0f;   // 최소 너비
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
                        ImGui::SetTooltip(u8"예: 'Cat'과 'cat'을 서로 다른 단어로 구분합니다.");

                    ImGui::Dummy(ImVec2(0, 1)); // 항목 간 간격용 더미

                    // 단어 전체 일치
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 단어 전체 일치", &DraftOptions.bWholeWord);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"예: 'cat'를 찾을 때 'polecat'는 제외하고 정확히 'cat'만 찾습니다.");

                    ImGui::Dummy(ImVec2(0, 1));

                    // 정규식 사용
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 정규식 사용", &DraftOptions.bUseRegex);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"정규 표현식을 사용하여 복잡한 패턴 매칭을 수행합니다.\n예: '^c.*s$'는 'c'로 시작하고 's'로 끝나는 단어를 찾습니다.\n'Cat', 'Cut', 'Craft', Count' 등이 매치됩니다.");

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

                    ImGui::Dummy(ImVec2(0, 1));

                    // 커스텀 필터 사용 (향후 확장용 스위치)
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(u8" 커스텀 메타데이터 사용", &DraftOptions.bUseCustomFillter);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(u8"사용자 정의 메타데이터 파이프라인을 활성화합니다.");

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            // ----------------------------------- 커스텀 메타데이터 -----------------------------------
            if (DraftOptions.bUseCustomFillter)
            {
                if (ImGui::BeginTabItem("커스텀"))
                {
                    
                    // 처음 켜질 때 입력칸이 하나도 없으면 1칸 생성
                    if (DraftOptions.CustomMetadatas.empty())
                        DraftOptions.CustomMetadatas.emplace_back();

                    ImGui::SeparatorText("커스텀 메타데이터(필터) 입력");

                    // 설명
                    ImGui::TextDisabled(u8"여기에 입력한 문자열이 셀 내용과 매치되면,\n해당 메타데이터 열이 결과창에 표시됩니다.");
                    ImGui::Dummy(ImVec2(0, 4));

                    // 커스텀 메타데이터 최대 개수
                    constexpr int MaxMetadataCount = 6;
                    // 현재 개수가 최대 개수보다 작은지 확인
                    bool bCanAddMore = (int)DraftOptions.CustomMetadatas.size() < MaxMetadataCount;

                    // 최대 개수 도달시 추가 버튼 비활성화
                    ImGui::BeginDisabled(!bCanAddMore);
                    {
                        // 추가 버튼
                        if (ImGui::Button(" " ICON_FA_PLUS " "))
                        {
                            if (bCanAddMore)
                            {
                                DraftOptions.CustomMetadatas.emplace_back();
                            }
                        }
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip(u8"새 커스텀 메타데이터 항목을 추가합니다.");
                        }
                    }
                    ImGui::EndDisabled();

                    ImGui::SameLine();
                    // 전체 삭제 버튼
                    if (ImGui::Button(ICON_FA_TRASH))
                    {
                        DraftOptions.CustomMetadatas.clear();
                        DraftOptions.CustomMetadatas.emplace_back();
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip(u8"모든 커스텀 메타데이터 항목을 삭제합니다.");
                    }
                    ImGui::SameLine();

                    ImGui::TextDisabled(u8"(현재 %d개, 최대 %d개)", (int)DraftOptions.CustomMetadatas.size(), MaxMetadataCount);

                    // 리스트(스크롤 영역)
                    const float rowH = ImGui::GetTextLineHeightWithSpacing();
                    if (ImGui::BeginChild("##CustomFilterList", ImVec2(0, rowH * 8.0f), true, ImGuiWindowFlags_None))
                    {
                        for (int MetadataIndex = 0; MetadataIndex < (int)DraftOptions.CustomMetadatas.size(); MetadataIndex++)
                        {
                            ImGui::PushID(MetadataIndex);

                            // 한 줄: 입력필드 + 삭제 버튼
                            const float removeBtnW = ImGui::CalcTextSize("삭제").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - removeBtnW - ImGui::GetStyle().ItemSpacing.x);

                            InputTextResizable("##Filter", &DraftOptions.CustomMetadatas[MetadataIndex]);

                            ImGui::SameLine();
                            if (ImGui::Button("삭제"))
                            {
                                DraftOptions.CustomMetadatas.erase(DraftOptions.CustomMetadatas.begin() + MetadataIndex);
                                ImGui::PopID();
                                if (DraftOptions.CustomMetadatas.empty())
                                    DraftOptions.CustomMetadatas.emplace_back();
                                break; // 현재 프레임 루프 정합성
                            }
                            if (ImGui::IsItemHovered())
                            {
                                ImGui::SetTooltip(u8"이 커스텀 메타데이터 항목을 삭제합니다.");
                            }

                            ImGui::PopID();
                        }

                        ImGui::EndChild();
                    }

                    // 가이드
                    ImGui::Dummy(ImVec2(0, 2));
                    ImGui::TextDisabled(u8"중복/공백 항목은 적용 시 자동 정리됩니다.");

                    ImGui::EndTabItem();
                }
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
        // 커스텀 메타데이터 정리 (공백/중복 제거)
        SanitizeCustomMetadatas(DraftOptions.CustomMetadatas);

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

void SearchOptionsWindow::Set(const FSearchOptions& In)
{
    Options = In;
    DraftOptions = Options;     // UI 열었을 때 일관성

    SetMetaColumnCount();
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

void SearchOptionsWindow::SanitizeCustomMetadatas(std::vector<std::string>& metas)
{
    // 결과를 임시로 저장할 벡터
    std::vector<std::string> out;
    out.reserve(metas.size());

    // 중복 체크용 unordered_set
    std::unordered_set<std::string> seen;

    for (const auto& str : metas)
    {
        // 문자열 전체 공백인지 확인
        bool allSpaces = true;
        for (unsigned char ch : str)
        {
            if (!std::isspace(ch))
            {
                allSpaces = false;
                break;
            }
        }
        // 전부 공백이라면 continue(제거)
        if (allSpaces) continue;

        // 중복 체크
        if (seen.insert(str).second)
        {
            // 처음 본 문자열만 추가
            out.emplace_back(str); 
        }
    }

    // 결과를 원본 벡터에 복사
    metas.swap(out);
}
