#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// ============================================================================
// 커스텀 메타데이터
// ============================================================================
using PendingSet = std::unordered_set<std::string>;             // 소문자/Trim 정규화된 타겟 문자열
using FoundMap = std::unordered_map<uint32_t, std::string>;     // col -> 라벨(원문)

struct FSheetMetaState
{
    PendingSet Pending;  // 아직 못 찾은 커스텀 메타 문자열들
    FoundMap   Found;    // 이미 발견한 메타 열(중복조사 방지)
};


inline std::string SheetKey(const std::string& file, const std::string& sheet)
{
    return file + "|" + sheet;
}



// ============================================================================
// 검색 결과 구조체
// ============================================================================
struct ExcelSearchResult
{
    std::string FileName;   // 엑셀 파일 이름
    std::string SheetName;  // 시트 이름
    std::string CellAddress;// 셀 주소
    std::string CellValue;  // 셀 값
    std::unordered_map<std::string, std::string> CustomMetadata; // 커스텀 메타데이터 (키-값 쌍)

    std::vector<std::string> FullRowData; // 셀이 포함된 행의 전체 데이터
};

