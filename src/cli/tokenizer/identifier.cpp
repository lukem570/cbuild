/**
 * @file identifier.cpp
 * @author lukem
 * @date 2025-12-29
 * @brief Methods for tokenizing identifiers
 */

#include "tokenizer.hpp"


/**
 * @brief Checks if a char is at the start of an identifier
 * 
 * @param c The char to check
 * @return `bool`
 */
bool Tokenizer::isIdentifierStart(wchar_t c) {
    return std::iswalpha(c) || c == L'_';
}


/**
 * @brief Checks if a char is part of an identifier
 * 
 * @param c The char to check
 * @return `bool`
 */
bool Tokenizer::isIdentifierPart(wchar_t c) {
    return std::iswalnum(c) || c == L'_';
}


/**
 * @brief Tokenizes an identifier and returns the identifier's token
 * 
 * @return `Token` The token
 */
Token Tokenizer::handleIdentifier() {
    std::wstringstream buf;

    auto location = stream.getLocation();
    
    for each(c, isIdentifierPart(c))
        buf << c;

    return Token(location, TokenType::eIdentifier, buf.str());
}