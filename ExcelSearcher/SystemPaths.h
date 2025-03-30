#pragma once

#include <string>
#include <windows.h>
#include <shlobj.h>  // SHGetKnownFolderPath
#include <knownfolders.h>

class SystemPaths
{
public:
    // Windows�� �˷��� ���� ��θ� ������
    static std::string GetKnownFolder(REFKNOWNFOLDERID folderId);

    // ���� ���� ��ε� ���� �Լ�
    static std::string GetDownloads();
    static std::string GetDocuments();
    static std::string GetDesktop();

private:
    // std::wstring�� UTF-8�� ��ȯ
    static std::string WStringToUTF8(const std::wstring& wstr);
};
