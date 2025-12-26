#include "module.cpp"

#include <thread>
#include <filesystem>
#include <fstream>

#include "util/util.cpp"
#include "preprocessor/preprocessor.cpp"

#include "util/catlog.hpp"

namespace fs = std::filesystem;

int main(int argc, const char* argv[]) {

    if (argc < 2) {
        Logger::error(L"Missing main file");
        return -1;
    }

    fs::path loc = argv[1];

    Module main = fs::absolute(loc);
}