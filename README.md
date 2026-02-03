\# Custom Assembler (C)



A two-pass assembler implemented in C for a custom assembly language.

Includes macro processing, symbol management, and detailed error handling.



\## Features

\- Two-pass assembly flow (first pass + second pass)

\- Macro expansion

\- Labels / symbol table handling

\- Directives support (data / string / extern / entry)

\- Produces output files: `.ob`, `.ent`, `.ext`

\- Clear syntax + semantic error reporting

\- Tested using multiple assembly input cases, including valid programs and error scenarios



\## Project Structure

\- `Assembler.c` / `Assembler.h` – main entry and core interfaces

\- `PreAssembler.c` – macro handling / preprocessing

\- `FirstPass.c` / `SecondPass.c` – two-pass assembly logic

\- `\*Functions.c` – parsing, directives, validation, label handling utilities

\- `docs/` – screenshots / notes (if relevant)



\## Build

Requirements:

\- GCC 

\- `make`



Build:

```bash

make



## Run
```bash
./Assembler test1.as test2.as



```md
## Motivation
This project was developed to practice systems-level programming concepts,
including multi-pass processing, memory management, and robust error handling.
It reflects real-world tooling patterns commonly found in compilers and build systems.

