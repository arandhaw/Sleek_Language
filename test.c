#include <stdio.h>

struct greet {
    int x;
    int y;
};

int main(){
    struct greet x = {3, 5};
    printf("hello world");
    return 0;
}