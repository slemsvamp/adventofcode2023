#define SILENT

#include "common.h"
#include "lexer.h"

typedef struct loc
{
    s64 x;
    s64 y;
    s64 pushX;
    s64 pushY;
} loc;

typedef struct map
{
    loc *locations;
    u64 count;
    u64 width;
    u64 height;
} map;

map
read_map(u64 expansionFactor)
{
    file_data file = read_to_end_of_file("input\\day11-input.txt");
    assert(file.size > 0);

    #define LOCATIONS_CAPACITY 20000
    #define GALAXIES_X_CAPACITY 256
    #define EXPANSIONS_CAPACITY 256

    loc locations[LOCATIONS_CAPACITY];
    u64 locationsCount = 0;

    u64 galaxiesXBuffer[GALAXIES_X_CAPACITY];

    u64 expansionsY[EXPANSIONS_CAPACITY];
    u64 expansionsYCount = 0;

    u64 expansionsX[EXPANSIONS_CAPACITY];
    u64 expansionsXCount = 0;

    u64 rows = 0;
    u64 columns = 0;
    u64 writeIndex = 0;
    b32 rowHadGalaxies = false;

    for (u64 index = 0; index < file.size; index++)
    {
        if (file.data[index] == '\r')
        {
            if (rows == 0)
                columns = index;
            continue;
        }
        else if (file.data[index] == '\n')
        {
            if (!rowHadGalaxies)
                expansionsY[expansionsYCount++] = rows;

            rowHadGalaxies = false;
            rows++;
            continue;
        }

        if (file.data[index] == '#')
        {
            u64 x = 0, y = 0;

            if (columns > 0)
            {
                x = writeIndex % columns;
                if (!galaxiesXBuffer[writeIndex % columns])
                    galaxiesXBuffer[writeIndex % columns] = 1;
            }
            else
            {
                x = writeIndex;
                if (!galaxiesXBuffer[writeIndex])
                    galaxiesXBuffer[writeIndex] = 1;
            }

            y = rows;
            rowHadGalaxies = true;
            locations[locationsCount++] = (loc) { .x = x, .y = y, .pushX = 0, .pushY = 0 };
        }
        writeIndex++;
    }

    for (u32 xIndex = 0; xIndex < columns; xIndex++)
        if (!galaxiesXBuffer[xIndex])
            expansionsX[expansionsXCount++] = xIndex;
    
    // slow version
    for (u32 yIndex = 0; yIndex < expansionsYCount; yIndex++)
    {
        u64 expansionY = expansionsY[yIndex];

        for (u32 galaxyIndex = 0; galaxyIndex < locationsCount; galaxyIndex++)
        {
            loc *l = locations + galaxyIndex;
            if (l->y > expansionY)
                l->pushY++;
        }
    }

    for (u32 xIndex = 0; xIndex < expansionsXCount; xIndex++)
    {
        u64 expansionX = expansionsX[xIndex];

        for (u32 galaxyIndex = 0; galaxyIndex < locationsCount; galaxyIndex++)
        {
            loc *l = locations + galaxyIndex;
            if (l->x > expansionX)
                l->pushX++;
        }
    }

    u64 multiplier = expansionFactor > 1 ? expansionFactor - 1 : 1;

    for (u32 galaxyIndex = 0; galaxyIndex < locationsCount; galaxyIndex++)
    {
        loc *l = locations + galaxyIndex;
        l->x += l->pushX * multiplier;
        l->pushX = 0;
        l->y += l->pushY * multiplier;
        l->pushY = 0;
    }

    map result =
    {
        .count = locationsCount,
        .locations = calloc(locationsCount, sizeof(loc)),
        .width = columns,
        .height = rows
    };

    memcpy(result.locations, locations, locationsCount * sizeof(loc));

    return result;
}

u64
part_1(map m)
{
    u64 result = 0;
    for (u64 index = 0; index < m.count; index++)
    {
        loc l = m.locations[index];

        for (u64 subIndex = index + 1; subIndex < m.count; subIndex++)
        {
            loc l2 = m.locations[subIndex];
            result += abs(l2.x - l.x) + abs(l2.y - l.y);
        }
    }
        
    return result;
}

u64
part_2(map m)
{
    u64 result = 0;
    for (u64 index = 0; index < m.count; index++)
    {
        loc l = m.locations[index];

        for (u64 subIndex = index + 1; subIndex < m.count; subIndex++)
        {
            loc l2 = m.locations[subIndex];
            result += abs(l2.x - l.x) + abs(l2.y - l.y);
        }
    }
        
    return result;
}

u32
main(s32 argumentCount, char *arguments[])
{
    map m1 = read_map(1);

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = part_1(m1);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    map m2 = read_map(1000000);

    u64 resultPart2 = part_2(m2);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 11 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}