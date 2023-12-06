#include "common.h"

char *digitsInText[] =
{
    "one", "two", "three", "four", "five",
    "six", "seven", "eight", "nine"
};

u64
parse_calibrations(file_data file, b8 textCanBeNumbers)
{
    char twoDigits[2] = { 0, 0 };
    u64 result = 0;
    b8 isNumberFound = false;
    char number = 0;

    for (u32 playhead = 0; playhead < file.size; playhead++)
    {
        assert(file.data[playhead] != 0);

        if ((file.data[playhead] == '\r' && file.data[playhead + 1] == '\n') || playhead == file.size - 1)
        {
            playhead++;
            int parsedNumber = atoi((char[3]) { twoDigits[0], twoDigits[1], 0 });
            assert(parsedNumber > 0);
            result += (u32)parsedNumber;
            memset(twoDigits, 0, 2);
            continue;
        }

        if (file.data[playhead] >= '0' && file.data[playhead] <= '9')
        {
            isNumberFound = true;
            number = file.data[playhead];
        }
        else if (textCanBeNumbers)
        {
            for (u32 digitsIndex = 0; digitsIndex < 9; digitsIndex++)
            {
                size_t maxCount = min(file.size - playhead, strlen(digitsInText[digitsIndex]));

                if (strncmp(file.data + playhead, digitsInText[digitsIndex], maxCount) == 0)
                {
                    isNumberFound = true;
                    number = '1' + digitsIndex;
                    break;
                }
            }
        }

        if (isNumberFound)
        {
            isNumberFound = false;

            if (twoDigits[0] == 0)
            {
                twoDigits[0] = twoDigits[1] = number;
                continue;
            }

            twoDigits[1] = number;
        }
    }

    return result;
}

internal u64
part_1()
{
    file_data file = read_to_end_of_file("input\\day01-input.txt");
    assert(file.size > 0);
    return parse_calibrations(file, false);
}

internal u64
part_2()
{
    file_data file = read_to_end_of_file("input\\day01-input.txt");
    assert(file.size > 0);
    return parse_calibrations(file, true);
}

u32
main(s32 argumentCount, char *arguments[])
{
    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = part_1();

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    u64 resultPart2 = part_2();

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 01 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}