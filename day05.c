#include "common.h"
#include "lexer.h"

typedef struct range
{
    u64 start;
    u64 end;
} range;

typedef struct transformation
{
    u64 source;
    u64 target;
    u64 range;
} transformation;

typedef struct transformation_collection
{
    u64 capacity;
    u64 count;
    transformation *items;
} transformation_collection;

typedef enum seed_reading_mode
{
    SEED_READING_MODE_one_by_one,
    SEED_READING_MODE_ranges,
} seed_reading_mode;

u64
transform_seeds(file_data file, seed_reading_mode seedReadingMode)
{
    #define TRANSFORMATION_COLLECTION_COUNT 7
    transformation_collection transformations[TRANSFORMATION_COLLECTION_COUNT] = {0};
    for (u64 transformationCollectionIndex = 0; transformationCollectionIndex < TRANSFORMATION_COLLECTION_COUNT; transformationCollectionIndex++)
        transformations[transformationCollectionIndex] = (transformation_collection)
        {
            .capacity = 64,
            .count = 0,
            .items = calloc(64, sizeof(transformation))
        };

    u64 seedCount = 0;
    range *seeds = calloc(4096, sizeof(range));

    lexer_tokenizer t = { .at = file.data };
    lexer_token token = lexer_get_next(&t, LEXER_TOKEN_TYPE_number);

    // how to read
    while (token.type == LEXER_TOKEN_TYPE_number)
    {
        u64 startSeed = atoll(token.text);

        if (seedReadingMode == SEED_READING_MODE_one_by_one)
        {
            seeds[seedCount++] = (range)
            {
                .start = startSeed,
                .end = startSeed
            };
        }
        else if (seedReadingMode == SEED_READING_MODE_ranges)
        {
            lexer_token peekToken = lexer_peek_token(&t);
            
            if (peekToken.type == LEXER_TOKEN_TYPE_newline)
                break;
            
            token = lexer_get_token(&t);

            u64 seedRange = atoll(token.text);

            seeds[seedCount++] = (range)
            {
                .start = startSeed,
                .end = startSeed + seedRange - 1
            };

            printf("SEED %zu - %zu\r\n", startSeed, startSeed+seedRange-1);
        }
        else
            invalid_code_path;

        token = lexer_get_token(&t);
    }

    for (u64 transformationIndex = 0; transformationIndex < TRANSFORMATION_COLLECTION_COUNT; transformationIndex++)
    {
        transformation_collection *collection = transformations + transformationIndex;

        token = lexer_get_next(&t, LEXER_TOKEN_TYPE_colon);
        token = lexer_require_token(&t, LEXER_TOKEN_TYPE_newline);

        lexer_token peekToken;

        do
        {
            transformation transform = {0};
            token = lexer_require_token(&t, LEXER_TOKEN_TYPE_number);
            transform.target = atoll(token.text);
            token = lexer_require_token(&t, LEXER_TOKEN_TYPE_number);
            transform.source = atoll(token.text);
            token = lexer_require_token(&t, LEXER_TOKEN_TYPE_number);
            transform.range = atoll(token.text);
            assert(collection->count < collection->capacity);
            collection->items[collection->count++] = transform;
            token = lexer_get_token(&t, LEXER_TOKEN_TYPE_newline);
            
            if (lexer_token_is_end(token))
                break;

            peekToken = lexer_peek_token(&t);
        } while (peekToken.type != LEXER_TOKEN_TYPE_newline);
    }

    u64 result = -1;

    #define RANGE_MAX 2048
    u64 rangeSeedCount = 0;
    range *rangeSeeds = calloc(RANGE_MAX, sizeof(range));

    u64 nextRangeSeedCount = 0;
    range *nextRangeSeeds = calloc(RANGE_MAX, sizeof(range));

    for (u64 seedIndex = 0; seedIndex < seedCount; seedIndex++)
    {
        rangeSeeds[rangeSeedCount++] = seeds[seedIndex];

        for (u64 transformationCollectionIndex = 0; transformationCollectionIndex < TRANSFORMATION_COLLECTION_COUNT; transformationCollectionIndex++)
        {
            transformation_collection collection = transformations[transformationCollectionIndex];

            printf("TRANSFORM COLLECTION %zu\r\n", transformationCollectionIndex + 1);

            for (u64 rangeSeedIndex = 0; rangeSeedIndex < rangeSeedCount; rangeSeedIndex++)
            {
                range currentRangeSeed = rangeSeeds[rangeSeedIndex];

                printf("  SEED %zu - %zu:\r\n", currentRangeSeed.start, currentRangeSeed.end);

                b32 foundTransform = false;

                for (u64 transformationIndex = 0; transformationIndex < collection.count; transformationIndex++)
                {
                    transformation transform = collection.items[transformationIndex];
                    printf("    TRANSFORM %zu:\r\n", transformationIndex + 1);

                    u64 transformEnd = transform.source + transform.range - 1;

#if 0
                    if (currentRangeSeed.start < transform.source)
                        printf("      <=  ");
                    else if (currentRangeSeed.start >= transform.source && currentRangeSeed.start <= transformEnd)
                        printf("       =  ");
                    else
                        printf("       => ");
                    printf("START (%zu - %zu)\r\n", transform.source, transformEnd);

                    if (currentRangeSeed.end < transform.source)
                        printf("      <=  ");
                    else if (currentRangeSeed.end >= transform.source && currentRangeSeed.end <= transformEnd)
                        printf("       =  ");
                    else
                        printf("       => ");
                    printf("END (%zu - %zu)\r\n", transform.source, transformEnd);

                    if (transformationCollectionIndex == 2 && currentRangeSeed.start == 53)
                    {
                        printf("!! SEED START: %zu, END: %zu, TRANSFORM SOURCE: %zu, RANGE: %zu, TARGET: %zu!\r\n",
                            currentRangeSeed.start, currentRangeSeed.end, transform.source, transform.range, transform.target);
                    }
                    //            [               ]
                    //    {             }
                    //            |     |
// ------------------------------------------------------------------------
                    //            [               ]
                    //    {                          }
                    //            |               |
// ------------------------------------------------------------------------
                    //            [               ]
                    //                {        }
                    //                |        |
// ------------------------------------------------------------------------
                    //            [               ]
                    //                {                 }
                    //                |           |
#endif

                    if (!(currentRangeSeed.end < transform.source || currentRangeSeed.start > transformEnd))
                    {
                        // some sort of collision, take the larger value of starts, take the lower value of ends
                        // apply the (+ transform.target - transform.source)
                        foundTransform = true;

                        nextRangeSeeds[nextRangeSeedCount++] = (range)
                        {
                            .start = max(currentRangeSeed.start, transform.source) + (transform.target - transform.source),
                            .end = min(currentRangeSeed.end, transformEnd) + (transform.target - transform.source)
                        };
                        printf("Always happens, man.. \r\n");
                    }

#if 0
                    if (currentRangeSeed.start <= transform.source)
                    {
                        //printf("DOINK? ");
                        // the lowest number will be in the transform range
                        if (currentRangeSeed.end >= transform.source)
                        {
                            printf("HIT (LEFT) %zu - %zu FALLS IN RANGE OF %zu - %zu\r\n", currentRangeSeed.start, currentRangeSeed.end, transform.source, transformEnd);
                            foundTransform = true;
                            // match, return transform.target
                            nextRangeSeeds[nextRangeSeedCount++] = (range)
                            {
                                .start = transform.target + (currentRangeSeed.start - transform.source),
                                .end = transform.target + (currentRangeSeed.start - transform.source)
                            };
                        }
                        //else printf("NO\r\n");
                    }
                    else if (currentRangeSeed.start >= transform.source)
                    {
                        //printf("BLORF? ");
                        // the lowest number will be in the seed range
                        if (transform.source + transform.range - 1 >= currentRangeSeed.start)
                        {
                            printf("HIT (RIGHT) %zu - %zu FALLS IN RANGE OF %zu - %zu\r\n", currentRangeSeed.start, currentRangeSeed.end, transform.source, transformEnd);
                            foundTransform = true;
                            // match, return current range start
                            nextRangeSeeds[nextRangeSeedCount++] = (range)
                            {
                                .start = transform.target - transform.source + currentRangeSeed.start,
                                .end = transform.target - transform.source + currentRangeSeed.start
                            };
                        }
                        //else printf("NO\r\n");
                    }
#endif
                }

                if (!foundTransform)
                {
                    nextRangeSeeds[nextRangeSeedCount++] = currentRangeSeed;
                    printf("BOINKED -> %zu - %zu\r\n", currentRangeSeed.start, currentRangeSeed.end);
                }
            }

            memcpy(rangeSeeds, nextRangeSeeds, nextRangeSeedCount * sizeof(range));
            rangeSeedCount = nextRangeSeedCount;
            nextRangeSeedCount = 0;

            // for (u64 i = 0; i < rangeSeedCount; i++)
            //     printf("{ TC: %zu -> Seed %zu - %zu }\r\n", transformationCollectionIndex + 1, rangeSeeds[i].start, rangeSeeds[i].end);
        }

        //printf("rangeSeedCount = %zu\r\n", rangeSeedCount);
        
        for (u32 rangeSeedIndex = 0; rangeSeedIndex < rangeSeedCount; rangeSeedIndex++)
        {
            printf("NEXT: %zu - %zu\r\n", nextRangeSeeds[rangeSeedIndex].start, nextRangeSeeds[rangeSeedIndex].end);
            printf("CURR: %zu - %zu\r\n", rangeSeeds[rangeSeedIndex].start, rangeSeeds[rangeSeedIndex].end);

            if (nextRangeSeeds[rangeSeedIndex].start < result)
                result = nextRangeSeeds[rangeSeedIndex].start;
        }


        rangeSeedCount = 0;
    }

    return result;
}

internal u64
part_1()
{
    file_data file = read_to_end_of_file("input\\day05-input.txt");
    assert(file.size > 0);

    return transform_seeds(file, SEED_READING_MODE_one_by_one);
}

internal u64
part_2()
{
    file_data file = read_to_end_of_file("input\\sample-input.txt");
    assert(file.size > 0);
    
    return 0; // transform_seeds(file, SEED_READING_MODE_ranges);
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

    assert(resultPart1 < 704533868 && "ERROR: TOO HIGH VALUE");

    debug_log("- Day 05 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}