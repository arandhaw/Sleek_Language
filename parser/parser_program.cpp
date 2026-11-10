#pragma once
// #include "parser_globals.h"

/////////////////////////////
// top level parsing functions
/////////////////////////////
// struct Function_start {
//     Iter start;
//     Iter name;
// };

// top level parsing function
ParseProgram parseProgram(Iter start){
    // func_table.printAll();
    bool foundMainFunction = false;
    ParseProgram ret;
    ret.valid = true;
    vector<Entity_start> struct_list;
    vector<Function_start> function_list;
    // vector<Entity_start> enum_list;

    for(Iter it = start; it != tokens.end(); it++){
        Token t = *it;
        if(t.type == EOL){ 
            continue; 
        } else if(t.value == "func"){
            Function_start fs{it, it+1};
            function_list.push_back(fs);
            if(fs.name->value == "main"){
                if(foundMainFunction == true){
                    print("Error - main function defined multiple times");
                    ret.valid = false; return ret;
                }
                foundMainFunction = true;
            }
            if((it+1)->type != NAME && (it+2)->value != "("){
                print("Error - invalid function declaration");
                printErrorLine(it);
                ret.valid = false; return ret;
            }
            Option<Iter> result = endOfBlock(it+2, fs.name->value);
            if(result.valid == false){
                print("Error - invalid function declaration");
                printErrorLine(it);
                ret.valid = false; return ret;
            }
            it = result.result;
            Iter start_of_codeblock;
            while(true){
                if(it->value == "{"){
                    start_of_codeblock = it;
                    break;
                } else if(it->type == EOL && (it+1)->value == "{"){
                    start_of_codeblock = it+1;
                    break;
                } else if(it->type == EOL && (it+1)->value != "{"){
                    print("Error - invalid function declaration, expected {");
                    ret.valid = false; return ret;
                } else {
                    it++;
                }
            }
            Option<Iter> result2 = endOfBlock(start_of_codeblock, fs.name->value);
            if(result2.valid == false){
                print("Error while parsing function - bracket mismatch");
                ret.valid = false; return ret;
            }
            it = result2.result;
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

    // finish parsing structs, enums, global scope, etc
    bool opt = parseStructs(struct_list);
    if(opt == false){
        print("Error while parsing structs");
        ret.valid = false; return ret;
    }
    printStructs();

    //// parse the functions
    if(foundMainFunction == false){
        print("Error - no main function found in program");
        print("Signature should look like this: func main(){ ... }");
        ret.valid = false; return ret;
    }
    for(const Function_start i : function_list){
        Iter it = i.start;
        ParseFunction result = parseFunctionSignature(i.start + 1);
        if(result.valid == false){
            print("Parsing error in function", (it + 1)->value);
            ret.valid = false; return ret;
        }
        if(i.name->value == "main"){
            if(!(result.result.param_names.size() == 0 && result.result.return_type == type_none)){
                print("Error - main function signature must have no parameters and return none");
                ret.valid = false; return ret;
            }
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
    }
    func_table.printAll();
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

// end of block is the token after closing bracket
// start: the opening bracket
// entity_name: name of function, for error messages only
// returns Option<Iterator>: if valid, returns the end  
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
