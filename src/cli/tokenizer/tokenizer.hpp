/**
 * @file tokenizer.hpp
 * @author lukem
 * @date 2025-12-28
 * @brief tokenizer classes for the preprocessor
 */

#ifndef CBUILD_TOKENIZER_HPP
#define CBUILD_TOKENIZER_HPP 1

#include <fstream>
#include <filesystem>
#include <string>
#include <cwchar>
#include <stack>

#include "../util/catlog.hpp"

namespace fs = std::filesystem;

#define each(var, cond) (               \
    wchar_t var = stream.read();        \
    !stream.eof() && cond;              \
    stream.next(), (var = stream.read())\
)

struct Location {
    Location() = default;
    Location(fs::path path) : filename(fs::relative(path).wstring()) {}

    std::wstring str() const {
        std::wstringstream buf;
        buf << line;
        buf << ":";
        buf << col;
        return buf.str();
    }

    std::wstring filename;
    std::size_t line = 1;
    std::size_t col  = 1;
};

enum class TokenType {
    eHeaderName,
    eIdentifier,
    eNumber,
    eCharacter,
    eString,
    eOperator,
    eEOF,
    eUnknown,
};

struct Token {
    Token() = default;
    Token(const Location& location, TokenType type, const std::wstring& value = L"") : 
        location(location), type(type), value(value) {}

    TokenType type;
    Location location;
    std::wstring value;
};

class Stream {
public:
    Stream(fs::path path) : location(path), file(path) {}

    wchar_t read() { return file.peek(); }
    wchar_t peek() { 
        next();
        wchar_t c = read();
        prev();
        return c;
    }

    void next() {
        file.get();
        wchar_t c = read();

        if (c == L'\n') {
            lineWidths.push(location.col);
            location.col   = 0;
            location.line += 1;
        } else {
            location.col  += 1;
        }
    }

    void prev() {
        file.clear();
        file.seekg(-1, std::ios::cur);
        wchar_t c = read();

        if (location.col <= 1) {
            location.col   = lineWidths.top();
            location.line -= 1;

            lineWidths.pop();
        } else {
            location.col  -= 1;
        }
    }

    void prevLine() {
        std::size_t curr = location.line;
        if (curr <= 1) {
            file.clear();
            file.seekg(0);
            return;
        }
        
        while (location.line == curr)
            prev();
    }

    std::wstring readLine() {
        while (location.col >= 1)
            prev();
        
        std::wstring line;
        std::getline(file, line);

        return line;
    }

    const Location& getLocation() { return location; }

    bool open() { return file.is_open(); }
    bool eof() { return file.eof(); }

private:
    Location location;
    std::stack<size_t> lineWidths;
    std::wifstream file;
};

class Tokenizer {
public:
    Tokenizer(const fs::path& path) : stream(path) {
        if (!stream.open())
            Logger::fatal(L"File '" + path.wstring() + L"' doesn't exist.");
    }

    Token getNext();

private:
    Stream stream;

    static bool isIdentifierStart(wchar_t c);
    static bool isIdentifierPart(wchar_t c);
    static bool isOperatorChar(wchar_t c);
    static bool isNumber(wchar_t current, wchar_t future);

    void fatal(Location location, std::wstring mesg);

    void skipWhitespace();

    Token handleIdentifier();
    Token handleNumber();
    Token handleString();
    Token handleCharacter();
    Token handleOperator();

    std::wstring handleEscapeSequence();
    std::wstring handleOctalSequence();
    std::wstring handleRepeatedOctalSequence();
    std::wstring handleHexSequence();
    std::wstring handleRepeatedHexSequence();
    std::wstring handleShortUnicodeSequence();
    std::wstring handleLongUnicodeSequence();
    std::wstring handleRepeatedUnicodeSequence();
};

#endif