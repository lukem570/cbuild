/**
 * @file next.cpp
 * @author lukem
 * @date 2025-12-28
 * @brief Methods for getting the next token
 */

#include "tokenizer.hpp"


/**
 * @brief Skips whitespaces in the stream
 */
void Tokenizer::skipWhitespace() {
    for each(c, std::iswspace(c))
        continue;
}


/**
 * @brief Gets the next token from the file
 * 
 * @return `Token` The token 
 */
Token Tokenizer::getNext() {

    std::size_t startLine = stream.getLocation().line;

    skipWhitespace();

    std::size_t endLine = stream.getLocation().line;
    stream.setNewLine(startLine != endLine);

    if (stream.eof())
        return Token(stream.getLocation(), TokenType::eEOF);
    
    if (isIdentifierStart(stream.read()))
        return handleIdentifier();

    if (isNumber(stream.read(), stream.peek()))
        return handleNumber();

    if (stream.read() == L'\"')
        return handleString();

    if (stream.read() == L'\'')
        return handleCharacter();

    if (isOperatorChar(stream.read()))
        return handleOperator();
    
    fatal(stream.getLocation(), L"Unknown token '" + towstring(stream.read()) + L"'");
    return Token();
}

