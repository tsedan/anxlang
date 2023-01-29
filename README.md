# Anx

Anx is a highly experimental systems programming language written in C++.

# Design Philosophy

The purpose of Anx is to simplify the most common use cases for C, while also including a select few of the quality-of-life features that modern languages provide.

We're starting simple, and expanding only once a first working version is created.

Here are a list of all of the datatypes available in Anx:
```
i32
bool
void
```

Here's a basic sample of the language syntax:

```
fn recurseAdd(n: i32, s: i32) i32 {
    if (n == 0) ret s;
    var x: i32 = s * 2 + 1;

    ret recurseAdd(n - 1, x);
}
```

Everything below this point is just a draft and should not be taken seriously.
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

```
fn openFile(path: *u8) !bool {
    # the ! indicates that this function can return an error

    # try to open the file. if an error occured, return it.
    try @fopen(path, "r");

    ret true; # if we got here, success!
}

fn main() void {
    # try to open a file.
    openFile("doesnt_exist.txt") catch |err| {
        # this block is executed if an error occured
        @out(f"Error: %s{err}\n");
    };
}
```
