#include <Windows.h>
#include <string_view>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#pragma comment(lib, "version.lib")
class LangCodePage {
public:
    WORD language;
    WORD code_page;
};
static constexpr int buffer_size = 16384;
static constexpr std::size_t lcp_size = sizeof(LangCodePage);
static constexpr std::string_view translation_string = "\\VarFileInfo\\Translation";
static constexpr std::string_view product_string = "\\StringFileInfo\\%04x%04x\\ProductVersion";
int wmain(int parameter_count, wchar_t **parameters) {
    if (parameter_count != 2)
        return 1;
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::wstring_view file_to_check = parameters[1];
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, file_to_check.data(), (int)file_to_check.size(), NULL, 0, NULL, NULL);
    std::string multibyte_file_string(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, file_to_check.data(), (int)file_to_check.size(), &multibyte_file_string[0], size_needed, NULL, NULL);
    char data[buffer_size];
    LPCSTR file = (LPCSTR)multibyte_file_string.c_str();
    if (!GetFileVersionInfoA(file, NULL, buffer_size, data))
        return 1;
    UINT codepage;
    LangCodePage *lang_code_page;
    SecureZeroMemory(&lang_code_page, lcp_size);
    if (!VerQueryValueA(data, translation_string.data(), (LPVOID*)&lang_code_page, &codepage))
        return 1;
    for (int i = 0; i < (codepage/lcp_size); i++) {
        char *version;
        char buffer[1024];
        UINT length;
        sprintf_s(buffer, product_string.data(), lang_code_page[i].language, lang_code_page[i].code_page);
        if (!VerQueryValueA(data, buffer, (LPVOID*)&version, &length))
            return 1;
        if (version != nullptr) {
            std::wcout << version << std::endl;
            return 0;
        }
    }
    return 1;
}
