# Advent of Code 2023 in C (C/C++ compiler)

## Introduction
In short, I'm just having a bit of fun playing around in C. My code won't be very optimized, it won't be written fast and I'm winning no prizes for the code itself. I am only leaning on the C standard library. I have made my own lexer.h that I use, it was not written specifically for Advent of Code, but I like using it for it.

I have a define called SILENT which is not very user-friendly. It is just an easy way for me to leave my logging around but not have them in my result runs. I don't expect anyone to USE my code, but I felt like I wanted to explain myself there.

## Compiling and Running
I compile with **Microsoft (R) C/C++ Optimizing Compiler Version 19.12.25835 for x64**.
(*I noticed that I'm suddenly using an older compiler for some reason, haven't bothered checking why..*)
```
Unoptimized:
cl -MTd -nologo -Gm- -Od -Oi -WX -W0 -wd4100 -FC -Z7 -D_CRT_SECURE_NO_WARNINGS day01.c /link -opt:ref user32.lib

Optimized:
cl -MTd -nologo -Gm- -O3 -Oi -WX -W0 -wd4100 -FC -Z7 -D_CRT_SECURE_NO_WARNINGS day01.c /link -opt:ref user32.lib
```

## No seriously, you're going to run this garbage?
I don't know why you want to, maybe you want to see the results turning out the same, maybe you want to make sure some random guy named slemsvamp isnt' CHEATING?!?!? All jokes aside, I haven't gone to any lengths to make my code all that easy to run. You'll have to make sure you create an input-folder according to the path, or change my code, but you'll see that all of the days are hardcoded to read files from a certain place. I typically change this from day01-sample.txt and day01-input.txt depending on what I want to run as input. You get the idea.

## Diary
### - Day 01 to Day 08 -
I haven't really had anything to say, for some reason day 5 is sticking out at me like a soar thumb.

### - Day 09 -
I have to say, second part solutions like "reverse the input" just feels cheeky but, isolated, perfectly fine.

### - Day 10 -
I am not really happy with the fact that the solution requires manual change if the input would change. I had to switch wether I check on the right side in the direction or left to suit the input. This current one solves MY input, but it would not work for all. I need to go back and fix this at some point, but not today!

### - Day 11 -
I'm not happy again, I don't want to make so many enumerations through series, but I had a bit of time-crunch today even if I said I shouldn't stress this years AoC in any way, not competing or anything. But yeah, maybe revisiting this one for optimizations, maybe forgetting about it...

### - Day 12 to Day 14 -
These solutions are either ugly, bruteforced or generally not fit for view. I got the answers, but I have to rewrite these. I will just commit them even if I planned not to, but I think I'll create entirely new files and replace these with - I need to optimize them.

### - Day 15 -
I have no real comment except it's nice to not have to feel like I have a ton of changes I want to do to this solution, as far as I remember at least.

### - Day 16 -
Could need optimization. I don't think I'll go back to this one, but I know some optimizations that I could do - more as a note to myself if I really want to put some time into.

## Optimization Needed
These are the solutions I would like to revisit and rewrite/optimize.
- Day 10
- Day 11
- Day 12
- Day 13
- Day 14

## Benchmarks (approximates)
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

- Day 05 -
Result Part 1: 240320250 (0 ms, 1219871 cycles passed)
Result Part 2: 28580589 (0 ms, 1016338 cycles passed)

- Day 06 -
Result Part 1: 140220 (0 ms, 381469 cycles passed)
Result Part 2: 39570185 (0 ms, 7397 cycles passed)

- Day 07 -
Result Part 1: 251106089 (2 ms, 5934431 cycles passed)
Result Part 2: 249620106 (2 ms, 6506961 cycles passed)

- Day 08 -
Result Part 1: 13019 (0 ms, 726504 cycles passed)
Result Part 2: 13524038372771 (6 ms, 15945908 cycles passed)

- Day 09 -
Result Part 1: 1581679977 (0 ms, 175749 cycles passed)
Result Part 2: 889 (0 ms, 158431 cycles passed)

- Day 10 -
Result Part 1: 6690 (0 ms, 301121 cycles passed)
Result Part 2: 525 (0 ms, 480791 cycles passed)

- Day 11 -
Result Part 1: 10077850 (0 ms, 327898 cycles passed)
Result Part 2: 504715068438 (0 ms, 639963 cycles passed)

- Day 12 -
Not committed to this branch.

- Day 13 -
Not committed to this branch.

- Day 14 -
Not committed to this branch.

- Day 15 -
Result Part 1: 514025 (0 ms, 815320 cycles passed)
Result Part 2: 244461 (0 ms, 1925981 cycles passed)

- Day 16 -
Result Part 1: 7472 (2 ms, 8036858 cycles passed)
Result Part 2: 7716 (211 ms, 612741784 cycles passed)
```