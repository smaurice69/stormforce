#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Engine
{
    std::wstring ToWide(const std::string& input);
    std::string ToNarrow(const std::wstring& input);
    std::vector<std::wstring> SplitWide(const std::wstring& input, wchar_t delimiter);
}
