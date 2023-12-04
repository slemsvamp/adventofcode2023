# Advent of Code 2023 in C (C/C++ compiler)

## Introduction
I'm hacking together the 2021 and 2022 folders I have so I might have to update this readme as I go.

## Compiling
I compile with **Microsoft (R) C/C++ Optimizing Compiler Version 19.12.25835 for x64**.
(*I noticed that I'm suddenly using an older compiler for some reason, haven't bothered checking why..*)
```
Unoptimized:
cl -MTd -nologo -Gm- -Od -Oi -WX -W0 -wd4100 -FC -Z7 -D_CRT_SECURE_NO_WARNINGS day01.c /link -opt:ref user32.lib

Optimized:
cl -MTd -nologo -Gm- -O3 -Oi -WX -W0 -wd4100 -FC -Z7 -D_CRT_SECURE_NO_WARNINGS day01.c /link -opt:ref user32.lib
```

# Benchmarks (approximates)
I'm using clock() for the time, basically record the clock() timestamp difference and multiply by 1000 and divide by CLOCKS_PER_SEC (which is 1000 on my end), and this will result in 0 ms when it's faster than a millisecond so it will show up as 0. I use __rdtsc() from <windows.h> to count cycles passed. As far as I know this is not cycles that MY executable used, it's how many passed since the timestamps, this means it varies a lot because of how much work windows needs the CPU to do in the background. In practice it's more of a timestamp than a clock-efficiency measurement.

```
- Day 01 -
Result Part 1: 54916 (0 ms, 711091 cycles passed)
Result Part 2: 54728 (1 ms, 3519743 cycles passed)

- Day 02 -
Result Part 1: 2101 (0 ms, 642980 cycles passed)
Result Part 2: 58269 (0 ms, 491953 cycles passed)

- Day 03 -
Result Part 1: 527369 (1 ms, 4966713 cycles passed)
Result Part 2: 73074886 (1 ms, 2875751 cycles passed)

- Day 04 -
Result Part 1: 22897, Result Part 2: 5095824 (0 ms, 1330647 cycles passed)
```