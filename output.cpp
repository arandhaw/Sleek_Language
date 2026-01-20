#include "builtins.cpp"
struct $Dog {
  $string name;
  $string type;
};
struct $Xray {
  $int size;
  $string y;
  $Dog dog;
};

PRIMITIVE_TYPE
struct output
{
  /* data */
};


int main(){
  $int x = $int{0};
  $int i = $int{0};
  while(lt(i, $int{10}).v){
    x = add(x, i);
    i = add(i, $int{1});
    }
    $print(x);
    return 0;
  }

