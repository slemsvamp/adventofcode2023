#include "common.h"
#include "lexer.h"

// #define WITH_DEBUG_PRINT

typedef struct poi
{
    s32 x;
    s32 y;
    lexer_token token;
} poi;

typedef struct pois
{
    u64 count;
    poi *items;
    u64 capacity;
} pois;

typedef struct schematic
{
    u64 width;
    u64 height;
    char *data;
    pois numbers;
    pois signs;
} schematic;

b8
within_distance(poi source, poi target)
{
    return !((source.x > target.x + target.token.length) || (source.x + source.token.length < target.x) || (source.y - 1 > target.y) || (source.y + 1 < target.y));
}

u64
get_width_of_schematic(file_data file)
{
    for (u64 playhead = 0; playhead < file.size; playhead++)
    {
        if (file.data[playhead] == '\r' || file.data[playhead] == '\n')
            return playhead;
    }
    return 0;
}

schematic
create_schematic(file_data file, u64 width, u64 height)
{
    u64 area = width * height;

    schematic schema =
    {
        .width = width,
        .height = height,
        .data = calloc(1, area),
        .numbers = { .capacity = 2048, .count = 0, .items = calloc(2048, sizeof(poi)) },
        .signs = { .capacity = 2048, .count = 0, .items = calloc(2048, sizeof(poi)) }
    };

    u64 dataPointer = 0;
    for (u64 playhead = 0; playhead < file.size; playhead++)
    {
        if (file.data[playhead] != '\r' && file.data[playhead] != '\n')
        {
            assert(dataPointer < area);
            schema.data[dataPointer++] = file.data[playhead];
        }
    }

    lexer_tokenizer tokenizer = { .at = file.data };
    lexer_token token = lexer_get_token(&tokenizer);
    u64 y = 0;
    
    while (!lexer_token_is_end(token))
    {
        u64 position = (u64)(tokenizer.at - token.length - file.data);
        u64 x = position - (y * (width + 2));

        if (token.type == LEXER_TOKEN_TYPE_newline)
        {
            y++;
        }
        else if (token.type == LEXER_TOKEN_TYPE_number)
        {
            assert(schema.numbers.count < schema.numbers.capacity);
            schema.numbers.items[schema.numbers.count++] = (poi) { .token = token, .x = x, .y = y };
#ifdef WITH_DEBUG_PRINT
            printf("Found %.*s at x=%zu,y=%zu\r\n", token.length, token.text, x, y);
#endif
        }
        else if (token.type != LEXER_TOKEN_TYPE_period)
        {
            assert(schema.signs.count < schema.signs.capacity);
            schema.signs.items[schema.signs.count++] = (poi) { .token = token, .x = x, .y = y };
#ifdef WITH_DEBUG_PRINT
            printf("Found '%.*s' at x=%zu,y=%zu\r\n", token.length, token.text, x, y);
#endif
        }
        
        token = lexer_get_token(&tokenizer);
    }

    return schema;
}

u64
check_schematic(schematic schema, b8 gearRatioMode)
{
    u64 result = 0;
    
    if (gearRatioMode)
    {
        poi adjacent[2] = {0};
        u64 count = 0;

        for (u64 signIndex = 0; signIndex < schema.signs.count; signIndex++)
        {
            poi poiSign = schema.signs.items[signIndex];

            if (poiSign.token.type == LEXER_TOKEN_TYPE_asterisk)
            {
                for (u64 numberIndex = 0; numberIndex < schema.numbers.count; numberIndex++)
                {
                    poi poiNumber = schema.numbers.items[numberIndex];
                    
                    // early exit if we're past the sign row + 1
                    if (poiNumber.y > poiSign.y + 1)
                        break;

#ifdef WITH_DEBUG_PRINT
                    printf("Comparing gear x=%i,y=%i with number x=%i,y=%i,l=%zu\r\n", poiSign.x, poiSign.y, poiNumber.x, poiNumber.y, poiNumber.token.length);
#endif
                    
                    if (within_distance(poiSign, poiNumber))
                    {
                        if (count == 2)
                        {
                            count = 3;
                            break;
                        }
                        adjacent[count++] = poiNumber;
                    }
                }

                if (count == 2)
                {
                    s32 gearProductA = atoi(adjacent[0].token.text);
                    s32 gearProductB = atoi(adjacent[1].token.text);
                    assert(gearProductA > 0);
                    assert(gearProductB > 0);
#ifdef WITH_DEBUG_PRINT
                    printf("Found %i and %i\r\n", gearProductA, gearProductB);
#endif
                    result += (u64)(gearProductA * gearProductB);
                }
            }

            count = 0;
            memset(adjacent, 0, sizeof(poi) * 2);
        }
    }
    else
    {
        for (u64 numberIndex = 0; numberIndex < schema.numbers.count; numberIndex++)
        {
            for (u64 signIndex = 0; signIndex < schema.signs.count; signIndex++)
            {
                poi poiNumber = schema.numbers.items[numberIndex];
                poi poiSign = schema.signs.items[signIndex];

                // early exit if we're past the numbers row + 1
                if (poiSign.y > poiNumber.y + 1)
                    break;

                if (within_distance(poiNumber, poiSign))
                {
                    s32 number = atoi(poiNumber.token.text);
                    assert(number > 0);
                    result += (u64)number;
                    break;
                }
            }
        }
    }
    return result;
}

internal u64
part_1()
{
    u64 result = 0;
    file_data file = read_to_end_of_file("input\\day03-input.txt");
    assert(file.size > 0);

    u64 width = get_width_of_schematic(file);
    u64 height = (file.size + 2) / (width + 2);

    schematic schema = create_schematic(file, width, height);

    return check_schematic(schema, false);
}

internal u64
part_2()
{
    file_data file = read_to_end_of_file("input\\day03-input.txt");
    assert(file.size > 0);

    u64 width = get_width_of_schematic(file);
    u64 height = (file.size + 2) / (width + 2);

    schematic schema = create_schematic(file, width, height);

    return check_schematic(schema, true);
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

    debug_log("- Day 03 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}