/**
 * @file number.cpp
 * @author lukem
 * @date 2025-12-29
 * @brief Methods for tokenizing numbers
 */

#include "tokenizer.hpp"


/**
 * @brief Checks if two chars are part of a number
 * 
 * @param current The first char
 * @param future  The second char
 * @return `bool`
 */
bool Tokenizer::isNumber(wchar_t current, wchar_t future) {
    return std::iswdigit(current) || (current == L'.' && std::iswdigit(future));
}


/**
 * @brief Tokenizes a number and returns it's token repersentation
 * 
 * @return `Token` The token
 */
Token Tokenizer::handleNumber() {
    std::wstringstream buf;

    auto location = stream.getLocation();

    for each(c, std::isalnum(c) || c == L'.' || c == L'_')
        buf << c;

    return Token(location, TokenType::eNumber, buf.str());
}