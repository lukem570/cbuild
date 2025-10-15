#define CLI_BUILD

#include <string>
#include <unordered_map>
#include <sstream>

#include <cbuild/cbuild.hpp>
#include <filesystem>

#include "path.cpp"

namespace fs = std::filesystem;

enum class Action {
    eHelp,
    eBuild,
    eRun,
    eInstall,
    eClean,
};

std::unordered_map<std::string, Action> actionMap = {
    {"help",    Action::eHelp},
    {"build",   Action::eBuild},
    {"run",     Action::eRun},
    {"clean",   Action::eClean},
    {"install", Action::eInstall},
};

void listHelp() {
    printf(
        "Actions:\n"
        "\thelp    - provides a list of commands\n"
        "\tbuild   - runs the project's build script\n"
        "\trun     - runs the project's build script then the routine outlined in the 'cbuild.toml'\n"
        "\tinstall - installs a package from the web\n"
    );
}

void build() {
    if (!fs::exists("./build.cpp")) {
        printf("No 'build.cpp' found in project\n");
        exit(0);
    }

    if (!fs::exists("./build")) {
        fs::create_directory("build");
    }

    if (!fs::exists("./build/.cbuild")) {
        fs::create_directory("build/.cbuild");
    }

    if (fs::exists("./build/.cbuild/libcbuild" SHARED_LIB_EXT)) {
        fs::remove("./build/.cbuild/libcbuild" SHARED_LIB_EXT);
    }

    fs::copy(
        removeExtension(getExecutablePath()) + "/libcbuild" SHARED_LIB_EXT, 
        "./build/.cbuild"
    );

    CBuild::Shared build(
        "./build.cpp", 
        "build",
        CBuild::CompileOptions{.output=CBUILD_COMPILE_LOCATION}
    );

    build.linkLibrary("cbuild");

    // TODO: get from cbuild.toml
    build.includeDirectory("include"); 

    build.compile();

    void* handle = loadLibrary("./build/.cbuild/libbuild" SHARED_LIB_EXT);

    if (!handle) {
        printf("Failed to load build shared library\n");
        exit(0);
    }

    int (*buildFunc)() = (int (*)())getFunctionFromLibrary(handle, "build");

    if (!buildFunc) {
        printf("Failed to load build function\n");
        exit(0);
    }

    buildFunc();

    freeLibrary(handle);
}

void clean() {
    fs::remove_all("build");
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Expected an action run 'cbuild help' for a list of commands\n");
        return 0;
    }

    std::string action = argv[1];

    if (actionMap.find(action) == actionMap.end()) {
        printf("Invalid action '%s' run 'cbuild help' for a list of commands\n", action.c_str());
        return 0;
    }

    switch (actionMap[action]) {
        case Action::eHelp: {
            listHelp();
        } break;
        case Action::eBuild: {
            build();
        } break;
        case Action::eRun: {

        } break;
        case Action::eClean: {
            clean();
        } break;
        case Action::eInstall: {

        } break;
    }
}