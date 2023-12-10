#define SILENT

#include "common.h"
#include "lexer.h"

// TODO: we can refactor visited into a flag, connections can become flags, we keep the connection flags but rename the enum to FLAG_CONNECTION_north etc and then att FLAG_VISITED
// same with enclosed
typedef struct field
{
    char *data;
    char *connections;
    char *visited;
    char *enclosed;
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

connection _connections[CONNECTION_COUNT] = { CONNECTION_north, CONNECTION_east, CONNECTION_south, CONNECTION_west };
connection _oppositeConnections[CONNECTION_COUNT] = { CONNECTION_south, CONNECTION_west, CONNECTION_north, CONNECTION_east };
s32 _directionsX[CONNECTION_COUNT] = { 0, 1, 0, -1 };
s32 _directionsY[CONNECTION_COUNT] = { -1, 0, 1, 0 };

// TODO: this is looking to the LEFT in the direction, switch places with -1 and 1 in these to look at RIGHT instead, need to solve this
s32 _directionsEnclosedX[CONNECTION_COUNT] = { -1, 0, 1, 0 };
s32 _directionsEnclosedY[CONNECTION_COUNT] = { 0, -1, 0, 1 };

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
                if (columns > 0)
                    startX = writeIndex % columns;
                else
                    startX = writeIndex;
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
        .enclosed = calloc(rows * columns, sizeof(char)),
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
#ifndef SILENT
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
#endif
}

void
print_field_visited(field fld)
{
#ifndef SILENT
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
#endif
}

void
print_field_enclosed(field fld)
{
#ifndef SILENT
    log("------------------------------------------------------------------------------------------------------------\r\n");
    for (u32 r = 0; r < fld.rows; r++)
    {
        for (u32 c = 0; c < fld.columns; c++)
            if (fld.enclosed[r * fld.columns + c])
                log("X");
            else
                log(".");
        log("\r\n");
    }
    log("------------------------------------------------------------------------------------------------------------\r\n");
    log("S: X%lld/Y%lld\r\n", fld.startX, fld.startY);
    log("CONNECTIONS: %d\r\n", fld.connections[fld.startY * fld.columns + fld.startX]);
#endif
}

b32
is_within_field(u64 columns, u64 rows, s32 x, s32 y)
{
    return x >= 0 && y >= 0 && x < columns && y < rows;
}

u64
traverse_field(field fld)
{
    s64 x = (s64)fld.startX;
    s64 y = (s64)fld.startY;
    b32 firstStep = true;
    u64 steps = 0;
    u64 previousConnection = 0;

    while (firstStep || !(x == fld.startX && y == fld.startY))
    {
        firstStep = false;
        fld.visited[y * fld.columns + x] = 1;

        for (u64 connectionIndex = 0; connectionIndex < CONNECTION_COUNT; connectionIndex++)
        {
            connection currentConnection = _connections[connectionIndex];
            
            if ((fld.connections[y * fld.columns + x] & currentConnection) == 0)
                continue;

            s32 newX = x + _directionsX[connectionIndex];
            s32 newY = y + _directionsY[connectionIndex];

            if ((currentConnection == CONNECTION_north && previousConnection == CONNECTION_south)
                || (currentConnection == CONNECTION_east && previousConnection == CONNECTION_west)
                || (currentConnection == CONNECTION_south && previousConnection == CONNECTION_north)
                || (currentConnection == CONNECTION_west && previousConnection == CONNECTION_east))
                continue;

            if (fld.startX == newX && fld.startY == newY)
            {
                x = newX;
                y = newY;
                previousConnection = currentConnection;
                break;
            }

            x = newX;
            y = newY;
            previousConnection = currentConnection;
            break;
        }
        
        steps++;
    }

    return steps;
}

u64
find_enclosed_areas(field fld)
{
    s64 x = (s64)fld.startX;
    s64 y = (s64)fld.startY;
    b32 firstStep = true;
    u64 steps = 0;

    u64 previousConnection = 0;

    while (firstStep || !(x == fld.startX && y == fld.startY))
    {
        firstStep = false;

        for (u64 connectionIndex = 0; connectionIndex < CONNECTION_COUNT; connectionIndex++)
        {
            connection currentConnection = _connections[connectionIndex];
            connection availableConnections = fld.connections[y * fld.columns + x];

            // if we have a connection this way or not, else continue
            if ((availableConnections & currentConnection) == 0)
                continue;

            s32 newX = x + _directionsX[connectionIndex];
            s32 newY = y + _directionsY[connectionIndex];

            // no backtracking
            if ((currentConnection == CONNECTION_north && previousConnection == CONNECTION_south) || (currentConnection == CONNECTION_east && previousConnection == CONNECTION_west)
                || (currentConnection == CONNECTION_south && previousConnection == CONNECTION_north) || (currentConnection == CONNECTION_west && previousConnection == CONNECTION_east))
                continue;

            if (fld.startX == newX && fld.startY == newY)
            {
                x = newX;
                y = newY;
                previousConnection = currentConnection;
                break;
            }

            if (!previousConnection) previousConnection = _oppositeConnections[_connections[connectionIndex]];

            s32 oppositeConnectionValue = previousConnection == 1 ? 4 : previousConnection == 2 ? 8 : previousConnection == 4 ? 1 : previousConnection == 8 ? 2 : 0;
            connection checkConnections = (availableConnections - oppositeConnectionValue) | previousConnection;

            for (u64 availableConnectionIndex = 0; availableConnectionIndex < CONNECTION_COUNT; availableConnectionIndex++)
            {
                // TODO: maybe this is where we could just implement wether we group in/out or at least have set the appropriate left/right strategy
                if ((checkConnections & _connections[availableConnectionIndex]) > 0)
                {
                    s32 enclosedDirectionX = _directionsEnclosedX[availableConnectionIndex];
                    s32 enclosedDirectionY = _directionsEnclosedY[availableConnectionIndex];

                    u64 stepsEnclosedCheck = 1;
                    s32 enclosedX = x + (enclosedDirectionX * stepsEnclosedCheck);
                    s32 enclosedY = y + (enclosedDirectionY * stepsEnclosedCheck);

                    while (is_within_field(fld.columns, fld.rows, enclosedX, enclosedY) && fld.visited[enclosedY * fld.columns + enclosedX] == 0)
                    {
                        fld.enclosed[enclosedY * fld.columns + enclosedX] = 1;

                        enclosedX = x + (enclosedDirectionX * stepsEnclosedCheck);
                        enclosedY = y + (enclosedDirectionY * stepsEnclosedCheck);

                        stepsEnclosedCheck++;
                    }
                }
            }

            x = newX;
            y = newY;
            previousConnection = currentConnection;
            break;
        }

        steps++;
    }

    u64 result = 0;

    for (u64 playhead = 0; playhead < fld.rows * fld.columns; playhead++)
        result += fld.enclosed[playhead];

    return result;
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
    print_field_visited(fld);
    u64 result = find_enclosed_areas(fld);
    print_field_enclosed(fld);

    return result;
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

    debug_log("- Day 10 -\n");
    debug_log("Result Part 1: %lld (%d ms, %lld cycles passed)\n", resultPart1, (part1Time - startTime) * 1000 / CLOCKS_PER_SEC, (part1Cycles - startCycles));
    debug_log("Result Part 2: %lld (%d ms, %lld cycles passed)\n", resultPart2, (endTime - part1Time) * 1000 / CLOCKS_PER_SEC, (endCycles - part1Cycles));
    debug_log("\n");

    return 0;
}