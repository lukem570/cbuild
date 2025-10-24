#include <cbuild/cbuild.hpp>
#include <filesystem>
namespace fs = std::filesystem;

namespace CBuild {

    void Binary::includeDirectory(std::string path) {
        includedDirectories.push_back(path);
    }

    void Binary::linkDirectory(std::string path) {
        linkedDirectories.push_back(path);
    }

    void Binary::linkLibrary(std::string alias) {
        linkedLibraries.push_back(alias);
    }

    void Binary::define(std::string definition) {
        definitions.push_back(definition);
    }

    int Binary::compile() {
        std::stringstream command;
        
        command << options.compiler.alias << " ";
        command << options.compiler.inputFlag << entry << " ";
        command << options.compiler.outputFlag << options.output << output() << " ";
        command << options.compiler.add << " ";

        for (auto& linkedLibrary : linkedLibraries) {
            command << options.compiler.linkLibraryFlag << linkedLibrary << " ";
        }

        for (auto& linkedDirectory : linkedDirectories) {
            command << options.compiler.linkDirectoryFlag << linkedDirectory << " ";
        }

        for (auto& includedDirectory : includedDirectories) {
            command << options.compiler.includeDirectoryFlag << includedDirectory << " ";
        }

        for (auto& definition : definitions) {
            command << options.compiler.defineFlag << definition << " ";
        }

        int ret = system(command.str().c_str());
        
        return ret;
    }

    std::string Shared::output() {
        return "/lib" + alias + SHARED_LIB_EXT + " " + options.compiler.sharedFlag;
    }

    std::string Static::output() {
        return "/lib" + alias + STATIC_LIB_EXT + " " + options.compiler.staticFlag;
    }

    std::string Executable::output() {
        return "/" + alias + EXECUTABLE_EXT;
    }
}
