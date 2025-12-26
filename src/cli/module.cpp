#include <fstream>
#include <filesystem>
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include <tuple>

#include "catlog.cpp"

namespace fs = std::filesystem;

enum class StructureType {
    eClass,
    eStruct,
    eUnion,
};

struct Function {
    std::wstring  name;
    std::wstring  returnType;
    std::vector<std::pair<std::wstring, std::wstring>> parameters;
};

struct Structure {
    StructureType type;
    std::wstring name;
    std::vector<Function> children;
};

using Element = std::variant<Function, Structure>;

std::wstring trim(const std::wstring& str) {
    std::size_t first = str.find_first_not_of(L" \t\n\r\"<");
    if (first == std::wstring::npos) return L"";

    std::size_t last = str.find_last_not_of(L" \t\n\r\">");
    return str.substr(first, last - first + 1);
}

static std::vector<std::pair<std::wstring, std::wstring>>
parseParameters(const std::wstring& paramList) {
    std::vector<std::pair<std::wstring, std::wstring>> result;

    size_t start = 0;
    while (start < paramList.size()) {
        size_t comma = paramList.find(L',', start);
        if (comma == std::wstring::npos) comma = paramList.size();

        std::wstring param = trim(paramList.substr(start, comma - start));
        if (!param.empty()) {
            size_t space = param.find_last_of(L' ');
            if (space != std::wstring::npos) {
                result.emplace_back(
                    trim(param.substr(0, space)),
                    trim(param.substr(space + 1))
                );
            }
        }
        start = comma + 1;
    }
    return result;
}

static Function parseFunction(
    const std::wstring& signature,
    StructureType type = StructureType::eStruct
) {
    Function fn{};

    size_t openParen = signature.find(L'(');
    size_t closeParen = signature.find(L')', openParen);

    std::wstring before = trim(signature.substr(0, openParen));
    std::wstring params = signature.substr(openParen + 1, closeParen - openParen - 1);

    size_t lastSpace = before.find_last_of(L' ');
    fn.returnType = trim(before.substr(0, lastSpace));
    fn.name = trim(before.substr(lastSpace + 1));
    fn.parameters = parseParameters(params);

    return fn;
}

std::vector<Element> getElements(std::wstring reduced) {
    std::vector<Element> elements;

    size_t i = 0;
    while (i < reduced.size()) {

        // ---- STRUCTURES ----
        if (reduced.compare(i, 5, L"class") == 0 ||
            reduced.compare(i, 6, L"struct") == 0 ||
            reduced.compare(i, 5, L"union") == 0) {

            StructureType st;
            size_t keywordLen;

            if (reduced[i] == L'c') { st = StructureType::eClass; keywordLen = 5; }
            else if (reduced[i] == L's') { st = StructureType::eStruct; keywordLen = 6; }
            else { st = StructureType::eUnion; keywordLen = 5; }

            size_t nameStart = i + keywordLen + 1;
            size_t braceOpen = reduced.find(L'{', nameStart);
            std::wstring name = trim(reduced.substr(nameStart, braceOpen - nameStart));

            Structure s{};
            s.name = name;

            // Extract body
            int depth = 1;
            size_t bodyStart = braceOpen + 1;
            size_t j = bodyStart;

            while (j < reduced.size() && depth > 0) {
                if (reduced[j] == L'{') depth++;
                else if (reduced[j] == L'}') depth--;
                j++;
            }

            std::wstring body = reduced.substr(bodyStart, j - bodyStart - 1);

            // Parse functions inside structure
            size_t pos = 0;
            while ((pos = body.find(L'(', pos)) != std::wstring::npos) {
                size_t sigStart = body.rfind(L';', pos);
                if (sigStart == std::wstring::npos) sigStart = 0;
                else sigStart++;

                size_t brace = body.find(L'{', pos);
                if (brace == std::wstring::npos) break;

                std::wstring sig = trim(body.substr(sigStart, brace - sigStart));
                s.children.push_back(parseFunction(sig, st));

                pos = brace + 1;
            }

            elements.emplace_back(std::move(s));
            i = j;
            continue;
        }

        // ---- FREE FUNCTIONS ----
        if (reduced.find(L'(', i) != std::wstring::npos) {
            size_t open = reduced.find(L'(', i);
            size_t brace = reduced.find(L'{', open);
            if (brace != std::wstring::npos) {
                size_t sigStart = reduced.rfind(L';', open);
                if (sigStart == std::wstring::npos) sigStart = 0;
                else sigStart++;

                std::wstring sig = trim(reduced.substr(sigStart, brace - sigStart));
                elements.emplace_back(parseFunction(sig));
                i = brace + 1;
                continue;
            }
        }

        i++;
    }

    return elements;
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
            Logger::debug(L"\e[1;31m[ERROR]  \e[0m Could not open " + loc.wstring());
            exit(-1);
        }

        wchar_t c;
        while (file.get(c)) {
            if (std::iswspace(c)) value << " ";
            while (std::iswspace(c)) file.get(c);

            value << c;
        }

        // std::wofstream modf (loc.parent_path() / (L"mod_" + loc.filename().wstring()));
        // modf << value.str();
        // modf.close();

        auto elements = getElements(value.str());

        for (auto element : elements) {
            if (!std::holds_alternative<Function>(element)) 
                continue;
            
            auto& func = std::get<Function>(element);
            
            Logger::debug(L"name: " + func.name);
            Logger::debug(L"type: " + func.returnType);
        }

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

        Logger::debug(L"\e[1;32m[SUCCESS]\e[0m Modularized " + fs::relative(loc).wstring());
    }
private:
    std::wstringstream value;
    std::vector<Module> children;
};