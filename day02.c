#include "common.h"
#include "lexer.h"

// #define WITH_DEBUG_PRINT

typedef struct game
{
    u32 red;
    u32 green;
    u32 blue;
} game;

b32
is_game_valid(game g)
{
    return g.red <= 12 && g.green <= 13 && g.blue <= 14;
}

u64
run_game(file_data file, b32 withLocalMax)
{
    u64 validGameSum = 0;
    u64 localMaxSum = 0;
    game shown = {0};

    lexer_tokenizer tokenizer = { .at = file.data };
    lexer_token token = __lexer_token_create(LEXER_TOKEN_TYPE_unknown, 0, 0);

    while (!lexer_token_is_end(token))
    {
        token = lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_identifier); // Game
        token = lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_number); // Identifier

        s32 id = atoi(token.text);
        assert(id > 0);

        lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_colon); // :

        token = lexer_get_token(&tokenizer);

        while (true)
        {
            if (token.type == LEXER_TOKEN_TYPE_semi_colon)
            {
                if (!withLocalMax && !is_game_valid(shown))
                {
                    token = lexer_get_next(&tokenizer, LEXER_TOKEN_TYPE_newline);
                    break;
                }
            }
            else if (token.type == LEXER_TOKEN_TYPE_newline)
            {
                break;
            }
            else if (lexer_token_is_end(token))
            {
                break;
            }
            else if (token.type == LEXER_TOKEN_TYPE_comma)
            {
            }
            else
            {
                assert(token.type == LEXER_TOKEN_TYPE_number);
                s32 amount = atoi(token.text);
                token = lexer_require_token(&tokenizer, LEXER_TOKEN_TYPE_identifier); // red | green | blue

                if (strncmp("red", token.text, 3) == 0 && shown.red < amount)
                    shown.red = amount;
                else if (strncmp("green", token.text, 5) == 0 && shown.green < amount)
                    shown.green = amount;
                else if (strncmp("blue", token.text, 4) == 0 && shown.blue < amount)
                    shown.blue = amount;
            }

            token = lexer_get_token(&tokenizer);
        }
        
        if (is_game_valid(shown))
        {
#ifdef WITH_DEBUG_PRINT
            printf("Game %i is valid, red: %i, green: %i, blue: %i\r\n", id, shown.red, shown.green, shown.blue);
#endif
            validGameSum += (u32)id;
        }
#ifdef WITH_DEBUG_PRINT
        else printf("Game %i is invalid, red: %i, green: %i, blue: %i\r\n", id, shown.red, shown.green, shown.blue);
#endif

        if (withLocalMax)
        {
             localMaxSum += shown.red * shown.green * shown.blue;
        }

        shown.red = shown.green = shown.blue = 0;

        if (lexer_token_is_end(token))
        {
            if (withLocalMax)
                return localMaxSum;
            return validGameSum;
        }
    }

    invalid_code_path;
}

internal u64
part_1()
{
    u64 result = 0;
    file_data file = read_to_end_of_file("input\\day02-input.txt");
    assert(file.size > 0);
    return run_game(file, false);
}

internal u64
part_2()
{
    file_data file = read_to_end_of_file("input\\day02-input.txt");
    assert(file.size > 0);
    return run_game(file, true);
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

    debug_log("- Day 02 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}