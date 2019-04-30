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

    char n = '\0';
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
    char prev = '\0';
    while ((n = fgetc (file)) != EOF)
    {
        if (prev == '/' && n == '*')
        {
            multiLineComment = true;
            if (state == 0)
            {
                letterNum--;
                word [letterNum] = '\0';
            }
        }
        else if (prev == '/' && n == '/')
        {
            oneLineComment = true;
            if (state == 0)
            {
                letterNum--;
                word [letterNum] = '\0';
            }
        }

        if (oneLineComment)
        {
            if (n == '\n')
            {
                oneLineComment = false;
            }
        }
        else if (multiLineComment)
        {
            if (prev == '*' && n == '/')
            {
                oneLineComment = false;
                multiLineComment = false;
            }
        }
        else
        {
            // findName - finds the return type of the function.
            if (state == 0)
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
        }
        prev = n;
    }

    return info;
}