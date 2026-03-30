//
// Created by Kejin on 25-5-20.
//

// ReSharper disable CppDeprecatedEntity
#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <znative/ZBase.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <locale>
#include <codecvt>

NAMESPACE_DEFAULT
class StrUtils {
public:
    static std::string toHex(const unsigned long long val, bool lowercase = false, int width = 2) {
        char buf[33] = {0};
        if (lowercase) {
            snprintf(buf, sizeof(buf), "%0*llx", width, val);
        } else {
            snprintf(buf, sizeof(buf), "%0*llX", width, val);
        }
        return std::string(buf);
    }

    static std::string to0x(const unsigned long long val, bool lowercase = false, int width = 2) {
        return "0x" + toHex(val, lowercase, width);
    }

    static int hex2Int(const std::string &s) {
        return std::stoi(s, nullptr, 16);
    }

    static std::string lower(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    static std::string upper(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    static std::string trim(const std::string &s) {
        std::string result = s;
        result.erase(0, result.find_first_not_of(" \t\r\n"));
        result.erase(result.find_last_not_of(" \t\r\n") + 1);
        return result;
    }

    static std::string replace(const std::string &s, const std::string &from, const std::string &to) {
        std::string result = s;
        size_t start_pos = 0;
        if ((start_pos = result.find(from, start_pos)) != std::string::npos) {
            result.replace(start_pos, from.length(), to);
        }
        return result;
    }

    static std::string replaceAll(const std::string &s, const std::string &from, const std::string &to) {
        std::string result = s;
        size_t start_pos = 0;
        while ((start_pos = result.find(from, start_pos))!= std::string::npos) {
            result.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return result;
    }

    static int cmp(const std::string &a, const std::string &b, bool ignoreCase = false) {
        if (ignoreCase) {
#ifdef _WIN32
            return stricmp(a.c_str(), b.c_str());
#else
            return strcasecmp(a.c_str(), b.c_str());
#endif
        }
        return strcmp(a.c_str(), b.c_str());
    }

    static bool equals(const std::string &a, const std::string &b, bool ignoreCase = false) {
        return cmp(a, b, ignoreCase) == 0;
    }

    static bool startWith(const std::string &s, const std::string &prefix, bool ignoreCase = false) {
        if (ignoreCase) {
            return lower(s).find(lower(prefix)) == 0;
        }
        return s.find(prefix) == 0;
    }

    static bool endWith(const std::string &s, const std::string &suffix, bool ignoreCase = false) {
        if (ignoreCase) {
            return lower(s).rfind(lower(suffix)) == (s.length() - suffix.length());
        }
        return s.rfind(suffix) == (s.length() - suffix.length());
    }

    static bool contains(const std::string &s, const std::string &sub, bool ignoreCase = false) {
        if (sub.empty()) {
            return false;
        }
        if (ignoreCase) {
            return lower(s).find(lower(sub)) != std::string::npos;
        }
        return s.find(sub) != std::string::npos;
    }

    static std::vector<std::string> split(
        const std::string& str,
        const std::string& delimiter,
        bool skip_empty = true
    ) {
        std::vector<std::string> tokens;
        size_t pos = 0, last_pos = 0;

        while ((pos = str.find(delimiter, last_pos)) != std::string::npos) {
            std::string token = str.substr(last_pos, pos - last_pos);
            if (!token.empty() || !skip_empty) {
                tokens.push_back(token);
            }
            last_pos = pos + delimiter.length();
        }

        // 添加最后一个子串
        std::string token = str.substr(last_pos);
        if (!token.empty() || !skip_empty) {
            tokens.push_back(token);
        }

        return tokens;
    }
#if 0
    // 将string转换为wstring
    // TODO: 已经废弃，使用 icu 库替代
    static std::wstring toWideString(const std::string& input) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(input);
    }

    // 将wstring转换为string
    // TODO: 已经废弃，使用 icu 库替代
    static std::string toUtf8String(const std::wstring& input) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(input);
    }
#endif
};

NAMESPACE_END
#if defined(_MSC_VER)
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif
#endif //STRINGUTILS_H
