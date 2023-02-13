# Anx

Anx is a highly experimental systems programming language written in C++.

**Note: Anx requires LLVM >= 15.0.7 in order to build properly (llvm-config must be available).**

# Design Philosophy

The purpose of Anx is to simplify the most common use cases for C, while also including a select few of the quality-of-life features that modern languages provide.

Please note that Anx is still in its early stages of development, and there are several key language features that are still being prototyped.
For a peek at language features in development, see the `test` directory.

Here is a list of all of the datatypes available in Anx:
```
i8, i16, i32, i64, i128
u8, u16, u32, u64, u128
f32, f64
void (only valid as a function return type)
```

Here's a basic sample of the language syntax:

```
fn recurseAdd(n: i32, s: i32) i32 {
    if (n == 0) ret s;
    var x: i32 = s * 2 + 1;

    ret recurseAdd(n - 1, x);
}
```

Here's Anx reading user input and printing it back out:

```
fn main() void { # return type required, even if void
    # make a zero-initialized buffer of 256 chars
    var buf: [256]u8 = "";

    # heap allocate a string of length 128
    # notice that the ptr is immutable ("let")
    # however, the data inside the ptr is mutable
    let string: *u8 = @alloc(128);

    # free the string
    @free(string);

    # ask the user for input
    @out("Input something: ");

    # read up to 256 chars from stdin into the buffer
    @in(buf, sizeof buf);

    # print the buffer to stdout using a formatted string
    @out(f"You inputted: %s{buf}\n");
}
```

Errors and safety are a first-class concern in Anx. Here's an example of error handing:
(note these features are still in the drafting phase)

```
fn openFile(path: *u8) !u8 {
    # the ! indicates that this function can return an error

    # try to open the file. if an error occured, return it.
    try @fopen(path, "r");

    ret 1; # if we got here, success!
}

fn main() void {
    # try to open a file.
    openFile("doesnt_exist.txt") catch |err| {
        # this block is executed if an error occured
        @out(f"Error: %s{err}\n");
    };
}
```
