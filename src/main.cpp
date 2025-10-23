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

void pullRepo(fs::path path) {
    std::stringstream packagePull;
    packagePull << "git pull ";
    packagePull << path << " ";
    packagePull << "> /dev/null 2>&1";

    int err = system(packagePull.str().c_str());

    if (err) {
        printf("Failed to pull %s\n", path.filename().c_str());
        exit(0);
    }
}

struct PackageOptions {
    std::string httpLink;
    std::string target;
    std::string version;
    bool nobuild = false;
};

PackageOptions generateOptions(toml::v3::table& table) {
    PackageOptions options;

    if (table.find("link") == table.end()) {
        printf("Package missing link.\n");
        exit(0);
    }

    if (table.find("version") == table.end()) {
        printf("Package missing version.\n");
        exit(0);
    }

    if (table.find("target") == table.end()) {
        printf("Package missing target.\n");
        exit(0);
    }

    options.httpLink = table.get("link")->as_string()->get();
    options.version  = table.get("version")->as_string()->get();
    options.target   = table.get("target")->as_string()->get();

    if (table.find("nobuild") != table.end())
        options.nobuild = table.get("nobuild")->as_boolean();

    return options;
}

struct PackageData {
    std::vector<std::string> includes;
    std::vector<std::string> links;
    fs::path path;
};

PackageData generateData(ParsedToml& cbuild, fs::path& packageRoot, PackageOptions& packOpt) {
    PackageData data;

    auto packageData = cbuild["package"].as_table();

    if (packageData->find("include") != packageData->end())
        for (auto include : semicolonSeparate(packageData->get("include")->as_string()->get())) 
            data.includes.push_back(packageRoot / include);
        
    if (packageData->find("link") != packageData->end() && !packOpt.nobuild)
        for (auto link : semicolonSeparate(packageData->get("link")->as_string()->get())) 
            data.links.push_back(link);

    data.path = packageRoot;

    return data;
}

void build(fs::path root = "./", std::unordered_map<std::string, PackageData> packages = {}) {

    makeDirectory(root / BUILD_DIR);
    makeDirectory(root / CBUILD_DIR);

    CBuild::Context buildContext;
    CBuild::Context mainContext;

    if (!fs::exists(root / ".packages.toml"))
        return;

    ParsedToml packagesToml = toml::parse_file((root / ".packages.toml").string());

    for (auto& [target, options] : packagesToml) {

        std::string name = std::string(target.str());
        
        fs::path packageRoot = fs::path(CBUILD_DIR) / name;
        PackageOptions packOpt = generateOptions(*options.as_table());

        if (packages.find(name) != packages.end()) {
            if (packOpt.target == "main") {
                mainContext.linkedLibraries = packages[name].links;
                mainContext.includedDirectories = packages[name].includes;
                mainContext.linkedDirectories = {packages[name].path};
            } else {
                buildContext.linkedLibraries = packages[name].links;
                buildContext.includedDirectories = packages[name].includes;
                buildContext.linkedDirectories = {packages[name].path};
            }
            continue;
        }


        if (!fs::exists(packageRoot)) 
            cloneRepo(packOpt.httpLink, packageRoot);
        else
            pullRepo(packageRoot);

        ParsedToml cbuild = toml::parse_file((packageRoot / "cbuild.toml").string());
        PackageData packDat = generateData(cbuild, packageRoot, packOpt);

        if (!packOpt.nobuild) {
            build(packageRoot, packages);

            if (packOpt.target == "main") 
                copyBuild(packageRoot / BUILD_DIR, root / BUILD_DIR, true);
            else
                copyBuild(packageRoot / BUILD_DIR, root / CBUILD_DIR, true);

            printf("Built %s for %s\n", name.c_str(), packOpt.target.c_str());
        }

        packages[name] = packDat;

        if (packOpt.target == "main") {
            mainContext.linkedLibraries = packages[name].links;
            mainContext.includedDirectories = packages[name].includes;
            mainContext.linkedDirectories = {packages[name].path};
        } else {
            buildContext.linkedLibraries = packages[name].links;
            buildContext.includedDirectories = packages[name].includes;
            buildContext.linkedDirectories = {packages[name].path};
        }
    }

    ParsedToml cbuild = toml::parse_file((root / "cbuild.toml").string());

    if (!fs::exists(root / "build.cpp")) {
        printf("No 'build.cpp' found in project\n");
        exit(0);
    }

    CBuild::Shared build(
        buildContext,
        root / "build.cpp", 
        "build",
        CBuild::CompileOptions{
            .output=root / CBUILD_DIR
        }
    );

    build.linkDirectory(CBUILD_DIR);

    build.compile();

    void* handle = loadLibrary((root / fs::path(CBUILD_DIR) / "libbuild" SHARED_LIB_EXT).c_str());

    if (!handle) {
        printf("Failed to load build shared library\n");
        exit(0);
    }

    int (*buildFunc)(CBuild::Context context) = (int (*)(CBuild::Context))getFunctionFromLibrary(handle, "build");

    if (!buildFunc) {
        printf("Failed to load build function\n");
        exit(0);
    }

    mainContext.linkedDirectories.push_back(BUILD_DIR);

    fs::path current = fs::current_path();
    fs::current_path(root);
    buildFunc(mainContext);
    fs::current_path(current);

    freeLibrary(handle);
}

void clean() {
    fs::remove_all(BUILD_DIR);
}

void install(std::string httpLink, bool buildDep = false) {
    
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
        {"version", version},
        {"target", buildDep ? "build" : "main"},
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
    build << "int build(CBuild::Context context) {\n";
    build << "    \n";
    build << "    /* Build steps go here */\n";
    build << "    \n";
    build << "    return 0;\n";
    build << "}";

    install("https://github.com/lukem570/cbuild.git", true);
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