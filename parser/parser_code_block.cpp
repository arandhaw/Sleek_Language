#pragma once
// #include "parser_globals.h"

// struct ParseIfOption {
//     bool valid;
//     IfBlock result;
//     Iter end;
// };

// struct CodeBlockOption {
//     bool valid;
//     Codeblock codeblock;
//     Iter end;
// };

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

// struct ParseWhileOption {
//     bool valid;
//     WhileBlock result;
//     Iter end;
// };

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

// struct ParseLoopOption {
//     bool valid;
//     LoopBlock result;
//     Iter end;
// };

ParseLoopOption parseLoop(SymbolTable& vars, int line, Iter start_it){
    Iter it = start_it;
    ParseLoopOption ret;
    ParseSimpleExpr result = parseSimpleExpr(vars, it + 1);
    if(result.valid == false){
        print("Error parsing number of iterations in loop");
        ret.valid = false; return ret;
    }
    if(result.expr.type != type_int){
        print("Error, loop number is not an integer");
        ret.valid = false; return ret;
    }
    it = result.end;
    if((result.end)->value != "{"){
        print("Error in loop, expected {");
        ret.valid = false; return ret;
    }
    ScopeInfo si;
    si.return_type = vars.returnType;
    si.parent = &vars;
    CodeBlockOption result2 = parseCodeBlock(result.end + 1, si);
    if(result2.valid = false){
        print("Error in codeblock of loop");
        ret.valid = false; return ret;
    }
    // TODO check memory - in particular, check that outside variables are not destroyed twice
    ret.valid = true;
    ret.result.number = result.expr;
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

// struct ParseDeclOption {
//     DeclVars result;
//     bool valid;
//     Iter end;
// };

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

// struct ParseDeleteOption {
//     DeleteVars result;
//     bool valid;
//     Iter end;
// };

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
        } else if(it->value == "loop"){
            ParseLoopOption result = parseLoop(vars, line, it);
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






