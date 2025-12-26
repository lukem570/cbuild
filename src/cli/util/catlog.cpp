/**
 * @file catlog.cpp
 * @author lukem
 * @date 2025-12-26
 * @brief Implementations for the catlog
 */

#include "catlog.hpp"

Logger* Logger::instance = nullptr;


/**
 * @brief Prints a message with the debug symbol
 */
void Logger::debug(std::wstring mesg) {
    print(L"\e[1;35m[DEBUG]  \e[0m " + mesg);
}


/**
 * @brief Prints a message with the info symbol
 */
void Logger::info(std::wstring mesg) {
    print(L"\e[1;34m[INFO]   \e[0m " + mesg);
}


/**
 * @brief Prints a message with the warn symbol
 */
void Logger::warn(std::wstring mesg) {
    print(L"\e[1;33m[WARN]   \e[0m " + mesg);
}


/**
 * @brief Prints a message with the success symbol
 */
void Logger::success(std::wstring mesg) {
    print(L"\e[1;32m[SUCCESS]\e[0m " + mesg);
}


/**
 * @brief Prints a message with the error symbol
 */
void Logger::error(std::wstring mesg) {
    print(L"\e[1;31m[ERROR]  \e[0m " + mesg);
}


/**
 * @brief Constructs a new Logger object
 */
Logger::Logger() {
    setlocale(LC_ALL, "");

    for (size_t y = 0; y < catHeight; y++)
        wprintf(L"\n");
    printCat();
}


/**
 * @brief Gets the logger instance
 * 
 * @return `Logger&`
 */
Logger& Logger::getInstance() {
    if (!instance)
        instance = new Logger();
    return *instance;
}


/**
 * @brief Prints the cat askii art
 */
void Logger::printCat() {
    wprintf(L"\e[s");
    wprintf(L"\r\e[%dA%ls", catHeight, cat2);
    wprintf(L"\e[u");
}


/**
 * @brief Clears the cat askii art
 */
void Logger::clearCat() {
    wprintf(L"\e[s");
    for (size_t y = 0; y < catHeight; y++) {
        for (size_t x = 0; x < catWidth; x++)
            wprintf(L" ");
        wprintf(L"\r\e[1A");
    }
    wprintf(L"\e[u");
}


/**
 * @brief Prints the message to the screen
 * 
 * @param mesg The message to print
 */
void Logger::print(std::wstring mesg) {
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