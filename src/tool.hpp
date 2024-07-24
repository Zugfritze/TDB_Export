#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

namespace Tool {
    inline bool equalsIgnoreCase(const std::string &str1, const std::string &str2) {
        if (str1.size() != str2.size()) {
            return false;
        }
        return std::ranges::equal(str1, str2,
                                  [](const char a, const char b) { return std::tolower(a) == std::tolower(b); });
    }

    inline std::optional<std::filesystem::path> findDirectoryIgnoreCase(const std::filesystem::path &baseDir,
                                                                        const std::string &targetDirName) {
        auto dirEntries = std::filesystem::directory_iterator(baseDir);
        const auto it = std::ranges::find_if(dirEntries, [&](const std::filesystem::directory_entry &entry) {
            return entry.is_directory() && equalsIgnoreCase(entry.path().filename().string(), targetDirName);
        });
        if (it != std::ranges::end(dirEntries)) {
            return it->path();
        }
        return std::nullopt;
    }

    template <typename T>
    concept StringType = std::same_as<T, std::string> || std::same_as<T, std::string_view>;

    template <StringType T> std::vector<T> split(const std::string_view &str, char delimiter) {
        std::vector<T> tokens;
        for (auto &&part : std::views::split(str, delimiter)) {
            tokens.emplace_back(part.begin(), part.end());
        }
        return tokens;
    }
} // namespace Tool
