#pragma once

#include <string>
#include <windows.h>
#include <shlobj.h>  // SHGetKnownFolderPath
#include <knownfolders.h>

class SystemUtils
{
public:
    // Windows�� �˷��� ���� ��θ� ������
    static std::string GetKnownFolder(REFKNOWNFOLDERID folderId);

    // ���� ���� ��ε� ���� �Լ�
    static std::string GetDownloads();
    static std::string GetDocuments();
    static std::string GetDesktop();

    // ���ڿ� ��ȯ �Լ� (���ڵ�)
    static std::string WStringToUTF8(const std::wstring& wstr);
    static std::wstring UTF8ToWString(const std::string& str);
private:
};
