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
    while n > 1 : n -= 1 {
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

Any datatype can be prefixed with `*` to signify that it is a pointer to that datatype.
For example, `*i32` represents the address of a 32 bit integer.
See the memory section to learn more.

## literals

There are four different kinds of literals in Anx: numbers, booleans, strings, arrays, and maps. Notice how these correspond to the different fundamental datatypes available.

A number literal is a sequence of digits.
For example, `123` is an integer literal. The size of the literal is at least 32 bits, but is automatically enlarged up to 128 bits if necessary.
If you add a decimal point, the number becomes a double (f64) precision float. For example, `123.456` is a double literal.
You can add underscores after the first digit to make the number more readable. For example, `12_345.6` is the same as `12345.6`.
If you want to specify a specific type or size, you can add a suffix to the number.
For example, `123u16` is a 16 bit unsigned integer literal.
Likewise, `123f32` is a 32 bit float literal.
You can only use numeric types as suffixes (meaning every primitive datatype, sans `bool`).
Also note that the literal integer `nil` is equivalent to `0`.

Boolean literals are `true` and `false`. These are equivalent to `1u8` and `0u8`, respectively.

A string literal is a sequence of characters surrounded by double quotes.
For example, `"hello world"` is a string literal.

An array literal is a sequence of values surrounded by square brackets.
For example, `[1, 2, 3]` is an array literal.
The type of the array is inferred from the type of the values.
However, note that all values in the list must be of the same type.

A map literal is a sequence of key-value pairs surrounded by curly braces.
For example, `{"a": 1, "b": 2, "c": 3}` is a map literal.
The type of the map is inferred from the types of the keys and values.
However, all keys must be of the same type, and all values must be of the same type.

## generics / templating

```
fn add[T](a: T, b: T): T {
    ret a + b;
}
```

## object orientation

```
class MyArray[T] {
    var length: u64;
    var capacity: u64;
    var start: *T;

    pub fn _init(len: u64) { # constructor
        start = @alloc(len * @size(T)); # compiler memory allocation builtin
        length = len;
        capacity = len;
    }

    pub fn _get(index: u64): T { # array indexing overload (i.e. x[5])
        if index >= length
            @panic("index out of bounds"); # compiler panic builtin

        ret start[index];
    }

    pub fn _set(index: u64, value: T) { # array indexing assignment overload (i.e. x[5] = 10)
        if index >= length
            @panic("index out of bounds");

        start[index] = value;
    }

    pub fn add(value: T) { # add an element to the end of the array
        if length == capacity {
            start = @realloc(start, length * 2 * @size(T)); # compiler memory reallocation builtin
            capacity <<= 1;
        }

        start[length] = value;
        length += 1;
    }
}

fn main() {
    var x = MyArray[i32](2); # create a new array of 2 integers

    x.add(5); # add 5 to the end of the array
    x.add(10); # add 10 to the end of the array

    x[0] = 1; # set the first element to 1
    x[1] = 2; # set the second element to 2

    # x.length would be an error because length is a private field. you would need to make it public or use a getter method

    var y = x[0] + x[3]; # get the first element and add it to the fourth element

    @out(y); # compiler output builtin, prints 11 here
}
```

## memory

Memory in Anx is garbage collected, so the user does not need to worry about freeing memory.

## input / output

## threading

## coercion
