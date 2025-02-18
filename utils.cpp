#pragma once
#include <iostream>
#include <ostream>
#include <vector>
#include <map>
#include <unordered_map>

class None {};


template<typename T, typename U>
class Result {
    private:
        bool data_valid;
        union {
            // bool dummy;
            T data_v;
            U error_v;
        };
    public:
        // copies data into result object
        Result(const T& data){
            data_valid = true;
            new (&data_v) T(data);
        }
        // copies error into result object
        Result(const U& error){
            data_valid = false;
            new (&error_v) U(error);
        }
        // check if result is valid
        bool valid() const {
            return data_valid;
        } 
        T& data() {
            if(!data_valid){
                std::cout << "ERROR - illegal union access in get_data" << std::endl;
            }
            return data_v;
        }
        U& error() {
            if(data_valid){
                std::cout << "ERROR - illegal union access in get_data" << std::endl;
            }
            return error_v;
        }
        // delete assignment operator - maybe implement another day
        Result& operator=(const Result&) = delete;

        // copy constructor
        Result(Result const& copy){
            data_valid = copy.data_valid;
            if(data_valid){
                new (&data_v) T(copy.data_v);
            } else {
                new (&error_v) U(copy.error_v);
            }
        }
        ~Result(){
            if(data_valid){
                data_v.~T();
            } else {
                error_v.~U();
            };
        }
};


template<typename T>
struct Option{
    T result;
    bool valid;

    Option(bool v) : valid(v){}
    Option(T t, bool v) : result(t), valid(v) {}
    Option(T t) : result(t) {
        valid = true;
    }
};

template<typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    out << "[";
    size_t last = v.size() - 1;
    for(size_t i = 0; i < v.size(); ++i) {
        out << v[i];
        if (i != last) 
            out << ", ";
    }
    out << "]";
    return out;
}
// function to printi std::map<T,V>
template<typename T, typename U>
std::ostream& operator<< (std::ostream& out, const std::map<T, U>& v) {
    if(v.empty()){
        out << "{}";
        return out;
    }
    out << "{";
    for(auto& i : v) {
        out << i.first << " : " << i.second << ", ";
    }
    out << "\b\b}";
    return out;
}

// function to printi std::unordered_map<T,V>
template<typename T, typename U>
std::ostream& operator<< (std::ostream& out, const std::unordered_map<T, U>& v) {
    if(v.empty()){
        out << "{}";
        return out;
    }
    out << "{";
    for(auto& i : v) {
        out << i.first << " : " << i.second << ", ";
    }
    out << "\b\b}";
    return out;
}

// Print each arguement separated by " "
// std::endl at end
template<typename T, typename...A>
void print(const T& first, const A&...args) {
    std::cout << first;
    ((std::cout << " ", std::cout << args), ...);
    std::cout << std::endl;
}

//print each arguement (except first two)
//@sep is printed between each arguement
//@end is printed at the end
template<typename T, typename...A>
void print_custom(const char* sep, const char* end, const T& first, const A&...args) {
    std::cout << first;
    // fold - fancy c++17 feature
    ((std::cout << sep, std::cout << args), ...);
    std::cout << end;
}

// print each arguement
// no spaces/newline added in
template<typename...A>
void print_raw(const A&...args) {
    ((std::cout << args), ...);
}