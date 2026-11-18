#include "rpf.hpp"
#include <fstream>
#include <array>
RpfHeaderInfo inspectRpf7(const std::filesystem::path& file) {
    RpfHeaderInfo info;
    std::ifstream in(file, std::ios::binary);
    if (!in) { info.message = "Не удалось открыть файл"; return info; }
    std::array<unsigned char, 16> h{};
    in.read(reinterpret_cast<char*>(h.data()), h.size());
    if (in.gcount() != 16) { info.message = "Файл короче заголовка RPF7"; return info; }
    if (h[0] != 'R' || h[1] != 'P' || h[2] != 'F' || h[3] != '7') { info.message = "Не найден magic RPF7"; return info; }
    auto u32 = [&](int off){ return uint32_t(h[off]) | (uint32_t(h[off+1])<<8) | (uint32_t(h[off+2])<<16) | (uint32_t(h[off+3])<<24); };
    info.valid = true; info.entryCount = u32(4); info.namesLength = u32(8); info.message = "Заголовок RPF7 распознан. Полный TOC-парсер ещё не подключён.";
    return info;
}
