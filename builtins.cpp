#pragma once

#include <cmath>
#include <iostream>
#include <exception>
#include "ast.cpp"
// features not in C: operator overloading, namespaces, private/public, references, templates

struct $none {};

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
void print($none var){
    printf("none");
}

void print($int var){
    printf("%d", var.v);
}

void print($float var){
    printf("%lf", var.v);
}

void print($byte var){
    printf("%d", var.v);
}

void print($char var){
    printf("%c", var.v);
}

void print($bool var){
    if(var.v == true){
        printf("true");
    } else {
        printf("false");
    }
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

// already exists in cmath

// int main(){

//     auto x = $int{4};
//     auto y = $int{0};
//     print(idiv(x, y));
//     print($char{':'});
//     print($char{')'});
//     return 0;
// }
