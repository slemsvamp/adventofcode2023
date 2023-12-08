#include "common.h"
#include "lexer.h"

char cardNames[] =
{
    '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'
};

u32 cardIndices[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

typedef enum hand_type
{
    HAND_TYPE_five_of_a_kind,
    HAND_TYPE_four_of_a_kind,
    HAND_TYPE_full_house,
    HAND_TYPE_three_of_a_kind,
    HAND_TYPE_two_pair,
    HAND_TYPE_one_pair,
    HAND_TYPE_high_card,
    HAND_TYPE_COUNT
} hand_type;

char *hand_type_names[] =
{
    "HAND_TYPE_five_of_a_kind",
    "HAND_TYPE_four_of_a_kind",
    "HAND_TYPE_full_house",
    "HAND_TYPE_three_of_a_kind",
    "HAND_TYPE_two_pair",
    "HAND_TYPE_one_pair",
    "HAND_TYPE_high_card"
};

#define HAND_SIZE 5

typedef struct hand
{
    char cards[HAND_SIZE];
    u64 bid;
    hand_type type;
} hand;

typedef struct game
{
    hand hands[1000];
    u64 count;
} game;

dictionary dict;

void
initialize_dictionary()
{
    dict_init(&dict, 20);
    dict_add(&dict, "A", (u32 *)(cardIndices + 12));
    dict_add(&dict, "K", (u32 *)(cardIndices + 11));
    dict_add(&dict, "Q", (u32 *)(cardIndices + 10));
    dict_add(&dict, "J", (u32 *)(cardIndices + 9));
    dict_add(&dict, "T", (u32 *)(cardIndices + 8));
    dict_add(&dict, "9", (u32 *)(cardIndices + 7));
    dict_add(&dict, "8", (u32 *)(cardIndices + 6));
    dict_add(&dict, "7", (u32 *)(cardIndices + 5));
    dict_add(&dict, "6", (u32 *)(cardIndices + 4));
    dict_add(&dict, "5", (u32 *)(cardIndices + 3));
    dict_add(&dict, "4", (u32 *)(cardIndices + 2));
    dict_add(&dict, "3", (u32 *)(cardIndices + 1));
    dict_add(&dict, "2", (u32 *)(cardIndices));
}

hand
parse_hand(char *line)
{
    hand result =
    {
        .cards = {0},
        .bid = 0,
        .type = HAND_TYPE_high_card
    };

    u32 sortedHand[HAND_SIZE] = {0};

    for (u32 cardIndex = 0; cardIndex < HAND_SIZE; cardIndex++)
        sortedHand[cardIndex] = *(u32 *)(dict_get(&dict, (char[]) { line[cardIndex], 0 })->data);
    qs_sort_u32(sortedHand, 0, 4);

    u64 copyCount = 0;
    u64 numberOfCopies[HAND_SIZE] = {0};
    u64 lookupIndex = 0;

    while (lookupIndex < HAND_SIZE)
    {
        u32 lookAhead = 1;
        u32 lookFor = sortedHand[lookupIndex];
        u32 amount = 1;
        while (lookupIndex + lookAhead < HAND_SIZE && sortedHand[lookupIndex + lookAhead] == lookFor)
        {
            amount++;
            lookAhead++;
        }
        numberOfCopies[copyCount++] = amount;
        lookupIndex += amount;
    }
    qs_sort_u64(numberOfCopies, 0, 4);

    if (numberOfCopies[4] == 5)
        result.type = HAND_TYPE_five_of_a_kind;
    else if (numberOfCopies[4] == 4)
        result.type = HAND_TYPE_four_of_a_kind;
    else if (numberOfCopies[4] == 3 && numberOfCopies[3] == 2)
        result.type = HAND_TYPE_full_house;
    else if (numberOfCopies[4] == 3)
        result.type = HAND_TYPE_three_of_a_kind;
    else if (numberOfCopies[4] == 2 && numberOfCopies[3] == 2)
        result.type = HAND_TYPE_two_pair;
    else if (numberOfCopies[4] == 2)
        result.type = HAND_TYPE_one_pair;

    // for (u32 i = 0; i < 5; i++)
    //     printf("NumberOfCopies: %d\r\n", numberOfCopies[i]);

    memcpy(result.cards, line, HAND_SIZE * sizeof(char));

    result.bid = atoll(line + 6);

    return result;
}

void
print_hand(hand h)
{
    printf("Hand: ");
    for (u32 i = 0; i < 5; i++)
        printf("%c", h.cards[i]);
    printf(", %lld -> %s\r\n", h.bid, hand_type_names[h.type]);
}

game
parse_hands()
{
    game result = {0};

    file_data file = read_to_end_of_file("input\\day07-input.txt");
    assert(file.size > 0);

    for (u64 playhead = 0; playhead < file.size; playhead++)
        if (file.data[playhead] == '\r' || file.data[playhead] == '\n')
            file.data[playhead] = 0;

    char *at = file.data;
    char buffer[256] = {0};

    while (at < file.data + file.size)
    {
        size_t lineLength = strlen(at);
        assert(lineLength <= file.data + file.size - at);
        memcpy(buffer, at, lineLength);
        buffer[lineLength] = 0;
        result.hands[result.count++] = parse_hand(buffer);
        at += lineLength + 2;
    }
    
    return result;
}

void
swap_hands(hand *first, hand *second)
{
    hand third = {0};
    third = *first;
    *first = *second;
    *second = third;
}

s32
compare_hands(hand first, hand second)
{
    // return -1 if first is less valuable than second
    // return 0 if the same value
    // return 1 if second is less valueable than first
    u64 result = 0;
    for (u64 cardIndex = 0; cardIndex < HAND_SIZE; cardIndex++)
    {
        u32 firstHandCardValue = *(u32 *)(dict_get(&dict, (char[]) { first.cards[cardIndex], 0 })->data);
        u32 secondHandCardValue = *(u32 *)(dict_get(&dict, (char[]) { second.cards[cardIndex], 0 })->data);
        if (firstHandCardValue < secondHandCardValue)
            return -1;
        else if (secondHandCardValue < firstHandCardValue)
            return 1;
    }
    return result;
}

u64
part_1(game entireGame)
{
    hand *handsByType[HAND_TYPE_COUNT] = {0};
    u64 handsByTypeCount[HAND_TYPE_COUNT] = {0};

    #define HANDS_PER_TYPE_CAPACITY 256
    for (u64 byHandTypeIndex = 0; byHandTypeIndex < HAND_TYPE_COUNT; byHandTypeIndex++)
        handsByType[byHandTypeIndex] = calloc(HANDS_PER_TYPE_CAPACITY, sizeof(hand));

    // 1. GROUPING BY HAND TYPE
    for (u64 gameIndex = 0; gameIndex < entireGame.count; gameIndex++)
    {
        u64 handTypeIndex = (u64)entireGame.hands[gameIndex].type;
        (handsByType[handTypeIndex])[handsByTypeCount[handTypeIndex]++] = entireGame.hands[gameIndex];
    }

    // for (u64 byHandTypeIndex = 0; byHandTypeIndex < HAND_TYPE_COUNT; byHandTypeIndex++)
    //     printf("Type %s: %d hand(s)\r\n", hand_type_names[byHandTypeIndex], handsByTypeCount[byHandTypeIndex]);

    // 2. SORTING BY RANK
    for (u64 byHandTypeIndex = HAND_TYPE_COUNT - 1; byHandTypeIndex >= 0 && byHandTypeIndex < HAND_TYPE_COUNT; byHandTypeIndex--)
    {
        // printf("Analyzing '%s':\r\n", hand_type_names[byHandTypeIndex]);

        u64 byHandTypeCount = handsByTypeCount[byHandTypeIndex];
        if (byHandTypeCount > 1)
        {
            hand *hands = handsByType[byHandTypeIndex];
            u64 endIndex = byHandTypeCount - 2;

            while (endIndex >= 0 && endIndex < byHandTypeCount - 1)
            {
                for (u64 playhead = 0; playhead <= endIndex; playhead++)
                    if (compare_hands(hands[playhead], hands[playhead+1]) == 1)
                        swap_hands(hands + playhead, hands + playhead + 1);
                endIndex--;
            }
            // printf("Sorted.\r\n");
        }
        // else printf("No need to sort, too few examples.\r\n");

    }

    // 3. CALCULATING WITH RANK
    u64 result = 0;
    u64 rank = 0;
    printf("Woop!\r\n");
    for (u64 byHandTypeIndex = HAND_TYPE_COUNT - 1; byHandTypeIndex >= 0 && byHandTypeIndex < HAND_TYPE_COUNT; byHandTypeIndex--)
    {
        hand *hands = handsByType[byHandTypeIndex];
        
        for (u64 byHandIndex = 0; byHandIndex < handsByTypeCount[byHandTypeIndex]; byHandIndex++)
        {
            rank++;
            printf("Rank: %lld, ", rank);
            print_hand(hands[byHandIndex]);
            result += rank * hands[byHandIndex].bid;
        }
    }

    return result;
}

u64
part_2(game entireGame)
{
    return 0;
}

u32
main(s32 argumentCount, char *arguments[])
{
    initialize_dictionary();

    game entireGame = parse_hands();

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = part_1(entireGame);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    u64 resultPart2 = part_2(entireGame);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    // 250798297 too low
    // 251106089
    debug_log("- Day 07 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}