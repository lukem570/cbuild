/**
 * @file next.cpp
 * @author lukem
 * @date 2025-12-31
 * @brief Preprocessor implementation for next function
 */

#include "preprocessor.hpp"

void Preprocessor::handleMacro() {
    Token macro = tokenizer.getNext();
    Token last = tokenizer.getLast();

    // handle the # directive
    if (macro.location.line != last.location.line) {
        tokenizer.backstep();
        return;
    }
    
    std::vector<Token> body;

    Token current;
    while ((current = tokenizer.getNext()).location.line == macro.location.line) {
        body.push_back(current);
    }

    tokenizer.backstep();

    if (macro.value == L"include") {
        // TODO: add module header to stack
        return;
    }

    if (macro.value == L"define") {
        // TODO: 
        return;
    }

    if (macro.value == L"undef") {
        // TODO: 
        return;
    }

    if (macro.value == L"if") {
        // TODO: 
        return;
    }

    if (macro.value == L"elif") {
        // TODO: 
        return;
    }

    if (macro.value == L"else") {
        // TODO: 
        return;
    }

    if (macro.value == L"line") {
        // TODO: 
        return;
    }

    if (macro.value == L"embed") {
        // TODO: 
        return;
    }

    if (macro.value == L"warning") {
        std::wstringstream msg;
        for (auto entry : body) msg << entry.value;
        Logger::warn(msg.str());
        return;
    }

    if (macro.value == L"error") {
        std::wstringstream msg;
        for (auto entry : body) msg << entry.value;
        Logger::fatal(msg.str());
        return;
    }

    if (macro.value == L"ifdef") {
        if (definitions.find(body[0].value) == definitions.end())
            return;
        // TODO:

        return;
    }

    if (macro.value == L"ifndef") {
        if (definitions.find(body[0].value) == definitions.end())
            return;
        // TODO:

        return;
    }

    if (macro.value == L"endif")
        return;

    if (macro.value == L"pragma") {
        // TODO: add once
        return;
    }

    Logger::fatal(L"Unknown macro directive '#" + macro.value + L"'");
}

Token Preprocessor::getNext() {

    if (!expansionStack.empty()) {
        auto token = expansionStack.front();
        expansionStack.pop();
        return token;
    }

    Token current = tokenizer.getNext();

    if (
        current.location.firstOnLine && 
        current.type == TokenType::eOperator &&
        current.value == L"#"
    ) {
        handleMacro();

        return getNext();
    }

    if (
        current.type == TokenType::eIdentifier && 
        definitions.find(current.value) != definitions.end()
    ) {
        Logger::debug(L"expanded " + current.value);

        auto tokens = definitions[current.value].expand();
        for (auto token : tokens) expansionStack.push(token);

        if (expansionStack.empty()) return getNext();

        auto token = expansionStack.front();
        expansionStack.pop();
        return token;
    }

    return current;
}