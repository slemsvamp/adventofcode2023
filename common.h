#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>

#define internal static
#define local_persist static
#define global_variable static

#if NO_ASSERTS
#define assert(expression)
#else
#define assert(expression) \
	if (!(expression))     \
	{                      \
		*(int *)0 = 0;     \
	}
#endif

#ifndef SILENT
    #define log(...) printf(__VA_ARGS__)
#else
    #define log(...)
#endif
#define debug_log(...) fprintf(stdout, __VA_ARGS__)
#define error_log(...) fprintf(stdout, __VA_ARGS__)

#define invalid_code_path assert(!"invalid_code_path")
#define not_implemented_yet assert(!"not_implemented_yet")
#define invalid_default_case \
	default:               \
	{                      \
		invalid_code_path; \
	}                      \
	break

#define array_count(array) (sizeof(array) / sizeof((array)[0]))

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8 b8;
typedef int8 s8;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;

typedef bool32 b32;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef float real32;
typedef double real64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;

typedef size_t memory_index;

#define true 1
#define false 0

typedef struct file_data
{
    size_t size;
    char *data;
} file_data;

file_data
read_to_end_of_file(char *filename)
{
    file_data result = {0};

    FILE *file = fopen(filename, "rb");

    if (file)
    {
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result.data = (char *)calloc(result.size + 1, 1);
        fread(result.data, result.size, 1, file);

        fclose(file);
    }
    else
    {
        debug_log("Could not open file %s\n", filename);
    }

    return result;
}

internal void
swap_s32(s32 *one, s32 *other)
{
    s32 temp = *one;
    *one = *other;
    *other = temp;
}

internal void
swap_u32(u32 *one, u32 *other)
{
    u32 temp = *one;
    *one = *other;
    *other = temp;
}

internal void
swap_u64(u64 *one, u64 *other)
{
    u64 temp = *one;
    *one = *other;
    *other = temp;
}

inline b8
contains_s32(s32 *values, u32 count, s32 value)
{
    for (u32 index = 0; index < count; index++)
        if (*(values + index) == value)
            return true;
    return false;
}

inline b8
contains_u32(u32 *values, u32 count, u32 value)
{
    for (u32 index = 0; index < count; index++)
        if (*(values + index) == value)
            return true;
    return false;
}

inline b8
contains_u64(u64 *values, u32 count, u64 value)
{
    for (u32 index = 0; index < count; index++)
        if (*(values + index) == value)
            return true;
    return false;
}

internal s32
qs_partition_s32(s32 *values, s32 low, s32 high) 
{ 
    u32 pivot = *(values + high);
    s32 lowerIndex = (low - 1);

    for (u32 index = low; index <= high - 1; index++)
    {
        if (*(values + index) < pivot)
        {
            lowerIndex++;
            swap_s32(values + lowerIndex, values + index);
        }
    }
    swap_s32(values + lowerIndex + 1, values + high);
    return lowerIndex + 1;
}

internal u32
qs_partition_u32(u32 *values, s32 low, s32 high) 
{ 
    u32 pivot = *(values + high);
    s32 lowerIndex = (low - 1);

    for (u32 index = low; index <= high - 1; index++)
    {
        if (*(values + index) < pivot)
        {
            lowerIndex++;
            swap_u32(values + lowerIndex, values + index);
        }
    }
    swap_u32(values + lowerIndex + 1, values + high);
    return lowerIndex + 1;
}

internal u64
qs_partition_u64(u64 *values, s32 low, s32 high) 
{ 
    u64 pivot = *(values + high);
    s32 lowerIndex = (low - 1);

    for (u32 index = low; index <= high - 1; index++)
    {
        if (*(values + index) < pivot)
        {
            lowerIndex++;
            swap_u64(values + lowerIndex, values + index);
        }
    }
    swap_u64(values + lowerIndex + 1, values + high);
    return lowerIndex + 1;
}

internal void
qs_sort_s32(s32 *values, s32 low, s32 high)
{
    if (low < high)
    { 
        u32 partitionIndex = qs_partition_s32(values, low, high); 
        qs_sort_s32(values, low, partitionIndex - 1); 
        qs_sort_s32(values, partitionIndex + 1, high); 
    } 
}

internal void
qs_sort_u32(u32 *values, s32 low, s32 high)
{
    if (low < high)
    { 
        u32 partitionIndex = qs_partition_u32(values, low, high); 
        qs_sort_u32(values, low, partitionIndex - 1); 
        qs_sort_u32(values, partitionIndex + 1, high); 
    } 
}

internal void
qs_sort_u64(u64 *values, s32 low, s32 high)
{
    if (low < high)
    { 
        u32 partitionIndex = qs_partition_u64(values, low, high); 
        qs_sort_u64(values, low, partitionIndex - 1); 
        qs_sort_u64(values, partitionIndex + 1, high); 
    } 
}

typedef struct dictionary_node
{
    char *key;
    void *data;
    struct dictionary_node *next;
} dictionary_node;

typedef struct dictionary
{
    u32 slots;
    dictionary_node **nodes;
    u32 adds;
} dictionary;

internal void
dict_init(dictionary *dict, u32 slots)
{
    dict->slots = slots;
    dict->nodes = (dictionary_node**)calloc(slots, sizeof(dictionary_node*));
}

internal u32
dict_hash_n(char *key, size_t length)
{
    // djb2
    u32 hash = 5381;
    u32 c;
    size_t n = 0;

    while ((c = *key++) && n < length)
    {
        hash = ((hash << 5) + hash) + c;
        n++;
    }

    return hash;
}


internal u32
dict_hash(char *key)
{
    // djb2
    u32 hash = 5381;
    u32 c;

    while (c = *key++)
        hash = ((hash << 5) + hash) + c;

    return hash;
}

internal void
dict_add(dictionary *dict, char *key, void *data)
{
    u32 nodeIndex = dict_hash(key) % dict->slots;

    dictionary_node *node = *(dict->nodes + nodeIndex);
    dictionary_node *newNode = (dictionary_node*)malloc(sizeof(dictionary_node));
    newNode->key = key;
    newNode->data = data;
    newNode->next = 0;

    if (node == NULL)
        *(dict->nodes + nodeIndex) = newNode;
    else
    {
        while (node->next)
            node = node->next;
        node->next = newNode;
    }

    dict->adds++;
}

internal dictionary_node *
dict_get(dictionary *dict, char *key)
{
    u32 nodeIndex = dict_hash(key) % dict->slots;
    dictionary_node *node = *(dict->nodes + nodeIndex);

    if (node == NULL)
        return 0;

    while (node)
    {
        if (strcmp(key, node->key) == 0)
            return node;
        node = node->next;
    }
    return 0;
}

internal b8
dict_contains_key(dictionary *dict, char *key)
{
    return dict_get(dict, key) != NULL;
}
#endif