#pragma once

#include <stack>
#include <unordered_map>
#include <iostream>
#include <string>
#include <algorithm>

#include "lexer.cpp"
#include "ast.cpp"
#include "builtins.cpp"

using namespace std;

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
// globals
unordered_map<std::string, int> precedence;
vector<Token> tokens;
unordered_map<Type, Tuple_type> tuple_table;
unordered_map<Type, Array_type> array_table;
unordered_map<Type, StructInfo> struct_table;
vector<Type> decl_order;
// defined in different scope:
// EntityTable entity_table;
// Function_table func_table

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
struct ParseDeclOption;
ParseDeclOption parseDecl(SymbolTable& vars, int line, Iter start_it);
struct ParseDeleteOption;
ParseDeleteOption parseDelete(SymbolTable& vars, int line, Iter start_it);

Type register_tuple(vector<Type> types, bool complete);
void printErrorLine(Iter it);
bool resolveMemoryScopes(bool hasElse, int line, 
            SymbolTable& parent_scope, const vector<Codeblock>& child_cb);
//helper functions
char get_closing_bracket(char c);
bool shunt(stack<Token>& operations, stack<Expression>& expr, Token token);

struct ParseTypeOption {
    Type type;
    bool valid;
    Iter end;
};

// parse a type signature
// can be named type, tuple, array, ...
// add new tuple types or array types to the type_table
// types added to the type table do not all have their information filled in
// used while parsing structs/enums
ParseTypeOption parseIncompleteType(Iter start){
    ParseTypeOption ret;
    ret.valid = true;
    Iter it = start;
    if(it->type == NAME){
        if(type_table.contains(it->value)){
            ret.type = type_table.get_type(it->value);
            ret.end = it+1;
        } else {
            print("Error: type", it->value, "does not exist");
            ret.valid = false; return ret;
        }
    } else if(it->type == OPEN_BRACKET && it->value == "("){
        StructInfo si;
        Tuple_type ti;
        si.flag_for_algo = false;
        si.fully_defined_type = true;
        if((it+1)->value == ")"){
            print("Error: () is not a valid type");
            ret.valid = false; return ret;
        }
        it++;
        for(int i = 0; ; i++){
            ParseTypeOption result = parseIncompleteType(it);
            if(result.valid == false){ ret.valid = false; return ret; }

            // add element to structInfo
            bool user_defined_type = primitive_types.count(result.type.to_string()) == 0;
            si.elements.push_back(struct_element{result.type, string("e") + to_string(i), user_defined_type});
            ti.elements.push_back(result.type);
            if(user_defined_type){
                si.fully_defined_type = false;
            }
            it = result.end;
            if(it->type == COMMA){
                it++;
                // (type,) is syntax for tuple with 1 element
                if(ti.elements.size() == 1){ 
                    if(it->type == CLOSED_BRACKET && it->value == ")"){
                        break;
                    }
                }
            } else if(it->value == ")"){
                it++;
                break;
            }
        }
        // insert into type table, tuple table
        Type type = register_tuple(ti.elements, false);
        // insert into the struct table
        struct_table.insert({type, si});
        ret.type = type;
        ret.end = it;
    } else if(it->type == OPEN_BRACKET && it->value == "["){
        print("array type parsing not yet supported");
        ret.valid = false; return ret;
        // Array_type x;
        // ret.type.name = "[";
        // ParseTypeOption result = parseIncompleteType(it + 1);
        // if(result.valid == false){ ret.valid = false; return ret; }
        // x.type_ptr = new Type_ast(result.type);
        // ret.type.name.append(result.type.name);
        // ret.type.name.append(",");
        // it++;
        // if(it->type != COMMA){
        //     print("Invalid type, missing comma");
        //     ret.valid = false; return ret;
        // }
        // it++;
        // x.array_size = 1;
        // while(true){
        //     if(it->type != INT_LITERAL){
        //         print("Error, expected size in array declaration");
        //         ret.valid = false; return ret;
        //     }
        //     ret.type.name.append(it->value);
        //     size_t dimension_size = stoi(it->value);
        //     x.array_size *= dimension_size;
        //     it++;
        //     if(it->type == COMMA){
        //         ret.type.name.append(",");
        //         continue;
        //     } else if(it->value == "]"){
        //         ret.type.name.append("]");
        //         it++;
        //         ret.end = it;
        //         break;
        //     } else {
        //         print("Error while parsing array type");
        //         ret.valid = false; return ret;
        //     }
        // }
    } else {
        print("Invalid type signature");
        ret.valid = false; return ret;
    }
    return ret;
}

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

struct ParseArrayLiteral {
    bool valid;
    Array_literal array_literal;
    Iter end;
};

struct SubArray {
    bool valid;
    Iter end;
    vector<size_t> dims;    // dimensions of subarray (reverse order)
};

// add a tuple to the type table
// this function assumed structs/enums have already been parsed
Type register_tuple(vector<Type> types, bool complete){
    string tuple_type_name = "(";    
    string tuple_cname = "$t";        
    Type ret; 
    for(const auto& t : types){
        if(tuple_type_name != "("){
            tuple_type_name += ",";
        }
        tuple_type_name += t.to_string();
        tuple_cname += "_" + t.cname();
    }
    tuple_type_name += ")";
    tuple_cname += "$";
    
    if(type_table.contains(tuple_type_name)){
        return type_table.get_type(tuple_type_name);
    
    } else {
        TypeInfo ti;
        ti.name = tuple_type_name;
        ti.cname = tuple_cname;
        ti.supertype = TUPLE;
        for(int i = 0; i < types.size(); i++)
            ti.fields.push_back(field{to_string(i), types[i]});
        ti.size = 0;
        if(complete){
            for(const auto& t : types)
                ti.size += type_table.get_info(t).size;
            ti.sized = true;
            ti.destructor = false;
            ti.copyable = true; 
            for(const auto& t : types)
                ti.copyable = ti.copyable && type_table.get_info(t).copyable;
        } 
        // add the type to the type table
        Type type = type_table.insert(ti);
        tuple_table.insert({type, Tuple_type{vector<Type>{types}}});

        if(complete){
            // add the type to the declaration order
            decl_order.push_back(type);
        } 
        return type;
    } 
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
                // print("String literals not yet supported");
                // ret.valid = false; return ret;
                Expression ex = Expression(Literal(token), type_string);
                expr.push( ex );
                current = ITEM;
                break;
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
                } 

                else if(entity_table.get(it->value) == STRUCT){
                    if((it+1)->value != "{"){
                        print("Error - invalid use of struct in expression");
                        ret.valid = false;
                        return ret;
                    }
                    ParseMulti result = parseMulti(vars, it+2);
                    if(result.valid == false){
                        print("Error while parsing expression initializing struct (1)");
                        ret.valid = false;
                        return ret;
                    }
                    if(result.end->type != EOL && (result.end+1)->value != "}"){
                        print("Error while parsing expression initializing struct (2)");
                        ret.valid = false;
                        return ret;
                    }
                    
                    Type struct_type = type_table.get_type(it->value);
                    const StructInfo& info = struct_table[struct_type];
                    if(result.multi_expr.size() != info.elements.size()){
                        print("Error initializing struct - number of arguements don't match signature");
                        ret.valid = false;
                        return ret;
                    }
                    for(int i = 0; i < info.elements.size(); i++){
                        if(info.elements[i].type != result.multi_expr[i].type){
                            print("Error initializing struct - arguement", i, "type doesn't match signature");
                            ret.valid = false;
                            return ret;
                        }
                    }
                    Struct_init si{*it};
                    si.args = result.multi_expr;
                    Expression ex = Expression(si, struct_type);
                    expr.push( ex );
                    current = ITEM;
                    it = result.end + 1;    // skip the added newline, end on }
                } else {
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
                    }
                }
                break;

            case BOP: case UOP: {
                if(token.value != "."){
                    bool success = shunt(operations, expr, token);
                    if(!success){ 
                        ret.valid = false; 
                        cout << "Error in shunting " << token.toString() << endl;
                        return ret; 
                    }
                    if(token.type == BOP){ current = B_OPERATOR; } else { current = ITEM; }
                } else {
                    if(current != ITEM){
                        print("Syntax error - unexpected . in expression");
                        ret.valid = false;
                        return ret;
                    }
                    Expression first = expr.top();
                    Type type = first.type;
                    TypeInfo& info = type_table.get_info(type);
                    
                    Token second = *(it + 1);
                    bool field_exists = false;
                    field field_data;
                    // search through fields
                    // vector search - inefficient, but ok since most structs have few fields
                    for(auto& f : info.fields){
                        if(f.name == second.value){
                            field_exists = true;
                            field_data = f;
                            break;
                        }
                    }
                    if(!field_exists){
                        print("Error, variable of type", type.to_string(), 
                            "does not have a field named", second.value);
                        ret.valid = false; return ret;
                    }
                    expr.pop();
                    Expression ex = Expression{Field_access{first, second}, field_data.type};
                    expr.push(ex);
                    it++;
                    current = ITEM;
                    goto ignore_checks;
                }
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
                // array declaration [1, 2, 3]
                if(prev != ITEM && opening == '['){
                    
                    ParseArrayLiteral result = parseArrayLiteral(vars, it);
                    if(result.valid == false){
                        print("Error parsing array literal");
                        ret.valid = false; return ret;
                    }
                    Array_literal& al = result.array_literal;
                    TypeInfo ti;
                    // name of type is [<subtype>,dim1,dim2,...]
                    ti.name = string("[");
                    ti.name += al.element_type.to_string();
                    for(size_t dim : al.dims){
                        ti.name += ",";
                        ti.name += to_string(dim);
                    }
                    ti.name += "]";
                    // calculate size of array = sizeof(type) * length
                    ti.size = type_table.get_info(al.element_type).size * al.elements.size();
                    ti.supertype = ARRAY;
                    ti.fields = vector<field>{field{"size", type_int}};
                    ti.copyable = type_table.get_info(al.element_type).copyable;
                    ti.sized = true;
                    ti.destructor = type_table.get_info(al.element_type).destructor;
                    Type type = type_table.insert(ti);
                    Expression ex(result.array_literal, type);
                    expr.push(ex);
                    it = result.end - 1;

                    current = ITEM;
                    break;
                }
                
                // recursively parse expression inside bracket
                // parse for comma separated expression
                ParseMulti result = parseMulti(vars, it + 1);
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
                    // function call
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
                    // array indexing operation
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
                } 
                // must be a bracket
                else if(result.multi_expr.size() == 1){
                    expr.push( result.multi_expr.at(0) );
                } 
                // must be a tuple
                else {
                    Tuple_literal tl;
                    tl.elements = result.multi_expr;
                    vector<Type> types;
                    types.reserve(tl.elements.size());
                    for(const auto& e : tl.elements){
                        types.push_back(e.type);
                    }
                    Type tuple_type = register_tuple(types, true);

                    Expression ex(tl, tuple_type);
                    expr.push(ex);
                }
                current = ITEM; // in all cases, item (bracket, function call, array index, array literal)
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
        if(top.type == BOP){
            if(expr.size() < 2 ){ ret.valid = false; cout << "2" << endl; return ret; }
            Expression right = expr.top(); expr.pop();
            Expression left = expr.top(); expr.pop();
            // determine type
            Option<Type> res = func_table.rtype(top.value, vector<Type>{left.type, right.type});
            if(res.valid == false){ 
                print("Type error - no operator defined for", left.type, top.value, right.type); 
                ret.valid = false; return ret;
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
        // print(expr.size(), operations.size(), current == ITEM);
        ret.valid = false;
        print("Error in expression parsing");
    } else {
        ret.expr = expr.top();
    }
    return ret;
}

// struct ParseArrayLiteral {
//     bool valid;
//     Array_literal array_literal;
//     Iter end;
// };

// struct SubArray {
//     bool valid;
//     Iter end;
//     vector<size_t> dims;    // dimensions of subarray (reverse order)
// };

// assume i is pointing to a square bracket
ParseArrayLiteral parseArrayLiteral(SymbolTable& vars, Iter it){
    ParseArrayLiteral ret; ret.valid = true;
    SubArray result = parseArrayRecurse(vars, it, ret.array_literal.elements);
    
    if(result.valid == false){ 
        print("Error while parsing array literal");
        ret.valid = false; return ret; 
    } 
    ret.end = result.end;
    
    // reverse the dims vector
    // the recursive function determines dimensions in reverse order for efficiency
    reverse(result.dims.begin(), result.dims.end());
    ret.array_literal.dims = result.dims;
    ret.array_literal.element_type = ret.array_literal.elements[0].type;
    // check each element has same type
    for(const Expression& e : ret.array_literal.elements){
        if(e.type != ret.array_literal.element_type){
            print("Error - array element types are not all the same");
            ret.valid = false; return ret;
        }
    }
    return ret;
}

SubArray parseArrayRecurse(SymbolTable& vars, Iter it, vector<Expression>& elements){
    SubArray ret;
    ret.valid = true;
    it++;
    size_t num_elements = 0;
    vector<size_t> first_element_dims;
    while(true){
        // subarray
        if(it->value == "["){
            SubArray sa = parseArrayRecurse(vars, it, elements);
            if(sa.valid == false){ ret.valid = false; return ret; }
            // check subarrays are same size
            if(num_elements == 0){
                first_element_dims = sa.dims;
            } else if(sa.dims != first_element_dims){
                print("Error - subarrays must have the same size");
                ret.valid = false; return ret;
            }
            it = sa.end;
        // unexpected closing bracket (error)
        } else if(it->value == "]"){
            if(num_elements == 0){
                print("Error: arrays with zero size are not allowed");
            } else {
                print("Error: unexpected ] while parsing array");
            }
            ret.valid = false; return ret;
        // normal element - possibly a expression
        } else {
            ParseSimpleExpr result = parseSimpleExpr(vars, it);
            if(result.valid == false){ ret.valid = false; return ret; }
            elements.push_back(result.expr);
            if(first_element_dims.size() != 0){
                print("Error - subarrays must have the same size (2)");
                ret.valid = false; return ret;
            }
            it = result.end;
        }
        num_elements++;
        // the next token should be , or ]
        if(it->type == COMMA){
            it++;
        } else if(it->value == "]"){
            ret.end = it + 1;
            break;
        } else {
            print("Error: syntax error in declaration of array");
            ret.valid = false; return ret;
        }
    }
    ret.dims = first_element_dims;
    ret.dims.push_back(num_elements);
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

// parse a basic line (assignment/expression ended with EOL
// if line ends with EOL, end is 1 token after EOL
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
        if(HAS<Variable>(second.expr.expr) && type_table.get_info(second.expr.type).copyable == false ){
            // variable has been moved - destroy it!
            vars.destroy(second.start->value, line);
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
    // must be assignment expression like += or -=, or error
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
        print("Invalid right side of assignment");
        return ret;
    }
    // TODO: more complex expression like expr = expr -- currently, only x = expr is allowed

    // check that second expression ends with EOL, parse special assignment (e.g., x += y)
    if(second.end->type == EOL){
        Token assign_op = *first.end;
        if(assign_op.value != "+=" && assign_op.value != "-=" && assign_op.value != "*=" && assign_op.value != "/="){
            ret.valid = false; 
            print("Invalid operator", assign_op.value);
            return ret;
        }
        Token equals = assign_op;
        equals.value = "=";
        Token op = assign_op;
        op.value = assign_op.value.substr(0, 1);
        //// check validity, e.g., x += y -> check x + y is valid
        Option<Type> res = func_table.rtype(op.value, vector<Type>{first.expr.type, second.expr.type});
        if(res.valid == false){ 
            print("Type error - no operator defined for", first.expr.type, assign_op.toString(), second.expr.type); 
            ret.valid = false;
            return ret;
        }
        Type type = res.result;
        Expression ex = Expression(Binary_expr{op, first.expr, second.expr}, type);
        
        // assignment, assume left side is just variable
        // TODO: fix to allow more general assignment
        int result = vars.assign(first.start->value, line, second.expr.type);
        if(result != 0){
            cout << "Error in assignment operation" << endl;
            ret.valid = false; return ret;
        } 
        Assignment a{first.expr, ex, equals};
        ret.end = second.end + 1;
        ret.line = a;
        return ret;
    } 
    
    // else if(second.end->value == "}"){
    //     Token op = *first.end;
    //     Assignment a = {first.expr, second.expr, op};
    //     ret.end = second.end;
    //     ret.line = a;
    //     //// add to symboltable, type check
    //     return ret;
    // }
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

// end of block is the token after closing bracket
Option<Iter> endOfBlock(Iter start, const string& entity_name){
    Option<Iter> ret(true);
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
    ret.result = it;
    return ret;
}

struct ParseProgram {
    bool valid;
    Program result;
};

struct Entity_start {
    Type name;
    Iter start;
};

Option<StructInfo> parseStruct(const Entity_start& obj){
    Iter it = obj.start;
    Option<StructInfo> ret(true);
    bool contains_user_defined_type = false;
    while(true){
        if(it->type == EOL){
            it++;
            continue;
        } 
        if(it->value == "}"){
            break;
        }
        ParseTypeOption result = parseIncompleteType(it);
        if(result.valid == false){
            print("Error parsing struct", obj.name, "- expected type. Line:");
            printErrorLine(it);
            ret.valid = false; return ret;
        }

        Type type = result.type;
        bool user_defined_type;
        TypeInfo& t_info = type_table.get_info(type);

        print(toString(t_info.supertype));
        if(t_info.supertype == PRIMITIVE_TYPE){
            user_defined_type = false;
        } else if(t_info.supertype == STRUCT){
            user_defined_type = true;
            contains_user_defined_type = true;
        } else if(t_info.supertype == TUPLE){
            user_defined_type = true;
            contains_user_defined_type = true;
        } else {
            print("Error - array type not supported in struct (yet!)");
            ret.valid = false; return ret;
        }
        it = result.end;
        if(it->type != NAME){
            print("Error in declaration of struct", obj.name, ", field", it->value, "invalid");
        }
        ret.result.elements.push_back(struct_element{type, type.to_string(), user_defined_type});
        it++;
        if(it->type != EOL){
            print("Error while parsing struct", obj.name, ", expected newline or }");
        }
        it++;
    }
    
    ret.result.fully_defined_type = !contains_user_defined_type;
    ret.result.flag_for_algo = false;
    return ret;
}
// recursive helper function to construct declaration order
// base case: if struct is fully defined, and 
// if struct not reached, check each element of the 
bool recurse_helper(const Type& struct_type){
    print("Parsing", struct_type);
    StructInfo& data = struct_table.at(struct_type);
    // check if struct has already been visited
    if(data.flag_for_algo == true){
        if(data.fully_defined_type){
            return true;
        } else {
            print("Error, cyclic definition found in struct", struct_type);
            return false;
        }
    }
    data.flag_for_algo = true;  // indicate struct has been visited
    // recursion base case - a struct composed of base types only
    if(data.fully_defined_type){
        decl_order.push_back(struct_type);
        return true;
    }
    // recursive step - continue for structs that are fields
    for(struct_element& e: data.elements){
        Entity supertype = type_table.get_info(e.type).supertype;
        if(supertype != PRIMITIVE_TYPE){
            bool valid = recurse_helper(e.type);
            if(!valid){
                print("Error, cyclic definition in struct", struct_type);
                return false;
            }
        }
    }
    decl_order.push_back(struct_type);
    data.fully_defined_type = true;
    return true;
}
// step 0 - get list of struct names & starting point, add all to entity table and type table 
// (done before this function)
// step 1 - for each struct, parse, and mark as concrete or not concrete, add to hashmap. 
// step 2 - iterate through structs in hashmap, and construct the declaration order, using the
// concrete flags
// step 3 - update the metadata in the type table with sizes, etc.

// any unfamiliar types, flag as unfamiliar
// vector<Type> decl_order - declaration order of structs
// vector<Entity_start> list - list of structs in initial program
// struct_table - hashmap of type -> StructInfo - list of structs in program
bool parseStructs(const vector<Entity_start>& list){

    // step 1 - for each struct, parse, add to struct table
    for(const Entity_start& entity : list){
        Option<StructInfo> opt = parseStruct(entity);
        if(opt.valid == false){
            print("Error parsing struct", entity.name);
            return false;
        }
        struct_table.insert({entity.name, opt.result});
    }
    
    // step 2 - iterate through structs, construct declaration order
    // this requires a recursive helper function
    decl_order.reserve(struct_table.size());
    // decl_order.reserve(list.size());
    print("struct_table");
    for(auto i : struct_table){
        print(i.first);
    }
    print("tuple_table");
    for(auto i : tuple_table){
        for(auto j : i.second.elements)
            print_raw(j.to_string(), ", ");
        print("\n");
    }
    for(const pair<Type, StructInfo>& struct_pair : struct_table){
        // helper determines if type is concrete - if a type is not concrete, return false
        // while doing so, construct the declaration order
        bool valid = recurse_helper(struct_pair.first);
        if(!valid){
            print("Error, cycle detected in struct definitions");
            return false;
        }
    }
    print("struct_table");
    for(auto i : struct_table){
        print(i.first);
    }
    print("tuple_table");
    for(auto i : tuple_table){
        for(auto j : i.second.elements)
            print_raw(j.to_string(), ", ");
        print("\n");
    }
    // step 3 - update metadata
    for(Type name : decl_order){
        TypeInfo& data = type_table.get_info(name);
        print("hi4");
        StructInfo& info = struct_table.at(name);
        print("bye4");
        data.copyable = true;
        for(struct_element& x : info.elements){
            data.copyable = data.copyable && type_table.get_info(x.type).copyable;
        }
        data.destructor = false;
        data.sized = true;
        for(struct_element& x : info.elements){
            data.size += type_table.get_info(x.type).size;
        }
        // fill out type fields
        for(struct_element& x : info.elements){
            data.fields.push_back(field{x.field, x.type}); 
        }
    }
    return true;
}

void printStructs(){
    for(Type i : decl_order){
        if(struct_table.count(i) != 1){
            print("Error - struct", i, "not in type table");
            continue;
        }
        StructInfo& info = struct_table.at(i);
        print("struct", i, "{");
        if(type_table.contains(i)){
            size_t size = type_table.get_info(i).size;
            print("Size:", size);
        } else {
            print("Error: not in type table");
        }
        for(const struct_element& e : info.elements){
            print(e.type.to_string(), e.field);
        }
        print("}");
    }
}

// struct Entity_start {
//     string name;
//     Iter start;
// };

// top level parsing function
ParseProgram parseProgram(Iter start){
    // func_table.printAll();
    bool foundMainFunction = false;
    ParseProgram ret;
    ret.valid = true;
    vector<Entity_start> struct_list;
    // vector<Entity_start> enum_list;

    for(Iter it = start; it != tokens.end(); it++){
        Token t = *it;
        if(t.type == EOL){ 
            continue; 
        } else if(t.value == "func"){
            ParseFunction result = parseFunctionSignature(it + 1);
            if(result.valid == false){
                print("Parsing error in function", (it + 1)->value);
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
            Option<Iter> result2 = endOfBlock(result.end, entity_name); 
            if(result2.valid == false){
                print("Bracket mismatch in", entity_name);
                ret.valid = false; return ret;
            }
            it = result2.result; // move up the iterator
        
        // initial parsing for structs
        } else if(t.value == "struct"){
            if((it+1)->type != NAME && (it+2)->value != "{"){
                print("Invalid struct declaration on line", it->line);
                ret.valid = false; return ret;
            }
            Iter name = it+1;
            Iter openingBracket = it + 2;
            Iter startOfBlock = it+3;
            // check for duplicate entry
            bool valid = entity_table.add(name->value, STRUCT);
            if(valid == false){
                Entity z = entity_table.get(name->value);
                print("Error while parsing struct", name->value, ", name is defined twice");
                ret.valid = false; return ret;
            }
            // add the struct to the type table
            Type type = type_table.insert(TypeInfo{name->value, "", 0, STRUCT, vector<field>{}, false, false, false});
            
            // save name of struct, iterator to beginning of block
            struct_list.push_back({type, startOfBlock});
            // parse rest of block later
            Option<Iter> opt = endOfBlock(openingBracket, "struct " + name->value);
            if(!opt.valid){
                ret.valid = false; return ret;
            } 
            it = opt.result;
        } else if(t.type == END){
            break;
        } else {
            print("Invalid token found during parseProgram");
            ret.valid = false; return ret;
        }
    }
    // func_table.printAll();
    // finish parsing structs, enums, global scope, etc
    bool opt = parseStructs(struct_list);
    if(opt == false){
        print("Error while parsing structs");
        ret.valid = false; return ret;
    }
    printStructs();

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