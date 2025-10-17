#define CLI_BUILD

#include <string>
#include <unordered_map>
#include <sstream>
#include <filesystem>

#include <cbuild/cbuild.hpp>
#include <toml++/toml.hpp>

#include "path.cpp"

namespace fs = std::filesystem;
using ParsedToml = toml::v3::ex::parse_result;

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

void makeDirectory(fs::path dir) {
    if (!fs::exists(dir))
        fs::create_directory(dir);
}

#define BUILD_DIR "build/"
#define CBUILD_DIR BUILD_DIR ".cbuild/"

void copyBuild(const fs::path& from, const fs::path& to) {

    for (const auto& entry : fs::recursive_directory_iterator(from)) {

        fs::path filename = entry.path().filename();

        if (filename.string()[0] == '.')
            continue;

        fs::path dest = to / filename;

        if (fs::is_directory(entry.status())) {
            fs::create_directories(dest);
        } else if (fs::is_regular_file(entry.status())) {
            fs::create_directories(dest.parent_path());
            fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
        }
    }
}

void build(fs::path);

struct PackageData {
    std::vector<std::string> includes;
    std::vector<std::string> links;
};

std::vector<PackageData> initPackages(fs::path root) {

    if (!fs::exists(root / ".packages.toml")) {
        return {};
    }

    ParsedToml packages = toml::parse_file((root / ".packages.toml").string());
    std::vector<PackageData> ret;
    ret.reserve(packages.size());

    for (auto& [target, options] : packages) {

        auto optionsTable = options.as_table();

        fs::path packageRoot = root / CBUILD_DIR / target.data();
        std::string httpLink = optionsTable->get("link")->as_string()->get();
        bool nobuild = false;

        if (optionsTable->find("nobuild") != optionsTable->end())
            nobuild = optionsTable[nobuild].as_boolean();

        std::stringstream packagePull;
        packagePull << "git clone ";
        packagePull << httpLink << " ";
        packagePull << packageRoot;
        packagePull << "> /dev/null 2>&1";

        int err = system(packagePull.str().c_str());

        if (err) {
            printf("Failed to clone %s\n", target.data());
            exit(0);
        }

        if (!nobuild) {
            build(packageRoot);
            copyBuild(packageRoot / BUILD_DIR, root / BUILD_DIR);
        }

        printf("Built %s\n", target.data());

        ParsedToml cbuild = toml::parse((packageRoot / "cbuild.toml").string());
        auto cbuildPackageData = cbuild["package"].as_table();

        PackageData package;
        if (cbuildPackageData->find("inlcude") != cbuildPackageData->end())
            package.includes = semicolonSeparate(cbuildPackageData->get("include")->as_string()->get());
        if (cbuildPackageData->find("link") != cbuildPackageData->end() && !nobuild)
            package.includes = semicolonSeparate(cbuildPackageData->get("link")->as_string()->get());

        ret.push_back(package);
    }

    return ret;
}

void build(fs::path root = "./") {

    makeDirectory(root / BUILD_DIR);
    makeDirectory(root / CBUILD_DIR);

    std::vector<PackageData> packages = initPackages(root);

    ParsedToml cbuild = toml::parse_file((root / "cbuild.toml").string());

    if (!fs::exists("./build.cpp")) {
        printf("No 'build.cpp' found in project\n");
        exit(0);
    }

    fs::copy(
        removeExtension(getExecutablePath()) + "/libcbuild" SHARED_LIB_EXT, 
        "./build/.cbuild",
        fs::copy_options::overwrite_existing
    );

    CBuild::Shared build(
        "./build.cpp", 
        "build",
        CBuild::CompileOptions{.output=CBUILD_COMPILE_LOCATION}
    );

    build.linkLibrary("cbuild");
    build.linkDirectory(root / BUILD_DIR);

    for (auto& package : packages) {
        for (auto& include : package.includes) {
            build.includeDirectory(include);
        }

        for (auto& link : package.links) {
            build.linkLibrary(link);
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
    fs::remove_all(BUILD_DIR);
}

void copy_recursive(const fs::path& source, const fs::path& destination) {

    if (!fs::exists(destination)) {
        fs::create_directories(destination);
    }

    for (const auto& entry : fs::recursive_directory_iterator(source)) {
        const auto& path = entry.path();
        auto relative_path = fs::relative(path, source);
        fs::path target_path = destination / relative_path;

        
        if (fs::is_directory(path)) {
            fs::create_directories(target_path);
        } else if (fs::is_regular_file(path)) {
            fs::copy_file(path, target_path, fs::copy_options::overwrite_existing);
        }
        
    }
}

void install(toml::v3::ex::parse_result cbuild, std::string link) {

    printf("NOT IMPLEMENTED\n");
    return;
    
    if (!fs::exists(".packages.toml")) {
        std::ofstream packagesFile(".packages.toml");
        packagesFile.close();
    }
    
    auto packages = toml::parse_file(".packages.toml");

    auto packageCbuild = toml::parse_file("build/.cbuild/.temp/cbuild.toml");

    std::string name = packageCbuild["package"]["name"].as_string()->get();
    std::string version = packageCbuild["package"]["version"].as_string()->get();

    packages.insert_or_assign(name, toml::table{
        {"link", link},
        {"version", version}
    });

    std::ofstream packagesFile(".packages.toml");
    packagesFile << "# This file has been generated by cbuild\n";
    packagesFile << "# Please do not edit :)\n\n";
    packagesFile << packages;
    packagesFile.close();
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
            build();
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