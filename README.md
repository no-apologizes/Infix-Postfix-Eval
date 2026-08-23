### What is this?

This is a infix to postfix evaluator, which in simple terms is infix math: ```3 + 2 * 1``` is converted into postfix math: ```3 2 1 * +```, and then evaluated ```Result: 5```.

Tested with:
- GCC 16.2.1 (or Clang 22.1.8)
- CMake 4.4.2
- Arch(Cachyos) Linux

This should work with any GCC version in the last two decades or any modern Clang version as it uses computed gotos, and at least CMake 3.25.

### Build:
```
git clone https://github.com/no-apologizes/Infix-Postfix-Eval.git

cd Infix-Postfix-Eval

mkdir build && cd build

cmake ..

cmake --build .
```

### Usage:

```
./infix_postfix_eval <file> // Or the provided 5thnm.exe

Debug:
./infix_postfix_eval <file> -d

```

### AI Disclaimer:
AI was used to find bugs and edge cases, all code was written by me, I can explain every line, AMA.
