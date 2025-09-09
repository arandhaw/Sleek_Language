#include "builtins.cpp"

$int haHa($int $x, $float $y);

int main(){
  $int x = $int{10};
  $float y = exp(mul(add($float{3.1}, $int{4}), $int{5}), $int{2});
  $int z = $haHa(x, y);
  $print(z);
  return 0;
}


$int $haHa($int x, $float y){
  $print(x);
  $print(y);
  return x;
}

