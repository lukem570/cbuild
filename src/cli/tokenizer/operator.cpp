/**
 * @file operator.cpp
 * @author lukem
 * @date 2025-12-29
 * @brief Methods for handling operator tokenization
 */

#include "tokenizer.hpp"


/**
 * @brief Checks if a char is part of an operator
 * 
 * @param c The char to check
 * @return `bool`
 */
bool Tokenizer::isOperatorChar(wchar_t c) {
    static const std::wstring ops = L"+-*/%=<>!&|^~?:.,;()[]{}#\\";
    return ops.find(c) != std::wstring::npos;
}


/**
 * @brief Tokenizes an operator and returns the operator's token
 * 
 * @return `Token` The token
 */
Token Tokenizer::handleOperator() {
    std::wstringstream buf;

    auto location = stream.getLocation();

    wchar_t current = stream.read(); stream.next();
    wchar_t future = stream.read();

    buf << current;

    if (current == L'/' && future == L'/') {
        std::size_t start = stream.getLocation().line;
        for each(c, stream.getLocation().line == start)
            continue;

        return getNext();
    }

    if (current == L'/' && future == L'*') {
        stream.next();
        for each(c, !(stream.read() == L'*' && stream.peek() == L'/'))
            continue;

        stream.next(), stream.next();

        return getNext();
    }

    if (
        // comparison
        (current == L'=' && future == L'=') ||
        (current == L'!' && future == L'=') ||
        (current == L'<' && future == L'=') ||
        (current == L'>' && future == L'=') ||
        
        // access
        (current == L'-' && future == L'>') ||
        (current == L':' && future == L':') ||

        // unary
        (current == L'-' && future == L'-') ||
        (current == L'+' && future == L'+') ||

        // bitwise
        (current == L'>' && future == L'>') ||
        (current == L'<' && future == L'<') ||
        (current == L'&' && future == L'&') ||
        (current == L'|' && future == L'|') ||

        // concat
        (current == L'#' && future == L'#')
    ) {
        stream.next();

        buf << future;
    }

    return Token(location, TokenType::eOperator, buf.str());
}