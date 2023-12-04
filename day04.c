#include "common.h"
#include "lexer.h"

typedef struct numbers
{
    s32 *items;
    u64 count;
    u64 capacity;
} numbers;

typedef struct card
{
    u64 index;
    numbers winning;
    numbers drawn;
} card;

typedef struct results
{
    u64 points;
    u64 scratchCardCount;
} results;

inline void
safe_add_number(numbers *nums, s32 number)
{
    if (nums->count < nums->capacity) nums->items[nums->count++] = number;
}

void
process_card(lexer_tokenizer *tokenizer, lexer_token *token, card *currentCard)
{
    *token = lexer_require_token(tokenizer, LEXER_TOKEN_TYPE_identifier);
    lexer_get_next(tokenizer, LEXER_TOKEN_TYPE_colon);
    *token = lexer_get_token(tokenizer);

    while (token->type == LEXER_TOKEN_TYPE_number)
    {
        safe_add_number(&currentCard->winning, atoi(token->text));
        *token = lexer_get_token(tokenizer);
    }

    *token = lexer_get_token(tokenizer);
    
    while (token->type == LEXER_TOKEN_TYPE_number)
    {
        safe_add_number(&currentCard->drawn, atoi(token->text));
        *token = lexer_get_token(tokenizer);
    }    
}

card
init_card(u64 capacity)
{
    return (card)
    {
        .index = 0,
        .winning = { .capacity = capacity, .count = 0, .items = calloc(capacity, sizeof(s32)) },
        .drawn = { .capacity = capacity, .count = 0, .items = calloc(capacity, sizeof(s32)) }
    };
}

results
process_cards(file_data file)
{
    results result = {0};

    lexer_tokenizer tokenizer = { .at = file.data };
    lexer_token token = __lexer_token_create(LEXER_TOKEN_TYPE_unknown, 0, 0);

    card currentCard = init_card(256);

    u32 cardMultiplier[256] = {0};
    u32 winningCount = 0;

    while (!lexer_token_is_end(token))
    {
        process_card(&tokenizer, &token, &currentCard);

        for (u32 winningNumberIndex = 0; winningNumberIndex < currentCard.winning.count; winningNumberIndex++)
            for (u32 ourNumberIndex = 0; ourNumberIndex < currentCard.drawn.count; ourNumberIndex++)
                if (currentCard.drawn.items[ourNumberIndex] == currentCard.winning.items[winningNumberIndex])
                    winningCount++;

        if (winningCount > 0)
        {
            result.points += 1 << winningCount - 1;
            for (u32 cardMultiplierIndex = 1; cardMultiplierIndex <= winningCount; cardMultiplierIndex++)
                cardMultiplier[currentCard.index + cardMultiplierIndex] += cardMultiplier[currentCard.index] + 1;
        }

        currentCard.winning.count = currentCard.drawn.count = winningCount = 0;
        currentCard.index++;
    }

    for (u32 i = 0; i < currentCard.index; i++)
        result.scratchCardCount += cardMultiplier[i] + 1;

    return result;
}

u32
main(s32 argumentCount, char *arguments[])
{
    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    file_data file = read_to_end_of_file("input\\day04-input.txt");
    assert(file.size > 0);

    results result = process_cards(file);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 04 -\n");
    debug_log("Result Part 1: %lld, Result Part 2: %lld (%d ms, %lld cycles passed)\n",
        result.points, result.scratchCardCount, (endTime - startTime) * 1000 / CLOCKS_PER_SEC, (endCycles - startCycles));
    debug_log("\n");

    return 0;
}