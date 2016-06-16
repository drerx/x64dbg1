#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "_global.h"

typedef std::string String;
typedef std::wstring WString;
typedef std::vector<String> StringList;
typedef std::vector<WString> WStringList;

class StringUtils
{
public:
    static StringList Split(const String & s, char delim, std::vector<String> & elems);
    static StringList Split(const String & s, char delim);
    static String Escape(const String & s);
    static String Trim(const String & s, String delim = StringUtils::WHITESPACE);
    static String TrimLeft(const String & s, String delim = StringUtils::WHITESPACE);
    static String TrimRight(const String & s, String delim = StringUtils::WHITESPACE);
    static String Utf16ToUtf8(const WString & wstr);
    static String Utf16ToUtf8(const wchar_t* wstr);
    static WString Utf8ToUtf16(const String & str);
    static WString Utf8ToUtf16(const char* str);
    static void ReplaceAll(String & s, const String & from, const String & to);
    static void ReplaceAll(WString & s, const WString & from, const WString & to);
    static String sprintf(const char* format, ...);
    static WString sprintf(const wchar_t* format, ...);
    static String ToLower(const String & s);
    static bool StartsWith(const String & h, const String & n);
    static String FromHex(const String & s, size_t size, bool reverse = false);
    static String ToHex(duint value);
    static String ToHex(void* buffer, size_t size, bool reverse = false);

    template<typename T>
    static String ToFloatingString(void* buffer)
    {
        auto value = *(T*)buffer;
        std::stringstream wFloatingStr;
        wFloatingStr << std::setprecision(std::numeric_limits<T>::digits10) << value;
        return wFloatingStr.str();
    }

    template<typename T>
    static String ToIntegralString(void* buffer)
    {
        auto value = *(T*)buffer;
        return ToHex(value);
    }


private:
    static const String WHITESPACE;
};

#endif //_STRINGUTILS_H