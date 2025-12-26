#include "module.cpp"

#include <thread>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int main(int argc, const char* argv[]) {

    if (argc < 2) {
        Logger::debug(L"\e[1;31m[ERROR]  \e[0m Missing main file");
        return -1;
    }

    fs::path loc = argv[1];

    Module main = fs::absolute(loc);
}