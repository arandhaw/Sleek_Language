#pragma once
// #include "parser_globals.h"

// struct ParseTypeOption {
//     Type type;
//     bool valid;
//     Iter end;
// };

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
            si.elements.push_back(struct_element{result.type, to_string(i), user_defined_type});
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

ParseTypeOption parseType(Iter start){
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
        
        vector<Type> elements;
        if((it+1)->value == ")"){
            print("Error: () is not a valid type");
            ret.valid = false; return ret;
        }
        it++;
        for(int i = 0; ; i++){
            ParseTypeOption result = parseType(it);
            if(result.valid == false){ ret.valid = false; return ret; }

            // add element to structInfo
            elements.push_back(result.type);
            
            it = result.end;
            if(it->type == COMMA){
                it++;
                // (type,) is syntax for tuple with 1 element
                if(elements.size() == 1){ 
                    if(it->type == CLOSED_BRACKET && it->value == ")"){
                        it++;
                        break;
                    }
                }
            } else if(it->value == ")"){
                it++;
                break;
            }
        }
        // insert into type table, tuple table
        Type type = register_tuple(elements, true);
        // insert into the struct table
        ret.type = type;
        ret.end = it;
    } else if(it->type == OPEN_BRACKET && it->value == "["){
        Array_type x;
        string name = "[";
        ParseTypeOption result = parseType(it + 1);
        if(result.valid == false){ ret.valid = false; return ret; }

        x.type = result.type;
        name.append(result.type.to_string());
        name.append(",");
        it++;
        if(it->type != COMMA){
            print("Invalid type, missing comma");
            ret.valid = false; return ret;
        }
        it++;
        x.array_size = 1;
        while(true){
            if(it->type != INT_LITERAL){
                print("Error, expected size in array declaration");
                ret.valid = false; return ret;
            }
            name.append(it->value);
            size_t dimension_size = stoi(it->value);
            x.array_size *= dimension_size;
            it++;
            if(it->type == COMMA){
                name.append(",");
                continue;
            } else if(it->value == "]"){
                name.append("]");
                it++;
                ret.end = it;
                break;
            } else {
                print("Error while parsing array type");
                ret.valid = false; return ret;
            }
        }
    } else {
        print("Invalid type signature");
        ret.valid = false; return ret;
    }
    return ret;
}

// struct ParseFunction {
//     bool valid;
//     Function result;
//     Iter end;
// };

// end is the '{' of the function
// start is name of function (after func keyword)
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
    if(index->value == ")"){
        index++;
        goto exit;
    }
    while(true){
        ParseTypeOption result = parseType(index);
        if(result.valid == false){
            print("Invalid function declaration (3) - invalid type parameter"); 
            ret.valid = false; return ret;
        } 
        if((result.end)->type != NAME){
            print("Invalid function declaration (3) - no parameter name");
        }
        // push type and parameter names
        Type t = result.type;
        ret.result.param_types.push_back(t);
        ret.result.param_names.push_back((result.end)->value);
        
        if((result.end + 1)->type == COMMA){
            index = result.end + 2;
        } else if((result.end + 1)->value == ")"){
            index = result.end + 2;
            break; 
        } else {
            cout << "Invalid function declaration (4) - expected comma or closing bracket" << endl; 
            ret.valid = false; return ret; 
        }
    }
    exit:

    if(index->value != "->"){
        ret.result.return_type = type_none;
    } else {
        index++;
        // modify for tuples/variants later
        auto result = parseType(index);
        if(result.valid = false){
            print("Invalid function declaration (6) - invalid return type"); 
            ret.valid = false; return ret;
        }
        index = result.end;
        ret.result.return_type = result.type;
    }
    
    if(index->value == "{"){
    } else if(index->value == "\n" && (index + 1)->value == "}"){
        index += 1;
    } else {
        print("Invalid function declaration (7) - expected opening bracket {");
        print(index->value);
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
        ret.valid = false; return ret;
    } else {
        ret.result.code = result.codeblock;
        cout << result.codeblock.vars.table.size() << endl;
        ret.end = result.end;
    }
    return ret;
}

