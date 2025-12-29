/**
 * @file error.cpp
 * @author lukem
 * @date 2025-12-29
 * @brief Methods for printing errors
 */

#include "tokenizer.hpp"


/**
 * @brief Fatal error message for the tokenizer
 * @note Using `Logger::fatal` in the `Tokenizer` class is highly discouraged use this instead
 * 
 * @param location The location of the error
 * @param mesg The message to apply to the error
 */
void Tokenizer::fatal(Location location, std::wstring mesg) {
    stream.prevLine();
    auto loc = stream.getLocation();

    std::wstringstream err;

    err << loc.filename << " " << loc.str() << "\n";
    err << " \e[1;34m>\e[0m " << mesg << "\n"; 
    err << " \e[1;34m>\e[0m \n"; 
    err << " \e[1;34m>\e[0m " << loc.line - 1 << "  " << stream.readLine() << "\n";
    err << " \e[1;34m>\e[0m " << loc.line     << "  " << stream.readLine() << "\n";
    err << " \e[1;34m>\e[0m " << loc.line + 1 << "  " << stream.readLine();

    Logger::fatal(err.str());
}