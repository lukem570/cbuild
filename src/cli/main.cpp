#include "util/util.cpp"
#include "tokenizer/tokenizer.cpp"

//#include "preprocessor/preprocessor.cpp"
#include "util/catlog.hpp"

namespace fs = std::filesystem;

int main(int argc, const char* argv[]) {

    if (argc < 2) {
        Logger::error(L"Missing main file");
        return -1;
    }

    fs::path loc = argv[1];

    Tokenizer mainTokenizer(loc);
    Token token;
    while ((token = mainTokenizer.getNext()).type != TokenType::eEOF) {
        if (token.type == TokenType::eUnknown) {
            Logger::error(L"Unknown token type at " + token.location.str());
            break;
        }

        //Logger::debug(token.location.str() + L" " + token.value);
    }

    Logger::success(L"Built project");
} 