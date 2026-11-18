#pragma once
#include <filesystem>
#include <cstdint>
#include <string>
#include <vector>

struct RpfHeaderInfo { bool valid{}; uint32_t entryCount{}; uint32_t namesLength{}; std::string message; };
RpfHeaderInfo inspectRpf7(const std::filesystem::path& file);
