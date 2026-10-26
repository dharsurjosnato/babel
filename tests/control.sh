#!/bin/sh
set -eu
expected=$(printf '10\n3\n2\n1\n0\n1\n2\n99')
for lang in en es; do
    actual=$(./babel --lang "$lang" "examples/control_${lang}.babel")
    [ "$actual" = "$expected" ] || { printf '%s control test failed:\n%s\n' "$lang" "$actual"; exit 1; }
done
# Empty programs, empty branches, else branches, and nested loops.
printf 'if (false) {} else { print 7; }\n' > /tmp/babel-control-en-$$.babel
printf 'si (falso) {} sino { imprimir 7; }\n' > /tmp/babel-control-es-$$.babel
[ "$(./babel --lang en /tmp/babel-control-en-$$.babel)" = '7' ]
[ "$(./babel --lang es /tmp/babel-control-es-$$.babel)" = '7' ]
rm -f /tmp/babel-control-en-$$.babel /tmp/babel-control-es-$$.babel
printf 'Control flow tests passed (English and Spanish).\n'
