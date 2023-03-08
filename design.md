# design philosophy

Please note that the syntax used in this document is not finalized and subject to change. If you have suggestions for improvement, open an issue or PR!

## syntax example

Here's a sample of the basic language syntax:

```
# a public function that returns the nth fibonacci number
pub fn fib(n: i32): i32 {
    if !n ret 0;

    var prev_prev: i32, prev = 0, curr = 1;

    # while n > 1, calculate the next number. decrement n by 1 each step.
    for n > 1 : n -= 1 {
        prev_prev = prev;
        prev = curr;
        curr = prev_prev + prev;
    }

    ret curr;
}
```

## datatypes

Here is a list of of the primitive datatypes available in Anx:

```
i8, i16, i32, i64, i128
u8, u16, u32, u64, u128
f32, f64
bool
```

`void` is also a datatype, but is only valid as a function return type.

There are also a few compound datatypes that are automatically available in every project:

```
str, map, arr
```

Any datatype can be prefixed with `*` to signify that it is an address.
For example, `*i32` is a pointer to a 32 bit integer.
See the memory section to learn more.

## generics / templating

```
fn add[T](a: T, b: T): T {
    ret a + b;
}
```

## object orientation

```
obj arr[T] {
    var length: u64;
    var start: *T;
}
```

## literals

## memory

Memory in Anx is garbage collected, so the user does not generally need to worry about memory management.

## input / output

## threading
