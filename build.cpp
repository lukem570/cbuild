#include <cbuild/cbuild.hpp>

#define ENTRY "src/cbuild.cpp"

int build() {

    CBuild::Shared exec(
        ENTRY, 
        "cbuild"
    );
    
    exec.includeDirectory("include");

    exec.compile();

    return 0;
}