#include "common.h"
#include "lexer.h"

typedef struct time_and_distance
{
    u64 *times;
    u64 *distances;
    u64 count;
} time_and_distance;

lexer_token_type numbersAndNewlines[] = { LEXER_TOKEN_TYPE_number, LEXER_TOKEN_TYPE_newline, LEXER_TOKEN_TYPE_end };

u64
calculate_distance_by_time_held(u64 time, u64 timeHeld)
{
    return (time - timeHeld) * timeHeld;
}

time_and_distance
parse_time_and_distance(file_data file, b32 asOne)
{
    time_and_distance result = {0};
    u64 count = 0;
    u64 times[20] = {0};
    u64 distances[20] = {0};
    b32 timeValues = true;

    if (asOne)
    {
        char time[256] = {0};
        u64 timeLength = 0;
        char distance[256] = {0};
        u64 distanceLength = 0;

        for (u64 playhead = 0; playhead < file.size; playhead++)
        {
            if (file.data[playhead] == '\n')
                timeValues = false;

            if (timeValues)
            {
                if (isdigit(file.data[playhead]))
                    time[timeLength++] = file.data[playhead];
            }
            else
            {
                if (isdigit(file.data[playhead]))
                    distance[distanceLength++] = file.data[playhead];
            }
        }

        result.count = 1;
        result.times = calloc(1, sizeof(u64));
        result.times[0] = atoll(time);
        result.distances = calloc(1, sizeof(u64));
        result.distances[0] = atoll(distance);

        return result;
    }

    lexer_tokenizer tokenizer = lexer_tokenizer_create(file.data);
    lexer_token token = __lexer_token_create(LEXER_TOKEN_TYPE_unknown, 0, 0);

    while (!lexer_token_is_end(token))
    {
        token = lexer_get_next_of(&tokenizer, numbersAndNewlines, array_count(numbersAndNewlines));

        switch (token.type)
        {
            case LEXER_TOKEN_TYPE_number:
            {
                if (timeValues)
                    times[count++] = atoll(token.text);
                else
                    distances[count++] = atoll(token.text);
            }
            break;
            case LEXER_TOKEN_TYPE_newline:
            {
                timeValues = false;
                count = 0;
            }
        }
    }

    result.count = count;
    result.times = calloc(count, sizeof(u64));
    result.distances = calloc(count, sizeof(u64));

    memcpy(result.times, times, count * sizeof(times[0]));
    memcpy(result.distances, distances, count * sizeof(times[0]));

    return result;
}

u64
find_pivot(u64 time, u64 distance, b32 low)
{
    u64 left = 0;
    u64 right = time;
    u64 searchIndex = 0;

    while (true)
    {
        if (right - left <= 1)
            return left;
        searchIndex = left + ((right - left) / 2);

        assert(searchIndex < time);
        u64 distanceTravelled = calculate_distance_by_time_held(time, searchIndex);
        u64 nextId = 1;
        assert(searchIndex + nextId < time);
        u64 distanceTravelledNext = calculate_distance_by_time_held(time, searchIndex + nextId);

        while (distanceTravelledNext == distanceTravelled)
        {
            nextId++;
            assert(searchIndex + nextId < time);
            distanceTravelledNext = calculate_distance_by_time_held(time, searchIndex + nextId);
        }

        b32 onDistance = distanceTravelled == distance;
        b32 onDistanceNext = distanceTravelledNext == distance;
        b32 trendingDownwards = distanceTravelledNext < distanceTravelled;

        if (low)
        {
            if (onDistance && !trendingDownwards)
                return searchIndex + nextId;

            if (!trendingDownwards && distanceTravelled < distance)
                left = searchIndex;
            else
                right = searchIndex;
        }
        else
        {
            if (onDistanceNext && trendingDownwards)
                return searchIndex + nextId - 1;

            if (trendingDownwards && distanceTravelled < distance)
                right = searchIndex;
            else
                left = searchIndex;
        }
    }
}

u64
calculate(time_and_distance timeAndDistance)
{
    u64 result = 0;
    for (u64 timeAndDistanceIndex = 0; timeAndDistanceIndex < timeAndDistance.count; timeAndDistanceIndex++)
    {
        u64 time = timeAndDistance.times[timeAndDistanceIndex];
        u64 distance = timeAndDistance.distances[timeAndDistanceIndex];
        u64 low = find_pivot(time, distance, true);
        u64 high = find_pivot(time, distance, false);

        if (result == 0)
            result = high - low;
        else
            result *= high - low;
    }

    return result;
}

u64
part_1(time_and_distance timeAndDistance)
{
    return calculate(timeAndDistance);
}

u64
part_2(time_and_distance timeAndDistance)
{
    return calculate(timeAndDistance);
}

u32
main(s32 argumentCount, char *arguments[])
{
    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    file_data file = read_to_end_of_file("input\\day06-input.txt");
    assert(file.size > 0);

    time_and_distance timeAndDistanceSeparately = parse_time_and_distance(file, false);

    u64 resultPart1 = part_1(timeAndDistanceSeparately);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    time_and_distance timeAndDistanceAsOne = parse_time_and_distance(file, true);

    u64 resultPart2 = part_2(timeAndDistanceAsOne);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 06 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}