#define CLI_BUILD

#include <string>
#include <unordered_map>
#include <sstream>
#include <filesystem>

#include <cbuild/cbuild.hpp>
#include <toml++/toml.hpp>

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
        "\tclean   - cleans the project\n"
        "\tinstall - installs a package from the web\n"
    );
}

std::vector<std::string> semicolonSeparate(const std::string& str) {
    std::vector<std::string> result;
    std::string current;

    for (char c : str) {
        if (c == ';') {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }

    result.push_back(current);

    return result;
}

void build(toml::v3::ex::parse_result cbuild) {
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

    if (cbuild.find("build") != cbuild.end()) {

        auto cbuildTable = cbuild["build"].as_table();

        if (cbuildTable->find("include") != cbuildTable->end()) {
            
            std::vector<std::string> includeDirs = 
                semicolonSeparate((*cbuildTable)["include"].as_string()->get());
            
            for (auto& include : includeDirs) {
                printf("include: %s\n", include.c_str());
                build.includeDirectory(include);
            }
        }
    }

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

void install(toml::v3::ex::parse_result cbuild, std::string alias) {
    cbuild.insert_or_assign("dependencies", toml::array{
        "cpptoml",
        "toml11",
        "Boost.TOML"
    });

    std::ofstream out("config.toml");
    out << cbuild;
    out.close();
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Expected an action run 'cbuild help' for a list of commands\n");
        return 0;
    }

    if (!fs::exists("cbuild.toml")) {
        printf("Could not find 'cbuild.toml'\n");
        return 0;
    }

    auto cbuild = toml::parse_file("cbuild.toml");

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
            build(cbuild);
        } break;
        case Action::eRun: {

        } break;
        case Action::eClean: {
            clean();
        } break;
        case Action::eInstall: {
            if (argc < 3) {
                printf("Missing argument\n");
                return 0;
            }
            std::string pack = argv[2];
            install(cbuild, pack);
        } break;
    }
}