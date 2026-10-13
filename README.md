## Babel

A multilingual programming language written in C.

The goal is to let people write code using keywords from their own languages, while Babel translates those keywords into a shared internal representation.

Current Features

English and Spanish keywords

Variables and assignments

Basic arithmetic

Printing values

A simple interpreter

## Build

make

## Run

./babel --lang en examples/hello_en.babel

./babel --lang es examples/hello_es.babel

## Tests

make test

Project Structure

src/ — C implementation

include/ — header files

languages/ — keyword dictionaries

examples/ — sample programs

tests/ — automated tests
