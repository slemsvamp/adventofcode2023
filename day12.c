//#define SILENT

#include "common.h"
#include "lexer.h"

#define MEMORY_CAPACITY (u64)(1024*1024*1024)

typedef struct record
{
    u64 *groups;
    u64 groupCount;

    char *line;
    u64 lineLength;
} record;

typedef struct records
{
    record *records;
    u64 count;
} records;

char *_memory;
u64 _allocatedBytes = 0;

char *
allocate_memory(char *data, u64 size)
{
    if (!(_allocatedBytes + size < MEMORY_CAPACITY))
    {
        printf("Tried adding %lld to already allocated %lld byte(s), max allowed is %lld\r\n", size, _allocatedBytes, MEMORY_CAPACITY);
        assert(false && "Out of memory.");
    }
    for (u64 i = 0; i < size; i++)
        _memory[_allocatedBytes+i] = data[i];
    //memcpy(_memory + _allocatedBytes, data, size);
    char *result = _memory + _allocatedBytes;
    _allocatedBytes += size + 1;
    return result;
}

records
parse_records(b32 normalMode)
{
    records result = {0};
    
    file_data file = read_to_end_of_file("input\\day12-input.txt");
    assert(file.size > 0);

    record recordBuffer[2048] = {0};
    u64 recordCount = 0;

    u64 groups[1024] = {0};
    u64 groupCount = {0};

    u64 subCount = 0;

    lexer_tokenizer tokenizer = lexer_tokenizer_create(file.data);
    lexer_token token = lexer_get_token(&tokenizer);

    b32 firstNumber = true;
    char *startOfLine = 0;

    while (true)
    {
        switch (token.type)
        {
            case LEXER_TOKEN_TYPE_number:
                groups[groupCount++] = atoll(token.text);
                if (firstNumber)
                    startOfLine = tokenizer.at - subCount - token.length - 1;
                firstNumber = false;
            break;
            case LEXER_TOKEN_TYPE_comma:
            break;
            case LEXER_TOKEN_TYPE_end:
            case LEXER_TOKEN_TYPE_newline:
                record rec =
                {
                    .line = startOfLine,
                    .lineLength = subCount,
                    .groups = calloc(groupCount, sizeof(u64)),
                    .groupCount = groupCount
                };
                memcpy(rec.groups, groups, groupCount * sizeof(u64));
                recordBuffer[recordCount++] = rec;
                
                firstNumber = true;
                subCount = 0;
                groupCount = 0;
            break;
            default:
                subCount++;
            break;
        }

        if (lexer_token_is_end(token))
            break;

        token = lexer_get_token(&tokenizer);
    }

    result.records = calloc(recordCount, sizeof(record));
    memcpy(result.records, recordBuffer, recordCount * sizeof(record));
    result.count = recordCount;

    // TODO: i got tired, forgive me. i feel bad as it is. i have excuses. many in fact.
    if (!normalMode)
    {
        for (u64 index = 0; index < result.count; index++)
        {
            record rec = result.records[index];
            record new =
            {
                .groupCount = rec.groupCount * 5,
                .lineLength = (rec.lineLength + 1) * 5 - 1,
                .groups = 0,
                .line = 0
            };
            
            new.groups = calloc(new.groupCount, sizeof(u64));
            new.line = calloc(new.lineLength, sizeof(char));
            
            for (u64 iteration = 0; iteration < 5; iteration++)
            {
                char *lineDest = new.line + iteration * rec.lineLength + iteration;
                memcpy(lineDest, rec.line, rec.lineLength);
                if (iteration != 4)
                    lineDest[rec.lineLength] = '?';
                memcpy((char *)new.groups + iteration * rec.groupCount * sizeof(u64), rec.groups, rec.groupCount * sizeof(u64));
            }
            
            // printf("old: \"%.*s\" group: ", rec.lineLength, rec.line);
            // for (u64 i = 0; i < rec.groupCount; i++)
            //     printf("%lld ", rec.groups[i]);
            // printf("\b\r\n");
            // printf("new: \"%.*s\"\r\n", new.lineLength, new.line);
            // for (u64 i = 0; i < new.groupCount; i++)
            //     printf("%lld ", new.groups[i]);
            // printf("\b\r\n");
            
            // free(rec.groups);
            // free(rec.line);

            result.records[index] = new;
        }
    }

    return result;
}

#ifndef SILENT
void
print_records(records recs)
{
    for (u64 index = 0; index < recs.count; index++)
    {
        record rec = recs.records[index];
        log("%lld: %.*s -> ", index, rec.lineLength, rec.line);
        for (u64 groupIndex = 0; groupIndex < rec.groupCount; groupIndex++)
            log("%lld ", rec.groups[groupIndex]);
        log("\r\n");
    }
}
#endif

b32
valid_arrangement(char *line, u64 lineLength, u64 *groups, u64 groupCount)
{
    u64 playhead = 0;
    for (u64 groupIndex = 0; groupIndex < groupCount; groupIndex++)
    {
        u64 group = groups[groupIndex];
        u64 springCount = 0;

        for (u64 index = playhead; index < lineLength; index++, playhead++)
        {
            if (line[index] == '#')
            {
                springCount++;
                if (springCount > group)
                    return false;
            }
            else if (line[index] == '.' && springCount != 0)
                break;
            else if (line[index] == '?')
                return false;
        }

        if (group != springCount)
            return false;
    }

    for (u64 index = playhead; index < lineLength; index++, playhead++)
    {
        if (line[index] != '.')
        {
            //log("invalidated arrangement because there were ? or # left\r\n");
            return false;
        }
    }
    
    return true;
}

b32
bad_arrangement(char *line, u64 lineLength, u64 *groups, u64 groupCount)
{
    u64 playhead = 0;
    for (u64 groupIndex = 0; groupIndex < groupCount; groupIndex++)
    {
        u64 group = groups[groupIndex];
        u64 springCount = 0;

        for (u64 index = playhead; index < lineLength; index++, playhead++)
        {
            if (line[index] == '#')
            {
                springCount++;
                if (springCount > group)
                    return true;
            }
            else if (line[index] == '.' && springCount != 0)
                break;
            else if (line[index] == '?')
                return false;
        }

        if (group != springCount)
            return true;
    }

    return false;
}

u64
recursive_arrangement_check(u64 playhead, char *line, u64 lineLength, u64 *groups, u64 groupCount)
{
    u64 result = 0;

    playhead++;

    // we don't want a full valid check, just a "is this even plausible" check
    if (bad_arrangement(line, lineLength, groups, groupCount))
        return result;

    for (u64 index = playhead; index < lineLength; index++)
    {
        // TODO: this is a bit ugly, but I left it like this because I wanted another solution than the other one.
        if (index > 0 && line[index - 1] == '?')
            break;

        if (line[index] != '?')
            continue;

        u64 backtrackAllocation = _allocatedBytes;
        char *damagedVersion = allocate_memory(line, lineLength);
        damagedVersion[index] = '#';
        //b32 breakpoint = strcmp(damagedVersion, ".#...#....###.");
        result += recursive_arrangement_check(index, damagedVersion, lineLength, groups, groupCount);
        _allocatedBytes = backtrackAllocation;

        char *operationalVersion = allocate_memory(line, lineLength);
        operationalVersion[index] = '.';
        result += recursive_arrangement_check(index, operationalVersion, lineLength, groups, groupCount);
        _allocatedBytes = backtrackAllocation;
    }

    u64 validity = valid_arrangement(line, lineLength, groups, groupCount);
    
    if (validity)
    {
        // log("We thought '%.*s' looked valid for groups (", lineLength, line);
        // for (u64 groupIndex = 0; groupIndex < groupCount; groupIndex++)
        //     log("%lld ", groups[groupIndex]);
        // log("\b)\r\n");
    }

    return result + validity;
}

u64
calculate_arrangements(record rec)
{
    u64 result = 0;

    // . operational
    // # damaged
    // ? unknown

    char *base = allocate_memory(rec.line, rec.lineLength);

    u64 index = 0;
    while (_memory[index] != '?')
        index++;

    u64 backtrackAllocation = _allocatedBytes;
    char *damagedVersion = allocate_memory(base, rec.lineLength);
    damagedVersion[index] = '#';
    result += recursive_arrangement_check(index, damagedVersion, rec.lineLength, rec.groups, rec.groupCount);
    _allocatedBytes = backtrackAllocation;

    char *operationalVersion = allocate_memory(base, rec.lineLength);
    operationalVersion[index] = '.';
    result += recursive_arrangement_check(index, operationalVersion, rec.lineLength, rec.groups, rec.groupCount);
    _allocatedBytes = backtrackAllocation;

    // even if it's valid here, there might be more arrangements that are valid, so we can't just assume
    // that we're done, this is likely where we have to send it into recursive checks to make sure we
    // create different versions

    return result;
}

u64
part_1()
{
    u64 result = 0;
    records recs = parse_records(true);
    for (u64 recordIndex = 0; recordIndex < recs.count; recordIndex++)
    {
        u64 sum = calculate_arrangements(recs.records[recordIndex]);
        //log("%lld: allocated bytes: %lld\r\n", recordIndex + 1, _allocatedBytes);
        //log("record %lld: %lld\r\n", recordIndex + 1, sum);
        result += sum;
        _allocatedBytes = 0;
    }
    return result;
}

u64
part_2()
{
    u64 result = 0;
    records recs = parse_records(false);
    for (u64 recordIndex = 0; recordIndex < recs.count; recordIndex++)
    {
        u64 sum = calculate_arrangements(recs.records[recordIndex]);
        //log("%lld: allocated bytes: %lld\r\n", recordIndex + 1, _allocatedBytes);
        log("\nrecord %lld: %lld\r\n", recordIndex + 1, sum);
        result += sum;
        _allocatedBytes = 0;
    }
    return result;
}

u32
main(s32 argumentCount, char *arguments[])
{
    if (MEMORY_CAPACITY > 1024*1024*1024)
        assert(false && "No.");

    _memory = calloc(MEMORY_CAPACITY, 0x01);

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = part_1();

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    debug_log("- Day 12 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));

    debug_log("Part 2 not complete.\n");
    return 0;

    // memoization?
    u64 resultPart2 = part_2();

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}