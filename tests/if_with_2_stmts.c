void foo() {
  int a;
  if (1) {
    a = 5 ? a : 1;
    a = 0;
  }
}
