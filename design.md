# design philosophy

Please note that the syntax used in this document is not finalized and subject to change. If you have suggestions for improvement, open an issue or PR!

## syntax example

Here's a basic sample of the language syntax:

```
# a public function that returns the nth fibonacci number
pub fn fib(n: i32): i32 {
    if !n ret 0;

    var prev_prev: i32, prev = 0, curr = 1;

    # while n > 1, calculate the next number. decrement n by 1 each step.
    while n > 1 : n = n - 1 {
        prev_prev = prev;
        prev = curr;
        curr = prev_prev + prev;
    }

    ret curr;
}
```

## datatypes

Here is a list of of the current datatypes available in Anx:
```
i8, i16, i32, i64, i128
u8, u16, u32, u64, u128
f32, f64
bool
```

## literals

## memory

## input/ouput

Here's Anx reading user input and printing it back out:

```
# main function is the entry point and always public
fn main() { # note that when we do not specify a return type, it defaults to void
    # make a zero-initialized buffer of 256 chars
    var buf: [256]u8 = "";

    # heap allocate a string of length 128
    # note that the string is garbage collected automatically
    var string: *u8 = @new(128);

    # ask the user for input
    @out("Input something: ");

    # read up to 256 chars from stdin into the buffer
    @in(buf, @size(buf));

    # print the buffer to stdout using a formatted string
    @out(`You inputted: %s{buf}\n`);
}
```

## threading
