#include "SystemUtils.h"
#include <codecvt>
#include <locale>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>


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

std::string SystemUtils::ToLower(const std::string& str)
{
    if (str.empty()) return "";

    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return lowerStr;
}


bool SystemUtils::IsAscii(const std::string& str)
{
    for (unsigned char ch : str)
    {
        if (ch > 127)
            return false; // 비 ASCII 문자 (한글 등)
    }
    return true;
}

std::string SystemUtils::Trim(const std::string& str)
{
    const char* ws = " \t\r\n";
    auto a = str.find_first_not_of(ws);
    auto b = str.find_last_not_of(ws);
    if (a == std::string::npos) return {};

    return str.substr(a, b - a + 1);
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

std::string SystemUtils::MakeDefaultResultFileName()
{
    // 현재 시간 가져오기
    auto now = std::chrono::system_clock::now();
    // 시간을 시간_t로 변환
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};

    // 지역 시간으로 변환
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    // 형식화된 문자열 생성
    std::ostringstream oss;
    oss << "ExcelSearch_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S");

    return oss.str();
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

std::string SystemUtils::GetParentDirectory(const std::string& path)
{
    std::filesystem::path p = SystemUtils::UTF8ToWString(path);
    std::filesystem::path parent = p.parent_path();

    return SystemUtils::WStringToUTF8(parent.wstring());
}
