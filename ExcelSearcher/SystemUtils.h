#pragma once

#include <string>
#include <windows.h>
#include <shlobj.h>  // SHGetKnownFolderPath
#include <knownfolders.h>

class SystemUtils
{
public:
    // Windows의 알려진 폴더 경로를 가져옴
    static std::string GetKnownFolder(REFKNOWNFOLDERID folderId);

    // 기본 결과 파일 이름 생성 (예: ExcelSearchResults_20231015_153045.xlsx)
    static std::string MakeDefaultResultFileName();

    // 자주 쓰는 경로들 편의 함수
    static std::string GetDownloads();
    static std::string GetDocuments();
    static std::string GetDesktop();

    // 문자열 변환 함수 (인코딩)
    static std::string WStringToUTF8(const std::wstring& wstr);
    static std::wstring UTF8ToWString(const std::string& str);

    // 문자열을 소문자로 변환
    static std::string ToLower(const std::string& str);

    // ASCII 문자열 검사
    static bool IsAscii(const std::string& str);

    // 문자열 공백 제거
    static std::string Trim(const std::string& str);

    // 부모 디렉토리 경로 가져오기
    static std::string GetParentDirectory(const std::string& path);
};
