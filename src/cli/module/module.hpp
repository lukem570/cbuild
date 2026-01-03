/**
 * @file module.hpp
 * @author lukem
 * @date 2026-01-02
 * @brief Module definiton
 */

#ifndef CBUILD_MODULE_HPP
#define CBUILD_MODULE_HPP 1

#include "../parser/parser.hpp"

/**
 * @brief Modularized file
 * 
 * A module is a file that has parsed and
 * a headerfile has been generated.
 * 
 */
class Module {
public:
    Module(const fs::path& path) {}

private:
    Parser parser;
};

#endif