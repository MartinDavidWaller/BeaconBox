/*
 *  StringHelper.h
 *  
 *  Author:   M.D. Waller - G0PJO
 *  Copyright (c) 2024
 */
 
#define MAX_TOKEN_COUNT 14

extern char *tokenPointers[];
extern int SplitString(char* stringToSplit);
extern int SplitStringOnSpace(const char *stringToSplit, char **partsBuffer[]);
extern int wildcmp(const char *wild, const char *string);
extern char *StringCopyWithLimit(char *dest, const char *source, int limit);
