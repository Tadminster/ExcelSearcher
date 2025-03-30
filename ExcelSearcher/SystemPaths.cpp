#include "SystemPaths.h"
#include <codecvt>
#include <locale>

std::string SystemPaths::WStringToUTF8(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

std::string SystemPaths::GetKnownFolder(REFKNOWNFOLDERID folderId)
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

std::string SystemPaths::GetDownloads()
{
    return GetKnownFolder(FOLDERID_Downloads);
}

std::string SystemPaths::GetDocuments()
{
    return GetKnownFolder(FOLDERID_Documents);
}

std::string SystemPaths::GetDesktop()
{
    return GetKnownFolder(FOLDERID_Desktop);
}
