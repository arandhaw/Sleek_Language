#include <stdio.h>

struct greet {
    int x;
    int y;
};

#define MY_MACRO 10

int main(){
    struct greet name = {3, 5};
    const int x = 10;
    int arr[x];
    printf("hello world");
    // fprintf(stderr, "Urk!\n");
    return 0;
}