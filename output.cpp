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


int main(){
  $t_$int_$int$ x = $t_$int_$int${$int{3}, $int{4}};
  $int y = (x.e0);
  return 0;
}

