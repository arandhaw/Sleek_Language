#include "builtins.cpp"

$int haHa($int $x, $float $y);

int main(){
  $int x = $int{10};
  $float y = $float{10.4};
  $int z = $haHa(x, y);
  $print(z);
  return 0;
}


$int $haHa($int x, $float y){
  $print(x);
  $print(y);
  return x;
}

