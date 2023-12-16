#define SILENT
#include "common.h"
#include "lexer.h"

typedef struct loc
{
    s64 x;
    s64 y;
} loc;

typedef struct cavern
{
    char *data;
    u64 length;
    u64 width;
    u64 height;
} cavern;

cavern
parse_cavern()
{
    file_data file = read_to_end_of_file("input\\day16-input.txt");
    assert(file.size > 0);

    u64 writeIndex = 0;
    cavern result = {0};

    char *buffer = calloc(file.size, sizeof(char));

    for (u64 readIndex = 0; readIndex < file.size; readIndex++)
    {
        char c = file.data[readIndex];

        if (c == '\r' && result.width == 0)
            result.width = readIndex;
        if (!(c == '\r' || c == '\n'))
            buffer[writeIndex++] = c;
        if (c == '\n')
            result.height++;
    }
    result.height++;

    result.length = result.width * result.height;
    result.data = calloc(result.length, sizeof(char));

    memcpy(result.data, buffer, result.length);

    return result;
}

typedef enum ortho_direction
{
    ORTHO_DIRECTION_up,
    ORTHO_DIRECTION_right,
    ORTHO_DIRECTION_down,
    ORTHO_DIRECTION_left,
    ORTHO_DIRECTION_COUNT
} ortho_direction;

typedef enum energy_flow
{
    ENERGY_FLOW_up = 0x01,
    ENERGY_FLOW_right = 0x02,
    ENERGY_FLOW_down = 0x04,
    ENERGY_FLOW_left = 0x08
} energy_flow;

typedef struct laser_beam
{
    loc position;
    ortho_direction direction;
    struct laser_beam *prev;
    struct laser_beam *next;
    b8 marked;
    loc split[10];
    u64 splits;
} laser_beam;

s64
get_index(cavern cav, laser_beam *laser)
{
    return laser->position.y * cav.width + laser->position.x;
}

b32
laser_within_bounds(cavern cav, laser_beam *laser)
{
    if (laser->position.x < 0 || laser->position.y < 0 || laser->position.x >= cav.width || laser->position.y >= cav.height)
        return false;
    return true;
}

#ifndef SILENT
void
print_energetic_states(cavern cav, b8 *energeticState)
{
    for (u64 index = 0; index < cav.length; index++)
    {
        if (!(index % cav.width))
            log("\n");
        
        if (energeticState[index] > 0)
            log("*");
        else
            log(".");
    }
    log("\n");
}

void
print_laser_chain(cavern cav, laser_beam *laser, laser_beam *mark)
{
    laser_beam *pointer = laser;
    
    if (pointer->prev)
        log("<--");
    while (pointer)
    {
        if (pointer == mark)
            log("(%lld)", pointer->position.y * cav.width + pointer->position.x);
        else
            log("%lld", pointer->position.y * cav.width + pointer->position.x);
        if (pointer->next && pointer->next->prev)
            log("<->");
        else if (pointer->next)
            log("-->");

        pointer = pointer->next;
    }
}
#endif

#define LASER_BEAM_BUFFER_CAPACITY (1<<16)

u64
run(laser_beam *buffer, u64 bufferCount, cavern cav)
{

    memset((char *)buffer + sizeof(laser_beam), 0, (LASER_BEAM_BUFFER_CAPACITY - 1) * sizeof(laser_beam));

    s64 moveX[ORTHO_DIRECTION_COUNT] = { 0, 1, 0, -1 };
    s64 moveY[ORTHO_DIRECTION_COUNT] = { -1, 0, 1, 0 };
    u64 laserBufferIndex = bufferCount;
    u8 *energeticStates = calloc(cav.width * cav.height, sizeof(u8));

    laser_beam *start = buffer;
    u64 eventCount = 0;

    while (start)
    {
        laser_beam *laser = start;

        while (laser)
        {
            eventCount++;

            assert(!laser->marked && "Re-using a dead laser, no bueno!");

            s64 currentLaserIndex = get_index(cav, laser);
            
            energy_flow currentFlow;

            switch (laser->direction)
            {
                case ORTHO_DIRECTION_up: currentFlow = ENERGY_FLOW_up; break;
                case ORTHO_DIRECTION_right: currentFlow = ENERGY_FLOW_right; break;
                case ORTHO_DIRECTION_down: currentFlow = ENERGY_FLOW_down; break;
                case ORTHO_DIRECTION_left: currentFlow = ENERGY_FLOW_left; break;
                invalid_default_case;
            }

            b32 energeticDuplicateFlow = false;
            
            if (eventCount > 0)
            {
                energeticDuplicateFlow = (energeticStates[currentLaserIndex] & currentFlow) > 0;

                if (laser_within_bounds(cav, laser) && !energeticDuplicateFlow)
                    energeticStates[currentLaserIndex] |= currentFlow;
            }

            laser->position.x += moveX[laser->direction];
            laser->position.y += moveY[laser->direction];

            currentLaserIndex = get_index(cav, laser);


            if (!laser_within_bounds(cav, laser) || energeticDuplicateFlow)
            {
                laser->marked = true;

                if (!laser->prev && !laser->next)
                {
                    laser = start = 0;
                    break;
                }
                else if (laser->prev && !laser->next)
                {
                    laser->prev->next = 0;
                }
                else if (!laser->prev && laser->next)
                {
                    start = laser->next;
                    start->prev = 0;
                }
                else if (laser->prev && laser->next)
                {
                    laser->prev->next = laser->next;
                    laser->next->prev = laser->prev;
                }
                else invalid_code_path;

                laser = laser->next;
                continue;
            }

            switch (cav.data[currentLaserIndex])
            {
                case '.': break;
                case '|':
                    if (laser->direction == ORTHO_DIRECTION_left || laser->direction == ORTHO_DIRECTION_right)
                    {
                        assert(laser->splits < 10);
                        laser->split[laser->splits++] = laser->position;
                        laser->direction = ORTHO_DIRECTION_up;

                        assert(laserBufferIndex < LASER_BEAM_BUFFER_CAPACITY && "no more lasers, sir");
                        // create a new laser that goes down
                        buffer[laserBufferIndex] = (laser_beam)
                        {
                            .position = laser->position,
                            .direction = ORTHO_DIRECTION_down,
                            .prev = 0,
                            .next = laser,
                            .marked = false
                        };
                        
                        laser_beam *new = buffer + laserBufferIndex++;
                        assert(new->splits < 10);
                        new->split[new->splits++] = laser->position;

                        if (laser->prev)
                        {
                            laser->prev->next = new;
                            new->prev = laser->prev;
                            laser->prev = new;
                        }
                        else
                        {
                            start = new;
                            laser->prev = start;
                        }

                        // mark the mirror position as energized
                        energeticDuplicateFlow = (energeticStates[currentLaserIndex] & currentFlow) > 0;

                        if (!energeticDuplicateFlow)
                            energeticStates[currentLaserIndex] |= currentFlow;
                    }
                break;
                case '-':
                    if (laser->direction == ORTHO_DIRECTION_up || laser->direction == ORTHO_DIRECTION_down)
                    {
                        assert(laser->splits < 10);
                        laser->split[laser->splits++] = laser->position;
                        laser->direction = ORTHO_DIRECTION_left;

                        assert(laserBufferIndex < LASER_BEAM_BUFFER_CAPACITY && "no more lasers, sir");

                        // create a new laser that goes right
                        buffer[laserBufferIndex] = (laser_beam)
                        {
                            .position = laser->position,
                            .direction = ORTHO_DIRECTION_right,
                            .prev = 0,
                            .next = laser,
                            .marked = false
                        };
                        
                        laser_beam *new = buffer + laserBufferIndex++;
                        assert(new->splits < 10);
                        new->split[new->splits++] = laser->position;

                        if (laser->prev)
                        {
                            laser->prev->next = new;
                            new->prev = laser->prev;
                            laser->prev = new;
                        }
                        else
                        {
                            start = new;
                            laser->prev = start;
                        }

                        // mark the mirror position as energized
                        energeticDuplicateFlow = (energeticStates[currentLaserIndex] & currentFlow) > 0;

                        if (!energeticDuplicateFlow)
                            energeticStates[currentLaserIndex] |= currentFlow;
                    }
                break;
                case '/':
                    switch (laser->direction)
                    {
                        case ORTHO_DIRECTION_up: laser->direction = ORTHO_DIRECTION_right; break;
                        case ORTHO_DIRECTION_right: laser->direction = ORTHO_DIRECTION_up; break;
                        case ORTHO_DIRECTION_down: laser->direction = ORTHO_DIRECTION_left; break;
                        case ORTHO_DIRECTION_left: laser->direction = ORTHO_DIRECTION_down; break;
                    }
                break;
                case '\\':
                    switch (laser->direction)
                    {
                        case ORTHO_DIRECTION_up: laser->direction = ORTHO_DIRECTION_left; break;
                        case ORTHO_DIRECTION_right: laser->direction = ORTHO_DIRECTION_down; break;
                        case ORTHO_DIRECTION_down: laser->direction = ORTHO_DIRECTION_right; break;
                        case ORTHO_DIRECTION_left: laser->direction = ORTHO_DIRECTION_up; break;
                    }
                break;
            }

            laser = laser->next;
        }

        laser = start;
    }

    u64 result = 0;
    for (u64 energyIndex = 0; energyIndex < cav.length; energyIndex++)
        result += energeticStates[energyIndex] > 0;

    free(energeticStates);

    return result;
}

u64
part_1(cavern cav)
{
    laser_beam *buffer = calloc(LASER_BEAM_BUFFER_CAPACITY, sizeof(laser_beam));
    buffer[0] = (laser_beam) { .position = (loc) { .x = -1, .y = 0 }, .direction = ORTHO_DIRECTION_right, .prev = 0, .next = 0 };
    
    u64 result = run(buffer, 1, cav);
    free(buffer);
    return result;
}

u64
part_2(cavern cav)
{
    laser_beam *buffer = calloc(LASER_BEAM_BUFFER_CAPACITY, sizeof(laser_beam));

    laser_beam corners[8] =
    {
        (laser_beam) { .position = (loc) { .x = -1, .y = 0 }, .direction = ORTHO_DIRECTION_right, .prev = 0, .next = 0 },
        (laser_beam) { .position = (loc) { .x = 0, .y = -1 }, .direction = ORTHO_DIRECTION_down, .prev = 0, .next = 0 },
        (laser_beam) { .position = (loc) { .x = (s64)cav.width, .y = 0 }, .direction = ORTHO_DIRECTION_left, .prev = 0, .next = 0 },
        (laser_beam) { .position = (loc) { .x = (s64)cav.width - 1, .y = -1 }, .direction = ORTHO_DIRECTION_down, .prev = 0, .next = 0 },
        (laser_beam) { .position = (loc) { .x = (s64)cav.width, .y = (s64)cav.height - 1 }, .direction = ORTHO_DIRECTION_left, .prev = 0, .next = 0 },
        (laser_beam) { .position = (loc) { .x = (s64)cav.width - 1, .y = (s64)cav.height }, .direction = ORTHO_DIRECTION_up, .prev = 0, .next = 0 },
        (laser_beam) { .position = (loc) { .x = -1, .y = (s64)cav.height - 1 }, .direction = ORTHO_DIRECTION_right, .prev = 0, .next = 0 },
        (laser_beam) { .position = (loc) { .x = 0, .y = (s64)cav.height }, .direction = ORTHO_DIRECTION_up, .prev = 0, .next = 0 }
    };

    u64 result = 0;
    u64 runResult = 0;

    // corner cases
    for (u64 cornerIndex = 0; cornerIndex < 8; cornerIndex++)
    {
        buffer[0] = corners[cornerIndex];
        runResult = run(buffer, 1, cav);
        result = max(runResult, result);
    }

    // edge cases
    for (u64 y = 1; y < cav.height - 1; y++)
    {
        buffer[0] = (laser_beam) { .position = (loc) { .x = -1, .y = y }, .direction = ORTHO_DIRECTION_right, .prev = 0, .next = 0 };
        runResult = run(buffer, 1, cav);
        result = max(runResult, result);

        buffer[0] = (laser_beam) { .position = (loc) { .x = (s64)cav.width, .y = y }, .direction = ORTHO_DIRECTION_left, .prev = 0, .next = 0 };
        runResult = run(buffer, 1, cav);
        result = max(runResult, result);
    }
    for (u64 x = 1; x < cav.width - 1; x++)
    {
        buffer[0] = (laser_beam) { .position = (loc) { .x = x, .y = -1 }, .direction = ORTHO_DIRECTION_down, .prev = 0, .next = 0 };
        runResult = run(buffer, 1, cav);
        result = max(runResult, result);

        buffer[0] = (laser_beam) { .position = (loc) { .x = x, .y = (s64)cav.height }, .direction = ORTHO_DIRECTION_up, .prev = 0, .next = 0 };
        runResult = run(buffer, 1, cav);
        result = max(runResult, result);
    }

    free(buffer);

    return result;
}

u32
main(s32 argumentCount, char *arguments[])
{
    cavern cav = parse_cavern();

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = part_1(cav);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    u64 resultPart2 = part_2(cav);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    debug_log("- Day 16 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}