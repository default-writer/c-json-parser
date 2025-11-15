# simple JSON parser in C

A simple JSON parser in C 

## tools

Ninja + Clang

## building

```
ninja -f build.linux.ninja && ./main
```

## testing

```
ninja -f build.linux.ninja -t clean && ninja -f build.linux.ninja && valgrind -s ./main
```