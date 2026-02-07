#include "builtins.cpp"
struct $A {
  $int int;
};
struct $B {
  $int int;
};
struct $C {
  $A A;
  $B B;
};
struct $t_$A_$C$ {
  $A e0;
  $C e1;
  $A ee0;
  $C ee1;
};
struct $t_$int_$t_$A_$C$$ {
  $int e0;
  $t_$A_$C$ e1;
  $int ee0;
  $t_$A_$C$ ee1;
};
struct $D {
  $int int;
  $t_$int_$t_$A_$C$$ (int,(A,C));
};
struct $t_$int_$int$ {
  $int e0;
  $int e1;
};
struct $t_$int_$string$ {
  $int e0;
  $string e1;
};


$(int,int) example($int $x, $(int,int) $y, $D $z);

int main(){
  $int x = $int{5};
  $float y = $float{3.0};
  $t_$int_$string$ z = $t_$int_$string${$int{3}, $string{"abcd", 4}};
  return 0;
}


$(int,int) $example($int x, $(int,int) y, $D z){
}

