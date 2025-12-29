/**
 * @file string.cpp
 * @author lukem
 * @date 2025-12-29
 * @brief Methods for handling string and character literals
 */

#include "tokenizer.hpp"


/**
 * @brief Tokenizes a string literal into a token
 * 
 * @return `Token` The string literal token
 */
Token Tokenizer::handleString() {
    std::wstringstream buf;

    auto location = stream.getLocation();
    
    stream.next();
    for each(c, c != L'\"') 
        if (c == L'\\')
            buf << handleEscapeSequence();
        else 
            buf << c;
    stream.next();

    return Token(location, TokenType::eString, buf.str());
}


/**
 * @brief Tokenizes a character literal into a token
 * 
 * @return `Token` The character literal token
 */
Token Tokenizer::handleCharacter() {
    std::wstringstream buf;

    auto location = stream.getLocation();

    stream.next();
    for each(c, c != L'\'') 
        if (c == L'\\')
            buf << handleEscapeSequence();
        else 
            buf << c;
    stream.next();

    return Token(location, TokenType::eString, buf.str());
}