/**
 * @file preprocessor.hpp
 * @author lukem
 * @date 2025-12-25
 * @brief Expands macros for semantic analysis
 */

#ifndef CBUILD_PREPROCESSOR
#define CBUILD_PREPROCESSOR

#include <filesystem>
#include <fstream>

#include "../util/catlog.hpp"

namespace fs = std::filesystem;

class Preprocessor {
public:
    Preprocessor() = default;

    void include(const fs::path& path);

    void define(const std::wstring& name, const std::wstring& value = L"");
    void undefine(const std::wstring& name);

    std::wostringstream process(std::wistream& stream);

private:
    void handleMacro(const std::wstring& macro);
};

#endif