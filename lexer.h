/*********************************************************************
 * Author:     Hampus Berggren
 * Caveat:     This lexer is vulnerable to reading out of bounds.
 *             Do NOT use this part of the code for your stuff
 *             unless you understand this vulnerability. I only
 *             use this for unimportant stuff.
 *********************************************************************/

#ifndef LEXER_H

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#ifndef u8
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed char         s8;
typedef short               s16;
typedef int                 s32;
typedef long long           s64;
typedef float               f32;
typedef double              f64;
typedef int                 b32;
#endif

#define LEXER_SKIP_whitespace   0x1
#define LEXER_SKIP_comments     0x2

#define LEXER_NEWLINE_CRLF      0x1
#define LEXER_NEWLINE_CR        0x2
#define LEXER_NEWLINE_LF        0x3

#define true 1
#define false 0

typedef enum lexer_token_type
{
    LEXER_TOKEN_TYPE_unknown,
    LEXER_TOKEN_TYPE_identifier,
    LEXER_TOKEN_TYPE_open_parenthesis,
    LEXER_TOKEN_TYPE_close_parenthesis,
    LEXER_TOKEN_TYPE_open_brace,
    LEXER_TOKEN_TYPE_close_brace,
    LEXER_TOKEN_TYPE_open_bracket,
    LEXER_TOKEN_TYPE_close_bracket,
    LEXER_TOKEN_TYPE_less_than,
    LEXER_TOKEN_TYPE_greater_than,
    LEXER_TOKEN_TYPE_period,
    LEXER_TOKEN_TYPE_dollar,
    LEXER_TOKEN_TYPE_comma,
    LEXER_TOKEN_TYPE_colon,
    LEXER_TOKEN_TYPE_semi_colon,
    LEXER_TOKEN_TYPE_forward_slash,
    LEXER_TOKEN_TYPE_equals,
    LEXER_TOKEN_TYPE_asterisk,
    LEXER_TOKEN_TYPE_plus,
    LEXER_TOKEN_TYPE_dash,
    LEXER_TOKEN_TYPE_string,
    LEXER_TOKEN_TYPE_newline,
    LEXER_TOKEN_TYPE_number,
    LEXER_TOKEN_TYPE_special_sign,
    LEXER_TOKEN_TYPE_end
} lexer_token_type;

char *__lexer_token_type_names[] = {
    "LEXER_TOKEN_TYPE_unknown",
    "LEXER_TOKEN_TYPE_identifier",
    "LEXER_TOKEN_TYPE_open_parenthesis",
    "LEXER_TOKEN_TYPE_close_parenthesis",
    "LEXER_TOKEN_TYPE_open_brace",
    "LEXER_TOKEN_TYPE_close_brace",
    "LEXER_TOKEN_TYPE_open_bracket",
    "LEXER_TOKEN_TYPE_close_bracket",
    "LEXER_TOKEN_TYPE_less_than",
    "LEXER_TOKEN_TYPE_greater_than",
    "LEXER_TOKEN_TYPE_period",
    "LEXER_TOKEN_TYPE_dollar",
    "LEXER_TOKEN_TYPE_comma",
    "LEXER_TOKEN_TYPE_colon",
    "LEXER_TOKEN_TYPE_semi_colon",
    "LEXER_TOKEN_TYPE_forward_slash",
    "LEXER_TOKEN_TYPE_equals",
    "LEXER_TOKEN_TYPE_asterisk",
    "LEXER_TOKEN_TYPE_plus",
    "LEXER_TOKEN_TYPE_dash",
    "LEXER_TOKEN_TYPE_string",
    "LEXER_TOKEN_TYPE_newline",
    "LEXER_TOKEN_TYPE_number",
    "LEXER_TOKEN_TYPE_special_sign",
    "LEXER_TOKEN_TYPE_end"
};

typedef struct lexer_token
{
    lexer_token_type type;
    char *text;
    size_t length;
    u32 row;
    u32 col;
} lexer_token;

typedef struct lexer_tokenizer
{
    char *at;
    char *bol;
    u32 row;
} lexer_tokenizer;

u8 __lexer_newline = LEXER_NEWLINE_CRLF;

lexer_tokenizer
lexer_tokenizer_create(char *at)
{
    return (lexer_tokenizer)
    {
        .at = at,
        .bol = at,
        .row = 0
    };
}

inline b32
lexer_token_is_end(lexer_token token)
{
    return token.type == LEXER_TOKEN_TYPE_end;
}

char *
lexer_token_allocate_string(lexer_token token)
{
    char *result = (char *)malloc(token.length + 1);
    memcpy(result, token.text, token.length);
    result[token.length] = 0;
    return result;
}

void
lexer_print_token(lexer_token token)
{
    if (token.type == LEXER_TOKEN_TYPE_newline)
        printf("{Token Type=\"%s\"}\r\n", __lexer_token_type_names[token.type]);
    else
        printf("{Token Type=\"%s\" Value=\"%.*s\"}\r\n", __lexer_token_type_names[token.type], token.length, token.text);
}

#if 0
#define isalpha(c) (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define isdigit(c) ((c >= '0') && (c <= '9'))
#else
#include <ctype.h>
#define isalpha(c) isalpha(c)
#define isdigit(c) isdigit(c)
#endif

inline b32
__lexer_char_is_alpha(char c)
{
    return isalpha(c);
}

inline b32
__lexer_char_is_number(char c)
{
    return isdigit(c);
}

inline b32
__lexer_char_is_whitespace(char c)
{
    return (c == ' ') || (c == '\t');
}

inline b32
__lexer_char_is_newline(char c)
{
    return c == '\n';
}

#if 1
void
__lexer_skip(lexer_tokenizer *t, u32 flags)
{   
    for (;;)
    {
        b32 acted = false;
        if (flags & LEXER_SKIP_whitespace)
        {
            b32 isWhitespace = __lexer_char_is_whitespace(t->at[0]);
            if (isWhitespace)
            {
                b32 isNewline = __lexer_char_is_newline(t->at[0]);
                t->at++;
                
                if (isNewline)
                {
                    t->bol = t->at;
                    t->row += 1; 
                }
                acted = true;
            }
        }
        
        if (flags & LEXER_SKIP_comments)
        {
            if (t->at[0] && t->at[1] && t->at[0] == '/' && t->at[1] == '/')
            {
                t->at+=2;
                while (t->at[0] && t->at[0] != '\n')
                {
                    t->at++;
                }
                acted = true;
            }
            else if (t->at[0] && t->at[1] && t->at[0] == '/' && t->at[1] == '*')
            {
                t->at+=2;
                while (t->at[0] && t->at[1] && !(t->at[0] == '*' && t->at[1] == '/'))
                    t->at++;
                if (t->at[0] == '*')
                    t->at+=2;

                acted = true;
            }
        }
        
        if (!acted) break;
    }
}
#else
void
__lexer_skip(lexer_tokenizer *t, u32 flags)
{   
    for (;;)
    {
        if ((flags & LEXER_SKIP_whitespace) && __lexer_char_is_whitespace(t->at[0]))
            t->at++;
        else if (flags & LEXER_SKIP_comments && t->at[0] && t->at[1] && t->at[0] == '/' && t->at[1] == '/')
        {
            t->at+=2;
            while (t->at[0] && t->at[0] != '\n')
            {
                t->at++;
            }
        }
        else if (flags & LEXER_SKIP_comments && t->at[0] && t->at[1] && t->at[0] == '/' && t->at[1] == '*')
        {
            t->at+=2;
            while (t->at[0] && t->at[1] && !(t->at[0] == '*' && t->at[1] == '/'))
                t->at++;
            if (t->at[0] == '*')
                t->at+=2;
        }
        else break;
    }
}
#endif

inline lexer_token
__lexer_token_create(lexer_token_type type, char *text, size_t length)
{
    lexer_token result;
    result.type = type;
    result.text = text;
    result.length = length;
    return result;
}

lexer_token
lexer_get_token(lexer_tokenizer *t)
{
    __lexer_skip(t, LEXER_SKIP_whitespace | LEXER_SKIP_comments);

    lexer_token result = __lexer_token_create
    (
    /** @tokentype:  */ LEXER_TOKEN_TYPE_unknown,
    /** @text:       */ t->at,
    /** @textlength: */ 1
    );

    result.row = t->row;
    result.col = (u32)(t->at - t->bol);

    char c = t->at[0];
    t->at++;

    switch (c)
    {
        case '\0': result.type = LEXER_TOKEN_TYPE_end; break;
        case '{': result.type = LEXER_TOKEN_TYPE_open_brace; break;
        case '}': result.type = LEXER_TOKEN_TYPE_close_brace; break;
        case '[': result.type = LEXER_TOKEN_TYPE_open_bracket; break;
        case ']': result.type = LEXER_TOKEN_TYPE_close_bracket; break;
        case '(': result.type = LEXER_TOKEN_TYPE_open_parenthesis; break;
        case ')': result.type = LEXER_TOKEN_TYPE_close_parenthesis; break;
        case '<': result.type = LEXER_TOKEN_TYPE_less_than; break;
        case '>': result.type = LEXER_TOKEN_TYPE_greater_than; break;
        case '$': result.type = LEXER_TOKEN_TYPE_dollar; break;
        case '.': result.type = LEXER_TOKEN_TYPE_period; break;
        case ',': result.type = LEXER_TOKEN_TYPE_comma; break;
        case ':': result.type = LEXER_TOKEN_TYPE_colon; break;
        case ';': result.type = LEXER_TOKEN_TYPE_semi_colon; break;
        case '*': result.type = LEXER_TOKEN_TYPE_asterisk; break;
        case '/': result.type = LEXER_TOKEN_TYPE_forward_slash; break;
        case '+': result.type = LEXER_TOKEN_TYPE_plus; break;
        case '-': result.type = LEXER_TOKEN_TYPE_dash; break;
        case '=': result.type = LEXER_TOKEN_TYPE_equals; break;
        
        case '#':
        case '~':
        case '^':
        case '|':
            result.type = LEXER_TOKEN_TYPE_special_sign;
            break;        
        
        case '\r':
        {
            switch (__lexer_newline)
            {
                case LEXER_NEWLINE_CRLF:
                {
                    if (t->at[0] == '\n')
                    {
                        result.type = LEXER_TOKEN_TYPE_newline;
                        result.length = 2;
                        t->at++;
                    }
                    else return lexer_get_token(t);
                }
                break;
                case LEXER_NEWLINE_CR: result.type = LEXER_TOKEN_TYPE_newline; break;
                default: return lexer_get_token(t);
            }
        }
        break;
        case '\n':
        {
            switch (__lexer_newline)
            {
                case LEXER_NEWLINE_LF: result.type = LEXER_TOKEN_TYPE_newline; break;
                default: return lexer_get_token(t);
            }
        }
        break;
        case '"': 
        {
            result.type = LEXER_TOKEN_TYPE_string;
            result.text = t->at;
            while (t->at[0] && t->at[0] != '"')
            {
                assert(t->at[0] != '\0');
                if (t->at[0] == '\\' && t->at[1])
                {
                    t->at++;
                    assert(t->at[0] != '\0');
                }
                t->at++;
            }
            result.length = t->at - result.text;
            if (t->at[0] == '"')
                t->at++;
        }
        break;
        default:
            if (__lexer_char_is_alpha(c))
            {
                result.type = LEXER_TOKEN_TYPE_identifier;
                while (__lexer_char_is_alpha(t->at[0]) ||
                    __lexer_char_is_number(t->at[0]) ||
                    t->at[0] == '_')
                {
                    assert(t->at[0] != '\0');
                    t->at++;
                }
                result.length = t->at - result.text;
            }
            else if (__lexer_char_is_number(c))
            {
                result.type = LEXER_TOKEN_TYPE_number;

                while (__lexer_char_is_number(t->at[0]))
                {
                    assert(t->at[0] != '\0');
                    t->at++;
                }
                result.length = t->at - result.text;
            }
        break;
    }

    return result;
}

void
lexer_token_error_report(lexer_token token)
{
    u32 margin = 0;
    if (token.col > 16)
    {
        margin = token.col - 16;
    }
    char *findEnd = token.text;
    while (*findEnd != '\0' && *findEnd != '\n')
        findEnd++;
    u32 length = 128;
    if ((u32)(findEnd - token.text) < length)
        length = (u32)(findEnd - token.text);

    printf("------");
    for (u32 marginIndex = 0; marginIndex < margin; marginIndex++)
        printf("-");
    printf("v\n");
    printf("Line: %.*s\n", length, token.text - margin);
}

lexer_token
lexer_require_token(lexer_tokenizer *t, lexer_token_type type)
{
    lexer_token result = lexer_get_token(t);
    if (result.type != type)
    {
        printf("token: %.*s, expectedType: %u, actual: %u\n", (int)result.length, result.text, type, result.type);
        *((u8 *)0) = 0;
    }
    return result;
}

lexer_token
lexer_peek_token(lexer_tokenizer *t)
{
    char *start = t->at;
    lexer_token result = lexer_get_token(t);
    t->at = start;
    return result;
}

b32
lexer_token_of(lexer_token *token, lexer_token_type *types, u32 count)
{
    for (u32 tokenTypeIndex = 0; tokenTypeIndex < count; tokenTypeIndex++)
        if (token->type == types[tokenTypeIndex])
            return true;
    return false;
}

lexer_token
lexer_get_next_of(lexer_tokenizer *t, lexer_token_type *types, u32 count)
{
    lexer_token result = lexer_get_token(t);
    if (lexer_token_of(&result, types, count))
        return result;
    while (!lexer_token_of(&result, types, count) && result.type != LEXER_TOKEN_TYPE_end)
        result = lexer_get_token(t);
    return result;
}

lexer_token
lexer_get_next(lexer_tokenizer *t, lexer_token_type type)
{
    lexer_token result = lexer_get_token(t);
    while (result.type != type && result.type != LEXER_TOKEN_TYPE_end)
        result = lexer_get_token(t);
    return result;
}

#define LEXER_H
#endif