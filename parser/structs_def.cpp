#pragma once

struct ParseArrayLiteral {
    bool valid;
    Array_literal array_literal;
    string cname;
    Iter end;
};

struct SubArray {
    bool valid;
    Iter end;
    vector<size_t> dims;    // dimensions of subarray (reverse order)
};

struct ParseIfOption {
    bool valid;
    IfBlock result;
    Iter end;
};

struct CodeBlockOption {
    bool valid;
    Codeblock codeblock;
    Iter end;
};

struct ParseWhileOption {
    bool valid;
    WhileBlock result;
    Iter end;
};


struct ParseLoopOption {
    bool valid;
    LoopBlock result;
    Iter end;
};

struct ParseDeclOption {
    DeclVars result;
    bool valid;
    Iter end;
};

struct ParseDeleteOption {
    DeleteVars result;
    bool valid;
    Iter end;
};

struct ParseSimpleExpr {
    Expression expr;
    Iter start;
    Iter end;
    bool valid;
};

struct ParseMulti {
    bool valid;
    Iter start;
    Iter end;
    vector<Expression> multi_expr;
};

struct ParseTypeOption {
    Type type;
    bool valid;
    Iter end;
};

struct ParseFunction {
    bool valid;
    Function result;
    Iter end;
};

struct ParseBasicLine {
    bool valid;
    bool empty;
    Basic_Line line;
    Iter end;
};

struct Function_start {
    Iter start;
    Iter name;
};

struct ParseProgram {
    bool valid;
    Program result;
};

struct Entity_start {
    Type name;
    Iter start;
};