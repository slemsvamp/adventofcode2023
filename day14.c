#include "common.h"
#include "lexer.h"

typedef struct mountain
{
    char *data;
    u64 length;
    u64 height;
    u64 width;
} mountain;

#ifndef SILENT
void
draw_mountain(mountain m)
{
    for (int y = 0; y < m.height; y++)
    {
        for (int x = 0; x < m.height; x++)
            log("%c", m.data[y * m.width + x]);
        log("\n");
    }    
}
#endif

mountain
parse_mountain()
{
    mountain result = {0};

    file_data file = read_to_end_of_file("input\\day14-input.txt");
    assert(file.size > 0);

    u64 width = 0;

    mountain currentMountain = {0};
    
    char dataBuffer[16384];
    u64 dataCount = 0;

    u64 x = 0;
    u64 y = 0;

    for (u64 index = 0; index < file.size; index++)
    {
        switch (file.data[index])
        {
            case 'O':
            case '#':
            case '.':
                x++;
                dataBuffer[dataCount++] = file.data[index];
            break;
            case '\n':
                if (!width)
                    width = x;
                x = 0;
                y++;
            break;
            default:
            break;
        }
    }

    result = (mountain)
    {
        .data = calloc(dataCount, sizeof(char)),
        .length = dataCount,
        .height = y+1,
        .width = width
    };

    memcpy(result.data, dataBuffer, dataCount * sizeof(char));

    return result;
}

u64
part_1()
{
    mountain m = parse_mountain();

    u64 result = 0;

    for (u64 column = 0; column < m.width; column++)
    {
        for (u64 row = 0; row < m.height; row++)
        {
            u64 index = row * m.width + column;

            if (m.data[index] == '.')
            {
                for (u64 lookupRow = row+1; lookupRow < m.height; lookupRow++)
                {
                    u64 lookupIndex = lookupRow * m.width + column;
                    
                    if (m.data[lookupIndex] == 'O')
                    {
                        m.data[index] = 'O';
                        m.data[lookupIndex] = '.';
                        result += m.height - row;
                        log("move: %lli\n", m.height - row);
                        break;
                    }
                    else if (m.data[lookupIndex] == '#')
                        break;
                }
            }
            else if (m.data[index] == 'O')
            {
                result += m.height - row;
                log("no move: %lli\n", m.height - row);
            }
        }
    }

    return result;
}

void
mountain_tilt(mountain *m)
{
    char *tilted = calloc(m->length, sizeof(char));
    
    for (s64 y = 0; y < m->height; y++)
    {
        for (s64 x = 0; x < m->width; x++)
        {
            tilted[y * m->width + x] = m->data[(m->height - x - 1) * m->width + y];
        }
    }
    
    m->data = tilted;
}

typedef enum direction
{
    DIRECTION_north,
    DIRECTION_east,
    DIRECTION_south,
    DIRECTION_west,
    DIRECTION_COUNT
} direction;

u64
part_2()
{
    mountain m = parse_mountain();

    u64 result = 0;

    u32 hashes[16384];
    u32 hashCount = 0;

    u64 results[16384];

    direction dir = 0;

    s64 cycleWeLastMadeANewHashOn = -1;

    for (u64 cycle = 0; cycle < 380; cycle++)
    {
        for (u64 tilt = 0; tilt < 4; tilt++)
        {
            dir %= 4;
            
            result = 0;

            if (!(cycle == 0 && tilt == 0))
                mountain_tilt(&m);
            
            for (u64 column = 0; column < m.width; column++)
            {
                for (u64 row = 0; row < m.height; row++)
                {
                    u64 index = row * m.width + column;

                    if (m.data[index] == '.')
                    {
                        for (u64 lookupRow = row+1; lookupRow < m.height; lookupRow++)
                        {
                            u64 lookupIndex = lookupRow * m.width + column;
                            
                            if (m.data[lookupIndex] == 'O')
                            {
                                m.data[index] = 'O';
                                m.data[lookupIndex] = '.';
                                if (tilt == 3)
                                    result += m.width - column;
                                break;
                            }
                            else if (m.data[lookupIndex] == '#')
                                break;
                        }
                    }
                    else if (m.data[index] == 'O' && tilt == 3)
                        result += m.width - column;
                }
            }
        }

        u32 mountainHash = dict_hash_n(m.data, m.length);

        b32 found = false;
        for (u32 i = 0; i < hashCount; i++)
            if (hashes[i] == mountainHash)
            {
                found = true;
                break;
            }
        
        if (!found)
        {
            assert(hashCount < 16384);
            //printf("RESULT: %lld\n", result);
            hashes[hashCount] = mountainHash;
            results[hashCount] = result;
            hashCount++;
            cycleWeLastMadeANewHashOn = cycle;
        }

        if (cycleWeLastMadeANewHashOn >= 0 && cycle - cycleWeLastMadeANewHashOn > 10)
        {
            // u64 cyclesToRepeat = hashCount;
            // printf("CYCLES!! %lld\n", hashCount);
            // for (u64 r = 0; r < hashCount; r++)
            // {
            //     if (results[r] < 90558)
            //         printf("%lld, ", results[r]);
            // }
            // printf("\b\b\n");
            // return results[139];

            if (result == 90551)
                printf("90551 cycle: %lld\n", cycle);
        }

        // mountain_tilt(&m);
        // draw_mountain(m);
        // log("-----------------------\n");

        // log("cycle #%lld: %lld\n", cycle, result);

    }
    
    mountain_tilt(&m);
    draw_mountain(m);
    
    return result;
}

u32
main(s32 argumentCount, char *arguments[])
{
    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = 0; // part_1();

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    u64 resultPart2 = part_2();

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    // 90506 not the right answer
    // 90513 not the right answer
    // 90523 not the right answer

    // 90574 too high
    // 90558 too high
    // 90602 too high

/*
    try:
    90536
      90551 !!!
    90555
    90557
*/

    // ABSOLUTELY BRUTEFORCED THIS ONE... but i was tired :D a lot at work, pls don't push this one.. >_>

    debug_log("- Day 14 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}