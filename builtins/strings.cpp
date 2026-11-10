#pragma once
#include <string.h>
#include <cstring>
#include "basics.cpp"
using namespace std;


struct $string {    
    enum class Tag {
        const_str,
        heap_str,
    } tag;
    
    union {
        std::string heap_str;
        struct {
            const char *ptr;
            size_t length;
        } const_str;
    };

    $string(const char *str_ptr, size_t length) 
        : tag(Tag::const_str), const_str{str_ptr, length}{};
    $string(const std::string& str) 
        : tag(Tag::heap_str), heap_str(str){};
    // weird stuff - implement copy and assignment operators to be simple memcopy
    // in sleek, all assignments are memcopies
    $string(const $string& other) {
        memcpy(this, &other, sizeof($string));
    }
    $string& operator=(const $string& other) {
        memcpy(this, &other, sizeof($string));
        return *this;
    }
    // custom destructor - deallocate heap memory
    ~$string(){
        if(tag == Tag::heap_str){
            heap_str.~string();
        };
    }
};

//length
//sadly, because sleek doesn't have 64-bit ints, we have to cast it down to $int
$int $length($string str){
    if(str.tag == $string::Tag::heap_str){
        return $int{ (int) str.heap_str.length()};
    } else {
        return $int{ (int) str.const_str.length };
    }
}

//copy string
$string copy($string str){
    if(str.tag == $string::Tag::heap_str){
        return $string{str.heap_str};   // copy the heap string
    } else {
        return $string{str};    // simple memcopy
    }
}
// get element
// char get(size_t index){
//     if(tag == Tag::heap_str){
//         if(index >= heap_str.size()){
//             throw std::runtime_error("Error - string index out of bounds");
//         }
//         return heap_str[index];
//     } else {
//         if(index >= const_str.length){
//             throw std::runtime_error("Error - string index out of bounds");
//         }
//         return const_str.ptr[index];
//     }
// }
// change element
// void set(size_t index, char c){
//     if(tag == Tag::heap_str){
//         if(index >= heap_str.size()){
//             throw std::runtime_error("Error - string index out of bounds");
//         }
//         heap_str[index] = c;
//     } else {
//         if(index >= const_str.length){
//             throw std::runtime_error("Error - string index out of bounds");
//         }
//         // update to heap string
//         tag = Tag::heap_str;
//         const char *old_str = const_str.ptr;
//         size_t length = const_str.length;
//         new (&heap_str) string(old_str, length);
//         heap_str[index] = c;
//     }
// }


// print to console
// fwrite is just printf without formatting and where length is known
// cout could have also been used, but fwrite is faster
$none $print($string str){
    if(str.tag == $string::Tag::heap_str){
        fwrite(str.heap_str.c_str(), sizeof(char), str.heap_str.length(), stdout);
    } else {
        fwrite(str.const_str.ptr, sizeof(char), str.const_str.length, stdout);
    }
    return $none{};
}