#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./ccc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

try 0 "0;"
try 42 "42;"
try 41 " 12 + 34 - 5 ; "
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"

try 1 "a=1;a;"
try 3 "a=5;z=2;a-z;"
try 3 "a=2+3;z=8/4;a-z;"
try 3 "abc=1;i_0=2;abc+i_0;"

try 1 "1==1;"
try 1 "1+(0==1);"
try 1 "a=1;b=2;a+a==b;"
try 1 "1!=2;"

echo OK
