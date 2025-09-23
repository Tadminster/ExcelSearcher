#pragma once
#include <string>
#include <vector> 

struct ExcelSearchResult
{
    std::string fileName;   // 엑셀 파일 이름
    std::string sheetName;  // 시트 이름
    std::string cellAddress;// 셀 주소
    std::string cellValue;  // 셀 값

    std::vector<std::string> fullRowData; // 셀이 포함된 행의 전체 데이터
};
