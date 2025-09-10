#pragma once

#define HAS holds_alternative

#include <vector>
#include <string>
#include <variant>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "lexer.cpp"
#include "utils.cpp"

// include the other file
#include "ast_symboltable.cpp"

using namespace std;
// ast for line : assignment, multiassignment, 
//valid Entities in a program
// struct Program;
// iterator for Tokens

using Iter = vector<Token>::iterator;

// struct Generic_type {    
// };

struct Expression;

struct Literal {
    Token value;
    Literal(void){};
    Literal(Token value) : value(value) {};
};

struct Variable {
    // add a type here
    Token name;
    Variable(void){};
    Variable(Token name) : name(name) {}
};

struct Binary_expr {
    Token op;
    Expression *left;
    Expression *right;
    Binary_expr(void){};
    Binary_expr(Token op, Expression left, Expression right);
};

struct Unary_expr {
    Token op;
    Expression *middle;
    Unary_expr(void){};
    Unary_expr(Token op, Expression middle);
};

struct Function_call {
    Token name;
    vector<Expression> args;
    Function_call(void){};
    Function_call(Token name);
};

struct Array_index {
    Token name;
    vector<Expression> args;
    Array_index(void){};
    Array_index(Token name);
};

using Expr_value = variant<Literal, Unary_expr, Binary_expr, Variable, Function_call, Array_index>;
struct Expression {
    Type type;
    Expr_value expr;
    bool assignable;
    Expression(void){};
    Expression(Expr_value expr, Type type) : type(type), expr(expr){};
};


struct Empty {};

struct Assignment {
    Expression left;
    Expression right;
    Token op;
};
// L0 = L1 = L2 = R
struct Multi_Assignment {
    vector<Expression> left;
    Expression right;
};

// Line, Codeblock, Function and Program definitions

struct Codeblock;
struct IfBlock;
struct WhileBlock;
struct DeclVars;
struct DeleteVars; 
struct ReturnLine;
using Basic_Line = variant<Expression, Assignment, Multi_Assignment, Empty>;
using Line = variant<Basic_Line, Codeblock, IfBlock, WhileBlock, 
            DeclVars, DeleteVars, ReturnLine>;
// class declarations
class SymbolTable;
class EntityTable;
struct StructInfo;


struct Codeblock {
    SymbolTable vars;
    vector<Line> lines;
};

struct ScopeInfo {
    Type return_type;
    vector<string> param_names;
    vector<Type> param_types;
    SymbolTable* parent;
};

struct Function {
    string name;
    Type return_type;
    vector<string> param_names;
    vector<Type> param_types;
    Codeblock code;
    Iter code_start; // start of codeblock
};

struct Signature {
    string name;
    Type return_type;
    // vector<string> param_names;
    vector<Type> param_types;
    Signature(){}
    Signature(string name, vector<Type> param_types, Type return_type):
        name(name), param_types(param_types), return_type(return_type){}
};

struct Program {
    Function main;
    unordered_map<string, Function> functions;
    // EntityTable entity_table;
    // unordered_map<string, StructInfo> struct_table;
};

struct IfBlock {
    bool hasElse;
    vector<Expression> conditions;
    vector<Codeblock> branches;
};

struct WhileBlock {
    Expression condition;
    Codeblock cb;
};

struct DeclVars {
    vector<Token> variables;
};

struct DeleteVars {
    vector<Token> variables;
};

struct ReturnLine {
    bool noneReturn;
    Expression expr;
};


/////////// Function_table
class Function_table {
    private:
        struct Func_Data {
            Signature signature;
            string cname;
        };
        unordered_multimap<string, Func_Data> table;
    public:
    
    // add to function table
    // return false if a duplicate signature exists
    // the cname is the name of the corresponding c function
    // if no name is provided, it will be automatically generated with $NAME
    bool add(Signature f);
    bool add(Signature f, const string& cname);
    // return type of function
    Option<Type> rtype(const string& name, const vector<Type>& params);

    // search the table using the name and parameters
    Option<Signature> search(const string& name, const vector<Type>& params);
    // obtain the cname "codegen name" of the function
    // precondition: the function exists
    string cname(const string& name, const vector<Expression>& args);
    // return number of function with said name
    int check(const string& name);
    
    void printAll();
};

