#include "common.h"
#include "lexer.h"

// . ash
// # rock

typedef struct loc
{
    s32 x;
    s32 y;
} loc;

typedef struct mountain
{
    char *raw;
    loc *rocks;
    u64 count;
    u64 width;
    u64 height;
} mountain;

typedef struct mountain_collection
{
    mountain *mountains;
    u64 count;
} mountain_collection;

mountain_collection
parse_mountains()
{
    mountain_collection result = {0};

    file_data file = read_to_end_of_file("input\\day13-input.txt");
    assert(file.size > 0);

    u64 width = 0;

    mountain mountains[2048];
    u64 mountainCount = 0;

    mountain currentMountain = {0};
    
    char dataBuffer[2048];
    u64 dataCount = 0;

    loc rocks[2048];
    u64 rockCount = 0;

    u64 x = 0;
    u64 y = 0;

    for (u64 index = 0; index < file.size; index++)
    {
        switch (file.data[index])
        {
            case '#':
                rocks[rockCount++] = (loc) { .x = x, .y = y };
            case '.':
                x++;
                dataBuffer[dataCount++] = file.data[index];
            break;
            case '\n':
                if (!width)
                    width = x;
                x = 0;
                y++;
                
                if (file.data[index+1] == '\r' && file.data[index+2] == '\n')
                {
                    mountains[mountainCount++] = (mountain)
                    {
                        .raw = calloc(dataCount, sizeof(char)),
                        .count = rockCount,
                        .rocks = calloc(rockCount, sizeof(loc)),
                        .height = y,
                        .width = width
                    };

                    log("Added mountain which is %lld wide and %lld tall with %lld rock(s).\n", width, y, rockCount);
                    memcpy(mountains[mountainCount - 1].raw, dataBuffer, dataCount * sizeof(char));
                    memcpy(mountains[mountainCount - 1].rocks, rocks, rockCount * sizeof(loc));
                    rockCount = 0;
                    y = 0;
                    dataCount = 0;
                    width = 0;
                    index += 2;
                }
            break;
            default:
            break;
        }
    }

    mountains[mountainCount++] = (mountain)
    {
        .raw = calloc(dataCount, sizeof(char)),
        .count = rockCount,
        .rocks = calloc(rockCount, sizeof(loc)),
        .height = y+1,
        .width = width
    };

    log("Added mountain which is %lld wide and %lld tall with %lld rock(s).\n", width, y, rockCount);
    memcpy(mountains[mountainCount - 1].raw, dataBuffer, dataCount * sizeof(char));
    memcpy(mountains[mountainCount - 1].rocks, rocks, rockCount * sizeof(loc));

    result.mountains = calloc(mountainCount, sizeof(mountain));
    memcpy(result.mountains, mountains, mountainCount * sizeof(mountain));
    result.count = mountainCount;

    return result;
}

typedef enum dimension
{
    DIMENSION_horizontal,
    DIMENSION_vertical
} dimension;

typedef struct line_history
{
    dimension dim;
    u64 value;
} line_history;

line_history *_history;

u64
part_1()
{
    mountain_collection mountainCollection = parse_mountains();

    _history = calloc(mountainCollection.count, sizeof(line_history));

    u64 lefties = 0;
    u64 abovies = 0;

    for (u64 part2Mode = 0; part2Mode < 2; part2Mode++)
    {
        lefties = 0;
        abovies = 0;

        for (u64 mountainIndex = 0; mountainIndex < mountainCollection.count; mountainIndex++)
        {
            mountain currentMountain = mountainCollection.mountains[mountainIndex];
            
            for (u64 horizontalSlice = 1; horizontalSlice < currentMountain.height; horizontalSlice++)
            {
                b32 mirroredRow = true;
                u64 w = currentMountain.width;
                u64 y = horizontalSlice;
                b32 usedDiff = false;

                // log("= Comparison: ============\r\n");
                for (u64 d = 0; d < currentMountain.height; d++)
                {
                    // outside
                    if (y + d >= currentMountain.height || (s32)(y + (d + 1) * -1) < 0)
                        break;
                    
                    u64 rowAbove = y + (d + 1) * -1;
                    u64 rowBelow = y + d;

                    // log("Above: %.*s\r\n", w, currentMountain.raw + rowAbove * w);
                    // log("Below: %.*s\r\n", w, currentMountain.raw + rowBelow * w);
                    // log("-------------------\r\n");

                    for (u64 x = 0; x < w; x++)
                    {
                        if (currentMountain.raw[rowAbove * w + x] != currentMountain.raw[rowBelow * w + x])
                        {
                            mirroredRow = false;
                            break;
                        }
                    }

                    if (part2Mode)
                    {
                        u64 difference = 0;
                        u64 diffAtX = 0;
                        char diffChangeTo = 0;
                        for (u64 x = 0; x < w; x++)
                        {
                            b32 isDifferent = currentMountain.raw[rowAbove * w + x] != currentMountain.raw[rowBelow * w + x];
                            difference += isDifferent;
                            if (isDifferent)
                            {
                                diffAtX = x;
                                diffChangeTo = currentMountain.raw[rowBelow * w + x];
                            }
                        }

                        if (difference == 1 && !usedDiff)
                        {
                            usedDiff = true;
                            mirroredRow = true;
                            // currentMountain.raw[rowAbove * w + diffAtX] = diffChangeTo;
                        }
                    }

                    if (!mirroredRow)
                        break;
                }

                if (mirroredRow)
                {
                    if (!part2Mode)
                    {
                        _history[mountainIndex] = (line_history)
                        {
                            .dim = DIMENSION_horizontal,
                            .value = y
                        };
                    }
                    else
                    {
                        // check history and ignore previous
                        if (_history[mountainIndex].dim == DIMENSION_horizontal && _history[mountainIndex].value == y)
                            continue;
                    }
                    log("mountain %lld: horizontally mirrored at %lld\r\n", mountainIndex, y);
                    abovies += y;
                }
            }

            for (u64 verticalSlice = 1; verticalSlice < currentMountain.width; verticalSlice++)
            {
                b32 mirroredColumn = true;
                u64 h = currentMountain.height;
                u64 w = currentMountain.width;
                u64 x = verticalSlice;
                b32 usedDiff = false;

                for (u64 d = 0; d < currentMountain.width; d++)
                {
                    // outside
                    if (x + d >= currentMountain.width || (s32)(x + (d + 1) * -1) < 0)
                        break;
                    
                    u64 columnBefore = x + (d + 1) * -1;
                    u64 columnAfter = x + d;

                    for (u64 y = 0; y < h; y++)
                    {
                        if (currentMountain.raw[y * w + columnBefore] != currentMountain.raw[y * w + columnAfter])
                        {
                            mirroredColumn = false;
                            break;
                        }
                    }

                    if (part2Mode)
                    {
                        u64 difference = 0;
                        u64 diffAtY = 0;
                        char diffChangeTo = 0;
                        for (u64 y = 0; y < h; y++)
                        {
                            b32 isDifferent = currentMountain.raw[y * w + columnBefore] != currentMountain.raw[y * w + columnAfter];
                            difference += isDifferent;
                            if (isDifferent)
                            {
                                diffAtY = y;
                                diffChangeTo = currentMountain.raw[y * w + columnAfter];
                            }
                        }

                        if (difference == 1 && !usedDiff)
                        {
                            usedDiff = true;
                            mirroredColumn = true;
                            // currentMountain.raw[diffAtY * w + columnBefore] = diffChangeTo;
                        }
                    }

                    if (!mirroredColumn)
                        break;
                }
                
                if (mirroredColumn)
                {
                    if (!part2Mode)
                    {
                        _history[mountainIndex] = (line_history)
                        {
                            .dim = DIMENSION_vertical,
                            .value = x
                        };
                    }
                    else
                    {
                        // check history and ignore previous
                        if (_history[mountainIndex].dim == DIMENSION_vertical && _history[mountainIndex].value == x)
                            continue;
                    }
                    log("mountain %lld: vertically mirrored at %lld\r\n", mountainIndex, x);
                    lefties += x;
                }
            }
        }

        printf("%lld: %lld\n", part2Mode, abovies * 100 + lefties);
    }

    return abovies * 100 + lefties;
}

u64
part_2()
{
    file_data file = read_to_end_of_file("input\\day13-sample.txt");
    assert(file.size > 0);
    return 0;
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

    // 41990 too low
    // 41996 too low
    // 73867 too high

    debug_log("- Day 13 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}