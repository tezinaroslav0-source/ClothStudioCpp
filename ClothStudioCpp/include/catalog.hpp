#pragma once
#include <filesystem>
#include <string>
#include <vector>

struct CatalogItem {
    std::filesystem::path path;
    std::string name;
    std::string extension;
    std::string gender;
    std::string category;
    bool texture{};
};
struct ScanResult {
    std::vector<CatalogItem> items;
    std::vector<std::string> errors;
};
ScanResult scanDlc(const std::filesystem::path& root);
std::string categoryFromName(const std::string& filename);
std::string genderFromPath(const std::filesystem::path& path, const std::string& filename);
