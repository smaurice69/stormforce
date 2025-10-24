#include "Engine/DataStore.h"

#include "Engine/StringUtil.h"

#include <fstream>
#include <utility>
#include <stdexcept>

namespace Engine
{
    DataStore::DataStore(std::wstring root)
        : m_root(std::move(root))
    {
    }

    void DataStore::Register(const std::wstring& relativePath, ReloadCallback callback)
    {
        Entry entry;
        entry.callback = std::move(callback);
        LoadEntry(relativePath, entry);
        m_entries[relativePath] = std::move(entry);
    }

    void DataStore::PollChanges()
    {
        for (auto& [path, entry] : m_entries)
        {
            auto fullPath = m_root / path;
            std::error_code ec;
            auto timestamp = std::filesystem::last_write_time(fullPath, ec);
            if (ec)
            {
                continue;
            }

            if (timestamp != entry.timestamp)
            {
                LoadEntry(path, entry);
            }
        }
    }

    const DataStore::Json& DataStore::Get(const std::wstring& relativePath) const
    {
        auto iter = m_entries.find(relativePath);
        if (iter == m_entries.end())
        {
            static Json empty;
            return empty;
        }
        return iter->second.json;
    }

    void DataStore::LoadEntry(const std::wstring& relativePath, Entry& entry)
    {
        auto fullPath = m_root / relativePath;
        std::ifstream stream(fullPath);
        if (!stream)
        {
            throw std::runtime_error("Failed to open data file");
        }

        Json json;
        stream >> json;
        entry.json = std::move(json);
        entry.timestamp = std::filesystem::last_write_time(fullPath);
        if (entry.callback)
        {
            entry.callback(relativePath, entry.json);
        }
    }
}
