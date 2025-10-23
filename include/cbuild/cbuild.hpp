#ifndef CBUILD_LIBRARY_HPP
#define CBUILD_LIBRARY_HPP

#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

#define BUILD_DIR "build/"
#define CBUILD_DIR BUILD_DIR ".cbuild/"
#define TEMP_DIR CBUILD_DIR ".temp/"

#if defined(_WIN32) || defined(_WIN64)
    #define SHARED_LIB_EXT ".dll"
    #define STATIC_LIB_EXT ".lib"
    #define EXECUTABLE_EXT ".exe"
#elif defined(__APPLE__) && defined(__MACH__)
    #define SHARED_LIB_EXT ".dylib"
    #define STATIC_LIB_EXT ".a"
    #define EXECUTABLE_EXT ""
#elif defined(__linux__)
    #define SHARED_LIB_EXT ".so"
    #define STATIC_LIB_EXT ".a"
    #define EXECUTABLE_EXT ""
#else
    #error "Unknown or unsupported platform. Please define SHARED_LIB_EXT and STATIC_LIB_EXT manually."
#endif

namespace CBuild {

    struct Context {
        std::vector<std::string> linkedLibraries;
        std::vector<std::string> linkedDirectories;
        std::vector<std::string> includedDirectories;
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
        std::string add = "-fPIC -Wall -Werror -Wl,-rpath,'$ORIGIN' -std=c++20";
    };

    struct CompileOptions {
        Compiler compiler;
        std::string output = "./build";
    };

    class Binary {
        public:
            Binary(Context context, std::string entry, std::string alias, CompileOptions options = {}) : 
                linkedLibraries(context.linkedLibraries),
                linkedDirectories(context.linkedDirectories),
                includedDirectories(context.includedDirectories),
                entry(entry), 
                alias(alias), 
                options(options) {}

            void includeDirectory(std::string path);
            void linkDirectory(std::string path);
            void linkLibrary(std::string alias);

            int compile();        
        protected:
            std::vector<std::string> linkedLibraries;
            std::vector<std::string> linkedDirectories;
            std::vector<std::string> includedDirectories;
            std::string entry;
            std::string alias;
            CompileOptions options;

            virtual std::string output() { return ""; }
    };

    class Shared : public Binary {
        public:
            Shared(Context context, std::string entry, std::string alias, CompileOptions options = {}) : Binary(context, entry, alias, options) {}

        private:
            std::string output() override;
    };

    class Static : public Binary {
        public:
            Static(Context context, std::string entry, std::string alias, CompileOptions options = {}) : Binary(context, entry, alias, options) {}

        private:
            std::string output() override;
    };

    class Executable : public Binary {
        public:
            Executable(Context context, std::string entry, std::string alias, CompileOptions options = {}) : Binary(context, entry, alias, options) {}

        private:
            std::string output() override;
    };
}

#ifndef CLI_BUILD
extern "C" int build(CBuild::Context context);
#endif

#endif