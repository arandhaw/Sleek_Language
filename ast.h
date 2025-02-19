#define HAS holds_alternative

#include <vector>
#include <string>
#include <variant>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "lexer.cpp"
#include "utils.cpp"
#include "symboltable.h"

using namespace std;
// ast for line : assignment, multiassignment, 
//valid Entities in a program
// struct Program;
// iterator for Tokens
using Iter = vector<Token>::iterator;

// encapsulation of a token iterator
// .val() - return string value
// struct Tokref {
//     Iter token;
//     string val(){
//         return token->value;
//     }
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
    string type;
    Expr_value expr;
    Expression(void){};
    Expression(Expr_value expr, string type) : type(type), expr(expr){};
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
using Basic_Line = variant<Expression, Assignment, Multi_Assignment, Empty>;
using Line = variant<Basic_Line, Codeblock>;
// using Codeblock = vector<Line>;

struct Codeblock {
    SymbolTable vars;
    vector<Line> lines;
};

struct Function {
    string name;
    string return_type;
    vector<string> param_names;
    vector<string> param_types;
    Codeblock code;
};

struct Signature {
    string name;
    string return_type;
    // vector<string> param_names;
    vector<string> param_types;
    Signature(){}
    Signature(string name, vector<string> param_types, string return_type):
        name(name), param_types(param_types), return_type(return_type)
    {}
};

struct Program {
    Function main;
    unordered_map<string, Function> functions;
};

// struct If_block {
//     vector<Expression> condition;
//     vector<CodeBlock> code;
// };


/////////////////////////////////////////////////

class Function_table {
    private:
        unordered_multimap<string, Signature> table;
    public:
    
    // add to function table
    // return false if a duplicate signature exists
    bool add(Signature f);
    // return type of function
    Option<string> rtype(string name, vector<string> params);

    // search the table using the name and parameters
    Option<Signature> search(string name, vector<string> params);
    // return number of function with said name
    int check(string name);

    void printAll();
    
};
