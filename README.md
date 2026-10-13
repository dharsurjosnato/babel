# Babel

Babel is an experimental multilingual programming language implemented in C.

The first milestone is deliberately small: localized keywords are normalized to
canonical tokens, then one shared parser/interpreter executes the program.
Language packs are plain text files so adding spellings does not require editing
the parser.

## Build

Requires a C11 compiler and `make`.

```sh
make
```

## Run

```sh
./babel --lang en examples/hello.babel
./babel --lang es examples/hello.babel
```

The selected language pack controls the keywords recognized in the source.
The example file is intentionally language-neutral only in its expression
syntax; its command words must match the selected pack.

## Current language

Statements:
- `let name = expression;`
- `name = expression;`
- `print expression;`

Expressions support numbers, variables, parentheses, unary minus, and `+ - * / %`
with standard precedence.

Example (English):
```text
let answer = 6 * 7;
print answer;
```

Example (Spanish):
```text
sea respuesta = 6 * 7;
imprimir respuesta;
```

## Language pack format

Each line is `canonical=localized_spelling`. Blank lines and lines beginning
with `#` are ignored. Keep canonical names stable; add alternate spellings as
additional lines. The initial implementation is ASCII-oriented; robust Unicode
identifier handling is a future milestone.

## Roadmap

- [x] Modular lexer, parser, AST, interpreter, dictionary loader
- [x] English and Spanish keyword packs
- [x] Automated smoke tests
- [ ] Unicode-aware identifiers and diagnostics
- [ ] `if`/`else`, loops, booleans, and functions
- [ ] Bengali language pack
- [ ] Optional C code-generation backend
