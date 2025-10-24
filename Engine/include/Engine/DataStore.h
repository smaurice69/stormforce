#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace Engine
{
    class DataStore
    {
    public:
        using Json = nlohmann::json;
        using ReloadCallback = std::function<void(const std::wstring&, const Json&)>;

        explicit DataStore(std::wstring root);

        void Register(const std::wstring& relativePath, ReloadCallback callback);
        void PollChanges();
        const Json& Get(const std::wstring& relativePath) const;

    private:
        struct Entry
        {
            ReloadCallback callback;
            std::filesystem::file_time_type timestamp;
            Json json;
        };

        std::filesystem::path m_root;
        std::unordered_map<std::wstring, Entry> m_entries;

        void LoadEntry(const std::wstring& relativePath, Entry& entry);
    };
}
