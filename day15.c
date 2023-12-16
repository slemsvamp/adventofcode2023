#include "common.h"
#include "lexer.h"

u64
calculate_hash(char *data, u64 length)
{
    u64 result = 0;
    
    for (u32 index = 0; index < length; index++)
    {
        result += data[index];
        result *= 17;
        result %= 256;
    }
    
    return result;
}

u64
part_1()
{
    file_data file = read_to_end_of_file("input\\day15-input.txt");
    assert(file.size > 0);

    u64 result = 0;

    char *token = strtok(file.data, ",");
    while (token)
    {
        result += calculate_hash(token, strlen(token));
        token = strtok(NULL, ",");
    }

    return result;
}

typedef struct step
{
    char *label;
    u8 focalLength;
    struct step *next;
} step;

typedef struct box
{
    step *steps;
} box;

u64
part_2()
{
    file_data file = read_to_end_of_file("input\\day15-input.txt");
    assert(file.size > 0);
    
    u64 result = 0;

    box boxes[256] = {0};
    char byteBuffer[64768] = {0};
    u64 byteBufferCount;
    char *instructionBuffer[4096] = {0};
    u64 instructionBufferCount = 0;
    step stepBuffer[2048] = {0};
    u64 stepBufferCount = 0;

    char *token = strtok(file.data, ",");
    while (token)
    {
        u64 tokenLength = strlen(token) + 1;
        assert(byteBufferCount < 64768);
        memcpy(byteBuffer + byteBufferCount, token, tokenLength);

        assert(instructionBufferCount < 4096);
        instructionBuffer[instructionBufferCount++] = byteBuffer + byteBufferCount;

        byteBufferCount += tokenLength;
        token = strtok(NULL, ",");
    }

    for (u64 index = 0; index < instructionBufferCount; index++)
    {
        char *s = instructionBuffer[index];
        char *needle = strstr(s, "-");
        b32 remove = true;
        if (!needle)
        {
            needle = strstr(s, "=");
            remove = false;
        }

        u64 labelLength = needle - s;
        char *label = calloc(labelLength + 1, sizeof(char));
        memcpy(label, s, labelLength);
        u64 boxId = calculate_hash(label, labelLength);
        u64 focalLength = atoll(needle + 1);
        
        if (remove)
        {
            step *nextStep = boxes[boxId].steps;
            step *prevStep = 0;
            
            while (nextStep)
            {
                if (strcmp(nextStep->label, label) == 0)
                {
                    if (prevStep)
                        prevStep->next = nextStep->next;
                    else
                        boxes[boxId].steps = nextStep->next;
                    break;
                }

                prevStep = nextStep;
                nextStep = nextStep->next;
            }
        }
        else
        {
            step *nextStep = boxes[boxId].steps;
            step *prevStep = 0;
            
            b32 addStep = true;

            while (nextStep)
            {
                if (strcmp(nextStep->label, label) == 0)
                {
                    nextStep->focalLength = focalLength;
                    addStep = false;
                    break;
                }

                prevStep = nextStep;
                nextStep = nextStep->next;
            }

            if (addStep)
            {
                assert(stepBufferCount < 2048);
                step *newStep = stepBuffer + stepBufferCount;
                stepBufferCount++;
                newStep->label = label;
                newStep->focalLength = focalLength;
                newStep->next = 0;

                if (prevStep)
                    prevStep->next = newStep;
                else
                    boxes[boxId].steps = newStep;
            }
        }
    }

    for (u64 boxIndex = 0; boxIndex < 256; boxIndex++)
    {
        if (!boxes[boxIndex].steps)
            continue;
        
        u64 slotIndex = 0;
        
        step *nextStep = boxes[boxIndex].steps;
        while (nextStep)
        {
            result += (boxIndex + 1) * (slotIndex + 1) * nextStep->focalLength;
            slotIndex++;
            nextStep = nextStep->next;
        }

    }
   
    return result;
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

    debug_log("- Day 15 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}