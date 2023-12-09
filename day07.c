#define SILENT

#include "common.h"
#include "lexer.h"

typedef enum parse_hands_mode
{
    PARSE_HANDS_MODE_j_is_for_knight,
    PARSE_HANDS_MODE_j_is_for_joker
} parse_hands_mode;

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

dictionary _dict;

void
initialize_dictionary(parse_hands_mode mode)
{
    if (_dict.nodes) free(_dict.nodes);
    
    dict_init(&_dict, 20);
    
    if (mode == PARSE_HANDS_MODE_j_is_for_joker)
        dict_add(&_dict, "J", 1);
    
    u32 value = 2;
        
    dict_add(&_dict, "2", value++);
    dict_add(&_dict, "3", value++);
    dict_add(&_dict, "4", value++);
    dict_add(&_dict, "5", value++);
    dict_add(&_dict, "6", value++);
    dict_add(&_dict, "7", value++);
    dict_add(&_dict, "8", value++);
    dict_add(&_dict, "9", value++);
    dict_add(&_dict, "T", value++);
    
    if (mode == PARSE_HANDS_MODE_j_is_for_knight)
        dict_add(&_dict, "J", value++);
    else
        value++;
    
    dict_add(&_dict, "Q", value++);
    dict_add(&_dict, "K", value++);
    dict_add(&_dict, "A", value++);
}

u32
get_card_value(char card)
{
    return (u32)(dict_get(&_dict, (char[]) { card, 0 })->data);
}

hand
parse_hand(char *line, parse_hands_mode mode)
{
    hand result =
    {
        .cards = {0},
        .bid = 0,
        .type = HAND_TYPE_high_card
    };

    u32 sortedHand[HAND_SIZE] = {0};

    for (u32 cardIndex = 0; cardIndex < HAND_SIZE; cardIndex++)
        sortedHand[cardIndex] = get_card_value(line[cardIndex]);
    qs_sort_u32(sortedHand, 0, 4);

    u64 copyCount = 0;
    u64 numberOfCopies[HAND_SIZE] = {0};
    u64 lookupIndex = 0;

    while (lookupIndex < HAND_SIZE)
    {
        u32 lookAhead = 1;
        u32 lookFor = sortedHand[lookupIndex];
        u32 amount = 1;
        
        if (lookFor == 1 /* VALUE OF JOKER */)
        {
            lookupIndex++;
            continue;
        }

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

    if (mode == PARSE_HANDS_MODE_j_is_for_joker)
    {
        u64 numberOfJokers = 0;
        for (u64 jokerSearchIndex = 0; jokerSearchIndex < HAND_SIZE; jokerSearchIndex++)
            if (sortedHand[jokerSearchIndex] == 1)
                numberOfJokers++;

        if (numberOfJokers > 0)
        {
            // UPGRADE
            switch (result.type)
            {
                case HAND_TYPE_high_card:
                    // high card + 1 Joker = one pair
                    // high card + 2 Joker = three of a kind
                    // high card + 3 Joker = four of a kind
                    // high card + 4 Joker = five of a kind
                    // high card + 5 Joker = five of a kind
                    switch (numberOfJokers)
                    {
                        case 1: result.type = HAND_TYPE_one_pair; break;
                        case 2: result.type = HAND_TYPE_three_of_a_kind; break;
                        case 3: result.type = HAND_TYPE_four_of_a_kind; break;
                        case 4: result.type = HAND_TYPE_five_of_a_kind; break;
                        case 5: result.type = HAND_TYPE_five_of_a_kind; break;
                    }
                break;
                case HAND_TYPE_one_pair:
                    // one pair + 1 Joker = three of a kind
                    // one pair + 2 Joker = four of a kind
                    // one pair + 3 Joker = five of a kind
                    switch (numberOfJokers)
                    {
                        case 1: result.type = HAND_TYPE_three_of_a_kind; break;
                        case 2: result.type = HAND_TYPE_four_of_a_kind; break;
                        case 3: result.type = HAND_TYPE_five_of_a_kind; break;
                    }
                break;
                case HAND_TYPE_two_pair:
                    // two pair + 1 Joker = full house
                    result.type = HAND_TYPE_full_house;
                break;
                case HAND_TYPE_three_of_a_kind:
                    // three of a kind + 1 Joker = four of a kind
                    // three of a kind + 2 Joker = five of a kind
                    switch (numberOfJokers)
                    {
                        case 1: result.type = HAND_TYPE_four_of_a_kind; break;
                        case 2: result.type = HAND_TYPE_five_of_a_kind; break;
                    }
                break;
                case HAND_TYPE_four_of_a_kind:
                    // four of a kind + 1 Joker = five of a kind
                    result.type = HAND_TYPE_five_of_a_kind;
                break;
            }
        }
    }

    memcpy(result.cards, line, HAND_SIZE * sizeof(char));

    result.bid = atoll(line + 6);

    return result;
}

#ifndef SILENT
void
print_hand(hand h)
{
    log("Hand: ");
    for (u32 i = 0; i < 5; i++)
        log("%c", h.cards[i]);
    log(", %lld -> %s\r\n", h.bid, hand_type_names[h.type]);
}
#endif

game
parse_hands(parse_hands_mode mode)
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
        result.hands[result.count++] = parse_hand(buffer, mode);
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
    u64 result = 0;
    for (u64 cardIndex = 0; cardIndex < HAND_SIZE; cardIndex++)
    {
        u32 firstHandCardValue = get_card_value(first.cards[cardIndex]);
        u32 secondHandCardValue =  get_card_value(second.cards[cardIndex]);
        if (firstHandCardValue < secondHandCardValue)
            return -1;
        else if (secondHandCardValue < firstHandCardValue)
            return 1;
    }
    return result;
}

u64
calculate_game(game entireGame)
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

    // 2. SORTING BY RANK
    for (u64 byHandTypeIndex = HAND_TYPE_COUNT - 1; byHandTypeIndex >= 0 && byHandTypeIndex < HAND_TYPE_COUNT; byHandTypeIndex--)
    {
        // log("Analyzing '%s':\r\n", hand_type_names[byHandTypeIndex]);

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
            log("Sorted.\r\n");
        }
#ifndef SILENT
        else log("No need to sort, too few examples.\r\n");
#endif

    }

    // 3. CALCULATING WITH RANK
    u64 result = 0;
    u64 rank = 0;
    for (u64 byHandTypeIndex = HAND_TYPE_COUNT - 1; byHandTypeIndex >= 0 && byHandTypeIndex < HAND_TYPE_COUNT; byHandTypeIndex--)
    {
        hand *hands = handsByType[byHandTypeIndex];
        
        for (u64 byHandIndex = 0; byHandIndex < handsByTypeCount[byHandTypeIndex]; byHandIndex++)
        {
            rank++;
            log("Rank: %lld, ", rank);
#ifndef SILENT
            print_hand(hands[byHandIndex]);
#endif
            result += rank * hands[byHandIndex].bid;
        }
    }

    return result;
}

u32
main(s32 argumentCount, char *arguments[])
{
    initialize_dictionary(PARSE_HANDS_MODE_j_is_for_knight);

    game entireGameKnightMode = parse_hands(PARSE_HANDS_MODE_j_is_for_knight);

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = calculate_game(entireGameKnightMode);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    initialize_dictionary(PARSE_HANDS_MODE_j_is_for_joker);

    game entireGameJokerMode = parse_hands(PARSE_HANDS_MODE_j_is_for_joker);

    u64 resultPart2 = calculate_game(entireGameJokerMode);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 07 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}