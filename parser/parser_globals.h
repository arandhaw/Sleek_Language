#pragma once

#include "../lexer.cpp"
#include "../ast.cpp"
#include "../builtins.cpp"

#include <stack>
#include <unordered_map>
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

const int MAX_ARRAY_DIMENSIONS = 3;

const vector<vector<std::string>> operator_precedence = 
{{"or"},
{"and"},
{"==", "!="},
{"<=", ">=", "<", ">"},
{"+", "-"},
{"*", "/", "//", "%"},
{"^", "**"},
{"!", "u-"},
{"&", "++", "--"},
{"(", "[", "."}};      // function call, array index and attribute operator

const unordered_set<string> primitive_types = {"int", "float", "char", "bool", "byte", "none"};
// initialized upon startup
Type type_none;
Type type_int;
Type type_float;
Type type_byte;
Type type_bool;
Type type_char;
Type type_function;
Type type_string;

// defined on startup in main.cpp
unordered_map<std::string, int> precedence;

vector<Token> tokens;
// unordered_map<Type, Tuple_type> tuple_table;
// unordered_map<Type, Array_type> array_table;
unordered_map<Type, StructInfo> struct_table;
vector<Type> decl_order;
// defined in different scope:
// EntityTable entity_table;
// Function_table func_table

/// For struct table
struct struct_element {
    Type type;
    string field;
    // a type that contains no user defined types is concrete
    bool user_defined_type;
};

struct StructInfo {
    vector<struct_element> elements;
    bool flag_for_algo; // touched by algorithm or not
    bool fully_defined_type;    
};

////////////////////////
// functions declarations
/////////////////////////

struct ParseProgram;
ParseProgram parseProgram(Iter start);
struct ParseFunction;
ParseFunction parseFunction(Iter start);
struct CodeBlockOption;
CodeBlockOption parseCodeBlock(Iter start, const ScopeInfo&);
struct ParseBasicLine;
ParseBasicLine parseBasicLine(SymbolTable& vars, int line, Iter it);
struct ParseMulti;
ParseMulti parseMulti(SymbolTable& vars, Iter it);
struct ParseSimpleExpr;
ParseSimpleExpr parseSimpleExpr(SymbolTable& vars, Iter it);
struct ParseArrayLiteral;
ParseArrayLiteral parseArrayLiteral(SymbolTable& vars, Iter it);
struct SubArray;
SubArray parseArrayRecurse(SymbolTable& vars, Iter it, vector<Expression>& elements);
struct ParseIfOption;
ParseIfOption parseIf(SymbolTable& vars, int line, Iter start_it);
struct ParseWhileOption;
ParseWhileOption parseWhile(SymbolTable& vars, int line, Iter start_it);
struct ParseLoopOption;
ParseLoopOption parseLoop(SymbolTable& vars, int line, Iter start_it);
struct ParseDeclOption;
ParseDeclOption parseDecl(SymbolTable& vars, int line, Iter start_it);
struct ParseDeleteOption;
ParseDeleteOption parseDelete(SymbolTable& vars, int line, Iter start_it);
struct ParseTypeOption;
ParseTypeOption parseType(Iter start);
struct Entity_start;
bool parseStructs(const vector<Entity_start>& list);
void printStructs();

Type register_tuple(vector<Type> types, bool complete);
void printErrorLine(Iter it);
bool resolveMemoryScopes(bool hasElse, int line, 
            SymbolTable& parent_scope, const vector<Codeblock>& child_cb);
//helper functions
char get_closing_bracket(char c);
bool shunt(stack<Token>& operations, stack<Expression>& expr, Token token);
Option<Iter> endOfBlock(Iter start, const string& entity_name);

// struct definition (separate file for clarity)
#include "structs_def.cpp"

// didn't feel like making header files
#include "parser_helpers.cpp"
#include "parser_arrays_tuples.cpp"
#include "parser_expressions.cpp"
#include "parser_lines.cpp"
#include "parser_code_block.cpp"
#include "parser_functions.cpp"
#include "parser_program.cpp"
#include "parser_structs.cpp"