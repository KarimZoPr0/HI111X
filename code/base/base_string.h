//
// Created by Abdik on 2025-03-18.
//

#ifndef BASE_STRING_H
#define BASE_STRING_H

#include "base_arena.h"
#include "base_core.h"

typedef struct String String;
struct String
{
    char *str;
    U64 size;
};

typedef struct StringNode StringNode;
struct StringNode
{
    StringNode *next;
    String string;
};

typedef struct StringList StringList;
struct StringList
{
    StringNode *first;
    StringNode *last;
    U64 node_count;
    U64 total_size;
};

typedef struct StringArray StringArray;
struct StringArray
{
    String *v;
    U64 count;
};

typedef struct StringJoin StringJoin;
struct StringJoin
{
    String pre;
    String sep;
    String post;
};

typedef U32 MatchFlags;
enum
{
    MatchFlag_CaseInsensitive  = (1<<0),
    MatchFlag_RightSideSloppy  = (1<<1),
    MatchFlag_SlashInsensitive = (1<<2),
};

B32  CharIsAlpha(char c);
B32  CharIsAlphaUpper(char c);
B32  CharIsAlphaLower(char c);
B32  CharIsDigit(char c);
B32  CharIsSymbol(char c);
B32  CharIsSpace(char c);
char CharToUpper(char c);
char CharToLower(char c);
char CharToForwardSlash(char c);

U64 CStringLength(char *cstr);
char *CStringFromString(Arena *arena, String string);
void PrintString(String s);

String Str(char *str, U64 size);
#define StrZero() Str(0, 0)
#define StrC(cstring) Str((char *)(cstring), CalculateCStringLength(cstring))
#define StrLit(s) Str((char *)(s), sizeof(s)-1)
#define StrLitComp(s) {(char *)(s), sizeof(s)-1}

// Substrings
String Substr(String str, U64 min, U64 max);
String StrSkip(String str, U64 min);
String StrChop(String str, U64 nmax);
String Prefix(String str, U64 size);
String Suffix(String str, U64 size);

String StrTrim(String str);
B32 StrMatch(String a, String b, MatchFlags flags);

// Allocation
String PushStrCopy(Arena *arena, String string);
String PushStrFV(Arena *arena, char *fmt, va_list args);
String PushStrF(Arena *arena, char *fmt, ...);
String PushStrFillByte(Arena *arena, U64 size, char byte);

void StrListPushNode(StringList *list, StringNode *n);
void StrListPushNodeFront(StringList *list, StringNode *n);
void StrListPush(Arena *arena, StringList *list, String str);
void StrListPushF(Arena *arena, StringList *list, char *fmt, ...);
void StrListPushFront(Arena *arena, StringList *list, String str);
void StrListConcatInPlace(StringList *list, StringList *to_push);
StringList StrSplit(Arena *arena, String string, U64 split_count, String *splits);

String StrListJoin(Arena *arena, StringList list, StringJoin *optional_params);

String UpperFromString(Arena *arena, String string);
String LowerFromString(Arena *arena, String string);

#endif //BASE_STRING_H
