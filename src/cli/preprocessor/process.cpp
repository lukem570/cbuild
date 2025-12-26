/**
 * @file process.cpp
 * @author lukem
 * @date 2025-12-26
 * @brief Preprocessor implementation for process function
 */

#include "preprocessor.hpp"

std::wostringstream Preprocessor::process(std::wistream& stream) {

    std::wostringstream out;
    std::wstring line;

    bool commentFlag = false;

    while (std::getline(stream, line)) {

        std::size_t start = 0;

        HandleComment:
        if (commentFlag) {
            start = line.find(L"*/");
            if (start == std::wstring::npos) 
                continue;
            start += 2;
            commentFlag = false;
        }

        std::size_t nonSpace = line.find_first_not_of(L" \t\n\r");
        if (nonSpace != std::wstring::npos) {
            if (line[nonSpace] == '#') {
                handleMacro(line.substr(nonSpace));
                continue;
            }
        }
        
        for (std::size_t i = start; i < line.size(); i++) {
            if (i+1 < line.size()) {
                if (line[i] == L'/' && line[i+1] == L'/')
                    break;
                if (line[i] == L'/' && line[i+1] == L'*') {
                    commentFlag = true;
                    goto HandleComment;
                }
            }
            
            out << line[i];
        }
    }

    return out;
}

void Preprocessor::handleMacro(const std::wstring& line) {

    std::size_t start = line.find_first_not_of(L" \n\t\r", 1);
    if (start == std::wstring::npos)
        return;
    
    std::size_t warnOff = line.rfind(L"warning", start);
    if (warnOff != std::wstring::npos) {
        Logger::warn(line.substr(warnOff + 8));
        return;
    }

    std::size_t errOff = line.rfind(L"error", start);
    if (errOff != std::wstring::npos) {
        Logger::error(line.substr(errOff + 6));
        exit(1);
    }

    std::size_t pragmaOff = line.rfind(L"pragma", start);
    if (pragmaOff != std::wstring::npos) {
        Logger::debug(line.substr(pragmaOff + 7));
        return;
    }
}