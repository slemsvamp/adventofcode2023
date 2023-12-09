#define SILENT

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

void
reading_seeds(lexer_tokenizer *t, lexer_token token, range *seeds, u64 *seedCount, seed_reading_mode mode)
{
    u64 count = 0;
    while (token.type == LEXER_TOKEN_TYPE_number)
    {
        u64 startSeed = atoll(token.text);

        if (mode == SEED_READING_MODE_one_by_one)
        {
            seeds[count++] = (range)
            {
                .start = startSeed,
                .end = startSeed
            };
        }
        else if (mode == SEED_READING_MODE_ranges)
        {
            lexer_token peekToken = lexer_peek_token(t);
            
            if (peekToken.type == LEXER_TOKEN_TYPE_newline)
                break;
            
            token = lexer_get_token(t);

            u64 seedRange = atoll(token.text);

            seeds[count++] = (range)
            {
                .start = startSeed,
                .end = startSeed + seedRange - 1
            };

            log("SEED %zu - %zu\r\n", startSeed, startSeed+seedRange-1);
        }
        else
            invalid_code_path;

        token = lexer_get_token(t);
    }

    *seedCount = count;
}

range
transform_range(u64 start, u64 end, transformation transform)
{
    return (range)
    {
        .start = (u64)(start + ((s64)transform.target - transform.source)),
        .end = (u64)(end + ((s64)transform.target - transform.source))
    };
}

s32
compare_transformation(transformation first, transformation second)
{
    if (first.source < second.source) return -1;
    else if (second.source < first.source) return 1;
    return 0;
}

void
switch_transformations(transformation *first, transformation *second)
{
    transformation third = {0};
    third = *first;
    *first = *second;
    *second = third;
}

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

    lexer_tokenizer t = lexer_tokenizer_create(file.data);
    lexer_token token = lexer_get_next(&t, LEXER_TOKEN_TYPE_number);

    reading_seeds(&t, token, seeds, &seedCount, seedReadingMode);

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

    // ORDERING TRANSFORMS
    for (u64 transformationCollectionIndex = 0; transformationCollectionIndex < TRANSFORMATION_COLLECTION_COUNT; transformationCollectionIndex++)
    {
        transformation_collection collection = transformations[transformationCollectionIndex];
        u64 transformCount = collection.count - 1;

        while (transformCount >= 1)
        {
            for (u64 transformationIndex = 0; transformationIndex < transformCount; transformationIndex++)
            {
                transformation *first = collection.items + transformationIndex;
                transformation *second = collection.items + transformationIndex + 1;

                if (compare_transformation(*first, *second) == 1)
                    switch_transformations(first, second);
            }
            transformCount--;
        }
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
            log("TRANSFORM COLLECTION %zu\r\n", transformationCollectionIndex + 1);

            for (u64 rangeSeedIndex = 0; rangeSeedIndex < rangeSeedCount; rangeSeedIndex++)
            {
                range currentRangeSeed = rangeSeeds[rangeSeedIndex];
                log("  SEED %zu - %zu:\r\n", currentRangeSeed.start, currentRangeSeed.end);

                b32 createdSubseeds = false;
                u64 marker = currentRangeSeed.start;

                for (u64 transformationIndex = 0; transformationIndex < collection.count && marker <= currentRangeSeed.end; transformationIndex++)
                {
                    transformation transform = collection.items[transformationIndex];
                    u64 transformEnd = transform.source + transform.range - 1;
                    log("    TRANSFORM %zu (%zu to %zu) CHANGE: %lld:", transformationIndex + 1, transform.source, transformEnd, (s64)transform.target - transform.source);

                    if (marker < transform.source)
                    {
                        u64 seedEnd = min(transform.source - 1, currentRangeSeed.end);

                        nextRangeSeeds[nextRangeSeedCount++] = (range)
                        {
                            .start = marker,
                            .end = seedEnd
                        };

                        seedEnd = min(transformEnd, currentRangeSeed.end);

                        nextRangeSeeds[nextRangeSeedCount++] = transform_range(transform.source, seedEnd, transform);

                        marker = seedEnd+1;
                        createdSubseeds = true;
                    }
                    else if (marker >= transform.source && marker <= transformEnd)
                    {
                        u64 seedStart = max(transform.source, marker);
                        u64 seedEnd = min(transformEnd, currentRangeSeed.end);

                        nextRangeSeeds[nextRangeSeedCount++] = transform_range(seedStart, seedEnd, transform);

                        marker = seedEnd+1;
                        createdSubseeds = true;
                    }
                    else if (marker > transformEnd)
                    {
                    }
                    else
                    {
                        log("marker: %lld, transform source: %lld, transform end: %lld\r\n", marker, transform.source, transformEnd);
                        invalid_code_path;
                    }

                    log(" nextRangeSeedCount: %lld\r\n", nextRangeSeedCount);
                }

                // if we did not create any sub seeds
                if (!createdSubseeds)
                    nextRangeSeeds[nextRangeSeedCount++] = currentRangeSeed;
            }

            memcpy(rangeSeeds, nextRangeSeeds, nextRangeSeedCount * sizeof(range));
            rangeSeedCount = nextRangeSeedCount;
            nextRangeSeedCount = 0;

            // for (u64 i = 0; i < rangeSeedCount; i++)
            //     log("{ TC: %zu -> Seed %zu - %zu }\r\n", transformationCollectionIndex + 1, rangeSeeds[i].start, rangeSeeds[i].end);
        }

        //log("rangeSeedCount = %zu\r\n", rangeSeedCount);
        
        for (u32 rangeSeedIndex = 0; rangeSeedIndex < rangeSeedCount; rangeSeedIndex++)
        {
            //log("NEXT: %zu - %zu\r\n", nextRangeSeeds[rangeSeedIndex].start, nextRangeSeeds[rangeSeedIndex].end);
            //log("CURR: %zu - %zu\r\n", rangeSeeds[rangeSeedIndex].start, rangeSeeds[rangeSeedIndex].end);

            if (nextRangeSeeds[rangeSeedIndex].start < result)
                result = nextRangeSeeds[rangeSeedIndex].start;
        }

        log(">> RESULT: %lld --\r\n", result);

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
    file_data file = read_to_end_of_file("input\\day05-input.txt");
    assert(file.size > 0);
    
    return transform_seeds(file, SEED_READING_MODE_ranges);
}

int
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

    debug_log("- Day 05 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}