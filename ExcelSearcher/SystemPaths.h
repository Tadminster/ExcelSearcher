#pragma once

#include <string>
#include <windows.h>
#include <shlobj.h>  // SHGetKnownFolderPath
#include <knownfolders.h>

class SystemPaths
{
public:
    // Windows의 알려진 폴더 경로를 가져옴
    static std::string GetKnownFolder(REFKNOWNFOLDERID folderId);

    // 자주 쓰는 경로들 편의 함수
    static std::string GetDownloads();
    static std::string GetDocuments();
    static std::string GetDesktop();

private:
    // std::wstring을 UTF-8로 변환
    static std::string WStringToUTF8(const std::wstring& wstr);
};
