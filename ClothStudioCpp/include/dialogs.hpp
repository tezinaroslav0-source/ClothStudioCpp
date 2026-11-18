#pragma once
#include <filesystem>
std::filesystem::path openFileDialog(const wchar_t* filter);
std::filesystem::path openFolderDialog();
