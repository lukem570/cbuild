/**
 * @file escape.cpp
 * @author lukem
 * @date 2025-12-29
 * @brief Methods for handling escape sequence conversion
 */

#include "tokenizer.hpp"


/**
 * @brief Converts a wchar_t to a std::wstring
 * 
 * @param c The char
 * @return `std::wstring` The resulting string 
 */
std::wstring towstring(wchar_t c) {
    return std::wstring(1, c);
}


/**
 * @brief Converts octal escape sequences into their string repersentation
 * @note usage: `\nnn`
 * 
 * @return `std::wstring` the string repersentation 
 */
std::wstring Tokenizer::handleOctalSequence() {
    int value = stream.read() - L'0';

    for (int i = 0; i < 2; ++i) {
        wchar_t p = stream.peek();
        if (p < L'0' || p > L'7') break;
        stream.next();
        value = (value << 3) + (stream.read() - L'0');
    }

    return towstring(value);
}


/**
 * @brief Converts octal escape sequences into their string repersentation
 * @note usage: `\o{n...}`
 * 
 * @return `std::wstring` the string repersentation 
 */
std::wstring Tokenizer::handleRepeatedOctalSequence() {
    stream.next();

    if (stream.read() != L'{')
        fatal(stream.getLocation(), L"Expected '{' got '" + towstring(stream.read()) + L"' instead.");

    int value = 0;
    bool found = false;

    for (std::size_t i = 0; i < 3; i++) {
        stream.next();
        wchar_t p = stream.read(); 
        if (p == L'}') break;
        if (p < L'0' || p > L'7')
            fatal(stream.getLocation(), L"Invalid char '" + towstring(p) + L"' in \\o{} escape");

        found = true;
        value = (value << 3) + (p - L'0');
    }

    if (!found)
        fatal(stream.getLocation(), L"Empty \\o{} escape");

    return towstring(value);
}


/**
 * @brief Converts hexadecimal escape sequences into their string repersentation
 * @note usage: `\xnn`
 * 
 * @return `std::wstring` the string repersentation 
 */
std::wstring Tokenizer::handleHexSequence() {
    int value = 0;
    bool found = false;

    while (true) {
        wchar_t p = stream.peek();
        if (!std::iswxdigit(p)) break;

        stream.next();
        found = true;
        value = (value << 4) +
            (std::isdigit(p) ? p - L'0'
                                : std::towlower(p) - L'a' + 10);
    }

    if (!found)
        Logger::fatal(L"Expected hex digits after \\x");

    return towstring(value);
}


/**
 * @brief Converts hexadecimal escape sequences into their string repersentation
 * @note usage: `\x{n...}`
 * 
 * @return `std::wstring` the string repersentation 
 */
std::wstring Tokenizer::handleRepeatedHexSequence() {
    stream.next();
    int value = 0;
    bool found = false;

    while (true) {
        wchar_t p = stream.read();
        if (p == L'}') break;
        if (!std::iswxdigit(p))
            Logger::fatal(L"Invalid hex digit in \\x{} escape");

        found = true;
        value = (value << 4) +
            (std::isdigit(p) ? p - L'0'
                                : std::towlower(p) - L'a' + 10);
    }

    if (!found)
        Logger::fatal(L"Empty \\x{} escape");

    return towstring(value);
}


/**
 * @brief Converts unicode escape sequences into their string repersentation
 * @note usage: `\unnnn`
 * 
 * @return `std::wstring` the string repersentation 
 */
std::wstring Tokenizer::handleShortUnicodeSequence() {
    int value = 0;
    
    for (int i = 0; i < 4; ++i) {
        wchar_t p = stream.read();
        if (!std::iswxdigit(p))
            Logger::fatal(L"Invalid Unicode escape \\u");

        value = (value << 4) +
            (std::isdigit(p) ? p - L'0'
                                : std::towlower(p) - L'a' + 10);
    }

    return towstring(value);
}


/**
 * @brief Converts unicode escape sequences into their string repersentation
 * @note usage: `\Unnnnnnnn`
 * 
 * @return `std::wstring` the string repersentation 
 */
std::wstring Tokenizer::handleLongUnicodeSequence() {
    uint32_t value = 0;
    for (int i = 0; i < 8; ++i) {
        wchar_t p = stream.read();
        if (!std::iswxdigit(p))
            Logger::fatal(L"Invalid Unicode escape \\U");

        value = (value << 4) +
            (std::isdigit(p) ? p - L'0'
                                : std::towlower(p) - L'a' + 10);
    }

    return towstring(value);
}


/**
 * @brief Converts unicode escape sequences into their string repersentation
 * @note usage: `\u{n...}`
 * 
 * @return `std::wstring` the string repersentation 
 */
std::wstring Tokenizer::handleRepeatedUnicodeSequence() {
    stream.next();
    uint32_t value = 0;
    bool found = false;

    while (true) {
        wchar_t p = stream.read();
        if (p == L'}') break;
        if (!std::iswxdigit(p))
            Logger::fatal(L"Invalid Unicode digit in \\u{} escape");

        found = true;
        value = (value << 4) +
            (std::isdigit(p) ? p - L'0'
                                : std::towlower(p) - L'a' + 10);
    }

    if (!found)
        Logger::fatal(L"Empty \\u{} escape");

    return towstring(value);
}


/**
 * @brief Converts escape sequences into their string repersentation
 * 
 * @return `std::wstring` 
 */
std::wstring Tokenizer::handleEscapeSequence() {
    stream.next();
    wchar_t c = stream.read();

    switch (c) {
        case L'\'': return towstring(L'\'');
        case L'\"': return towstring(L'\"');
        case L'\?': return towstring(L'\?');
        case L'\\': return towstring(L'\\');
        case L'a': return towstring(L'\a');
        case L'b': return towstring(L'\b');
        case L'f': return towstring(L'\f');
        case L'n': return towstring(L'\n');
        case L'r': return towstring(L'\r');
        case L't': return towstring(L'\t');
        case L'v': return towstring(L'\v');
        case L'e': return towstring(L'\e');
    }

    if (c == L'o')
        return handleRepeatedOctalSequence();

    if (c >= L'0' && c <= L'7')
        return handleOctalSequence();

    if (c == L'x' && stream.peek() == L'{') 
        return handleRepeatedHexSequence();
        
    if (c == L'x') 
        return handleHexSequence();
    
    if (c == L'u' && stream.peek() == L'{') 
        return handleRepeatedUnicodeSequence();

    if (c == L'u') 
        return handleShortUnicodeSequence();

    if (c == L'U') 
        return handleLongUnicodeSequence();

    fatal(stream.getLocation(), L"Unknown escape sequence '" + towstring(c) + L"' in literal");
    return L"";
}
