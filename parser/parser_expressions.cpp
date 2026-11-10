#pragma once
// #include "parser_globals.h"

// forgive me, this function is rather long
// algorithm: shunting yard algorithm
// capabilities: brackets, binary operators, unary operators, arrays, functions, tuples

// a 'simple expression' is anything that could be on the right of an equals sign
// a 'multi expression' is a series of comma separated expressions

// end is one after end
// struct ParseSimpleExpr {
//     Expression expr;
//     Iter start;
//     Iter end;
//     bool valid;
// };

// struct ParseMulti {
//     bool valid;
//     Iter start;
//     Iter end;
//     vector<Expression> multi_expr;
// };

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

                // all the following if blocks break/goto at the end
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
                    if(al.dims.size() > MAX_ARRAY_DIMENSIONS){
                        print("Error - array literal with ", al.dims.size(), " dimensions found");
                        print("Arrays cannot have more than ", MAX_ARRAY_DIMENSIONS, " dimensions");
                        ret.valid = false; return ret;
                    }
                    TypeInfo ti;
                    // name of type is [<subtype>,dim1,dim2,...]
                    ti.name = string("[");
                    ti.name += al.element_type.to_string();
                    for(size_t dim : al.dims){
                        ti.name += ",";
                        ti.name += to_string(dim);
                    }
                    ti.name += "]";
                    ti.cname = result.cname;
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
                // this must be a function call or array index
                if(prev == ITEM){   
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
                    current = ITEM; // in all cases, item (bracket, function call, array index, array literal)
                    it = result.end;
                    goto ignore_checks;     // a break causes error in case of function
                } 

                // check for tuple with one element with syntax (<element>,)
                Option<Iter> option = endOfBlock(it,"");
                if(option.valid == false){ print("Deep error: brackets mismatched while parsing expression"); }
                Iter ending_bracket = option.result - 1;
                
                if((ending_bracket - 1)->type == COMMA){
                    ParseSimpleExpr result = parseSimpleExpr(vars, it + 1);
                    if(result.valid == false){ ret.valid = false; return ret; }
                    if((result.end)->type != COMMA || (result.end+1)->value != ")"){
                        print("Syntax error, bracket ending with ,)");
                        ret.valid = false; return ret;
                    }
                    // tuple with one element confirmed
                    Tuple_literal tl;
                    tl.elements.push_back(result.expr);
                    vector<Type> types = {result.expr.type};
                    // register tuple
                    Type tuple_type = register_tuple(types, true);
                    Expression ex(tl, tuple_type);
                    expr.push(ex);
                    // continue on
                    current = ITEM;
                    it = result.end+1;
                    goto ignore_checks; 
                }
                // must be bracket or tuple with multiple elements
                else {
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
                    // must be a bracket
                    if(result.multi_expr.size() == 1){
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
                    goto ignore_checks;     // a break causes error 
                }
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

///////////////////////////////////
///////////////////////////////////

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