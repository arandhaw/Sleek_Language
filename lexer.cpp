#pragma once

#include <iostream>
#include <cctype>
#include <vector>
#include <string>
#include <unordered_set>
// #include <unordered_set>
// #include <cstring>
using namespace std;

namespace lexer_globals {


    // characters that operators can start with
    const std::unordered_set<char> operators = 
        {'+', '-', '*', '/', '^', '%', '&', '<', '>', '|', '!', ':', '=', '@', '?', '.'};
    // words that are operators
    const std::unordered_set<std::string> operator_words = {"and", "or"};
    // the list of allowed operators
    const std::unordered_set<std::string> allowed_operators = 
        {"+", "-", "*", "/", "^", "%", "&", "<", ">", ">=", "<=",  
        "|", "!", "++", "--", "**", "//", "==", "->", "::", 
        "=", "!=", "+=", "-=", "*=", "/=", "@", "."};

    const std::unordered_set<std::string> unary_operators = {"!", "&"};
    // operators that are unary or binary
    const std::unordered_set<std::string> ambiguous_operators = {"+", "-"};
    const std::unordered_set<std::string> assignment = {"=", "+=", "-=", "*=", "/=", "++", "--"};
    const std::unordered_set<std::string> keywords = {"fn", "return", "decl", "delete",
                    "if", "elif", "else", "while", "do", "match", "struct", "for", "loop"};

    //brackets
    const std::unordered_set<char> open_brackets = {'[', '{', '('};
    const std::unordered_set<char> closed_brackets = {']', ')', '}'};
    
    const std::unordered_set<char> whitespace = {' ', '\r', '\t'};
    const std::unordered_set<char> escape_sequences = {'"', '\\', '\'', 'a', 'b', 'f', 'n', 'r', 't', 'v'};  

}

// public functions

//true if symbol is within options
// bool oneOf(char symbol, string options){
//     for(auto& i : options){
//         if( i == symbol ){
//             return true;
//         }
//     }
//     return false;
// }

// bool oneOf(string str, vector<string> options){
//     for(auto& i : options){
//         if( i == str ){
//             return true;
//         }
//     }
    
//     return false;
// }

bool oneOf(char c, unordered_set<char> options){
    return options.count(c);
}
bool oneOf(string str, unordered_set<string> options){
    return options.count(str);
}

enum tokenType{
    EOL,    // ; \n not preceded by a , or backslash
    OPEN_BRACKET,       //{}, [], ()
    CLOSED_BRACKET,
    KEYWORD,        //name that matches a specific value
    COMMA,      //, or ,\n 
    NAME,           //_ or L -> alphanumeric
    UOP,
    BOP,        //symbol or two symbols
    ASSIGNMENT,     // assignment operator (none | int)
    END,            // End of file
    INT_LITERAL,        
    FLOAT_LITERAL,         //contains decimal or scientific notation
    CHAR_LITERAL,       // single quotes ''
    BOOL_LITERAL,       // true or false
    STRING_LITERAL,         //starts with <modifier>"
    SLICE_LITERAL,          // two or three numbers
    UNDERLINE,
};

string getType(tokenType val){
    std::string ret;
    switch(val){
        case 0: ret = "EOL";
        break;
        case 1: ret = "OPEN_BRACKET";
        break;
        case 2: ret = "CLOSED_BRACKET";
        break;
        case 3: ret = "KEYWORD";
        break;
        case 4: ret = "COMMA";
        break;
        case 5: ret = "NAME";
        break;
        case 6: ret = "UOP";
        break;
        case 7: ret = "BOP";
        break;
        case 8: ret = "ASSIGNMENT";
        break;
        case 9: ret = "END";
        break;
        case 10: ret = "INT";
        break;
        case 11: ret = "FLOAT";
        break;
        case 12: ret = "CHAR";
        break;
        case 13: ret = "BOOL";
        break;
        case 14: ret = "STRING";
        break;
        case 15: ret = "SLICE";
        break;
        case 16: ret = "UNDERLINE";
        break;
        default: ret = "INVALID";
        break;
    }
    return ret;
}

struct Token {
    tokenType type;
    unsigned int line;
    unsigned int column;
    unsigned int length;
    string value;
    
    string toString(void){
        return getType(type) + ":" + value;
    }
};

struct LexerOutput{
    std::vector<Token> tokens;
    bool success;
};



//////////////////////////////////////////////////////////////////////////////////////////
// beggining of private namespace ////////////////////////////////////////////////////////
namespace lexer_internals {
    using namespace lexer_globals;


    void printTokensDeep(std::vector<Token> list){
        cout << "[";
        for(auto& token : list){
            cout << token.toString() << "(" << 
                token.line << ", " << token.column << ")" << ", ";
        }
        cout << "]" << endl;
    }

    bool lexNumber(std::string str, int start, Token& token){
        bool foundDecimal = false;
        bool foundE = false;
        token.value = "";

        int i;
        for(i = start; i != str.size(); i++){
            char c = str[i];
            if(isdigit(c)){
                token.value += c;
            } else if(c == '_'){
                continue;
            } else if(c == '.'){
                if(foundDecimal == true){
                    cout << "Error - multiple decimals in number";
                    return false;  // multiple decimal points - should be error
                } else {
                    foundDecimal = true;
                    token.value += c;
                }
            } else if(c == 'e' || c == 'E'){
                if(foundE == true){
                    cout << "Error - unexpected E";
                    return false;  // multiple E - should be error
                }
                foundE = true;
                foundDecimal = true;
                token.value += c;
                if(str[i + 1] == '+' || str[i + 1] == '-'){
                    token.value += str[i + 1];
                    i++;
                }
            } else {
                break;
            }
        }
        if(foundDecimal == true){
            token.type = FLOAT_LITERAL;
        } else {
            token.type = INT_LITERAL;
        }
        token.length = i - start;
        return true;
    }

    Token lexName(std::string str, int i, Token token){
        int length = 1;
        while(i + length != str.size()){
            char c = str[i + length];
            if(!isdigit(c) &&  c != '_' && !isalpha(c) ){
                break;
            }
            length++;
        }
        token.length = length;
        return token;
    }

    Token lexOperator(std::string str, int i, Token token){
        int length = 1;
        while(i + length != str.size()){
            char c = str[i + length];
            if(!oneOf(c, operators)){
                break;
            }
            length++;
        }
        token.length = length;
        return token;
    }

    struct CommentOption {
        int number_of_lines;
        int length;
    };

    CommentOption parseComment(std::string str, int i){
        
        CommentOption ret;
        ret.number_of_lines = 1;
        int length = 1;
        if(i + 1 == str.length()){
            ret.length = 1;
            return ret;
        }
        if(str[i + 1] == '('){
            length = 3;
            while(i + length < str.size()){
                if(str[i + length - 1] == ')' && str[i + length] == '#'){
                    length++;
                    break;
                }
                if(str[i - 1] == '\n'){
                    ret.number_of_lines++;
                }
                length++;
            }
        } else {
            length = 1;
            while(i + length < str.size()){
                char d = str[i + length];
                if(d == '\n'){
                    break;
                }
                length++;
            }
        }
        
        ret.length = length;
        return ret;
    }


    // resolve ambiguities with unary -
    // flag incorrect operators
    bool validate_operator(std::vector<Token>& lex, Token& token){
        
        if(!oneOf(token.value, allowed_operators)){
            cout << "Unsupported operator: " << token.value << endl;
            return false;
        }
        if(!oneOf(token.value, ambiguous_operators)){
            return true;
        }
        
        if(lex.size() == 0){
            token.type = UOP;
            return true;
        }
        bool unary = true;
        Token prev = lex[lex.size() - 1];
        switch( prev.type ){
            // case: closed bracket or literal type
            case CLOSED_BRACKET: 
            case NAME: case INT_LITERAL: case FLOAT_LITERAL: 
            case BOOL_LITERAL: case CHAR_LITERAL: case STRING_LITERAL:
                unary = false;  // must be a binary operator
                break;
            default:        unary = true;
        }
        
        // cout << prev.value << " " << unary << endl;
        if(unary){ token.type = UOP; }
        return true;
    }

}
// some more public functions
void printTokens(std::vector<Token> list){
    cout << "[";
    for(auto& token : list){
        cout << token.toString() << ", ";
    }
    cout << "]" << endl;
}

void printToken(Token token){
    cout << token.toString() << endl;
}

// main lexing function
LexerOutput lexer(std::string& input_string){
    input_string += "          ";   // whitespace buffer
    using namespace lexer_globals;
    using namespace lexer_internals;

    std::vector<Token> lex;
    unsigned int line = 1;
    unsigned int col = 1;
    bool success = true;
    

    for(int i = 0; i < input_string.size(); i++){
        int skip = 1;
        char c = input_string[i];
        Token token;
        token.line = line;
        token.column = col;

        // variables whose values to set:
        // token.type, token.value, token.length
        // must update line on every newline
        // must specify skip : number of characters to move forward

        // completely ignore space, \t, \r 
        if(oneOf(c, whitespace)){ 
            continue; 
        } 
        // if character is a backslash followed by whitespace then a newline,
        // it joins two lines together
        else if(c == '\\'){
            i++;
            while(i < input_string.size()){
                c = input_string[i];
                if(oneOf(c, whitespace)){} 
                else if(c == '\n'){ break; } 
                else { cout << "Invalid use of backslash" << endl;
                    success = false; goto exit;
                }
                i++;
            }
            line++; col = 1;
            continue;   //skips to after next newline
        } 
        // code comment
        else if( c == '#'){
            CommentOption result = parseComment(input_string, i);
            line += result.number_of_lines;
            i += result.length - 1;
            continue;
        // newlines and semicolons
        } else if(c == '\n'){
            token.type = EOL;
            token.length = 1;
            line++;
            col = 0;
            token.value = "\n";
        } else if(c == ';'){
            token.type = EOL;
            token.length = 1;
            col = 0;
            token.value = ";";
        } 
        // comma
        // any newlines after the last comma on a line are ignored
        else if(c == ','){
            token.type = COMMA;
            token.length = 1;
            while(i + skip != input_string.size()){
                char d = input_string[i + skip];
                if(d == '\n'){ line++; col = 0; } 
                else if(!oneOf(d, whitespace)){
                    break;
                }
                skip++;
            }
            token.value = ',';
        } 
        // float or int literal
        else if(isdigit(c)){
            // lexNumber modifies token
            bool success = lexNumber(input_string, i, token);
            if(!success){ success = false; goto exit; }
            skip = token.length;
        // character literal '
        // currently only single char and c escape sequence's allowed
        } else if(c == '\''){
            token.type = CHAR_LITERAL;
            int offset = 0;
            if(input_string[i + 1] == '\''){ 
                // ''
                success = false; cout << "Empty char literal not allowed" << endl; goto exit;
            } else if(input_string[i + 1] == '\\'){
                // escape sequence
                if(!oneOf(input_string[i + 2], escape_sequences) || input_string[i + 3] != '\''){
                    success = false; cout << "Invalid char literal" << endl; goto exit;
                }
                token.length = 4;
                skip = 4;
                token.value = input_string.substr(i, 4);
            } else {
                // single character
                if(input_string[i + 2] != '\''){
                    success = false; cout << "Invalid char literal" << endl; goto exit;
                }
                token.length = 3;
                skip = 3;
                token.value = input_string.substr(i, 3);
            }
        } 
        // string literal - currently only normal chars and c-escape sequences allowed
        else if(c == '\"'){
            token.type = STRING_LITERAL;
            int index = i + 1;
            while(true){
                if(index >= input_string.size()){
                    cout << "Lexing error: string does not end" << endl;
                    success = false;
                    goto exit;
                }
                char current = input_string[index];
                if(current == '\\'){
                    if(!oneOf(input_string[index + 1], escape_sequences)){
                        success = false; cout << "Invalid escape sequence in string" << endl; goto exit;
                    }
                    index = index + 2;
                } else if(current == '\"'){
                    index++;
                    break;
                } else {
                    index++;
                }
            }
            token.length = index - i;
            skip = token.length;
            token.value = input_string.substr(i, token.length);
        } else if(isalpha(c) || c == '_'){
            //handles words
            token.type = NAME;
            token = lexName(input_string, i, token);
            skip = token.length;
            token.value = input_string.substr(i, token.length);
            // handle operators that are words
            if(oneOf(token.value, operator_words)){
                token.type = BOP;
            }
            if(token.value == "true" || token.value == "false"){
                token.type = BOOL_LITERAL;
            }
            // handle keywords (if, func, etc.)
            else if(oneOf(token.value, keywords)){
                token.type = KEYWORD;
            } else if(token.value == "_"){
                token.type = UNDERLINE;
            }
        } else if(oneOf(c, operators)){
            // either unary or binary operator
            token.type = BOP;
            token = lexOperator(input_string, i, token);
            skip = token.length;
            token.value = input_string.substr(i, token.length);
            //assignment operator
            if(oneOf(token.value, assignment)){
                token.type = ASSIGNMENT;
            }
            if(validate_operator(lex, token) == false){
                success = false;
                goto exit;
            }
        } 
        // open bracket (, {, [
        else if(oneOf(c, open_brackets)){
            token.type = OPEN_BRACKET;
            token.length = 1;
            token.value = c;
        } 
        // closed bracket ), ]
        else if(c == ']' || c == ')'){
            token.type = CLOSED_BRACKET;
            token.length = 1;
            token.value = c;
        } 
        // closed bracket }
        else if(c == '}'){
            token.type = CLOSED_BRACKET;
            // to make parsing easier, add EOL if the previous token is not an EOL
            if(lex.back().type != EOL){
                Token t2;
                t2.line = line;
                t2.column = col;
                t2.type = EOL;
                t2.value = ';';
                lex.push_back(t2);
            }
            token.length = 1;
            token.value = c;
        }
        // invalid char
        else {
            success = false;
            cout << "Error: Non-supported character found: " << input_string[i] << endl; 
            break;
        }

        //update loop variables
        col++;            //token # of line
        i += skip - 1;    //skip over a few values
        lex.push_back(token);
    } 
    exit:


    Token endline = { EOL, line + 1, 0, 0, ";" };
    Token end = { END, line + 2, 0, 0, "" };
    lex.push_back(endline);
    lex.push_back(end);

    if(success){
        cout << "Lexing complete" << endl;
    } else {
        cout << "Lexing error" << endl;
    }
    //printTokensDeep(lex);
    printTokens(lex);
    LexerOutput retval = {lex, success};
    return retval;
}


// int main(){
//     string input = "3 + 3.3e-10 + 'a' + '\\n' + \"Hi\"";
//     cout << input << endl;
//     LexerOutput output = lexer(input);
//     if(output.success){
//         cout << "Lexing successful" << endl;
//         printTokens(output.tokens);
//     } else {
//         cout << "Oh no, Lexing failed" << endl;
//         printTokens(output.tokens);
//     }
//     return 0;
// }