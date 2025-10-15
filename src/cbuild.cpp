#include <cbuild/cbuild.hpp>

namespace CBuild {

    void Shared::includeDirectory(std::string path) {
        includedDirectories.push_back(path);
    }

    void Shared::linkDirectory(std::string path) {
        linkedDirectories.push_back(path);
    }

    void Shared::linkLibrary(std::string alias) {
        linkedLibraries.push_back(alias);
    }

    int Shared::compile() {
        std::stringstream command;

        command << options.compiler.alias << " ";
        command << options.compiler.inputFlag << entry << " ";
        command << options.compiler.outputFlag 
                << options.output << "/lib" << alias << SHARED_LIB_EXT << " ";
        command << options.compiler.add << " ";
        command << options.compiler.sharedFlag << " ";

        for (auto& linkedLibrary : linkedLibraries) {
            command << options.compiler.linkLibraryFlag << linkedLibrary << " ";
        }

        for (auto& linkedDirectory : linkedDirectories) {
            command << options.compiler.linkDirectoryFlag << linkedDirectory << " ";
        }

        for (auto& includedDirectory : includedDirectories) {
            command << options.compiler.includeDirectoryFlag << includedDirectory << " ";
        }

        int ret = system(command.str().c_str());
        
        return ret;
    }
}
