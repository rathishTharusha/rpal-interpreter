#!/bin/bash
success=0
fail=0

make clean
make

for file in rpal_test_programs/*; do
    echo "Testing $file..."
    ./rpal.exe "$file" > expected.txt 2>/dev/null
    ./rpal20 "$file" > actual.txt 2>/dev/null
    
    if diff -q expected.txt actual.txt >/dev/null; then
        echo "[PASS] $file"
        ((success++))
    else
        echo "[FAIL] $file"
        diff expected.txt actual.txt | head -n 20
        ((fail++))
    fi
done

echo "------------------------"
echo "Success: $success"
echo "Fail: $fail"
echo "------------------------"

rm -f expected.txt actual.txt
