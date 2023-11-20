#!/bin/bash
#
# Tests for 2. IZP project [2023]
# author: pseja
# heavily inspired by: _ramb0_
# Usage:
#     (1) Download the gist to your "maze.c" directory: wget https://gist.githubusercontent.com/pseja/12a4e5635d9231649a2d449cb94bc8d8/raw/5d326fa8b58342b5532f03f3ce938a87d183e0e3/test_maze.sh
#     (2) Then (for adding permission):                 chmod u+x test_maze.sh
#     (3) Execute this command in "maze.c" directory:   ./test_maze.sh
#     (4) Debug :D

#     For Makefile enjoyers:
#     (1) Add this to your Makefile:              test: maze
#	                                                  @./test_maze.sh
#     (2) Run this command for compile and tests: make test


# color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NORMAL='\033[0m'

# test variables
test_count=0
correct=0

# compile maze.c just in case
gcc -std=c11 -DNDEBUG -Wall -Wextra -Werror maze.c -o maze

run_test() {
    input_file=$1
    test_arg=$2
    expected_output=$3
    
    echo -n -e "Running $input_file, argument ${test_arg}\n"
    
    actual_output=$(./maze $test_arg $input_file)
    
    if [[ "$actual_output" == "$expected_output" ]]; then
        echo -e "${GREEN} [OK] ${NORMAL}"
        correct=$((correct + 1))
    else
        echo -e "${RED}[FAIL]${NORMAL}"
        echo "Expected: $expected_output"
        echo "Got:      $actual_output"
    fi
    test_count=$((test_count + 1))
}

# tests
echo -e "6 7\n1 4 4 2 5 0 6\n1 4 4 0 4 0 2\n1 0 4 0 4 6 1\n1 2 7 1 0 4 2\n3 1 4 2 3 1 2\n4 2 5 0 4 2 5" > test_01.txt
run_test "test_01.txt" "--lpath 6 1" "6,1
6,2
5,2
5,3
5,4
6,4
6,5
6,6
5,6
5,7
4,7
4,6
4,5
5,5
4,5
4,4
3,4
3,3
3,2
4,2
4,1
5,1
4,1
4,2
3,2
3,1
2,1
2,2
2,3
2,4
1,4
1,3
1,2
1,1"

run_test "test_01.txt" "--lpath 6 7" "6,7"

run_test "test_01.txt" "--rpath 6 1" "6,1
6,2
5,2
5,3
5,4
6,4
6,3
6,4
6,5
6,6
5,6
5,7
4,7
4,6
4,5
4,4
3,4
3,5
3,6
3,5
3,4
3,3
3,2
3,1
2,1
2,2
2,3
2,4
2,5
2,6
2,7
3,7"

run_test "test_01.txt" "--rpath 6 7" "6,7"

# tests by cubko
run_test "test_01.txt" "--lpath 3 7" "3,7
2,7
2,6
2,5
2,4
2,3
2,2
2,1
3,1
3,2
3,3
3,4
3,5
3,6
3,5
3,4
4,4
4,5
4,6
4,7
5,7
5,6
6,6
6,5
6,4
6,3
6,4
5,4
5,3
5,2
6,2
6,1"

run_test "test_01.txt" "--rpath 1 1" "1,1
1,2
1,3
1,4
2,4
2,3
2,2
2,1
3,1
3,2
4,2
4,1
5,1
4,1
4,2
3,2
3,3
3,4
4,4
4,5
5,5
4,5
4,6
4,7
5,7
5,6
6,6
6,5
6,4
5,4
5,3
5,2
6,2
6,1"

run_test "test_01.txt" "--test" "Valid"

echo -e "6 7\n1 4 4 2 5 0 6\n1 4 4 0 4 0 2\n1 0 4 0 3 6 1\n1 2 7 1 0 4 2\n3 1 4 2 3 1 2\n4 2 5 0 4 2 5" > test_02.txt
run_test "test_02.txt" "--test" "Invalid"

echo -e "6 7\n1 4 4 2 5 0 6\n1 4 4 0 4 0 2\n1 0 4 0 H 6 1\n1 2 7 1 0 4 2\n3 1 4 2 3 1 2\n4 2 5 0 4 2 5" > test_03.txt
run_test "test_03.txt" "--test" "Invalid"

echo -e "6 7\n1 4 4 2 5 0 6\n1 4 4 0 4 0 2\n1 0 4 0 4 6 1\n1 2 7 1 0 4 2\n3 1 4 2 3 1 2\n4 2 5 0 4 2 5 6" > test_04.txt
run_test "test_04.txt" "--test" "Invalid"


# print test results
if [[ "$correct" == "$test_count" ]]; then
    echo -e "\nPassed $correct / $test_count 🎉"
else
    echo -e "\nPassed $correct / $test_count"
fi

# remove temp test files
# if you want individual tests comment line with the test you want to keep
# make sure to later uncomment tho :D
rm test_04.txt
rm test_03.txt
rm test_02.txt
rm test_01.txt
