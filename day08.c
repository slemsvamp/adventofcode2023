#define SILENT

#include "common.h"
#include "lexer.h"

typedef struct choice
{
    char key[4];
    char left[4];
    char right[4];
} choice;

typedef struct instructions
{
    char *directions;
    u64 directionsLength;
    dictionary dict;
} instructions;

u64 _choices_count = 0;
choice _choices[1024] = {0};

u64 _keysEndingWithACount = 0;
char _allKeysEndingWithA[1024][4];

instructions
parse_instructions()
{
    #define DIRECTIONS_CAPACITY 1024

    instructions result =
    {
        .directions = 0,
        .directionsLength = 0,
        .dict = {0}
    };

    dict_init(&result.dict, 256);

    file_data file = read_to_end_of_file("input\\day08-input.txt");
    assert(file.size > 0);

    lexer_tokenizer tokenizer = lexer_tokenizer_create(file.data);
    lexer_token token = lexer_get_token(&tokenizer);

    result.directionsLength = token.length;
    result.directions = calloc(token.length + 1, sizeof(char));
    memcpy(result.directions, token.text, token.length * sizeof(char));
    result.directions[token.length] = 0;

    while (true)
    {
        char key[4] = {0}, left[4] = {0}, right[4] = {0};
        token = lexer_get_next(&tokenizer, LEXER_TOKEN_TYPE_identifier);
        memcpy(key, token.text, 3 * sizeof(char));
        lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_equals);
        lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_open_parenthesis);
        token = lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_identifier);
        memcpy(left, token.text, 3 * sizeof(char));
        lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_comma);
        token = lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_identifier);
        memcpy(right, token.text, 3 * sizeof(char));
        lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_close_parenthesis);
        
        memcpy(_choices[_choices_count].key, key, 4 * sizeof(char));
        memcpy(_choices[_choices_count].left, left, 4 * sizeof(char));
        memcpy(_choices[_choices_count].right, right, 4 * sizeof(char));
        
        if (key[2] == 'A')
            memcpy(_allKeysEndingWithA + _keysEndingWithACount++, key, 4 * sizeof(char));

        dict_add(&result.dict, _choices[_choices_count].key, _choices + _choices_count);

        _choices_count++;

        if (lexer_token_is_end(lexer_peek_token(&tokenizer)))
            break;
    }

    return result;
}

choice
get_choice(dictionary *dict, char *key)
{
    return *(choice *)(dict_get(dict, key)->data);
}

u64
part_1(instructions instr)
{
    choice ch = get_choice(&instr.dict, "AAA");

    u64 directionPointer = 0;
    u64 result = 0;
    while (strncmp(ch.key, "ZZZ", 3) != 0)
    {
        char direction = instr.directions[directionPointer++];

        if (direction == 'L')
            ch = get_choice(&instr.dict, ch.left);
        else
            ch = get_choice(&instr.dict, ch.right);

        result++;

        if (directionPointer >= instr.directionsLength)
            directionPointer = 0;
    }

    return result;
}

u64 _cadence_set_count = 0;
u64 _lcm = 1;

b32
all_end_with_z_or_all_cadences_found(choice *choices, u64 count, u64 *cadences, u64 turn)
{
    for (u64 index = 0; index < count; index++)
        if (choices[index].key[2] == 'Z' && cadences[index] == 0)
        {
            cadences[index] = turn;
            log("Cadence found for index %lld on turn %lld.\r\n", index, turn);
        }

    b32 cadencesDone = true;
    for (u64 index = 0; index < count; index++)
        if (cadences[index] == 0)
        {
            cadencesDone = false;
            break;
        }
    if (cadencesDone)
        return true;

    for (u64 index = 0; index < count; index++)
        if (choices[index].key[2] != 'Z')
            return false;
    return true;
}

b32
is_divisible_by(u64 target, u64 divisor)
{
    return (target / divisor) * divisor == target;
}

u64
part_2(instructions instr)
{
    choice currentChoices[1024] = {0};
    u64 count = 0;

    for (u64 keyIndex = 0; keyIndex < _keysEndingWithACount; keyIndex++)
        currentChoices[count++] = get_choice(&instr.dict, _allKeysEndingWithA[keyIndex]);

    u64 directionPointer = 0;
    u64 result = 0;

    u64 *cadences = calloc(count, sizeof(u64));

    while (!all_end_with_z_or_all_cadences_found(currentChoices, count, cadences, result))
    {
        char direction = instr.directions[directionPointer];

        u64 add = 1;

        directionPointer = (directionPointer + add) % instr.directionsLength;

        choice buffer[1024] = {0};

        for (u64 choiceIndex = 0; choiceIndex < count; choiceIndex++)
            buffer[choiceIndex] = get_choice(&instr.dict, direction == 'L' ? currentChoices[choiceIndex].left : currentChoices[choiceIndex].right);

        memcpy(currentChoices, buffer, count * sizeof(choice));

        result += add;

        if (directionPointer >= instr.directionsLength)
            directionPointer = 0;
    }

    result = 0;
    u64 divisor = 2;
    b32 done = false;

    u64 divisors[6] = {0};

    for (u64 index = 0; index < count; index++)
    {
        while (!is_divisible_by(cadences[index], divisor) && divisor < cadences[index])
            divisor++;
        log("divisor for index %lld is %lld\r\n", index, divisor);
        divisors[index] = divisor;
        divisor = 2;
    }

    result = cadences[0];

    for (u64 index = 1; index < count; index++)
        result *= divisors[index];

    return result;
}

u32
main(s32 argumentCount, char *arguments[])
{
    instructions instr = parse_instructions();

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = part_1(instr);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    u64 resultPart2 = part_2(instr);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 08 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}