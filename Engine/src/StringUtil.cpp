#include "Engine/StringUtil.h"

#include <codecvt>
#include <locale>

namespace Engine
{
    std::wstring ToWide(const std::string& input)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(input);
    }

    std::string ToNarrow(const std::wstring& input)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(input);
    }

    std::vector<std::wstring> SplitWide(const std::wstring& input, wchar_t delimiter)
    {
        std::vector<std::wstring> tokens;
        std::wstring token;
        for (wchar_t ch : input)
        {
            if (ch == delimiter)
            {
                if (!token.empty())
                {
                    tokens.emplace_back(token);
                    token.clear();
                }
            }
            else
            {
                token.push_back(ch);
            }
        }
        if (!token.empty())
        {
            tokens.emplace_back(token);
        }
        return tokens;
    }
}
