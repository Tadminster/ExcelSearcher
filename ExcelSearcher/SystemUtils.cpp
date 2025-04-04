#include "SystemUtils.h"
#include <codecvt>
#include <locale>


std::string SystemUtils::WStringToUTF8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

std::wstring SystemUtils::UTF8ToWString(const std::string& str)
{
    if (str.empty()) return {};

    int wideSize = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
    std::wstring wstr(wideSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), wstr.data(), wideSize);

    return wstr;
}

std::string SystemUtils::GetKnownFolder(REFKNOWNFOLDERID folderId)
{
    PWSTR path = nullptr;
    HRESULT hr = SHGetKnownFolderPath(folderId, 0, nullptr, &path);
    if (SUCCEEDED(hr) && path)
    {
        std::string result = WStringToUTF8(path);
        CoTaskMemFree(path);
        return result;
    }
    return "";
}

std::string SystemUtils::GetDownloads()
{
    return GetKnownFolder(FOLDERID_Downloads);
}

std::string SystemUtils::GetDocuments()
{
    return GetKnownFolder(FOLDERID_Documents);
}

std::string SystemUtils::GetDesktop()
{
    return GetKnownFolder(FOLDERID_Desktop);
}
