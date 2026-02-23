struct A {
    int x;

    int foo() { return 42; }
};

int main() {
    A a = A(5);

    return a.foo();

}