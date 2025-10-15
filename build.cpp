#include <cbuild/cbuild.hpp>

#define ENTRY_LIB "src/cbuild.cpp"
#define ENTRY_CLI "src/main.cpp"

int build() {

    CBuild::Shared shared (
        ENTRY_LIB, 
        "cbuild"
    );
    
    shared.includeDirectory("include");
    shared.compile();

    CBuild::Executable cli (
        ENTRY_CLI,
        "cbuild"
    );

    cli.includeDirectory("include");
    cli.linkLibrary("cbuild");
    cli.compile();

    return 0;
}