Const <int>

Const <float>

Const <string>

Const <aggregate>

Load <>

Store 

Access

Extract

Offptr

Call

Ret

Jump

Brif

Phi

Unop

Binop

Cast

Cmp


## Aggregates

Assume Vec3 = { f64, f64, f64 }.


(Vec3) -> ... 

(...) -> Vec3


use:
    %1 = call: Vec3 (...) 


(Vec3) -> Vec3
    



foo :: () -> Vec3;

a: Vec3 = foo();
b: f64 = a.y;

%1 = call foo: Vec3 ()
%2 = extract %1: Vec3, 1: i64

call foo (%x: *Vec3)
%2 = access %x: *Vec3, 1: i64
%3 = load f64, %2: *f64

