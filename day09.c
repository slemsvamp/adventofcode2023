#include "common.h"
#include "lexer.h"

typedef struct history
{
    s64 *values;
    u64 count;
} history;

typedef struct environmental_report
{
    history *histories;
    u64 count;
} environmental_report;

environmental_report
get_environmental_report()
{
    file_data file = read_to_end_of_file("input\\day09-input.txt");
    assert(file.size > 0);

    char *line = strtok(file.data, "\r\n");

    history histories[1024] = {0};
    u64 historiesCount = 0;    

    s64 values[1024] = {0};
    u64 valuesCount = 0;

    while (line)
    {
        size_t length = strlen(line);
        
        u64 playhead = 0;
        while (playhead < length)
        {
            values[valuesCount++] = atoll(line+playhead);
            
            while (line[playhead] != ' ' && playhead < length)
                playhead++;
            
            if (playhead < length)
                playhead++;
        }

        history hist = {0};
        hist.values = calloc(valuesCount, sizeof(s64));
        memcpy(hist.values, values, valuesCount * sizeof(s64));
        hist.count = valuesCount;
        
        valuesCount = 0;
        histories[historiesCount++] = hist;

        line = strtok(NULL, "\r\n");
    }

    return (environmental_report)
    {
        .histories = histories,
        .count = historiesCount
    };
}

b32
all_values_are_zero(s64 *buffer, u64 count)
{
    for (u64 index = 0; index < count; index++)
        if (buffer[index] != 0)
            return false;
    return true;
}

typedef enum history_mode
{
    HISTORY_MODE_end,
    HISTORY_MODE_start
} history_mode;

s64
parse_history(history hist, history_mode mode)
{
    u64 count = hist.count;
    s64 *buffer = calloc(count, sizeof(s64));

    if (mode == HISTORY_MODE_end)
        memcpy(buffer, hist.values, count * sizeof(s64));
    else if (mode == HISTORY_MODE_start)
        for (u64 index = 0; index < count; index++)
            buffer[count - index - 1] = hist.values[index];

    s64 relevantNumberBuffer[256] = {0};
    u64 relevantNumberCount = 0;

    relevantNumberBuffer[relevantNumberCount++] = buffer[count-1];

    while (count > 0 && !all_values_are_zero(buffer, count))
    {
        for (u64 playhead = 0; playhead < count-1; playhead++)
            buffer[playhead] = buffer[playhead+1] - buffer[playhead];
        count--;
        relevantNumberBuffer[relevantNumberCount++] = buffer[count-1];
    }

    s64 result = 0;
    
    for (u64 index = 0; index < relevantNumberCount; index++)
        result += relevantNumberBuffer[index];

    return result;
}

s64
parse_report(environmental_report report, history_mode mode)
{
    s64 result = 0;

    for (u64 historyIndex = 0; historyIndex < report.count; historyIndex++)
        result += parse_history(report.histories[historyIndex], mode);

    return result;
}

s64
part_1(environmental_report report)
{
    return parse_report(report, HISTORY_MODE_end);
}

s64
part_2(environmental_report report)
{
    return parse_report(report, HISTORY_MODE_start);
}

// NEGATIVE NUMBERS

u32
main(s32 argumentCount, char *arguments[])
{
    environmental_report report = get_environmental_report();

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    s64 resultPart1 = part_1(report);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    s64 resultPart2 = part_2(report);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 09 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}