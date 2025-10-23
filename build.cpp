#include <cbuild/cbuild.hpp>

#define ENTRY_LIB "src/cbuild.cpp"
#define ENTRY_CLI "src/main.cpp"

int build(CBuild::Context context) {

    CBuild::Shared shared (
        context,
        ENTRY_LIB, 
        "cbuild"
    );
    
    shared.includeDirectory("include");
    shared.compile();

    CBuild::Executable cli (
        context,
        ENTRY_CLI,
        "cbuild"
    );

    cli.includeDirectory("include");
    cli.includeDirectory("mod/toml/include");
    cli.linkLibrary("cbuild");
    cli.compile();

    return 0;
}