#!/usr/bin/env bash
set -e

# Build msh cleanly
gcc -Wall -Wextra -Werror -g msh.c -o msh

echo "=== Running Custom msh Tests ==="

# Test 1: Built-in cd + exit status tracking + pwd verification
# Verifies that cd changes the parent shell's working directory and updates status
out=$(printf "cd /tmp\npwd\ncd /nonexistent_dir_xyz\nexit\n" | ./msh 2>/dev/null)
if echo "$out" | grep -q "/tmp" && echo "$out" | grep -q "msh\[1\]>"; then
    echo "[PASS] Test 1: cd updates directory and reports non-zero exit status on failure"
else
    echo "[FAIL] Test 1: Unexpected output: $out"
    exit 1
fi

# Test 2: Output redirection truncation (O_TRUNC) and parent stdout preservation
# Verifies '>' overwrites existing files and keeps subsequent commands printing to stdout
rm -f test_out.txt
out=$(printf "echo first > test_out.txt\necho second > test_out.txt\necho visible_on_stdout\nexit\n" | ./msh)
file_content=$(cat test_out.txt)
rm -f test_out.txt
if [ "$file_content" = "second" ] && echo "$out" | grep -q "visible_on_stdout"; then
    echo "[PASS] Test 2: Output redirection truncates file and preserves shell stdout"
else
    echo "[FAIL] Test 2: file='$file_content', stdout='$out'"
    exit 1
fi

# Test 3: Malformed redirection syntax error handling
# Verifies 'echo hi >' does not execute echo or crash, and sets exit status to 1
out=$(printf "echo should_not_print >\nexit\n" | ./msh 2>&1)
if ! echo "$out" | grep -q "should_not_print" && echo "$out" | grep -q "msh\[1\]>"; then
    echo "[PASS] Test 3: Missing redirection filename caught without running command"
else
    echo "[FAIL] Test 3: Unexpected output: $out"
    exit 1
fi

# Test 4: Background execution (&) and # Test 4: Background execution (&) and non-blocking zombie reaping
rm -f ps_out.txt
out=$(printf "sleep 0.2 &\nsleep 0.4\nps -eo state,comm > ps_out.txt\nexit\n" | ./msh)
zombies=$(grep -E '^Z.*sleep' ps_out.txt || true)
rm -f ps_out.txt
if echo "$out" | grep -q "\[started pid" && [ -z "$zombies" ]; then
    echo "[PASS] Test 4: Background job runs asynchronously and is reaped without zombies"
else
    echo "[FAIL] Test 4: out='$out', zombies='$zombies'"
    exit 1
fi

echo "=== All 4 Custom Tests Passed! ==="
