#pragma once
// #include "parser_globals.h"

// helper functions
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

// given an iterator, print the rest of the line (until EOL) along with line number
void printErrorLine(Iter it){
    cout << "Error occured on line " << (*it).line << ":" << endl;
    cout << endl;
    while(it->type != EOL && it->type != END){
        cout << (*it).value << ' ';
        it = it + 1;
    }
    cout << endl;
}