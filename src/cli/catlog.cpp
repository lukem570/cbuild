#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>
#include <clocale>

const wchar_t* cat1 = 
L"                  \n"
L"                  \n"
L"     ╱ト⸜         \n"
L"    (⁰‿ₒ ア       \n"
L"     ト  ~\\      \n"
L"     U ᒐf_,)ノ    \n"
L"▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
;

const wchar_t* cat2 = 
L"                  \n"
L"    ⁰             \n"
L"     ⁰            \n"
L"   ⁰ _╱|𝁈———.⸜    \n"
L" -—  ᷇ 𝁈 \\   _ \\ \n"
L"  \\𝅪 -  ৴--`——৴   \n"
L"▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n"
;

const std::size_t catWidth = 18, catHeight = 7;

class Logger {
public:
    static void printCat() {
        wprintf(L"\e[s");
        wprintf(L"\r\e[%dA%ls", catHeight, cat2);
        wprintf(L"\e[u");
    }

    static void clearCat() {
        wprintf(L"\e[s");
        for (size_t y = 0; y < catHeight; y++) {
            for (size_t x = 0; x < catWidth; x++)
                wprintf(L" ");
            wprintf(L"\r\e[1A");
        }
        wprintf(L"\e[u");
    }

    static void debug(std::wstring mesg) {
        auto& inst = getInstance();

        std::size_t jump = std::max(0, (int)catHeight - (int)inst.advance);

        clearCat();
        if (++inst.advance >= catHeight+1)
            wprintf(L"\n");
        wprintf(L"\e[s");
        wprintf(L"\r\e[%dA\e[%dC %ls", jump, catWidth, mesg.c_str());
        wprintf(L"\e[u");

        printCat(); 
        fflush(stdout);
    }

private:
    static Logger* instance;
    std::size_t advance = 0;

    Logger() {
        setlocale(LC_ALL, "");

        for (size_t y = 0; y < catHeight; y++)
            wprintf(L"\n");
        printCat();
    }

    static Logger& getInstance() {
        if (!instance)
            instance = new Logger();
        return *instance;
    }
};

Logger* Logger::instance = nullptr;