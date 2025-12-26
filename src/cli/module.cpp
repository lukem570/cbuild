#include <fstream>
#include <filesystem>
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include <tuple>

#include "util/catlog.hpp"
#include "preprocessor/preprocessor.hpp"

namespace fs = std::filesystem;

std::wstring trim(const std::wstring& str) {
    std::size_t first = str.find_first_not_of(L" \t\n\r\"<");
    if (first == std::wstring::npos) return L"";

    std::size_t last = str.find_last_not_of(L" \t\n\r\">");
    return str.substr(first, last - first + 1);
}


std::optional<fs::path> includeMacro(const std::wstring& line) {
    const std::string include = "#include";
    std::size_t idx = 0;

    for (auto c : line) {
        if (std::iswspace(c))
            continue;
        
        if (include[idx] != c) {
            idx = 0;
            continue;
        }

        if (include.size() <= ++idx) {
            return trim(line.substr(idx));
        }
    }

    return std::nullopt;
}

class Module {
public:
    Module(const fs::path& loc) {

        std::wifstream file(loc);
        if (!file.is_open()) {
            Logger::error(L"Could not open " + loc.wstring());
            exit(-1);
        }

        Preprocessor processor;
        auto value = processor.process(file);

        std::wofstream modf (loc.parent_path() / (L"mod_" + loc.filename().wstring()));
        modf << value.str();
        modf.close();

        file.clear();
        file.seekg(0, std::ios::beg);

        std::wstring line;
        while (std::getline(file, line)) {
            auto include = includeMacro(line);
            if (include.has_value()) {
                fs::path includeDir = loc.parent_path() / include.value();
                if (fs::exists(includeDir))
                    children.push_back(includeDir);
            }
        }

        Logger::success(L"Modularized " + fs::relative(loc).wstring());
    }
private:
    std::vector<Module> children;
};
