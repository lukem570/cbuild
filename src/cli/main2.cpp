#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <optional>
#include <stack>

namespace fs = std::filesystem;




bool isFunctionSignature(const std::string &line) {
    // Must contain '(' and ')' and not end with ';' (skip declarations)
    std::string trimmed;
    for (char c : line) if (c != ' ' && c != '\t') trimmed += c;

    return (trimmed.find('(') != std::string::npos &&
            trimmed.find(')') != std::string::npos &&
            trimmed.back() != ';');
}


std::string extractFunctionName(const std::string &line) {
    std::istringstream iss(line);
    std::string word, lastWord;
    while (iss >> word) lastWord = word;
    size_t pos = lastWord.find('(');
    if (pos != std::string::npos) lastWord = lastWord.substr(0, pos);
    return lastWord;
}

int main(int argc, const char* argv[]) {

    if (argc < 2) {
        printf("args\n");
        return -1;
    }

    fs::path filep = argv[1];

    std::ifstream file(filep);
    if (!file.is_open()) {
        std::cerr << "Could not open file.\n";
        return 1;
    }

    std::string line, current;
    std::stack<std::string> classStack;

    while (std::getline(file, line)) {
        std::string trimmed;
        for (char c : line) if (c != ' ' && c != '\t') trimmed += c;

        // Detect class start
        if (trimmed.find("class") == 0 || trimmed.find("struct") == 0) {
            std::istringstream iss(line);
            std::string tmp, className;
            iss >> tmp >> className;
            size_t pos = className.find('{');
            if (pos != std::string::npos) className = className.substr(0, pos);
            classStack.push(className);
            continue;
        }

        // Detect class end
        if (trimmed == "};" && !classStack.empty()) {
            classStack.pop();
            continue;
        }

        // Accumulate lines until we see '{'
        current += line + " ";
        if (trimmed.find('{') != std::string::npos) {
            if (isFunctionSignature(current)) {
                std::string funcName = extractFunctionName(current);
                if (!classStack.empty())
                    std::cout << classStack.top() << "::" << current << "\n";
                else
                    std::cout << funcName << "\n";
            }
            current.clear(); // reset for next function
        }
    }


    file.close();
    return 0;
}