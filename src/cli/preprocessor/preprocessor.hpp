/**
 * @file preprocessor.hpp
 * @author lukem
 * @date 2025-12-25
 * @brief Expands macros for parsing
 */

#ifndef CBUILD_PREPROCESSOR
#define CBUILD_PREPROCESSOR

#include <filesystem>
#include <fstream>
#include <memory>
#include <ctime>
#include <queue>
#include <unordered_map>

#include "../util/catlog.hpp"
#include "../tokenizer/tokenizer.hpp"

namespace fs = std::filesystem;

class Macro {
public:
    Macro() = default;
    Macro(const std::vector<Token>& value) : value(value) {}

    std::vector<Token> expand() {
        return value;
    }

private:
    std::vector<Token> value;
};

class TokenizerStream {
public:
    TokenizerStream(const fs::path& entry) : tokenizer(entry) {}

    const Token& getNext() {
        if (doBackstep) {
            doBackstep = false;
            return last;
        }

        last = current;
        current = tokenizer.getNext();
        return current;
    }

    const Token& getLast() {
        return last;
    }

    void backstep() {
        doBackstep = true;
    }

private:
    Tokenizer tokenizer;
    Token last;
    Token current;
    bool doBackstep = false;
};

class Preprocessor {
public:
    Preprocessor(const fs::path& entry) : tokenizer(entry) {} 

    void include(const fs::path& path);

    void define(const std::wstring& name, const std::wstring& value = L"");
    void undefine(const std::wstring& name);

    Token getNext();

private:
    TokenizerStream tokenizer;
    std::queue<Token> expansionStack;

    std::vector<fs::path> includePaths;
    std::unordered_map<std::wstring, Macro> definitions = {
        /**
         * TODO:
         *  __FILE__
         *  __LINE__
         *  __DATE__
         *  __TIME__
         *  __TIMESTAMP__
         *  __cplusplus
         *  __func__
         */
        {L"__CBUILD__", Macro({
            Token({}, TokenType::eNumber, L"1"),
            Token({}, TokenType::eNumber, L"2"),
            Token({}, TokenType::eNumber, L"3"),
        })}
    };

    void handleMacro();
};

#endif