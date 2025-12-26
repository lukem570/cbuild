/**
 * @file catlog.hpp
 * @author lukem
 * @date 2025-12-26
 * @brief Logging with cats, meow.
 */

#ifndef CBUILD_CATLOG_HPP
#define CBUILD_CATLOG_HPP

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

    static void debug(std::wstring mesg);
    static void info(std::wstring mesg);
    static void warn(std::wstring mesg);
    static void success(std::wstring mesg);
    static void error(std::wstring mesg);

private:
    static Logger* instance;
    std::size_t advance = 0;

    Logger();
    static Logger& getInstance();

    static void printCat();
    static void clearCat();

    static void print(std::wstring mesg);
};

#endif