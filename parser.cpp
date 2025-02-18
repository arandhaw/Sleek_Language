#include <stack>
#include <unordered_map>
#include <iostream>
#include <string>

#include "lexer.cpp"
#include "ast.cpp"
#include "builtins.cpp"

using namespace std;

// globals
vector<vector<std::string>> operator_precedence = 
{{"or"},
{"and"},
{"==", "!=", "<=", ">=", "<", ">"},
{"+", "-"},
{"*", "/", "//", "%"},
{"^", "**"},
{"!", "u-"},
{"&", "++", "--"},
{"(", "[", "."}};      // function call, array index and attribute operator

unordered_map<std::string, int> precedence;
Function_table func_table;
////////////////////////
// functions declarations
/////////////////////////

struct ParseProgram;
ParseProgram parseProgram(const vector<Token>& tokens, Iter start);
struct ParseFunction;
ParseFunction parseFunction(const vector<Token>& tokens, Iter start);
struct CodeBlockOption;
CodeBlockOption parseCodeBlock(const vector<Token>& tokens, SymbolTable& parent, Iter start);
struct ParseBasicLine;
ParseBasicLine parseBasicLine(const vector<Token>& tokens, SymbolTable& vars, int line, Iter it);
struct ParseMulti;
ParseMulti parseMulti(const vector<Token>& tokens, SymbolTable& vars, Iter it);
struct ParseSimpleExpr;
ParseSimpleExpr parseSimpleExpr(const vector<Token>& tokens, SymbolTable& vars, Iter it);


//helper functions
char get_closing_bracket(char c);
bool shunt(stack<Token>& operations, stack<Expression>& expr, Token token);

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
            string type = "";
            if(left.type == "" || right.type == ""){
                cout << "Type error in expression - type of operand is unknown" << endl;
                return false;
            } else {
                Option<string> res = func_table.rtype(top.value, vector<string>{left.type, right.type});
                if(res.valid == false){ 
                    print("Type error - no operator defined for", left.type, top.value, right.type); 
                    return false;
                }
                type = res.result;
            }
            Expression ex = Expression(Binary_expr(top, left, right), type);
            expr.push(ex);
        } else if (top.type == UOP){
            if(expr.size() < 1){ return false; }
            Expression only = expr.top(); expr.pop();
            // determine type
            string type = "";
            if(only.type == ""){
                cout << "Type error in expression - type of operand is unknown (uop)" << endl;
                return false;
            } else {
                Option<string> res = func_table.rtype(top.value, vector<string>{only.type});
                if(res.valid == false){ 
                    print("Type error - no operator defined for", top.value, only.type); 
                    return false;
                }
                type = res.result;
            }
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
// TODO: check for validity of expression
// TODO: get return type of expression
// Returns on the following tokens: keyword, closing bracket, EOL, comma, or assignment
ParseSimpleExpr parseSimpleExpr(const vector<Token>& tokens, SymbolTable& vars, Iter it){
    ParseSimpleExpr ret;
    ret.start = it;
    ret.valid = true;
    // need to add expression and end of expression to ret
    enum state {START, ITEM, B_OPERATOR} prev, current;     // to validate whether expression is valid
    prev = START; // starting value only
    stack<Token> operations;
    stack<Expression> expr;

    bool terminate = false;
    // shunting yard algorithm
    
    for(; it != tokens.end(); it++){
        Token token = *it;
        // cout << token.toString() << endl;
        switch(token.type){
            case INT_LITERAL: {
                Expression ex = Expression(Literal(token), "int");
                expr.push( ex );
                current = ITEM;
                break;
                }
            case FLOAT_LITERAL: {
                Expression ex = Expression(Literal(token), "float");
                expr.push( ex );
                current = ITEM;
                break;
            }
            case CHAR_LITERAL: {
                Expression ex = Expression(Literal(token), "char");
                expr.push( ex );
                current = ITEM;
                break;
            }
            case STRING_LITERAL: {
                Expression ex = Expression(Literal(token), "string");
                expr.push( ex );
                current = ITEM;
                break;
            }

            case BOOL_LITERAL: {
                // TODO search up in variable table
                Expression ex = Expression(Literal(token), "bool");
                expr.push( ex );
                current = ITEM;
                break;
            }

            case NAME: {
                // TODO: type table
                Option<string> optional = vars.use_var(it->value);
                string var_type;
                if(!optional.valid){
                    if(func_table.check(it->value) != 0){
                        var_type = "$function";
                    } else {
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
                    }
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
            case ASSIGNMENT: case KEYWORD: case CLOSED_BRACKET: case DELIMITOR: case EOL: case END: {
                ret.end = it;
                goto exit;
                break;
                }
            case OPEN_BRACKET: {
                char opening = token.value[0];
                char expected = get_closing_bracket(token.value[0]);
                // special case brackets like this: func()
                if((it-1)->type == NAME && it->value == "(" && (it+1)->value == ")"){
                    Function_call fnc;
                    fnc.name = *(it-1);
                    // no arguements
                    // TODO
                    Option<string> type_maybe = func_table.rtype(fnc.name.value, vector<string>{});
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
                ParseMulti result = parseMulti(tokens, vars, it + 1);
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
                        vector<string> arg_types;
                        arg_types.reserve(fnc.args.size());

                        for(const Expression& e : fnc.args){
                            if(e.type == ""){
                                print("Type of expression in function call unknown");
                                ret.valid = false; return ret;
                            }
                            arg_types.push_back(e.type);
                        }
                        Option<string> type_maybe = func_table.rtype(fnc.name.value, arg_types);
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
                        vector<string> arg_types;
                        arg_types.reserve(arr.args.size());

                        for(const Expression& e : arr.args){
                            if(e.type == ""){
                                print("Type of expression in array call unknown");
                                ret.valid = false; return ret;
                            }
                            arg_types.push_back(e.type);
                        }
                        Option<string> type_maybe = func_table.rtype(arr.name.value, arg_types);
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
        /// end of main loop
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
            string type = "";
            if(left.type == "" || right.type == ""){
                cout << "Type error in expression - type of operand is unknown" << endl;
                ret.valid = false; return ret;
            } else {
                Option<string> res = func_table.rtype(top.value, vector<string>{left.type, right.type});
                if(res.valid == false){ 
                    print("Type error - no operator defined for", left.type, top.value, right.type); 
                    ret.valid = false;
                    return ret;
                }
                type = res.result;
            }
            Expression ex = Expression(Binary_expr{top, left, right}, type);
            expr.push(ex);
        } else if (top.type == UOP){
            if(expr.size() < 1 ){ ret.valid = false; ret.valid = false; cout << "3" << endl; return ret; }
            Expression only = expr.top(); expr.pop();
            // determine type
            string type = "";
            if(only.type == ""){
                cout << "Type error in expression - type of operand is unknown (uop)" << endl;
                ret.valid = false; return ret;
            } else {
                Option<string> res = func_table.rtype(top.value, vector<string>{only.type});
                if(res.valid == false){ 
                    print("Type error - no operator defined for", top.value, only.type); 
                    ret.valid = false; return ret;
                }
                type = res.result;
            }
            Expression ex = Expression(Unary_expr(top, only), type);
            expr.push(ex);
        }
    }

    if(expr.size() != 1 || operations.size() != 0 || current != ITEM){
        ret.valid = false;
        cout << "Error in expression parsing" << endl;
    } else {
        ret.expr = expr.top();
    }
    return ret;
}

// parse a series of comma separated expressions
// return vector of expressions + info
// expression of at least length 1
ParseMulti parseMulti(const vector<Token>& tokens, SymbolTable& vars,  Iter it){
    ParseMulti ret;
    ret.start = it;
    ret.valid = true;
    while(true){
        ParseSimpleExpr result = parseSimpleExpr(tokens, vars, it);
        if(result.valid == false){
            ret.valid = false;
            
            // perhaps print line
            break;
        }
        ret.multi_expr.push_back(result.expr);
        if(result.end->type != DELIMITOR){
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

// parse a basic line (assignment/expression)
// calls parseSimpleExpression, 
ParseBasicLine parseBasicLine(const vector<Token>& tokens, SymbolTable& vars, int line, Iter it){
    ParseBasicLine ret;
    ret.valid = true;
    ret.empty = false;
    // empty line - line with EOL only, or simply nothing
    if(it->type == EOL || it->value == "}"){
        ret.valid = true;
        ret.empty = true;
        ret.line = Empty();
        ret.end = it;
        return ret;
    }
    if(it->type == NAME && (it+1)->value == "="){
        ParseSimpleExpr second = parseSimpleExpr(tokens, vars, it+2);
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
        ret.end = second.end;
        return ret;
    }
    // check if simple assignment, e.g., x 
    // expression or left-side of assignment
    ParseSimpleExpr first = parseSimpleExpr(tokens, vars, it);
    if(first.valid == false){
        cout << "Invalid expression in parseBasicLine" << endl;
        ret.valid = false;
        return ret;
    }
    // if line is expression
    if(first.end->type == EOL || first.end->value == "}"){
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
    ParseSimpleExpr second = parseSimpleExpr(tokens, vars, it);
    if(second.valid == false){
        ret.valid = false;
        cout << "Invalid right side of assignment";
        return ret;
    }
    // must be a normal assignment operation
    if(second.end->type == EOL || second.end->value == "}"){
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
    //         ParseSimpleExpr next = parseSimpleExpr(tokens, vars, it);
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

// void printLexedLine(const vector<Token>& tokens, Iter it){
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


struct CodeBlockOption {
    bool valid;
    Codeblock codeblock;
    Iter end;
};

// top level function, parse {}
CodeBlockOption parseCodeBlock(const vector<Token>& tokens, SymbolTable& parent, Iter start) {
    CodeBlockOption ret;
    ret.valid = true;
    ret.codeblock.vars = SymbolTable{&parent};
    Iter tok;
    int line = 0;
    for(tok = start; ; line++){
        if(tok->type == END){
            cout << "Error: Reached end of file unexpectedly in parseCodeBlock" << endl;
            ret.valid = false;
            break;
        }
        // cout << "Parsing line: ";
        printToken(*tok);
        // print(line);
        ParseBasicLine line_result = parseBasicLine(tokens, ret.codeblock.vars, line, tok);
        if(line_result.valid == false){
            ret.valid = false;
            cout << "Invalid line" << endl;
            break;
        } else if(line_result.empty == false){
            // push line if not empty
            // print(ret.codeblock.lines.size());
            ret.codeblock.lines.push_back(line_result.line);

        } 
        if(line_result.end->value == "}"){
            ret.end = line_result.end;
            break;
        }
        tok = line_result.end + 1;  // start of next line
    }
    return ret;
}

struct ParseFunction {
    bool valid;
    Function result;
    Iter end;
};

ParseFunction parseFunction(const vector<Token>& tokens, Iter start){
    // int errorcode;
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
            cout << "Invalid function declaration (3)" << endl; 
            ret.valid = false; return ret;
        } else {
            // push type and parameter names
            ret.result.param_types.push_back(index->value);
            ret.result.param_names.push_back((index + 1)->value);
        }
        if((index + 2)->type == DELIMITOR){
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
        cout << "Invalid function declaration (5)" << endl; 
        ret.valid = false; return ret;
    } 
    index++;
    // modify for tuples/variants later
    if(index->type != NAME){
        cout << "Invalid function declaration (6)" << endl; 
        ret.valid = false; return ret;
    } 
    ret.result.return_type = index->value;
    index++;
    if(index->value == "{"){
        index++;
    } else if(index->value == "\n" && (index + 1)->value == "}"){
        index += 2;
    } else {
        cout << "Invalid function declaration (7)" << endl; 
        ret.valid = false; return ret;
    }

    // parse the code block 
    SymbolTable vars{};
    CodeBlockOption result = parseCodeBlock(tokens, vars, index);
    if(!result.valid){
        ret.valid = false; return ret;
    } else {
        ret.result.code = result.codeblock;
        cout << result.codeblock.vars.table.size() << endl;
        ret.end = result.end;
    }
    return ret;
}


struct ParseProgram {
    bool valid;
    Program result;
};

// top level parsing function
ParseProgram parseProgram(const vector<Token>& tokens, Iter start){
    func_table = make_ft();
    // func_table.printAll();
    ParseProgram ret;
    ret.valid = true;
    bool foundMainFunction;
    for(Iter i = start; i != tokens.end(); i++){
        Token t = *i;
        if(t.type == EOL){ continue; }
        else if(t.value == "func"){
            ParseFunction result = parseFunction(tokens, i + 1);
            if(result.valid == false){
                cout << "Parsing error in function " << (i + 1)->value << endl;
                ret.valid = false;
                return ret;
            }
            if(result.result.name == "main"){
                if(foundMainFunction == true){
                    cout << "Main function already defined" << endl;
                    ret.valid = false; return ret;
                } else { foundMainFunction == true; }
                ret.result.main = result.result;
            } else {
                if(ret.result.functions.count(result.result.name) != 0){
                    cout << "Function defined twice: " << result.result.name << endl;
                    ret.valid = false; return ret;
                }
                ret.result.functions.insert({result.result.name, result.result});
            }
            i = result.end; // move up the iterator
        } else if(t.type == END){
            break;
        } else {
            cout << "Invalid token found" << endl;
            ret.valid = false; return ret;
        }
    }
    return ret;
}