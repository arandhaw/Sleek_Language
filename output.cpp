#include "builtins.cpp"


int main(){
  $array1D<$int, 5> myArray = $array1D<$int, 5> {$int{1}, $int{2}, $int{3}, $int{4}, $int{5}};
  $int x = $int{4};
  while(gt(x, $int{0}).v){
    $print(myArray.get(x.v));
  }
  return 0;
}

