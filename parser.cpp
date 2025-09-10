#pragma once

#include <stack>
#include <unordered_map>
#include <iostream>
#include <string>

#include "lexer.cpp"
#include "ast.cpp"
#include "builtins.cpp"

using namespace std;

const vector<vector<std::string>> operator_precedence = 
{{"or"},
{"and"},
{"==", "!=", "<=", ">=", "<", ">"},
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

// globals
unordered_map<std::string, int> precedence;
Function_table func_table;
vector<Token> tokens;
unordered_map<string, StructInfo> struct_table;
vector<string> struct_order;
EntityTable entity_table;

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
struct ParseIfOption;
ParseIfOption parseIf(SymbolTable& vars, int line, Iter start_it);
struct ParseWhileOption;
ParseWhileOption parseWhile(SymbolTable& vars, int line, Iter start_it);
struct ParseDeclOption;
ParseDeclOption parseDecl(SymbolTable& vars, int line, Iter start_it);
struct ParseDeleteOption;
ParseDeleteOption parseDelete(SymbolTable& vars, int line, Iter start_it);
void printErrorLine(Iter it);
bool resolveMemoryScopes(bool hasElse, int line, 
            SymbolTable& parent_scope, const vector<Codeblock>& child_cb);
//helper functions
char get_closing_bracket(char c);
bool shunt(stack<Token>& operations, stack<Expression>& expr, Token token);

struct ParseTypeOption {
    Type_ast type;
    bool valid;
    Iter end;
};

// ParseTypeOption parseType(Iter start){
//     ParseTypeOption ret;
//     ret.valid = true;
//     Iter it = start;
//     if(it->type == NAME){
//         Named_type x;
//         x.
//         // if(primitive_types.count(it->value)){
//         //     x = {it->value, PRIMITIVE_TYPE};
//         // } 
//         // // todo: add structs and enum type
//         // else {
//         //     print("Error: Unknown type:", it->value);
//         //     ret.valid = false; return ret;
//         //     // entity_table.get(it->value);
//         // }
//         ret.end = it+1;
//     } else if(it->type == OPEN_BRACKET){
//         if(it->value == "("){
//             Tuple_type x;
            
//             if((it+1)->value == ")"){
//                 print("Error: () is not a valid type");
//                 ret.valid = false; return ret;
//             } 
//             while(true){
//                 ParseTypeOption result = parseType(it + 1);
//                 if(result.valid == false){ ret.valid = false; return ret; }
//                 x.elements.push_back(result.type);
//                 if(result.end->type == COMMA){
//                     it = result.end + 1;
//                     // (type,) is syntax for tuple with 1 element
//                     if(x.elements.size() == 1){ 
//                         if(it->type == CLOSED_BRACKET && it->value == ")"){
//                             break;
//                         }
//                     }
//                     continue;
//                 } else if(result.end->value == ")"){
//                     ret.end = result.end + 1;
//                     break;
//                 }
//             }
//         } else if(it->value == "["){
//             Array_type x;
//             ParseTypeOption result = parseType(it + 1);
//             if(result.valid == false){ ret.valid = false; return ret; }
//             x.type_ptr = new Type_ast(result.type);
//             it++;
//             if(it->type != COMMA){
//                 print("Invalid type, missing comma");
//                 ret.valid = false; return ret;
//             }
//             it++;
//             x.array_size = 1;
//             while(true){
//                 if(it->type != INT_LITERAL){
//                     print("Error, expected size in array declaration");
//                     ret.valid = false; return ret;
//                 }
//                 size_t dimension_size = stoi(it->value);
//                 x.array_size *= dimension_size;
//                 it++;
//                 if(it->type == COMMA){
//                     continue;
//                 } else if(it->value == "]"){
//                     it++;
//                     ret.end = it;
//                     break;
//                 } else {
//                     print("Error while parsing array type");
//                     ret.valid = false; return ret;
//                 }
//             }
//         } else {
//             ret.valid = false; return ret;
//         }
//         return ret;
//     }
//     // the type is also a string
//     for(Iter i = start; i < ret.end; i++){
//         ret.type.name.append(i->value);
//     }
//     return ret;
// }

///////////////////////////////////
///////////////////////////////////
struct ParseMulti {
    bool valid;
    Iter start;
    Iter end;
    vector<Expression> multi_expr;
};

char get_closing_bracket(char c){
    if(c == '{'){
        return '}';
    }
    if(c == '['){
        return ']';
    }
    if(c == '('){
        return ')';
    } else {
        cout << "ERROR in get_closing_bracket, char is " << c << endl;
        return ' ';
    }
}

// helper function for parsing
// given operator token, performs a shunt operation 
bool shunt(stack<Token>& operations, stack<Expression>& expr, Token token){
    if(token.type != BOP && token.type != UOP){
        cout << "Unexpected error: shunt token is not operator. Token: " << token.toString() << endl;
        return false;
    }
    while(!operations.empty() && precedence[token.value] <= precedence[operations.top().value]){
        Token top = operations.top(); operations.pop();
        if(top.type == BOP){
            if(expr.size() < 2){ return false; }
            Expression right = expr.top(); expr.pop();
            Expression left = expr.top(); expr.pop();
            // determine type
            Option<Type> res = func_table.rtype(top.value, vector<Type>{left.type, right.type});
            if(res.valid == false){ 
                print("Type error - no operator defined for", left.type, top.value, right.type); 
                return false;
            }
            Type type = res.result;
            
            Expression ex = Expression(Binary_expr(top, left, right), type);
            expr.push(ex);
        } else if (top.type == UOP){
            if(expr.size() < 1){ return false; }
            Expression only = expr.top(); expr.pop();
            // determine type
            Option<Type> res = func_table.rtype(top.value, vector<Type>{only.type});
            if(res.valid == false){ 
                print("Type error - no operator defined for", top.value, only.type); 
                return false;
            }
            Type type = res.result;
            Expression ex = Expression(Unary_expr(top, only), type);
            expr.push(ex);
        }
    }
    // finally, push token
    operations.push(token);
    return true;
}

// end is one after end
struct ParseSimpleExpr {
    Expression expr;
    Iter start;
    Iter end;
    bool valid;
};

// parse an expression with operators, including primitives, function calls, array indexing
// Returns on the following tokens: keyword, closing bracket, EOL, comma, or assignment
ParseSimpleExpr parseSimpleExpr(SymbolTable& vars, Iter it){
    ParseSimpleExpr ret;
    ret.start = it;
    ret.valid = true;
    // need to add expression and end of expression to ret
    enum state {START, ITEM, B_OPERATOR} prev, current;     // to validate whether expression is valid
    prev = START; // starting value only
    stack<Token> operations;
    stack<Expression> expr;

    // shunting yard algorithm
    
    for(; it != tokens.end(); it++){
        Token token = *it;
        // cout << token.toString() << endl;
        switch(token.type){
            case INT_LITERAL: {
                Expression ex = Expression(Literal(token), type_int);
                expr.push( ex );
                current = ITEM;
                break;
                }
            case FLOAT_LITERAL: {
                Expression ex = Expression(Literal(token), type_float);
                expr.push( ex );
                current = ITEM;
                break;
            }
            case CHAR_LITERAL: {
                Expression ex = Expression(Literal(token), type_char);
                expr.push( ex );
                current = ITEM;
                break;
            }
            case STRING_LITERAL: {
                print("String literals not yet supported");
                ret.valid = false; return ret;
                // Expression ex = Expression(Literal(token), Type_string);
                // expr.push( ex );
                // current = ITEM;
                // break;
            }

            case BOOL_LITERAL: {
                Expression ex = Expression(Literal(token), type_bool);
                expr.push( ex );
                current = ITEM;
                break;
            }

            case NAME: {
                if(entity_table.get(it->value) == FUNCTION){
                    Expression ex = Expression(Variable(token), type_function);
                    expr.push( ex );
                    current = ITEM;
                    break;
                }
                // TODO: type table
                Option<Type> optional = vars.use_var(it->value);
                Type var_type;
                if(optional.valid == false){
                    Var_status vs = vars.getStatus(it->value);
                    cout << "Error: cannot use variable " << it->value << endl;
                    switch(vs){
                        case DESTROYED:
                            cout << "Variable already destroyed" << endl;
                            break;
                        case MISSING:
                            cout << "Variable undefined" << endl;
                            break;
                        case DECLARED_NO_TYPE:
                            cout << "Type unknown" << endl;
                            break;
                        case DECLARED_W_TYPE:
                            cout << "Variable unitialized (garbage value)" << endl;
                            break;
                        default:
                            cout << "DEEP ERROR: unexpected result" << endl;
                            break;
                    }
                    ret.valid = false;
                    return ret;
                
                } else {
                    var_type = optional.result;
                }
                Expression ex = Expression(Variable(token), var_type);
                expr.push( ex );
                current = ITEM;
                break;
                }
            case BOP: case UOP: {
                bool success = shunt(operations, expr, token);
                if(!success){ 
                    ret.valid = false; 
                    cout << "Error in shunting " << token.toString() << endl;
                    return ret; 
                }
                if(token.type == BOP){ current = B_OPERATOR; } else { current = ITEM; }
                break;
                }
            case ASSIGNMENT: case KEYWORD: case CLOSED_BRACKET: case COMMA: case EOL: case END: {
                ret.end = it;
                goto exit;
                }
            case OPEN_BRACKET: {
                char opening = token.value[0];
                char expected = get_closing_bracket(token.value[0]);
                // open { - end of parsing
                if(opening == '{'){
                    ret.end = it;
                    goto exit;
                }
                // special case brackets like this: func()
                if((it-1)->type == NAME && it->value == "(" && (it+1)->value == ")"){
                    Function_call fnc;
                    fnc.name = *(it-1);
                    // no arguements
                    // TODO
                    Option<Type> type_maybe = func_table.rtype(fnc.name.value, vector<Type>{});
                    if(type_maybe.valid == false){
                        print("(0) No function with signature ", fnc.name.value, "()");
                    }
                    Expression ex(fnc, type_maybe.result);
                    expr.pop(); expr.push(ex);
                    // skip ahead
                    current = ITEM; 
                    it = it + 1;
                    goto ignore_checks; 
                }
                // recursively parse expression inside bracket
                // parse for comma separated expression
                ParseMulti result = parseMulti(vars, it + 1);
                // cout << "Size of multi-parse " << result.multi_expr.size() << endl;
                // cout << "result" << result.valid << endl;
                // check for validity
                if(result.valid == false){ ret.valid = false; return ret; }
                //check bracket is closed
                
                char closing = (*result.end).value[0];
                if(expected != closing){
                    cout << "Error: parenthesis mismatch on line " << token.line << endl;
                    cout << expected << " does not match " << closing << endl;
                    ret.valid = false; return ret;
                }
                
                if(prev == ITEM){   // this must be a function call or array index
                    Token fnc_name = *(it-1);
                    if( (it-1)->type != NAME){
                        cout << "Parsing error while parsing OPEN_BRACKET 1" << endl;
                    }
                    if(opening == '('){
                        Function_call fnc;
                        fnc.name = fnc_name;
                        fnc.args = result.multi_expr;
                        // type checking
                        vector<Type> arg_types;
                        arg_types.reserve(fnc.args.size());

                        for(const Expression& e : fnc.args){
                            arg_types.push_back(e.type);
                        }
                        Option<Type> type_maybe = func_table.rtype(fnc.name.value, arg_types);
                        if(type_maybe.valid == false){
                            print("No function with signature ", fnc.name.value, "()", arg_types);
                        }
                        Expression ex(fnc, type_maybe.result);
                        expr.pop();
                        expr.push(ex);
                    } else if(opening == '['){
                        Array_index arr;
                        arr.name = fnc_name;
                        arr.args = result.multi_expr;
                        // TODO type checking
                        vector<Type> arg_types;
                        arg_types.reserve(arr.args.size());

                        for(const Expression& e : arr.args){
                            arg_types.push_back(e.type);
                        }
                        Option<Type> type_maybe = func_table.rtype(arr.name.value, arg_types);
                        if(type_maybe.valid == false){
                            print("No function with signature ", arr.name.value, "[]", arg_types);
                        }
                        Expression ex(arr, type_maybe.result);
                        expr.pop();
                        expr.push(ex);
                    } else {
                        cout << "Parsing error while parsing OPEN_BRACKET 2" << endl;
                    }
                } else {    // must be a bracket or tuple
                    if(result.multi_expr.size() == 1){
                        expr.push( result.multi_expr.at(0) );
                    } else {
                        cout << "Error - tuples literals not yet supported" << endl;
                    }
                }
                current = ITEM; // in all cases, item
                it = result.end;
                goto ignore_checks;     // a break causes error in case of function
                } 
            default: {
                std::cout << "INTERNAL ERROR: unrecognized token" << std::endl;
                ret.valid = false; return ret;
                break;
                }
        }
        // check for validity
        if(current == B_OPERATOR && prev != ITEM){
            cout << "Error - operator is not preceded by an item. Token: " << token.toString() << endl;
            ret.valid = false; return ret;
        }
        if(current == ITEM && prev == ITEM){
            cout << "Error - item is precededed by item without operator. Token: " << token.toString() << endl;
            ret.valid = false; return ret;
        }
        ignore_checks:
        // update loop conditions
        prev = current;
        // end of main loop
    }

    exit:
    // empty stack
    while(!operations.empty()){
        Token top = operations.top(); operations.pop();
        // printToken(top);
        if(top.type == BOP){
            if(expr.size() < 2 ){ ret.valid = false; cout << "2" << endl; return ret; }
            Expression right = expr.top(); expr.pop();
            Expression left = expr.top(); expr.pop();
            // determine type
            Option<Type> res = func_table.rtype(top.value, vector<Type>{left.type, right.type});
            if(res.valid == false){ 
                print("Type error - no operator defined for", left.type, top.value, right.type); 
                ret.valid = false;
                return ret;
            }
            Type type = res.result;

            Expression ex = Expression(Binary_expr{top, left, right}, type);
            expr.push(ex);
        } else if (top.type == UOP){
            if(expr.size() < 1 ){ ret.valid = false; ret.valid = false; cout << "3" << endl; return ret; }
            Expression only = expr.top(); expr.pop();
            // determine type

            Option<Type> res = func_table.rtype(top.value, vector<Type>{only.type});
            if(res.valid == false){ 
                print("Type error - no operator defined for", top.value, only.type); 
                ret.valid = false; return ret;
            }
            Type type = res.result;

            Expression ex = Expression(Unary_expr(top, only), type);
            expr.push(ex);
        }
    }

    if(expr.size() != 1 || operations.size() != 0 || current != ITEM){
        ret.valid = false;
        print("Error in expression parsing");
    } else {
        ret.expr = expr.top();
    }
    return ret;
}

// parse a series of comma separated expressions
// return vector of expressions + info
// expression of at least length 1
ParseMulti parseMulti(SymbolTable& vars,  Iter it){
    ParseMulti ret;
    ret.start = it;
    ret.valid = true;
    while(true){
        ParseSimpleExpr result = parseSimpleExpr(vars, it);
        if(result.valid == false){
            ret.valid = false;
            
            // perhaps print line
            break;
        }
        ret.multi_expr.push_back(result.expr);
        if(result.end->type != COMMA){
            ret.end = result.end;
            break;
        }
        it = result.end + 1;
    }
    return ret;
}

struct ParseBasicLine {
    bool valid;
    bool empty;
    Basic_Line line;
    Iter end;
};

// parse a basic line (assignment/expression ended with EOL or }
// if line ends with EOL, end is 1 token after EOL
// if line ends with }, end is }
// calls parseSimpleExpression
ParseBasicLine parseBasicLine(SymbolTable& vars, int line, Iter it){
    ParseBasicLine ret;
    ret.valid = true;
    ret.empty = false;
    // empty line - line with EOL only, or simply nothing
    if(it->type == EOL){
        ret.valid = true;
        ret.empty = true;
        ret.line = Empty();
        ret.end = it + 1;
        return ret;
    // assignment operation like x = ...
    } else if(it->type == NAME && (it+1)->value == "="){
        ParseSimpleExpr second = parseSimpleExpr(vars, it+2);
        if(second.valid == false){
            cout << "Invalid expression in parseBasicLine" << endl;
            ret.valid = false;
            return ret;
        }
        
        // Assignment a = {first.expr, second.expr, op};
        int result = vars.assign(it->value, line, second.expr.type);
        if(result != 0){
            cout << "Error in assignment operation" << endl;
            ret.valid = false; return ret;
        } 
        Expression single = {Variable{*it}, second.expr.type};
        Assignment a{single, second.expr, *(it + 1)};
        ret.line = a;
        if(second.end->type == EOL){ ret.end = second.end + 1; }
        else if(second.end->value == "}"){ ret.end = second.end; }
        else { cout << "Invalid token at end of expression" << endl; ret.valid = false; }
        return ret;
    }
    // expression or left-side of assignment
    ParseSimpleExpr first = parseSimpleExpr(vars, it);
    if(first.valid == false){
        cout << "Invalid expression in parseBasicLine" << endl;
        ret.valid = false;
        return ret;
    }
    // if line has no equals sign, is expression
    if(first.end->type == EOL){
        ret.end = first.end + 1;
        ret.line = first.expr;
        return ret;
    } else if(first.end->value == "}"){
        ret.end = first.end;
        ret.line = first.expr;
        return ret;
    }
    // must be assignment or we have a error
    if(first.end->type != ASSIGNMENT){
        ret.valid = false;
        cout << "Error: Expected assignment or newline at end of expression" << endl;
        return ret;
    }
    it = first.end + 1;
    // parse the second half
    ParseSimpleExpr second = parseSimpleExpr(vars, it);
    if(second.valid == false){
        ret.valid = false;
        cout << "Invalid right side of assignment";
        return ret;
    }
    // check that second expression ends with EOL or }
    if(second.end->type == EOL){
        Token op = *first.end;
        Assignment a = {first.expr, second.expr, op};
        ret.end = second.end + 1;
        ret.line = a;
        //// add to symboltable, type check
        return ret;
    } else if(second.end->value == "}"){
        Token op = *first.end;
        Assignment a = {first.expr, second.expr, op};
        ret.end = second.end;
        ret.line = a;
        //// add to symboltable, type check
        return ret;
    }
    // handle multiassignment case
    // else if (second.end->value == "="){
    //     Multi_Assignment ma;
    //     ma.left.push_back(first.expr);
    //     ma.left.push_back(second.expr);
    //     it = second.end + 1;
    //     while(true){
    //         ParseSimpleExpr next = parseSimpleExpr(vars, it);
    //         if(next.valid == false){
    //             ret.valid = false;
    //             return ret;
    //         } 
    //         if(next.end->value == "="){
    //             it = next.end + 1;
    //             ma.left.push_back(next.expr);
    //         } else if (next.end->type == EOL || next.end->value == "}"){
    //             ma.right = next.expr;
    //             ret.end = next.end;
    //             break;
    //         } else {
    //             ret.valid = false;
    //             cout << "Unexpected end to multiassignment " << next.end->toString() << endl;
    //             return ret;
    //         }
    //     }
    //     ret.line = ma;
    //     return ret;
    // }

    else {
        cout << "Error in line parsing: unexpected token at end of line: " << second.end->toString() << endl;
        ret.valid = false;
        return ret;
    }
}

// void printLexedLine(Iter it){
//     Iter startOfLine = it;
//     while(startOfLine != tokens.begin()){
//         if((startOfLine - 1)->line == it->line){
//             startOfLine--;
//         } else {
//             break;
//         }
//     }
//     // now startOfLine is the beginning of the first line
//     while(startOfLine->type != EOL || startOfLine->type != END){
//         cout << it->value << " ";
//         it++;
//     }
//     cout << endl;
// }

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

// vars is the parent scope
ParseIfOption parseIf(SymbolTable& vars, int line, Iter start_it){
    ParseIfOption ret;
    Iter it = start_it;
    // parse if ... elif .. elif (etc)
    int num_branches = 0;
    do { 
        ParseSimpleExpr result = parseSimpleExpr(vars, it+1);
        if(result.valid == false){
            print("Invalid expression in condition of if statement");
            ret.valid = false; return ret;
        } else if(result.expr.type != type_bool){
            print("If condition is not a boolean");
            ret.valid = false; return ret;
        }
        else if(result.end->value != "{"){
            print("Invalid syntax, missing {");
            ret.valid = false; return ret;
        }
        ret.result.conditions.push_back( result.expr );
        it = result.end + 1;
        ScopeInfo si;
        si.return_type = vars.returnType;
        si.parent = &vars;
        CodeBlockOption result2 = parseCodeBlock(it, si);
        if(result2.valid == false){ 
            print("Invalid codeblock in if statement");
            ret.valid = false; return ret; }
        ret.result.branches.push_back( result2.codeblock );
        it = result2.end;

    } while(it->value == "elif");

    if(it->value == "else"){
        ret.result.hasElse = true;
        it++;
        if(it->value != "{"){
            ret.valid = false; return ret;
        }
        it++;
        ScopeInfo si;
        si.return_type = vars.returnType;
        si.parent = &vars;
        CodeBlockOption result2 = parseCodeBlock(it, si);
        if(result2.valid == false){ ret.valid = false; return ret; }
        ret.result.branches.push_back(result2.codeblock);
        it = result2.end;
    }
    
    // verify memory rules
    bool result = resolveMemoryScopes(ret.result.hasElse, line, vars, ret.result.branches);
    ret.valid = true;
    ret.end = it;
    return ret;
}

struct ParseWhileOption {
    bool valid;
    WhileBlock result;
    Iter end;
};

ParseWhileOption parseWhile(SymbolTable& vars, int line, Iter start_it){
    Iter it = start_it;
    ParseWhileOption ret;
    ParseSimpleExpr result = parseSimpleExpr(vars, it + 1);
    if(result.valid == false){
        print("Error parsing condition in while loop");
        ret.valid = false; return ret;
    }
    if(result.expr.type != type_bool){
        print("Error, while condition is not a boolean");
        ret.valid = false; return ret;
    }
    it = result.end;
    if((result.end)->value != "{"){
        print("Error in while loop, expected {");
        ret.valid = false; return ret;
    }
    ScopeInfo si;
    si.return_type = vars.returnType;
    si.parent = &vars;
    CodeBlockOption result2 = parseCodeBlock(result.end + 1, si);
    if(result2.valid = false){
        print("Error in codeblock of while loop");
        ret.valid = false; return ret;
    }
    // check memory - in particular, check that outside variables are not destroyed twice
    ret.valid = true;
    ret.result.condition = result.expr;
    ret.result.cb = result2.codeblock;
    ret.end = result2.end;
    return ret;
}

// update parent scope using information from child scope
// hasElse determines whether the child scopes contain all possible branches
// return boolean representing success
// only DEEP ERRORs (aka, unexpected errors) are possible
bool resolveMemoryScopes(bool hasElse, int line, 
            SymbolTable& parent_scope, const vector<Codeblock>& child_cb){
    // verify memory rules
    // defined variables
    if(hasElse){
        // check if all branches return
        bool allBranchesReturn = true;
        for(const auto& cb : child_cb){
            if(cb.vars.hasReturn == false){
                allBranchesReturn = false;
                break;
            } 
        }
        if(allBranchesReturn){
            parent_scope.hasReturn = true;
        }
        // check that variables were defined in all branches
        for(const auto& entry : child_cb[0].vars.table){
            const string& name = entry.first;
            const VarInfo& vi = entry.second;
            // variables that come from child scope and were defined
            if(vi.decl == -1 && vi.def != INT_MAX && vi.def != -1){
                // check that they were defined in all scopes
                // they will only be in symbol table if they were used in that scope
                bool existsInAllScopes = true;
                auto it = child_cb.begin();
                auto end = child_cb.end();
                it++;
                for(; it != end; it++){
                    bool exists = it->vars.table.count(name) == 1;
                    if(!exists){
                        existsInAllScopes = false;
                        break;
                    }
                }
                if(existsInAllScopes){
                    int result = parent_scope.assign(name, line, entry.second.type);
                    if(result != 0){
                        print("DEEP ERROR in ParseIf, 1");
                        return false;
                    }
                }
            }
        }
    }
    // check if variable was destroyed in any branch
    for(const auto& branch : child_cb){
        for(const auto& entry : branch.vars.table){
            const VarInfo& vi = entry.second;
            // variables destroyed in this scope
            if(vi.decl == -1 && vi.dest != INT_MAX && vi.dest != -1){
                int result = parent_scope.destroy(entry.first, line);
                // error shouldn't be possible, check just in case
                if(result != 0){
                    print("DEEP ERROR in parseIf, 2");
                    return false;
                }
            }
        }
    }

    // check if variable was destroyed in any branch
    for(const auto& branch : child_cb){
        for(const auto& entry : branch.vars.table){
            const VarInfo& vi = entry.second;
            // variables destroyed in this scope
            if(vi.decl == -1 && vi.dest != INT_MAX && vi.dest != -1){
                int result = parent_scope.destroy(entry.first, line);
                // error shouldn't be possible, check just in case
                if(result != 0){
                    print("DEEP ERROR in parseIf, 2");
                    return false;
                }
            }
        }
    }
    return true;
}

struct ParseDeclOption {
    DeclVars result;
    bool valid;
    Iter end;
};

ParseDeclOption parseDecl(SymbolTable& vars, int line, Iter start_it){
    // optimize by pre-allocating array
    int count = 0;
    for(Iter it = start_it; it->type != EOL; it++){
        if(it->type == COMMA){
            count++;
        }
    }
    ParseDeclOption ret;
    ret.result.variables.reserve(count);
    // parse comma separated list of variables
    Iter it = start_it + 1;
    do{
        if(it->type != NAME){
            ret.valid = false; return ret;
        } 
        ret.result.variables.push_back(*it);
        it++;
        if(it->type == EOL){ ret.end = it + 1; break; }
        else if(it->type == COMMA){ it++; }
        else { ret.valid = false; return ret; }
    } while(true);

    // deal with memory stuff - declare each variable in the decl statement
    for(Token& tok : ret.result.variables){
        cout << "adding " << tok.value << endl;
        bool valid = vars.declare(tok.value, line);
        if(!valid){
            cout << "Error in decl statement: Variable " << tok.value << " was already declared" << endl;
            ret.valid = false;
            return ret;
        }
    }
    ret.valid = true;
    return ret;
}

struct ParseDeleteOption {
    DeleteVars result;
    bool valid;
    Iter end;
};

ParseDeleteOption parseDelete(SymbolTable& vars, int line, Iter start_it){
    // optimize by pre-allocating array
    int count = 0;
    for(Iter it = start_it; it->type != EOL; it++){
        if(it->type == COMMA){
            count++;
        }
    }
    ParseDeleteOption ret;
    ret.result.variables.reserve(count);
    // parse comma separated list of variables
    Iter it = start_it + 1;
    do{
        if(it->type != NAME){
            ret.valid = false; return ret;
        } 
        ret.result.variables.push_back(*it);
        it++;
        if(it->type == EOL){ ret.end = it + 1; break; }
        else if(it->type == COMMA){ it++; }
        else { ret.valid = false; return ret; }
    } while(true);
    // deal with memory stuff - delete each variable in the delete statement
    for(Token& tok : ret.result.variables){
        int valid = vars.destroy(tok.value, line);
        if(valid == 0){
            continue;
        } else if(valid == 1){
            cout << "Error in decl statement: Variable " << tok.value << " was already deleted" << endl;
            ret.valid = false;
            return ret;
        } else if(valid == 2){
            cout << "Error in delete statement: Attempted to delete variable that doesn't exist: " << tok.value << endl;
            ret.valid = false; 
            return ret;
        }
    }
    ret.valid = true;
    ret.end = it + 1;
    return ret;
}

// top level function, parse {}, including inside a function
// end is token after }
CodeBlockOption parseCodeBlock(Iter start, const ScopeInfo& info) {
    CodeBlockOption ret;
    ret.valid = true;
    ret.codeblock.vars = SymbolTable{info.parent};
    SymbolTable& vars = ret.codeblock.vars; // for convieniance
    ret.codeblock.vars.returnType = info.return_type;
    ret.codeblock.vars.hasReturn = false;
    // add parameters into symbol table
    for(int i = 0; i < info.param_names.size(); i++){
        vars.assign(info.param_names[i], 0, info.param_types[i]);
    }
    Iter it;
    int line = 1;
    for(it = start; ; line++){
        if(it->type == END){
            // end loop
            cout << "Error: Reached end of file unexpectedly in parseCodeBlock" << endl;
            ret.valid = false;
            break;
        } else if(it->type == EOL){
            // skip token
            line--; it++;
            continue;
        } else if(it->type == CLOSED_BRACKET){
            // end loop
            if(it->value == "}"){
                ret.end = it + 1;
                break;
            } else {
                ret.valid = false;
                cout << "Error: unexpected closing bracket" << endl;
                goto endf;
            }
        // parse a scope
        } else if(it->value == "{"){
            ScopeInfo si;
            si.return_type = vars.returnType;
            si.parent = &vars;
            CodeBlockOption result = parseCodeBlock(it + 1, si);
            if(!result.valid){
                ret.valid = false;
                goto endf;
            } 
            Line x = {result.codeblock};
            ret.codeblock.lines.push_back(x);
            it = result.end;
            // check return
            if(result.codeblock.vars.hasReturn == true){
                ret.codeblock.vars.hasReturn = true;
            }
            // do memory stuff
            for(const auto& entry : result.codeblock.vars.table){
                const VarInfo& vi = entry.second;
                // variables that come from child scope
                if(vi.decl == -1){
                    // if variable was defined in this scope
                    if(vi.def != INT_MAX && vi.def != -1){
                        int result = vars.assign(entry.first, line, entry.second.type);
                        if(result != 0){
                            cout << "DEEP ERROR in parseCodeblock for {}, decl" << endl;
                            ret.valid = false; goto endf;
                        }
                    } 
                    // variable was destroyed in this scope, it 
                    // is destroyed in parent scope
                    if(vi.dest != INT_MAX && vi.dest != -1){
                        int result = vars.destroy(entry.first, line);
                        // error shouldn't be possible, check just in case
                        if(result != 0){
                            cout << "DEEP ERROR in parseCodeblock {}, delete" << endl;
                            ret.valid = false; goto endf;
                        }
                    }
                }
            }
            
            continue;
        // assignment or expression
        } else if (it->type != KEYWORD){
            cout << "Parsing basic line: ";
            printToken(*it);
            print(line);
            ParseBasicLine line_result = parseBasicLine(vars, line, it);
            if(line_result.valid == false){
                ret.valid = false;
                cout << "Invalid line" << endl;
                break;
            } else if(line_result.empty == false){
                // push line if not empty
                // print(ret.codeblock.lines.size());
                ret.codeblock.lines.push_back(line_result.line);
            }
            it = line_result.end;

        // if statement
        } else if(it->value == "if"){
            ParseIfOption result = parseIf(vars, line, it);
            if(result.valid == false){
                cout << "Error while parsing if statement" << endl;
                ret.valid = false;
                break;
            }
            ret.codeblock.lines.push_back(result.result);
            it = result.end;
        } else if(it->value == "while"){
            ParseWhileOption result = parseWhile(vars, line, it);
            if(result.valid == false){
                print("Error while parsing while loop");
                ret.valid = false;
                break;
            }
            ret.codeblock.lines.push_back(result.result);
            it = result.end;
        } else if(it->value == "decl"){
            ParseDeclOption result = parseDecl(vars, line, it);
            ret.codeblock.lines.push_back(result.result);
            if(result.valid == false){ 
                cout << "Error in decl statement" << endl;
                ret.valid = false; goto endf; }
            it = result.end;
        } else if(it->value == "delete"){
            ParseDeleteOption result = parseDelete(vars, line, it);
            ret.codeblock.lines.push_back(result.result);
            if(result.valid == false){ 
                cout << "Error in delete statement" << endl;
                ret.valid = false; goto endf; }
            it = result.end;
        // parse return statement
        } else if(it->value == "return"){
            if((it+1)->type == EOL){
                if(vars.returnType != type_none){
                    print("Error in return statement - expected type none");
                    ret.valid = false; goto endf;
                }
                ReturnLine line;
                line.noneReturn = true;
                ret.codeblock.lines.push_back(line);
                it = it + 2;
            } else {
                ParseSimpleExpr result = parseSimpleExpr(vars, it + 1);
                if(result.valid == false){ 
                    cout << "Error in delete statement" << endl;
                    ret.valid = false; goto endf; }
                if(result.expr.type != vars.returnType){
                    print("Error in return statement - type does not match signature");
                    ret.valid = false; goto endf;
                }
                ReturnLine line;
                line.noneReturn = false;
                line.expr = result.expr;
                ret.codeblock.lines.push_back(line);
                it = result.end + 1;
            }
            ret.codeblock.vars.hasReturn = true;
            
        } else {
            cout << "Error in parseCodeBlock - unknown keyword" << endl;
        }
    }
    endf:
    if(ret.valid == false){
        printErrorLine(it);
    }
    return ret;
}

void printErrorLine(Iter it){
    cout << "Error occured on line " << (*it).line << ":" << endl;
    cout << endl;
    while(it->type != EOL && it->type != END){
        cout << (*it).value << ' ';
        it = it + 1;
    }
    cout << endl;
}

struct ParseFunction {
    bool valid;
    Function result;
    Iter end;
};

// end is the '{' of the function
ParseFunction parseFunctionSignature(Iter start){
    
    ParseFunction ret;
    ret.valid = true;
    Iter index = start;
    if(start->type == NAME){
        ret.result.name = start->value;
    } else {
        cout << "Invalid function declaration (1)" << endl; 
        ret.valid = false; return ret;
    }
    index++;
    if(index->value != "("){  
        cout << "Invalid function declaration (2)" << endl; 
        ret.valid = false; return ret;
    }
    index++;
    while(index->value != ")"){
        if(index->type != NAME || (index + 1)->type != NAME){
            print("Invalid function declaration (3)"); 
            ret.valid = false; return ret;
        } else {
            // push type and parameter names
            if(!type_table.contains(index->value)){
                print("Invalid type in function declaration");
            } 
            Type t = type_table.get_type(index->value);
            ret.result.param_types.push_back(t);
            ret.result.param_names.push_back((index + 1)->value);
        }
        if((index + 2)->type == COMMA){
            index += 3;
        } else if((index + 2)->value == ")"){
            index = index + 2;
            break; 
        } else {
            cout << "Invalid function declaration (4)" << endl; 
            ret.valid = false; return ret; 
        }
    }
    index++;
    if(index->value != "->"){
        ret.result.return_type = type_none;
    } else {
        index++;
        // modify for tuples/variants later
        if(index->type != NAME){
            print("Invalid function declaration (6)"); 
            ret.valid = false; return ret;
        } 
        if(!type_table.contains(index->value)){
            print("Return type of function is not valid:", index->value);
        }
        ret.result.return_type = type_table.get_type(index->value);
        index++;
    }
    if(index->value == "{"){
    } else if(index->value == "\n" && (index + 1)->value == "}"){
        index += 1;
    } else {
        cout << "Invalid function declaration (7)" << endl; 
        ret.valid = false; return ret;
    }
    ret.result.code_start = index + 1;
    ret.end = index;
    return ret;
}

ParseFunction parseFunctionBody(Iter start, Function f){
    ParseFunction ret;
    ret.valid = true;
    ret.result = f;

    // insert the arguements of the function into the symbol table
    SymbolTable vars{};

    ScopeInfo si;
    si.return_type = f.return_type;
    si.param_names = f.param_names;
    si.param_types = f.param_types;
    si.parent = nullptr;
    
    CodeBlockOption result = parseCodeBlock(start, si);
    if(!result.valid){
        ret.valid = false; 
    } else if(result.codeblock.vars.hasReturn == false && ret.result.return_type != type_none){
        print("Error, function", ret.result.name, "missing return");
    } else {
        ret.result.code = result.codeblock;
        cout << result.codeblock.vars.table.size() << endl;
        ret.end = result.end;
    }
    return ret;
}

struct EndOfBlock {
    Iter end;
    bool valid;
};

// end of block is the token after closing bracket
EndOfBlock endOfBlock(Iter start, const string& entity_name){
    EndOfBlock ret;
    ret.valid = true;
    stack<char> brackets;
    Iter it = start;
    if(it->type != OPEN_BRACKET){
        print("Deep error - expected open bracket in endOfBlock");
        ret.valid = false; return ret;
    } 
    brackets.push(it->value[0]);
    it++;
    
    while(!brackets.empty()){
        if(it->type == OPEN_BRACKET){
            brackets.push(it->value[0]);
        } else if(it->type == CLOSED_BRACKET){
            if(get_closing_bracket(brackets.top()) == it->value[0]){
                brackets.pop();
            } else {
                print("Error in", entity_name, ", bracket mismatch on line", it->line);
                print("Opening bracket:", brackets.top(), "Closing bracket:", it->value);
                ret.valid = false; return ret;
            }
        } else if(it->type == END){
            print("Error in", entity_name, ", missing closing bracket", 
                get_closing_bracket(brackets.top()));
            ret.valid = false; return ret;
        }
        it++;
    }
    ret.end = it;
    ret.valid = true;
    return ret;
}

struct ParseProgram {
    bool valid;
    Program result;
};


// void parseStruct(const Entity_start& obj){
//     Iter it = obj.start;
//     StructInfo ret;
//     while(true){
//         if(it->type == EOL){
//             it++;
//             continue;
//         } else if(it->type == NAME){
//             Option<Entity> opt = entity_table.get(it->value);
//             if(opt.valid = false){
//                 print("Unrecognized type", it->value, "in struct", obj.name);
//             }
//             Entity& obj = opt.result;
//             if(obj == PRIMITIVE_TYPE){
//             }
//         } else {
//             print("Error while parsing struct", obj.name);
//             // TODO: return value
//         }
//         const string& type = (it->value);
//         if(type_table.contains(type)){         
//         }
//     }
// }

// step 1 - parse a struct
// any unfamiliar types, flag as unfamiliar
// Option<vector<string>> parseStructs(vector<Entity_start> list){
//     Option<vector<string>> ret;
//     ret.valid = true;

//     for(const Entity_start& entity : list){
//         Option<structInfo> opt = parseStruct(entity.start);
//         if(opt.valid == false){
//             ret.valid = false; return false;
//         }
//     }
// }

struct Entity_start {
    string name;
    Iter start;
};

// top level parsing function
ParseProgram parseProgram(Iter start){
    // func_table.printAll();
    bool foundMainFunction = false;
    ParseProgram ret;
    ret.valid = true;
    vector<Entity_start> struct_list;
    // vector<Entity_start> enum_list;

    for(Iter i = start; i != tokens.end(); i++){
        Token t = *i;
        if(t.type == EOL){ 
            continue; 
        } else if(t.value == "func"){
            ParseFunction result = parseFunctionSignature(i + 1);
            if(result.valid == false){
                print("Parsing error in function", (i + 1)->value);
                ret.valid = false; return ret;
            }
            if(result.result.name == "main"){
                if(foundMainFunction){
                    print("Error - main function defined multiple times");
                    ret.valid = false; return ret;
                }
                if(!(result.result.param_names.size() == 0 && result.result.return_type == type_none)){
                    print("Error - main function signature must have no parameters and return none");
                    ret.valid = false; return ret;
                }
                foundMainFunction = true;
                ret.result.main = result.result;
            } else {
                // check if function already defined
                if(ret.result.functions.count(result.result.name) != 0){
                    cout << "Function defined twice: " << result.result.name << endl;
                    ret.valid = false; return ret;
                }
                // add signature to function table
                Function &f = result.result;
                ret.result.functions.insert({f.name, f});
                func_table.add(Signature{f.name, f.param_types, f.return_type}, '$' + f.name);
                // add function to entity table
                bool success = entity_table.add(f.name, FUNCTION);
                if(!success){
                    Entity x = entity_table.get(f.name);
                    if(x != FUNCTION){
                        print("Error, function", f.name, "has same name as", toString(x), f.name);
                    }
                }
            }
            // verify that the brackets are correct
            string entity_name = "function " + result.result.name;
            EndOfBlock result2 = endOfBlock(result.end, entity_name); 
            if(result2.valid == false){
                print("Bracket mismatch in", entity_name);
                ret.valid = false; return ret;
            }
            i = result2.end; // move up the iterator
        
        // initial parsing for structs
        } else if(t.value == "struct"){
            if((i+1)->type != NAME && (i+2)->value != "{"){
                print("Invalid struct declaration on line", i->line);
                ret.valid = false; return ret;
            }
            bool valid = entity_table.add((i+1)->value, STRUCT);
            if(valid == false){
                Entity z = entity_table.get((i+1)->value);
                print("Error while parsing struct", (i+2)->value, ", name is defined twice");
                ret.valid = false; return ret;
            }
            // name of struct, iterator to beginning of block
            struct_list.push_back({(i+1)->value, i+3});
        } else if(t.type == END){
            break;
        } else {
            print("Invalid token found");
            ret.valid = false; return ret;
        }
    }
    func_table.printAll();
    // finish parsing structs, enums, global scope, etc
    // parseStructs(struct_list);


    // complete parsing of functions
    // start with main
    ParseFunction result = parseFunctionBody(ret.result.main.code_start, ret.result.main);
    if(!result.valid){
        print("Error while parsing main function");
        ret.valid = false; return ret;
    } 
    ret.result.main.code = result.result.code;
    // parse the rest of the functions
    for(auto& pair : ret.result.functions){
        ParseFunction result = parseFunctionBody(pair.second.code_start, pair.second);
        if(!result.valid){
            print("Error while parsing function", pair.second.name);
            ret.valid = false; return ret;
        } 
        pair.second.code = result.result.code;
    }
    
    ret.valid = true;
    return ret;
}