#pragma once

#include <cmath>
#include <iostream>
#include <exception>
#include "ast.cpp"
// features not in C: operator overloading, namespaces, private/public, references, templates

struct $none{};

struct $int {
    const static int MAX = 2147483647;
    const static int MIN = -2147483648;
    int v;
};

struct $float {
    double v;
};

struct $byte {
    unsigned char v;
};

struct $bool {
    bool v;
};

struct $char {
    char v;
};

// basic operators:
// 
// print declarations
$none $print($none var){
    printf("none");
    return $none{};
}

$none $print($int var){
    printf("%d", var.v);
    return $none{};
}

$none $print($float var){
    printf("%lf", var.v);
    return $none{};
}

$none $print($byte var){
    printf("%d", var.v);
    return $none{};
}

$none $print($char var){
    printf("%c", var.v);
    return $none{};
}

$none $print($bool var){
    if(var.v == true){
        printf("true");
    } else {
        printf("false");
    }
    return $none{};
}

$int add($int x, $int y){
    return $int{x.v + y.v};
}

$float add($float x, $int y){
    return $float{x.v + y.v};
}

$float add($int x, $float y){
    return $float{x.v + y.v};
}

$float add($float x, $float y){
    return $float{x.v + y.v};
}

$int sub($int x, $int y){
    return $int{x.v - y.v};
}

$float sub($float x, $int y){
    return $float{x.v - y.v};
}

$float sub($int x, $float y){
    return $float{x.v - y.v};
}

$float sub($float x, $float y){
    return $float{x.v - y.v};
}

$int mul($int x, $int y){
    return $int{x.v * y.v};
}

$float mul($float x, $int y){
    return $float{x.v * y.v};
}

$float mul($int x, $float y){
    return $float{x.v * y.v};
}

$float mul($float x, $float y){
    return $float{x.v * y.v};
}

$float div($int x, $int y){
    return $float{ (double) x.v / y.v };
}

$float div($float x, $int y){
    return $float{x.v / y.v};
}

$float div($int x, $float y){
    return $float{x.v / y.v};
}

$float div($float x, $float y){
    return $float{x.v / y.v};
}

// integer division operator
$int idiv($int x, $int y){
    if(y.v == 0){
        throw std::runtime_error("Error - division by zero");
    }
    return $int{x.v / y.v};
}

$int iexp($int base, $int exponent){
    int x = base.v;
    int b = exponent.v;
    if(b >= 0){
        int ans = 1;
        while(b != 0){
            if(b % 2 == 1)
                ans *= b;
            x = x*x; //x, x^2, x^4, x^8, ...
            b = b/2;
        }
        return $int{ans};
    } else {
        throw std::runtime_error("Error - negative exponent in integer exponent");
    }
}

$float exp($int x, $int y){
    return $float{pow((double) x.v, (double) y.v)};
}

$float exp($float x, $int y){
    return $float{pow(x.v, (double) y.v)};
}

$float exp($int x, $float y){
    return $float{pow((double) x.v, y.v)};
}

$float exp($float x, $float y){
    return $float{pow(x.v, y.v)};
}

// modulus operator %
// unsure of behaviour for negative numbers
$int mod($int x, $int y){
    if(y.v == 0){
        throw std::runtime_error("Error - modulus of zero is not allowed");
    }
    return $int{x.v % y.v};
}

$int increment($int x){
    return $int{x.v + 1};
}

$int decrement($int x){
    return $int{x.v - 1};
}

// and operator
$bool logic_and($bool x, $bool y){
    return $bool{x.v && y.v};
}

// or operator
$bool logic_or($bool x, $bool y){
    return $bool{x.v || y.v};
}

// not operator
$bool logic_not($bool x, $bool y){
    return $bool{!x.v};
}

// == operator
// no equals operator for float
$bool equals($int x, $int y){
    return $bool{x.v == y.v};
}

$bool equals($char x, $char y){
    return $bool{x.v == y.v};
}

$bool equals($byte x, $byte y){
    return $bool{x.v == y.v};
}

$bool equals($bool x, $bool y){
    return $bool{x.v == y.v};
}

// != operator
// no equals operator for float
$bool neq($int x, $int y){
    return $bool{x.v != y.v};
}

$bool neq($char x, $char y){
    return $bool{x.v != y.v};
}

$bool neq($byte x, $byte y){
    return $bool{x.v != y.v};
}

$bool neq($bool x, $bool y){
    return $bool{x.v != y.v};
}

// >, >=, <, <=

$bool gt($int x, $int y){
    return $bool{x.v > y.v};
}

$bool gt($float x, $float y){
    return $bool{x.v > y.v};
}

$bool gt($byte x, $byte y){
    return $bool{x.v > y.v};
}

$bool geq($int x, $int y){
    return $bool{x.v >= y.v};
}

$bool geq($float x, $float y){
    return $bool{x.v >= y.v};
}

$bool geq($byte x, $byte y){
    return $bool{x.v >= y.v};
}

$bool lt($int x, $int y){
    return $bool{x.v < y.v};
}

$bool lt($float x, $float y){
    return $bool{x.v < y.v};
}

$bool lt($byte x, $byte y){
    return $bool{x.v < y.v};
}

$bool leq($int x, $int y){
    return $bool{x.v <= y.v};
}

$bool leq($float x, $float y){
    return $bool{x.v <= y.v};
}

$bool leq($byte x, $byte y){
    return $bool{x.v <= y.v};
}


// unary minus
$int unary_minus($int x){
    return $int{-x.v};
}

$float unary_minus($float x){
    return $float{-x.v};
}

// int main(){

//     auto x = $int{4};
//     auto y = $int{0};
//     print(idiv(x, y));
//     print($char{':'});
//     print($char{')'});
//     return 0;
// }
