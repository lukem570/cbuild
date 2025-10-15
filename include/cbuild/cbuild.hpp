#ifndef CBUILD_LIBRARY_HPP
#define CBUILD_LIBRARY_HPP

#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

#define CBUILD_COMPILE_LOCATION "./build/.cbuild/"

#if defined(_WIN32) || defined(_WIN64)
    #define SHARED_LIB_EXT ".dll"
    #define STATIC_LIB_EXT ".lib"
#elif defined(__APPLE__) && defined(__MACH__)
    #define SHARED_LIB_EXT ".dylib"
    #define STATIC_LIB_EXT ".a"
#elif defined(__linux__)
    #define SHARED_LIB_EXT ".so"
    #define STATIC_LIB_EXT ".a"
#else
    #error "Unknown or unsupported platform. Please define SHARED_LIB_EXT and STATIC_LIB_EXT manually."
#endif

#ifndef CLI_BUILD
extern "C" int build();
#endif

namespace CBuild {

    struct Context {
        
    };

    struct Compiler {
        std::string alias = "g++";
        std::string inputFlag = ""; 
        std::string outputFlag = "-o"; 
        std::string sharedFlag = "-shared";
        std::string staticFlag = "-static";
        std::string includeDirectoryFlag = "-I";
        std::string linkDirectoryFlag = "-L";
        std::string linkLibraryFlag = "-l";
        std::string standardFlag = "-std=";
        std::string add = "-fPIC -Wall -Wextra -Werror";
    };

    struct CompileOptions {
        Compiler compiler;
        std::string output = "./build";
    };

    class Shared;
    class Static;

    class Shared {
        public:
            Shared(std::string entry, std::string alias, CompileOptions options = {}) : entry(entry), alias(alias), options(options) {}

            void includeDirectory(std::string path);
            void linkDirectory(std::string path);
            void linkLibrary(std::string alias);

            int compile();        
        private:
            std::vector<std::string> linkedLibraries;
            std::vector<std::string> linkedDirectories = {CBUILD_COMPILE_LOCATION};
            std::vector<std::string> includedDirectories;
            std::string entry;
            std::string alias;
            CompileOptions options;

    };
}

#endif