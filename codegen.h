#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include "utils.cpp"

#include "parser.cpp"


class Codegen {
    private:
        string output_file;
        std::ofstream file_stream;
        bool file_open;
        string indent_str;
    public:

    void genProgram(const Program& p);
    void genFunction(const Function& program);
    void declareFunctions(const unordered_map<string, Function>& functions);
    void declareStructs(const vector<Type>& decl_order);
    void genCodeblock(const Codeblock& cb);
    void genMain(const Function& program);
    void genBasicLine(int line_num, const Basic_Line& line, const SymbolTable& vars);
    void genExpression(const Expression& e);
    void genIfBlock(const IfBlock& ifBlock);
    void genWhileBlock(const WhileBlock& whileBlock);

    Codegen(){}
    // prepare file stream
    // return false if can't be opened
    bool init(string output_file){
        this->output_file = output_file;
        file_stream.open(output_file);
        if (file_stream.is_open()) {
            file_open = true;
            return true;
        } else {
            file_open = false;
            return false;
        }
    }
    
    ~Codegen(){
        if(file_open){
            file_stream.close();
        }
    }

    // increase indent by 1
    void indent(){
        indent_str.push_back(' ');
        indent_str.push_back(' ');
        return;
    }
    // decrease the indent by 1, to minimum of zero
    void dindent(){
        if(indent_str.size() < 2){
            cout << "Oops - negative indent in writer" << endl;
        } else {
            indent_str.pop_back();
            indent_str.pop_back();
        }
    }

    // write tab to file
    void tab(){
        raw(indent_str);
    }

    // print tab, then arguements separated by space
    template<typename T, typename...A>
    void tout(const T& first, const A&...args) {
        tab();
        file_stream << first;
        ((file_stream << " ", file_stream << args), ...);
    }

    // print arguement
    template<typename T, typename...A>
    void out(const T& first, const A&...args) {
        file_stream << first;
        ((file_stream << " ", file_stream << args), ...);
    }

    // write arguements, separated by space, end with newline
    template<typename T, typename...A>
    void nout(const T& first, const A&...args) {
        file_stream << first;
        ((file_stream << " ", file_stream << args), ...);
    }


    // print tab, then arguements separated by space
    template<typename T, typename...A>
    void line(const T& first, const A&...args) {
        tab();
        file_stream << first;
        ((file_stream << " ", file_stream << args), ...);
        file_stream << "\n";
    }

    // print arguement
    template<typename T, typename...A>
    void raw(const T& first, const A&...args) {
        file_stream << first;
        ((file_stream << args), ...);
    }
    //print each arguement (except first two)
    //@sep is printed between each arguement
    //@end is printed at the end
    template<typename T, typename...A>
    void custom(const char* sep, const char* end, const T& first, const A&...args) {
        file_stream << first;
        ((file_stream << sep, file_stream << args), ...);
        file_stream << end;
    }
};

// int main(){
//     using Codegen;
//     Codegen cg("output.c");
//     cg.init()
//     cg.out("hello, world!", "My name is Armaan");
//     cg.out("This is file writer!");
//     cg.indent++;
//     cg.out("y = 0;")
//     cg.lb
//     br()
    
//     return 0;
// }