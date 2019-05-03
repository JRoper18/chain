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

const static int MAX_PARAMS = 20;

stack* newStack ()
{
    stack* ret = (stack*)malloc (sizeof (stack));
    ret->vals = (char*)malloc (STACK_SIZE * sizeof (char));
    ret->height = 0;
    return ret;
}

void push (stack* s, char c)
{
    s->vals [s->height] = c;
    s->height++;
}

char pop (stack* s)
{
    s->height--;
    return s->vals [s->height];
}

char* findLastWord (char* s)
{
    int len = strlen (s);
    char* i;
    for (i = s + len - 1; i >= s && *i != ' '; i--);
    return i + 1;
}

char* paramToString (param* p)
{
    printf ("\tName: %s\n", p->name);
    printf ("\tModifiers: %s\n", p->modifiers);
    printf ("\tFeeders: ");
    int i;
    for (i = 0; i < p->numFuncs; i++)
    {
        printf ("%s, ", p->functions [i]);
    }
    printf ("\n");
    return NULL;
}

char* infoToString (functionInfo* info)
{
    printf ("Name: %s\n", info->name);
    printf ("Modifiers: %s\n", info->modifiers);
    printf ("Parameters: %d\n", info->numParams);
    int i;
    for (i = 0; i < info->numParams; i++)
    {
        paramToString (info->parameters [i]);
        printf ("\n");
    }
    return NULL;
}

functionInfo** interpret (char* fileName, char* to)
{
    FILE* file = fopen (fileName, "r");
    FILE* cFile = fopen (to, "w");
    if (file == NULL)
    {
        printf ("Error while reading file: %s.\n", fileName);
        exit (1);
    }
    if (cFile == NULL)
    {
        printf ("Error writing to file: %s.\n", to);
        exit (1);
    }

    char n;
    char* word = (char*)malloc (200 * sizeof (char));
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
    bool singleQuote = 0;
    bool doubleQuote = 0;
    bool valid = 1;
    bool escaped = 0;
    char prev = '\0';
    char sec = '\0';
    stack* s = newStack ();
    while ((n = fgetc (file)) != EOF)
    {
        if (oneLineComment)
        {
            if (n == '\n')
            {
                oneLineComment = false;
                hashComment = false;
                singleQuote = false;
                doubleQuote = false;
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
                singleQuote = false;
                doubleQuote = false;
                if (letterNum > 0 && word [letterNum - 1] != ' ' && word [letterNum - 1] != '\t' && word [letterNum - 1] != '\n' && word [letterNum - 1] != '\r')
                {
                    word [letterNum] = ' ';
                    letterNum++;
                    word [letterNum] = '\0';
                }
            }
        }
        else if (singleQuote)
        {
            fprintf (cFile, "%c", n);
            if (n == '\'' && (prev != '\\' || sec == '\\'))
            {
                singleQuote = false;
            }
        }
        else if (doubleQuote)
        {
            fprintf (cFile, "%c", n);
            if (n == '"' && (prev != '\\' || sec == '\\'))
            {
                doubleQuote = false;
            }
        }
        else if (hashComment)
        {
            fprintf (cFile, "%c", n);
            if (!escaped && n == '\r')
            {
                hashComment = false;
                if (letterNum > 0 && word [letterNum - 1] != ' ' && word [letterNum - 1] != '\t' && word [letterNum - 1] != '\n' && word [letterNum - 1] != '\r')
                {
                    word [letterNum] = ' ';
                    letterNum++;
                    word [letterNum] = '\0';
                }
            }
            if (n == '\\')
            {
                escaped = true;
            }
            else
            {
                escaped = false;
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
                fprintf (cFile, "%c", n);
                letterNum = 0;
                word [letterNum] = '\0';
                hashComment = true;
            }
            else if (n == '\'')
            {
                fprintf (cFile, "%c", n);
                letterNum = 0;
                word [letterNum] = '\0';
                singleQuote = true;
            }
            else if (n == '"')
            {
                fprintf (cFile, "%c", n);
                letterNum = 0;
                word [letterNum] = '\0';
                doubleQuote = true;
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
                    info [infoNum + 1] = NULL;
                    info [infoNum]->modifiers = (char*)malloc (200 * sizeof (char));
                    info [infoNum]->parameters = (param**)malloc (MAX_PARAMS * sizeof (param*));
                    info [infoNum]->numParams = 0;
                    char* idx = findLastWord (word);
                    while (*idx == '*')
                    {
                        *(idx - 1) = '*';
                        *idx = ' ';
                        idx++;
                    }
                    if (*(idx - 1) == ' ')
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
                    info [infoNum]->parameters [info [infoNum]->numParams] = (param*)malloc (200 * sizeof (param));
                    info [infoNum]->parameters [info [infoNum]->numParams]->isArray = false;
                    if (word [letterNum - 1] == ' ')
                    {
                        letterNum--;
                        word [letterNum] = '\0';
                    }
                    if (word [letterNum - 1] == ']')
                    {
                        info [infoNum]->parameters [info [infoNum]->numParams]->isArray = true;
                        letterNum -= 2;
                        if (word [letterNum - 1] == '[')
                        {
                            letterNum--;
                        }
                        if (word [letterNum - 1] == ' ')
                        {
                            letterNum--;
                        }
                        word [letterNum] = '\0';
                    }
                    info [infoNum]->parameters [info [infoNum]->numParams]->modifiers = (char*)malloc (200 * sizeof (char));
                    info [infoNum]->parameters [info [infoNum]->numParams]->functions = (char**)malloc (100 * sizeof (char*));
                    info [infoNum]->parameters [info [infoNum]->numParams]->numFuncs = 0;
                    char *idx = findLastWord (word);
                    while (*idx == '*')
                    {
                        *(idx - 1) = '*';
                        *idx = ' ';
                        idx++;
                    }
                    if (*(idx - 1) == ' ')
                    {
                        *(idx - 1) = '\0';
                    }
                    info [infoNum]->parameters [info [infoNum]->numParams]->name = (char*)malloc ((strlen (word) - (word - idx)) * sizeof (char));
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->name, idx);
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->modifiers, word);
                    info [infoNum]->numParams++;
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
                    param* cur = info [infoNum]->parameters [info [infoNum]->numParams - 1];
                    cur->functions [cur->numFuncs] = (char*)malloc (letterNum * sizeof (char));
                    strcpy (cur->functions [cur->numFuncs], word);
                    cur->numFuncs++;
                    letterNum = 0;
                    word [letterNum] = '\0';
                    if (n == ']')
                    {
                        state = 4;
                    }
                }
            }





            else if (state == 4)
            {
                if (n == ')')
                {
                    int i;
                    int j;
                    functionInfo* inf = info [infoNum];
                    for (i = 0; i < strlen (inf->modifiers); i++)
                    {
                        fprintf (cFile, "%c", inf->modifiers [i]);
                    }
                    fprintf (cFile, " ");
                    for (i = 0; i < strlen (inf->name); i++)
                    {
                        fprintf (cFile, "%c", inf->name [i]);
                    }
                    fprintf (cFile, " ");
                    fprintf (cFile, "(");
                    for (i = 0; i < inf->numParams; i++)
                    {
                        for (j = 0; j < strlen (inf->parameters [i]->modifiers); j++)
                        {
                            fprintf (cFile, "%c", inf->parameters [i]->modifiers [j]);
                        }
                        fprintf (cFile, " ");
                        for (j = 0; j < strlen (inf->parameters [i]->name); j++)
                        {
                            fprintf (cFile, "%c", inf->parameters [i]->name [j]);
                        }
                        if (inf->parameters [i]->isArray)
                        {
                            fprintf (cFile, " []");
                        }
                        if (i != inf->numParams - 1)
                        {
                            fprintf (cFile, ", ");
                        }
                    }
                    fprintf (cFile, ")");
                    state = 5;
                }
                else if (n == ',')
                {
                    state = 1;
                }
            }



            else if (state == 5)
            {
                fprintf (cFile, "%c", n);
                if (n == '{')
                {
                    push (s, '{');
                }
                else if (n == '}')
                {
                    pop (s);
                    if (s->height == 0)
                    {
                        infoNum++;
                        /*
                         * TODO
                         *
                         * JACK, THIS IS THE SECTION WHERE YOU PUT YOUR CODE.  I HAVE
                         * THE STRUCT DEFINITIONS IN CInterpreter.h, AND THE STRUCT YOU
                         * NEED TO LOOK IN IS info [infoNum].  IDK IF YOU NEED TO ALTER
                         * THE FUNCTION HEADER TO MAKE IT A THREAD, IF SO, PLEASE TEXT
                         * ME SO WE CAN CHANGE IT TOGETHER.  USE
                         * fprintf (cFile, "[insert stuff here]");
                         * TO PRINT OUT TO THE FILE.
                         */
                        fprintf (cFile, "\n");
                        state = 0;
                    }
                }
            }
        }
        sec = prev;
        prev = n;
    }

    return info;
}