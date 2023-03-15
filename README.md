# Anx

Anx is an experimental, modern, statically-typed programming language written in C++.
It seeks to provide the quality-of-life features of more dynamic languages without the performance penalties typically associated with them.

Visit <a href="design.md">design.md</a> for more information about the philosophies guiding Anx.

Please note that Anx is in its early stages of development, and there are several key language features that are still being prototyped.
For a peek at features in development, see the <a href="dev">dev</a> directory.

# Building

**Anx requires LLVM >= 15.0.7 to build properly.** Any reasonable installation method should be fine as long as `llvm-config` is available in the path.

If your editor/language server is acting up about not finding the LLVM include files, I recommend using a tool like [bear](https://github.com/rizsotto/Bear) to generate `compile_commands.json` when you compile.

After cloning this repository and making sure LLVM is installed on your machine simply run `make`. The compiled binary can be found in the resultant `bin` directory.
