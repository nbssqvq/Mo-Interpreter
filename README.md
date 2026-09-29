# Mo-Interpreter

[中文版](README.zh-CN.md)

A Mo language interpreter written in C++17. It reads `.mo` source files, parses statements into a syntax tree, and executes them. The language supports integer arithmetic, variables, conditional branches, loops, character and integer I/O, and addressing operations backed by virtual-machine memory.

## Features

- Variables can be assigned without prior declarations and store signed 32-bit integers.
- Supports prefix expressions and parenthesized infix expressions.
- Supports addition, subtraction, multiplication, division, modulo, comparisons, bitwise AND, and bitwise OR.
- Supports `if` / `else` branches and `while` loops.
- Supports integer and character input/output.
- Supports virtual-machine memory access through `@`. The VM provides 262,144 32-bit integer cells, totaling 1 MiB.

## Quick Start

### Requirements

- `g++` with C++17 support.
- GNU Make.
- On Windows, `g++` and `make` can be used through MinGW-w64 or MSYS2.

Using a path containing only ASCII characters is recommended to reduce path-encoding issues on Windows.

### Build

Run the following command in the project root:

```sh
make
```

This creates `Mo.exe` on Windows and `Mo` on Linux and macOS. To remove generated files:

```sh
make clean
```

### Run

Windows PowerShell:

```powershell
.\Mo.exe
```

Linux or macOS:

```sh
./Mo
```

After startup, the interpreter scans the `MoProgram` directory next to the executable and lists its `.mo` programs by filename. Enter a number to run the corresponding program, enter `0` to type a filename (for example, `factorial` or `factorial.mo`), or enter `-1` to exit. When a program finishes, Windows prompts you to press any key; other platforms prompt you to press Enter.

Do not include spaces when entering a filename manually; files whose names contain spaces can still be selected from the menu. On Linux and macOS, start the executable from a location where it can find `MoProgram`; on macOS, you can run it from the project root.

## Mo Language Syntax

Mo uses whitespace (spaces, newlines, and so on) to separate tokens. Writing one statement per line is recommended, although whitespace and line breaks inside expressions may be adjusted freely. Statements do not use semicolons.

### Statements

| Purpose | Syntax | Description |
| --- | --- | --- |
| Assignment | `let <variable> <expression>` | A variable is created automatically on its first assignment. |
| Memory assignment | `let @ <address-expression> <value-expression>` | Writes a value to the specified memory address. |
| Integer input | `read <variable>` | Reads an integer from standard input. |
| Character input | `get <variable>` | Reads the next non-whitespace character and stores its character code. |
| Integer output | `write <expression>` | Prints the integer result without automatically adding a newline. |
| Character output | `put <expression>` | Converts the result to a character and prints it without a newline. `put 10` is commonly used for a newline. |
| Conditional | `if <condition> ... end` | Executes the branch when the condition is nonzero. See the example below for the optional `else` branch. |
| Loop | `while <condition> ... end` | Repeats the loop body while the condition is nonzero. |

`if` / `else` example:

```text
read score
if > score 59
  put 89
else
  put 78
end
put 10
```

This prints character code `89` (`Y`) when the condition is true, or `78` (`N`) otherwise. The then and else branches of an `if` share one `end`; each nested block needs its own `end`. A `while` loop also ends with `end`. A final top-level `end` is allowed, matching the examples in this repository.

### Expressions

Prefix expressions place the operator before its operands. A binary operator is followed by two operands in order. Expressions can be nested:

```text
let total + price tax
let result * + a b 2
```

Infix expressions must be enclosed in parentheses, which can also be used to change precedence:

```text
let total (price + tax)
let result ((a + b) * 2)
```

Supported operators and their infix precedence, from highest to lowest:

| Operators | Meaning |
| --- | --- |
| `*` `/` `%` | Multiplication, division, and modulo |
| `+` `-` | Addition and subtraction |
| `==` `!=` `<` `>` `<=` `>=` | Equality, inequality, and ordering comparisons |
| `&` | Bitwise AND |
| `|` | Bitwise OR |

Comparison expressions produce `0` or `1`; `if` and `while` treat `0` as false and any nonzero value as true. `&` and `|` are integer bitwise operations. Integer addition, subtraction, and multiplication wrap using 32-bit two's-complement arithmetic. Dividing the minimum integer by `-1` wraps to the minimum integer with remainder `0`. A zero divisor or modulus produces a runtime error.

### Memory Addressing

`@` is followed by an address expression and reads from or writes to that address. Valid addresses range from `1` to `262144`:

```text
let @ 123 42
write @ 123
put 10
```

This program writes `42` to address `123`, then prints the value followed by a newline. Do not use addresses outside the valid range.

## Example Program

The following program reads an integer and prints its factorial:

```text
read n
let result 1
while > n 0
  let result * result n
  let n - n 1
end
write result
put 10
end
```

Save the code as `MoProgram/factorial.mo` to run it from the menu. The `examples/` directory also contains programs for factorials, maximum values, leap-year checks, Collatz sequences, case conversion, and more. The run menu reads programs from `MoProgram/`.

## Project Structure

```text
.
├── src/main.cpp              # Entry point, file listing, and interactive menu
├── include/                  # Expression, statement, interpreter, and VM implementations
├── MoProgram/                # Directory scanned by the run menu for .mo programs
├── examples/                 # Example Mo programs
├── tests/                    # Test inputs and expected-output files
├── docs/report.pdf           # Project report
└── Makefile                  # Build and clean targets
```

## Current Syntax Limitations

- Identifiers consist of ASCII letters and are case-sensitive. Numeric literals are nonnegative decimal integers.
- Values are signed 32-bit integers; an oversized numeric literal produces a parse error.
- Every binary operator requires two operands. Unary minus is not supported; write `0 - n` to express a negative value.
- Infix expressions must be enclosed in parentheses; ordinary prefix expressions do not need parentheses.
- The current lexer does not support source-code comments or string literals. Do not add `//` comments to `.mo` files.
- `put` / `get` operate on characters, while `write` / `read` operate on integers. I/O instructions do not add newlines automatically.
- Loops have an execution guard: a runtime error is raised if a single loop exceeds 100,000,000 iterations or runs for more than 10 seconds.

Parse errors and runtime errors are displayed in the terminal. The Makefile currently provides build and clean targets, but no separate automated test target.
