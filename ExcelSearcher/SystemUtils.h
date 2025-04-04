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

    // 자주 쓰는 경로들 편의 함수
    static std::string GetDownloads();
    static std::string GetDocuments();
    static std::string GetDesktop();

    // 문자열 변환 함수 (인코딩)
    static std::string WStringToUTF8(const std::wstring& wstr);
    static std::wstring UTF8ToWString(const std::string& str);
private:
};
