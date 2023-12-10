#include "common.h"
#include "lexer.h"

// TODO: we can refactor visited into a flag, connections can become flags, we keep the connection flags but rename the enum to FLAG_CONNECTION_north etc and then att FLAG_VISITED
typedef struct field
{
    char *data;
    char *connections;
    char *visited;
    u64 rows;
    u64 columns;
    u64 startX;
    u64 startY;
} field;

typedef enum connection
{
    CONNECTION_north = 0x01,
    CONNECTION_east = 0x02,
    CONNECTION_south = 0x04,
    CONNECTION_west = 0x08,
    CONNECTION_COUNT = 4
} connection;

char *_connection_names[] =
{
    "north", "east", "south", "west"
};

field
read_field()
{
    #define MAX_FIELD_CAPACITY 32768
    file_data file = read_to_end_of_file("input\\day10-input.txt");
    assert(file.size > 0 && file.size < MAX_FIELD_CAPACITY);

    char dataBuffer[MAX_FIELD_CAPACITY];
    char connectionsBuffer[MAX_FIELD_CAPACITY];
    
    u64 rows = 0;
    u64 columns = 0;
    u64 writeIndex = 0;
    u64 startX = 0;
    u64 startY = 0;

    for (u64 index = 0; index < file.size; index++)
    {
        if (file.data[index] == '\r')
        {
            if (rows == 0)
                columns = index;
            continue;
        }
        else if (file.data[index] == '\n')
        {
            rows++;
            continue;
        }

        // | is a vertical pipe connecting north and south.
        // - is a horizontal pipe connecting east and west.
        // L is a 90-degree bend connecting north and east.
        // J is a 90-degree bend connecting north and west.
        // 7 is a 90-degree bend connecting south and west.
        // F is a 90-degree bend connecting south and east.
        // . is ground; there is no pipe in this tile.
        // S is the starting position of the animal; there is a pipe on this tile, but your sketch doesn't show what shape the pipe has.

        switch (file.data[index])
        {
            case '|':
                connectionsBuffer[writeIndex] = CONNECTION_north | CONNECTION_south;
            break;
            case '-':
                connectionsBuffer[writeIndex] = CONNECTION_east | CONNECTION_west;
            break;
            case 'L':
                connectionsBuffer[writeIndex] = CONNECTION_north | CONNECTION_east;
            break;
            case 'J':
                connectionsBuffer[writeIndex] = CONNECTION_north | CONNECTION_west;
            break;
            case '7':
                connectionsBuffer[writeIndex] = CONNECTION_south | CONNECTION_west;
            break;
            case 'F':
                connectionsBuffer[writeIndex] = CONNECTION_south | CONNECTION_east;
            break;
            case '.':
                connectionsBuffer[writeIndex] = 0;
            break;
            case 'S':
                startX = writeIndex % columns;
                startY = rows;
            break;
        }
        dataBuffer[writeIndex] = file.data[index];
        writeIndex++;
    }
    rows++;

    u64 northIndex = (startY - 1) * columns + startX;
    u64 eastIndex = startY * columns + startX + 1;
    u64 southIndex = (startY + 1) * columns + startX;
    u64 westIndex = startY * columns + startX - 1;

    char connectionsStart = 0;

    if (connectionsBuffer[northIndex] & CONNECTION_south) connectionsStart |= CONNECTION_north;
    if (connectionsBuffer[southIndex] & CONNECTION_north) connectionsStart |= CONNECTION_south;
    if (connectionsBuffer[westIndex] & CONNECTION_east) connectionsStart |= CONNECTION_west;
    if (connectionsBuffer[eastIndex] & CONNECTION_west) connectionsStart |= CONNECTION_east;
    
    connectionsBuffer[startY * columns + startX] = connectionsStart;

    field result =
    {
        .data = calloc(rows * columns, sizeof(char)),
        .connections = calloc(rows * columns, sizeof(char)),
        .visited = calloc(rows * columns, sizeof(char)),
        .rows = rows,
        .columns = columns,
        .startX = startX,
        .startY = startY
    };
   
    memcpy(result.data, dataBuffer, rows * columns * sizeof(char));
    memcpy(result.connections, connectionsBuffer, rows * columns * sizeof(char));

    return result;
}

void
print_field(field fld)
{
    log("------------------------------------------------------------------------------------------------------------\r\n");
    for (u32 r = 0; r < fld.rows; r++)
    {
        for (u32 c = 0; c < fld.columns; c++)
            log("%c", fld.data[r * fld.columns + c]);
        log("\r\n");
    }
    log("------------------------------------------------------------------------------------------------------------\r\n");
    log("S: X%lld/Y%lld\r\n", fld.startX, fld.startY);
    log("CONNECTIONS: %d\r\n", fld.connections[fld.startY * fld.columns + fld.startX]);
}

void
print_field_visited(field fld)
{
    log("------------------------------------------------------------------------------------------------------------\r\n");
    for (u32 r = 0; r < fld.rows; r++)
    {
        for (u32 c = 0; c < fld.columns; c++)
            if (fld.visited[r * fld.columns + c])
                log("X");
            else
                log(".");
        log("\r\n");
    }
    log("------------------------------------------------------------------------------------------------------------\r\n");
    log("S: X%lld/Y%lld\r\n", fld.startX, fld.startY);
    log("CONNECTIONS: %d\r\n", fld.connections[fld.startY * fld.columns + fld.startX]);
}

u64
traverse_field(field fld)
{
    connection connections[CONNECTION_COUNT] = { CONNECTION_north, CONNECTION_east, CONNECTION_south, CONNECTION_west };
    s32 directionsX[CONNECTION_COUNT] = { 0, 1, 0, -1 };
    s32 directionsY[CONNECTION_COUNT] = { -1, 0, 1, 0 };

    s64 x = (s64)fld.startX;
    s64 y = (s64)fld.startY;
    b32 firstStep = true;
    u64 steps = 0;

    while (firstStep || !(x == fld.startX && y == fld.startY))
    {
        firstStep = false;
        fld.visited[y * fld.columns + x] = 1;

        for (u64 connectionIndex = 0; connectionIndex < CONNECTION_COUNT; connectionIndex++)
        {
            connection currentConnection = connections[connectionIndex];
            
            if ((fld.connections[y * fld.columns + x] & currentConnection) == 0)
                continue;

            s32 newX = x + directionsX[connectionIndex];
            s32 newY = y + directionsY[connectionIndex];

            if (fld.startX == newX && fld.startY == newY)
            {
                x = newX;
                y = newY;
                break;
            }

            if (fld.visited[newY * fld.columns + newX])
                continue;
            
            x = newX;
            y = newY;
            break;
        }
        
        steps++;
    }
    return steps;
}

u64
part_1(field fld)
{
    u64 steps = traverse_field(fld);
    return steps / 2;
}

u64
part_2(field fld)
{
    // field is already traversed, look for fills
    print_field_visited(fld);
    return 0;
}

u32
main(s32 argumentCount, char *arguments[])
{
    field fld = read_field();

    clock_t startTime = clock();
    u64 startCycles = __rdtsc();

    u64 resultPart1 = part_1(fld);

    clock_t part1Time = clock();
    u64 part1Cycles = __rdtsc();

    u64 resultPart2 = part_2(fld);

    clock_t endTime = clock();
    u64 endCycles = __rdtsc();

    // 766 too high

    debug_log("- Day 10 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}