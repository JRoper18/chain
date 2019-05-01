//
// Created by John on 4/27/2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CInterpreter.h"

/* Parameter for a function,
 * includes the type, name, and
 * functions which feed it.
 */

/* Stores information about a
 * given function, including
 * type, name, and a list of
 * parameters.
 */

/* Reads through a chain file
 * to find the information
 * required to convert chain
 * code to c.
 */

const static int MAX_PARAMS = 200;

char* findLastWord (char* s)
{
    int len = strlen (s);
    char* i;
    for (i = s + len - 1; i >= s && *i != ' '; i--);
    return i + 1;
}

functionInfo** interpret (char* fileName)
{
    FILE* file = fopen (fileName, "r");

    if (file == NULL)
    {
        printf ("Error while reading file: %s.\n", fileName);
        exit (1);
    }

    char n;
    char* word = (char*)malloc (2000 * sizeof (char));
    int letterNum = 0;
    functionInfo** info = (functionInfo**)malloc (100 * sizeof (functionInfo*));
    int infoNum = 0;

    int state = 0;
    /* STATE OF THE PROGRAM
     * 0 - findType
     * 1 - findName
     * 2 - findParams
     * 3 - find{
     * 4 - traverse
     * 5 - find=
     * 6 - find;
     */

    bool oneLineComment = 0;
    bool multiLineComment = 0;
    bool hashComment = 0;
    bool valid = 1;
    char prev = '\0';
    while ((n = fgetc (file)) != EOF)
    {
        if (oneLineComment)
        {
            if (n == '\n')
            {
                oneLineComment = false;
                hashComment = false;
                if (letterNum > 0 && word [letterNum - 1] != ' ' && word [letterNum - 1] != '\t' && word [letterNum - 1] != '\n' && word [letterNum - 1] != '\r')
                {
                    word [letterNum] = ' ';
                    letterNum++;
                    word [letterNum] = '\0';
                }
            }
        }
        else if (multiLineComment) {
            if (prev == '*' && n == '/') {
                oneLineComment = false;
                multiLineComment = false;
                if (letterNum > 0 && word [letterNum - 1] != ' ' && word [letterNum - 1] != '\t' && word [letterNum - 1] != '\n' && word [letterNum - 1] != '\r')
                {
                    word [letterNum] = ' ';
                    letterNum++;
                    word [letterNum] = '\0';
                }
            }
        }
        else if (hashComment)
        {
            if (prev != '\\' && n == '\n')
            {
                hashComment = false;
                if (letterNum > 0 && word [letterNum - 1] != ' ' && word [letterNum - 1] != '\t' && word [letterNum - 1] != '\n' && word [letterNum - 1] != '\r')
                {
                    word [letterNum] = ' ';
                    letterNum++;
                    word [letterNum] = '\0';
                }
            }
        }
        else
        {
            if (prev == '/' && n == '*')
            {
                multiLineComment = true;
                if (state == 0 || state == 1)
                {
                    letterNum--;
                    word [letterNum] = '\0';
                }
            }
            else if (prev == '/' && n == '/')
            {
                hashComment = false;
                oneLineComment = true;
                if (state == 0 || state == 1)
                {
                    letterNum--;
                    word [letterNum] = '\0';
                }
            }
            else if (n == '#')
            {
                letterNum = 0;
                word [letterNum] = '\0';
                hashComment = true;
            }
            // findName - finds the return type of the function.
            else if (state == 0)
            {
                if (n != '(' && n != ';' && n != '=')
                {
                    if (n == ' ' || n == '\t' || n == '\n' || n == '\r')
                    {
                        if (letterNum > 0 && word [letterNum - 1] != ' ' && word [letterNum - 1] != '\t' && word [letterNum - 1] != '\n' && word [letterNum - 1] != '\r')
                        {
                            word [letterNum] = ' ';
                            letterNum++;
                            word [letterNum] = '\0';
                        }
                    }
                    else
                    {
                        word [letterNum] = n;
                        letterNum++;
                        word [letterNum] = '\0';
                    }
                }
                else if (n == ';')
                {
                    letterNum = 0;
                    word [letterNum] = 0;
                }
                else if (n == '(')
                {
                    if (word [letterNum - 1] == ' ')
                    {
                        letterNum--;
                        word [letterNum] = '\0';
                    }
                    info [infoNum] = (functionInfo*)malloc (sizeof (functionInfo));
                    info [infoNum]->modifiers = (char*)malloc (2000 * sizeof (char));
                    info [infoNum]->parameters = (param**)malloc (MAX_PARAMS * sizeof (param*));
                    info [infoNum]->numParams = 0;
                    char* idx = findLastWord (word);
                    if (*idx == '*')
                    {
                        *(idx - 1) = '*';
                        *idx = '\0';
                        idx++;
                    }
                    else if (*(idx - 1) == ' ')
                    {
                        *(idx - 1) = '\0';
                    }
                    info [infoNum]->name = (char*)malloc ((strlen (word) - (word - idx)) * sizeof (char));
                    strcpy (info [infoNum]->name, idx);
                    strcpy (info [infoNum]->modifiers, word);
                    letterNum = 0;
                    word [letterNum] = '\0';
                    state = 1;
                }
                else
                {
                    /*
                     * TODO
                     * ----
                     * Atomic variables.
                     */
                }
            }
            else if (state == 1)
            {
                if (n != ':' && n != ')')
                {
                    if (n == ' ' || n == '\t' || n == '\n' || n == '\r')
                    {
                        if (letterNum > 0 && word [letterNum - 1] != ' ' && word [letterNum - 1] != '\t' && word [letterNum - 1] != '\n' && word [letterNum - 1] != '\r')
                        {
                            word [letterNum] = ' ';
                            letterNum++;
                            word [letterNum] = '\0';
                        }
                    }
                    else
                    {
                        word [letterNum] = n;
                        letterNum++;
                        word [letterNum] = '\0';
                    }
                }
                else if (n == ')')
                {
                    valid = 0;
                    state = 4;
                }
                else
                {
                    if (word [letterNum - 1] == ' ')
                    {
                        letterNum--;
                        word [letterNum] = '\0';
                    }
                    info [infoNum]->parameters [info [infoNum]->numParams]->modifiers = (char*)malloc (2000 * sizeof (char));
                    info [infoNum]->parameters [info [infoNum]->numParams]->functions = (char**)malloc (100 * sizeof (char*));
                    info [infoNum]->parameters [info [infoNum]->numParams]->numFuncs = 0;
                    char *idx = findLastWord (word);
                    if (*idx == '*')
                    {
                        *(idx - 1) = '*';
                        *idx = '\0';
                        idx++;
                    }
                    else if (*(idx - 1) == ' ')
                    {
                        *(idx - 1) = '\0';
                    }
                    info [infoNum]->parameters [info [infoNum]->numParams]->name = (char*)malloc ((strlen (word) - (word - idx)) * sizeof (char));
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->name, idx);
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->modifiers, word);
                    letterNum = 0;
                    word [letterNum] = '\0';
                    state = 2;
                }
            }
            else if (state == 2)
            {
                if (n == '[')
                {
                    state = 3;
                }
            }
            else if (state == 3)
            {
                if (n != ',' && n != ']')
                {
                    if (n != ' ' && n != '\t' && n != '\n' && n != '\r')
                    {
                        word [letterNum] = n;
                        letterNum++;
                        word [letterNum] = '\0';
                    }
                }
                else
                {
                    param* cur = info [infoNum]->parameters [info [infoNum]->numParams];
                    cur->functions [cur->numFuncs] = (char*)malloc (letterNum * sizeof (char));
                    strcpy (cur->functions [cur->numFuncs], word);
                    cur->numFuncs++;
                    if (n == ']')
                    {
                        state = 4;
                    }
                }
            }
            else if (state == 4)
            {
                // TODO
            }
        }
        prev = n;
    }

    return info;
}