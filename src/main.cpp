#define CLI_BUILD

#include <string>
#include <unordered_map>
#include <sstream>
#include <filesystem>

#include <cbuild/cbuild.hpp>
#include <toml++/toml.hpp>

#include "path.cpp"

enum class Action {
    eHelp,
    eBuild,
    eRun,
    eInstall,
    eClean,
    eInit,
};

namespace fs = std::filesystem;
using ParsedToml = toml::v3::ex::parse_result;

struct PackageData {
    std::vector<fs::path> includes;
    std::vector<fs::path> links;
};

void build(fs::path);

std::unordered_map<std::string, Action> actionMap = {
    {"help",    Action::eHelp},
    {"build",   Action::eBuild},
    {"run",     Action::eRun},
    {"clean",   Action::eClean},
    {"install", Action::eInstall},
    {"init",    Action::eInit},
};

void listHelp() {
    printf(
        "Actions:\n"
        "\thelp    - provides a list of commands\n"
        "\tbuild   - runs the project's build script\n"
        "\trun     - runs the project's build script then the routine outlined in the 'cbuild.toml'\n"
        "\tclean   - cleans the project\n"
        "\tinstall - installs a package from the web\n"
        "\tinit    - creates a cbuild.toml and a build.cpp\n"
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

void copyBuild(const fs::path& from, const fs::path& to, bool libOnly = false) {

    for (const auto& entry : fs::directory_iterator(from)) {

        fs::path filename = entry.path().filename();

        if (filename.string()[0] == '.')
            continue;

        if (libOnly && !fs::is_directory(entry.status()) &&
            (filename.extension() != SHARED_LIB_EXT &&
             filename.extension() != STATIC_LIB_EXT))
            continue;

        fs::path dest = to / filename;

        if (fs::is_directory(entry.status())) {
            fs::create_directories(dest);
            copyBuild(from / filename, dest, libOnly);
        } else if (fs::is_regular_file(entry.status())) {
            fs::create_directories(dest.parent_path());
            fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
        }
    }
}

void cloneRepo(std::string httpLink, fs::path path) {
    std::stringstream packagePull;
    packagePull << "git clone ";
    packagePull << httpLink << " ";
    packagePull << path << " ";
    packagePull << "--recursive ";
    packagePull << "> /dev/null 2>&1";

    int err = system(packagePull.str().c_str());

    if (err) {
        printf("Failed to clone %s\n", httpLink.c_str());
        exit(0);
    }
}

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
            nobuild = optionsTable->get("nobuild")->as_boolean();

        if (!fs::exists(packageRoot)) 
            cloneRepo(httpLink, packageRoot);

        ParsedToml cbuildPackage = toml::parse_file((packageRoot / "cbuild.toml").string());
        auto cbuildPackageData = cbuildPackage["package"].as_table();

        PackageData package;

        if (cbuildPackageData->find("include") != cbuildPackageData->end())
            for (auto include : semicolonSeparate(cbuildPackageData->get("include")->as_string()->get())) 
                package.includes.push_back(packageRoot / include);
            
        if (cbuildPackageData->find("link") != cbuildPackageData->end() && !nobuild)
            for (auto link : semicolonSeparate(cbuildPackageData->get("link")->as_string()->get())) 
                package.links.push_back(link);

        if (!nobuild) {
            build(packageRoot);


            copyBuild(packageRoot / BUILD_DIR, root / LIB_DIR, true);
            printf("Built %s\n", target.data());
        }

        for (auto& include : package.includes)
            copyBuild(include, root / INCLUDE_DIR);


        ret.push_back(package);
    }

    return ret;
}

void build(fs::path root = "./") {

    makeDirectory(root / BUILD_DIR);
    makeDirectory(root / CBUILD_DIR);
    makeDirectory(root / INCLUDE_DIR);
    makeDirectory(root / LIB_DIR);

    std::vector<PackageData> packages = initPackages(root);

    ParsedToml cbuild = toml::parse_file((root / "cbuild.toml").string());

    if (!fs::exists(root / "build.cpp")) {
        printf("No 'build.cpp' found in project\n");
        exit(0);
    }

    CBuild::Shared build(
        root / "build.cpp", 
        "build",
        CBuild::CompileOptions{
            .output=root / CBUILD_DIR
        }
    );

    build.linkDirectory(root / CBUILD_DIR);
    build.linkDirectory(root / LIB_DIR);
    build.includeDirectory(root / INCLUDE_DIR);

    build.compile();

    void* handle = loadLibrary((root / fs::path(CBUILD_DIR) / "libbuild" SHARED_LIB_EXT).c_str());

    if (!handle) {
        printf("Failed to load build shared library\n");
        exit(0);
    }

    int (*buildFunc)() = (int (*)())getFunctionFromLibrary(handle, "build");

    if (!buildFunc) {
        printf("Failed to load build function\n");
        exit(0);
    }

    fs::path current = fs::current_path();
    fs::current_path(root);
    buildFunc();
    fs::current_path(current);

    freeLibrary(handle);
}

void clean() {
    fs::remove_all(BUILD_DIR);
}

void install(std::string httpLink) {
    
    if (!fs::exists(".packages.toml")) {
        std::ofstream packagesFile(".packages.toml");
        packagesFile.close();
    }
    
    fs::path packageRoot = TEMP_DIR;

    if (fs::exists(packageRoot))
        fs::remove_all(packageRoot);

    cloneRepo(httpLink, packageRoot);

    ParsedToml packages = toml::parse_file(".packages.toml");
    ParsedToml packageCbuild = toml::parse_file((packageRoot / "cbuild.toml").string());

    std::string name = packageCbuild["package"]["name"].as_string()->get();
    std::string version = packageCbuild["package"]["version"].as_string()->get();

    packages.insert_or_assign(name, toml::table{
        {"link", httpLink},
        {"version", version}
    });

    std::ofstream packagesFile(".packages.toml");
    packagesFile << "# This file has been generated by cbuild\n";
    packagesFile << "# please do not edit :)\n\n";
    packagesFile << packages;
}

void init() {
    std::ofstream cbuild("cbuild.toml");
    cbuild << "[package]\n";
    cbuild << "name=\"\"\n";
    cbuild << "version=\"0.0.1\"\n";
    cbuild << "description=\"\"\n";

    std::ofstream build("build.cpp");
    build << "#include <cbuild/cbuild.hpp>\n";
    build << "\n";
    build << "int build() {\n";
    build << "    \n";
    build << "    /* Build steps go here */\n";
    build << "    \n";
    build << "    return 0;\n";
    build << "}";

    install("https://github.com/lukem570/cbuild.git");
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
            if (argc < 3) {
                printf("Missing link\n");
                return 0;
            }
            std::string pack = argv[2];
            install(pack);
        } break;
        case Action::eInit: {
            init();
        } break;
    }
}