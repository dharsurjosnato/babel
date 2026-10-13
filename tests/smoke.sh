#!/bin/sh
set -eu
out_en=$(./babel --lang en examples/hello_en.babel)
out_es=$(./babel --lang es examples/hello_es.babel)
[ "$out_en" = "42" ] || { echo "English test failed: $out_en"; exit 1; }
[ "$out_es" = "42" ] || { echo "Spanish test failed: $out_es"; exit 1; }
echo "All Babel smoke tests passed."
