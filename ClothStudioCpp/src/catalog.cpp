#include "catalog.hpp"
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;
static std::string lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); }); return s; }
std::string categoryFromName(const std::string& filename) {
    auto n = lower(filename);
    const std::pair<const char*, const char*> map[] = {
      {"^head", "Голова"}, {"^berd", "Маски"}, {"^hair", "Причёски"}, {"^uppr", "Торс / руки"},
      {"^lowr", "Штаны"}, {"^hand", "Сумки / парашюты"}, {"^feet", "Обувь"}, {"^teef", "Шарфы / цепи"},
      {"^accs", "Нижняя одежда"}, {"^task", "Бронежилеты"}, {"^decl", "Декали"}, {"^jbib", "Верхняя одежда"},
      {"p_head", "Головные уборы"}, {"p_eyes", "Очки"}, {"p_ears", "Серьги"}, {"p_lwrist", "Часы / браслеты"},
      {"p_rwrist", "Часы / браслеты"}, {"p_mouth", "Прочее"}, {"p_lhand", "Прочее"}, {"p_rhand", "Прочее"},
      {"p_hip", "Прочее"}, {"p_lfoot", "Прочее"}, {"p_rfoot", "Прочее"}
    };
    for (auto [prefix, category] : map) if (n.find(prefix) != std::string::npos) return category;
    return "Прочее";
}
std::string genderFromPath(const fs::path& path, const std::string& filename) {
    auto s = lower(path.string() + " " + filename);
    if (s.find("mp_f_freemode_01") != std::string::npos || s.find("female") != std::string::npos || s.find("_f_") != std::string::npos) return "Женское";
    if (s.find("mp_m_freemode_01") != std::string::npos || s.find("male") != std::string::npos || s.find("_m_") != std::string::npos) return "Мужское";
    return "Прочее";
}
ScanResult scanDlc(const fs::path& root) {
    ScanResult out;
    if (!fs::exists(root)) { out.errors.emplace_back("Путь не существует"); return out; }
    const std::vector<std::string> allowed{ ".ydd", ".ydr", ".yft", ".ytd" };
    try {
      fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied), end;
      for (; it != end; ++it) {
        if (!it->is_regular_file()) continue;
        auto ext = lower(it->path().extension().string());
        if (std::find(allowed.begin(), allowed.end(), ext) == allowed.end()) continue;
        CatalogItem item; item.path = it->path(); item.name = it->path().filename().string(); item.extension = ext;
        item.gender = genderFromPath(it->path().parent_path(), item.name); item.category = categoryFromName(item.name);
        item.texture = ext == ".ytd"; out.items.push_back(std::move(item));
      }
    } catch (const std::exception& e) { out.errors.emplace_back(e.what()); }
    std::sort(out.items.begin(), out.items.end(), [](const auto& a, const auto& b){ return a.name < b.name; });
    return out;
}
