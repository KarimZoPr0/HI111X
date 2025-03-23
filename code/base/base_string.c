//
// Created by Abdik on 2025-03-18.
//

#include "base_string.h"


B32 CharIsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

B32 CharIsAlphaUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

B32 CharIsAlphaLower(char c)
{
    return c >= 'a' && c <= 'z';
}

B32 CharIsDigit(char c)
{
    return c >= '0' && c <= '9';
}

B32 CharIsSymbol(char c)
{
    return (c == '~' || c == '!' || c == '$' || c == '%' || c == '^' ||
        c == '&' || c == '*' || c == '-' || c == '=' || c == '+' ||
        c == '<' || c == '.' || c == '>' || c == '/' || c == '?' ||
        c == '|' || c == '\\' || c == '{' || c == '}' || c == '(' ||
        c == ')' || c == '\\' || c == '[' || c == ']' || c == '#' ||
        c == ',' || c == ';' || c == ':' || c == '@');
}

B32 CharIsSpace(char c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v' || c == '\n';
}

char CharToUpper(char c)
{
    return (c >= 'a' && c <= 'z') ? (c & ~0x20) : c;
}

char CharToLower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (c | 0x20) : c;
}

char CharToForwardSlash(char c)
{
    return (c == '\\' ? '/' : c);
}

U64 CStringLength(char* cstr)
{
    U64 length = 0;
    for (; cstr[length]; length += 1);
    return length;
}

char* CStringFromString(Arena* arena, String string)
{
    char* cstr = push_array(arena, char, string.size);
    MemoryCopy(cstr, string.str, string.size);
    cstr[string.size] = '\0';
    return cstr;
}

void PrintString(String s)
{
    printf("%.*s\n", (int)s.size, s.str);
}

String Str(char* str, U64 size)
{
    String string;
    string.str = str;
    string.size = size;
    return string;
}

String Substr(String str, U64 min, U64 max)
{
    if (min > str.size) min = str.size;
    if (max > str.size) max = str.size;

    if (min >= max)
    {
        str.str += str.size;
        str.size = 0;
    }
    else
    {
        str.str += min;
        str.size = max - min;
    }
    return str;
}

String StrSkip(String str, U64 min)
{
    return Substr(str, min, str.size);
}

String StrChop(String str, U64 nmax)
{
    U64 cut = (nmax > str.size ? 0 : str.size - nmax);
    return Substr(str, 0, cut);
}

String Prefix(String str, U64 size)
{
    return Substr(str, 0, size);
}

String Suffix(String str, U64 size)
{
    if (size > str.size) size = str.size;
    return Substr(str, str.size - size, str.size);
}

String StrTrim(String str)
{
    if (str.size == 0) return str;

    U64 start = 0;
    U64 end = str.size;

    while (start < end && CharIsSpace(str.str[start])) start++;
    while (end > start && CharIsSpace(str.str[end - 1])) end--;

    str.str = str.str + start;
    str.size = end - start;

    return str;
}

B32 StrMatch(String a, String b, MatchFlags flags)
{
    B32 result = 0;
    if (a.size == b.size || flags & MatchFlag_RightSideSloppy)
    {
        result = 1;
        for (U64 i = 0; i < a.size; i += 1)
        {
            B32 match = (a.str[i] == b.str[i]);
            if (flags & MatchFlag_CaseInsensitive)
            {
                match |= (CharToLower(a.str[i]) == CharToLower(b.str[i]));
            }
            if (flags & MatchFlag_SlashInsensitive)
            {
                match |= (CharToForwardSlash(a.str[i]) == CharToForwardSlash(b.str[i]));
            }
            if (match == 0)
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

// Allocation

String PushStrCopy(Arena* arena, String string)
{
    String res;
    res.size = string.size;
    res.str = push_array_no_zero(arena, char, string.size + 1);
    MemoryCopy(res.str, string.str, string.size);
    res.str[string.size] = '\0';
    return res;
}

String PushStrFV(Arena* arena, char* fmt, va_list args)
{
    String result = {0};
    va_list args_copy;
    va_copy(args_copy, args);
    int needed_chars = vsnprintf(NULL, 0, fmt, args);
    if (needed_chars < 0)
    {
        va_end(args_copy);
        return result;
    }
    result.str = push_array_no_zero(arena, char, needed_chars + 1);
    vsnprintf(result.str, needed_chars + 1, fmt, args_copy);
    result.size = (size_t)needed_chars;
    va_end(args_copy);
    return result;
}

String PushStrF(Arena* arena, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String result = PushStrFV(arena, fmt, args);
    va_end(args);
    return result;
}

String PushStrFillByte(Arena* arena, U64 size, char byte)
{
    String result = {0};
    result.str = push_array_no_zero(arena, char, size);
    MemorySet(result.str, byte, size);
    result.size = size;
    return result;
}


void StrListPushNode(StringList* list, StringNode* n)
{
    SLLQueuePush(list->first, list->last, n);
    list->node_count += 1;
    list->total_size += n->string.size;
}

void StrListPushNodeFront(StringList* list, StringNode* n)
{
    SLLQueuePushFront(list->first, list->last, n);
    list->node_count += 1;
    list->total_size += n->string.size;
}

void StrListPush(Arena* arena, StringList* list, String str)
{
    StringNode* n = push_array(arena, StringNode, str.size);
    n->string = str;
    StrListPushNode(list, n);
}

void StrListPushF(Arena* arena, StringList* list, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String string = PushStrFV(arena, fmt, args);
    va_end(args);
    StrListPush(arena, list, string);
}

void StrListPushFront(Arena* arena, StringList* list, String str)
{
    StringNode* n = push_array(arena, StringNode, str.size);
    n->string = str;
    StrListPushNodeFront(list, n);
}

void StrListConcatInPlace(StringList* list, StringList* to_push)
{
    if (to_push->first)
    {
        list->node_count += to_push->node_count;
        list->total_size += to_push->total_size;
        if (list->last == 0)
        {
            *list = *to_push;
        }
        else
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
        }
    }
    MemoryZero(to_push, sizeof(*to_push));
}

StringList StrSplit(Arena* arena, String string, U64 split_count, String* splits)
{
    StringList list = {0};
    U64 split_start = 0;
    U64 i = 0;
    while (i < string.size)
    {
        B32 found = 0;
        for (U64 j = 0; j < split_count; j++)
        {
            if (i + splits[j].size <= string.size)
            {
                B32 match = 1;
                for (U64 k = 0; k < splits[j].size; k++)
                {
                    if (string.str[i + k] != splits[j].str[k])
                    {
                        match = 0;
                        break;
                    }
                }
                if (match)
                {
                    String token = Str(string.str + split_start, i - split_start);
                    StrListPush(arena, &list, token);
                    i += splits[j].size;
                    split_start = i;
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
        {
            i++;
        }
    }
    if (split_start < string.size)
    {
        String token = Str(string.str + split_start, string.size - split_start);
        StrListPush(arena, &list, token);
    }
    return list;
}


String StrListJoin(Arena* arena, StringList list, StringJoin* optional_params)
{
    StringJoin join = {0};
    if (optional_params)
    {
        MemoryCopyStruct(&join, optional_params);
    }

    // calculate size & allocate
    U64 sep_count = Max(0, list.node_count - 1);
    String result = {0};
    result.size = list.total_size + join.pre.size + (join.sep.size * sep_count) + join.post.size;
    result.str = push_array_no_zero(arena, char, result.size + 1);

    // fill
    U64 offset = 0;
    MemoryCopy(result.str, join.pre.str, join.pre.size);
    offset += join.pre.size;
    for (StringNode* n = list.first; n; n = n->next)
    {
        MemoryCopy(result.str + offset, n->string.str, n->string.size);
        offset += n->string.size;
        if (n->next)
        {
            MemoryCopy(result.str + offset, join.sep.str, join.sep.size);
            offset += join.sep.size;
        }
    }
    MemoryCopy(result.str + offset, join.post.str, join.post.size);
    offset += join.post.size;

    // add null
    result.str[offset] = '\0';
    return result;
}

String UpperFromString(Arena *arena, String string)
{
    String result = PushStrCopy(arena, string);
    for (U64 i = 0; i < string.size; i++)
    {
        result.str[i] = CharToUpper(result.str[i]);
    }
    return result;
}

String LowerFromString(Arena *arena, String string)
{
    String result = PushStrCopy(arena, string);
    for (U64 i = 0; i < string.size; i++)
    {
        result.str[i] = CharToLower(result.str[i]);
    }
    return result;
}