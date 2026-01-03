/**
 * @file parser.hpp
 * @author lukem
 * @date 2025-12-29
 * @brief Parser to process preprocessed tokens into syntax tree
 */

#ifndef CBUILD_PARSER_HPP
#define CBUILD_PARSER_HPP 1

#include <string>
#include <memory>
#include <vector>

#include "../preprocessor/preprocessor.hpp"

/**
 * @brief Declaration Syntax Tree (DST)
 *
 * Abstract base node for all declarations
 *
 * ```
 * DeclNode
 *  ├─ DeclMacro
 *  │   ├─ DeclObjectMacro
 *  │   └─ DeclFunctionMacro
 *  │
 *  ├─ DeclContext
 *  │   ├─ Root
 *  |   ├─ DeclNamespace
 *  │   └─ DeclRecord : Decltype
 *  │
 *  ├─ AliasContext
 *  |   └─ AliasNamespace
 *  │
 *  ├─ DeclVariable
 *  │
 *  ├─ DeclEnum
 *  │
 *  ├─ DeclFunction
 *  │
 *  ├─ DeclTemplate
 *  │
 *  └─ DeclType
 *      ├─ BuiltinType
 *      └─ AliasType
 * ```
 */
class DeclNode {
public:
    virtual ~DeclNode() = default;

    const Location& begin() const { return start; }
    const Location& end()   const { return stop; }

protected:
    Location start;
    Location stop;

    std::wstring name;
};


/**
 * @brief Abstract preprocessor declaration node
 */
class DeclMacro : public virtual DeclNode {
protected:
    DeclMacro() = default;
};


/**
 * @brief Object like macro declaration node
 * 
 * ```
 * #define MACRO ...
 * ```
 */
class DeclObjectMacro : public DeclMacro {

};


/**
 * @brief Function like macro declaration node
 * 
 * ```
 * #define MACRO(...) ...
 * ```
 */
class DeclFunctionMacro : public DeclMacro {

};


/**
 * @brief Abstract context declaration node
 * 
 * Context is just scope outside functions
 */
class DeclContext : public virtual DeclNode {
public:
    std::vector<std::shared_ptr<DeclNode>> children;
protected:
    DeclContext() = default;
};


/**
 * @brief Root declaration syntax tree node
 */
class Root : public DeclContext {

};


/**
 * @brief Namespace declaration node
 * 
 * ```
 * namespace Foo {}
 * ```
 */
class DeclNamespace : public DeclContext {

};


enum class RecordType {
    eUnion,
    eClass, 
    eStruct,
};

/**
 * @brief Abstruct structure declaration node
 */
class DeclRecord : public DeclContext, public DeclType {
public:
    RecordType type;
};


/**
 * @brief Struct declaration node
 * 
 * ```
 * struct Foo {}
 * ```
 */
class DeclStruct : public DeclRecord {

};


/**
 * @brief Class declaration node
 * 
 * ```
 * class Foo {}
 * ```
 */
class DeclClass : public DeclRecord {

};


/**
 * @brief Union declaration node
 * 
 * ```
 * union Foo {}
 * ```
 */
class DeclUnion : public DeclRecord {

};


/**
 * @brief Abstract context alias class
 */
class AliasContext : public virtual DeclNode {
protected:
    AliasContext() = default;
};


/**
 * @brief Namespace alias declaration node
 * 
 * ```
 * namespace Foo {}
 * namespace Bar = Foo;
 * ```
 */
class AliasNamespace : public AliasContext {
public:
    std::shared_ptr<DeclNamespace> target;
};


/**
 * @brief Variable declaration node
 * 
 * ```
 * type name;
 * ```
 * to
 * ```
 * extern type name;
 * ```
 */
class DeclVariable : public DeclNode {
public:
    std::shared_ptr<DeclType> type;
};


/**
 * @brief Function declaration node
 * 
 * ```
 * type name(args...);
 * ```
 */
class DeclFunction : public DeclNode {
public:
    std::shared_ptr<DeclType> returnType;
    std::vector<std::shared_ptr<DeclType>> parameters;
    bool variatic;
};


/**
 * @brief Template declaration node
 */
class DeclTemplate : public DeclNode {

};


/**
 * @brief Abstract type declaration node
 */
class DeclType : public virtual DeclNode {
protected:
    DeclType() = default;
};


/**
 * @brief Builtin c++ type node
 * 
 * e.g.
 * int, float, etc
 */
class BuiltinType : public DeclType {

};


/**
 * @brief Alias type node
 * 
 * e.g.
 * using Foo = Bar;
 * 
 * or 
 * typedef Bar Foo;
 */
class AliasType : public DeclType {
public:
    std::shared_ptr<DeclType> target;
};


/**
 * @brief Parses the c++ file
 */
class Parser {
public:
    void parse();

private:
    Preprocessor preprocessor;
    Root rootNode;
};

#endif