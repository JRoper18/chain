//
// Created by John on 4/27/2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CInterpreter.h"

#define extension "_HELPER_D82KT6KF9"

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
    if (s->height > 0)
    {
        s->height--;
        return s->vals[s->height];
    }
    return 0;
}

char* findLastWord (char* s)
{
    int len = strlen (s);
    char* i;
    for (i = s + len - 1; i >= s && *i != ' '; i--);
    return i + 1;
}

char* toValue (char* in, bool isArray)
{
    if (!isArray)
    {
        if (strcmp (in, "char*") == 0)
        {
            return "String";
        }
        else if (strcmp (in, "char") == 0)
        {
            return "Char";
        }
        else if (strcmp (in, "short") == 0)
        {
            return "Short";
        }
        else if (strcmp (in, "int") == 0)
        {
            return "Int";
        }
        else if (strcmp (in, "long") == 0)
        {
            return "Long";
        }
        else if (strcmp (in, "long long") == 0)
        {
            return "LongLong";
        }
        else if (strcmp (in, "uint64_t") == 0)
        {
            return "U64";
        }
        else if (strcmp (in, "uint32_t") == 0)
        {
            return "U32";
        }
        else if (strcmp (in, "uint16_t") == 0)
        {
            return "U16";
        }
        else if (strcmp (in, "uint8_t") == 0)
        {
            return "U8";
        }
        else
        {
            return "Pointer";
        }
    }
    else
    {
        if (strcmp (in, "char") == 0)
        {
            return "String";
        }
        else
        {
            return "Pointer";
        }
    }
}

int searchFor (functionInfo** info, int infoNum, char* name)
{
    int i;
    for (i = 0; i < infoNum; i++)
    {
        if (strcmp (info [i]->name, name) == 0)
        {
            return i;
        }
    }
    return i;
}

char* paramToString (param* p)
{
    printf ("\tName: %s\n", p->name);
    printf ("\tType: %s\n", p->type);
    printf ("\tModifiers: %s%s\n", p->modifiers, (p->isArray) ? " []" : "");
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
    printf ("Type: %s\n", info->type);
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

functionInfo** interpret (char* fileName, char* to, bool safeMode)
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
    fprintf (cFile, "#include \"functions.h\"\n#include \"pool.h\"\n\nFunction** functions;\n\n");
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
    bool escaped = 0;
    char prev = '\0';
    char sec = '\0';
    stack* s = newStack ();
    bool startSlash = false;
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
            else if (startSlash && n == '/')
            {
                startSlash = false;
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
                    int i;
                    for (i = 0; i < letterNum; i++)
                    {
                        fprintf (cFile, "%c", word [i]);
                    }
                    fprintf (cFile, ";\n");
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
                    char* idx1 = findLastWord (word);
                    while (*idx1 == '*')
                    {
                        *(idx1 - 1) = '*';
                        *idx1 = ' ';
                        idx1++;
                    }
                    if (*(idx1 - 1) == ' ')
                    {
                        *(idx1 - 1) = '\0';
                    }
                    char* idx2 = findLastWord (word);
                    if (idx2 != word && *(idx2 - 1) == ' ')
                    {
                        *(idx2 - 1) = '\0';
                    }
                    if (idx2 != word && strcmp (idx2, "long") == 0)
                    {
                        if (strcmp (findLastWord (word), "long") == 0)
                        {
                            idx2 = findLastWord (word);
                            *(idx2 + strlen (idx2)) = ' ';
                            if (idx2 != word && *(idx2 - 1) == ' ')
                            {
                                *(idx2 - 1) = '\0';
                            }
                        }
                    }
                    info [infoNum]->name = (char*)malloc ((strlen (word) - (word - idx1)) * sizeof (char));
                    info [infoNum]->type = (char*)malloc ((idx1 - idx2) * sizeof (char));
                    strcpy (info [infoNum]->name, idx1);
                    if (word != idx2)
                    {
                        strcpy (info [infoNum]->modifiers, word);
                    }
                    else
                    {
                        info [infoNum]->modifiers = "";
                    }
                    strcpy (info [infoNum]->type, idx2);
                    letterNum = 0;
                    word [letterNum] = '\0';
                    state = 1;
                }
                else
                {
                    word [letterNum] = '=';
                    letterNum++;
                    word [letterNum] = '\0';
                }
            }





            else if (state == 1)
            {
                if (n != ':' && n != ')' && n != ',')
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
                    state = 4;
                    if (letterNum != 0)
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
                        char* idx1 = findLastWord (word);
                        while (*idx1 == '*')
                        {
                            *(idx1 - 1) = '*';
                            *idx1 = ' ';
                            idx1++;
                        }
                        if (*(idx1 - 1) == ' ')
                        {
                            *(idx1 - 1) = '\0';
                        }
                        char* idx2 = findLastWord (word);
                        if (idx2 != word && *(idx2 - 1) == ' ')
                        {
                            *(idx2 - 1) = '\0';
                        }
                        if (idx2 != word && strcmp (idx2, "long") == 0)
                        {
                            if (strcmp (findLastWord (word), "long") == 0)
                            {
                                idx2 = findLastWord (word);
                                *(idx2 + strlen (idx2)) = ' ';
                                if (idx2 != word && *(idx2 - 1) == ' ')
                                {
                                    *(idx2 - 1) = '\0';
                                }
                            }
                        }
                        info [infoNum]->parameters [info [infoNum]->numParams]->name = (char*)malloc ((strlen (word) - (word - idx1)) * sizeof (char));
                        info [infoNum]->parameters [info [infoNum]->numParams]->type = (char*)malloc ((idx1 - idx2) * sizeof (char));
                        strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->name, idx1);
                        if (word != idx2)
                        {
                            strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->modifiers, word);
                        }
                        else
                        {
                            info [infoNum]->parameters [info [infoNum]->numParams]->modifiers = "";
                        }
                        strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->type, idx2);
                        info [infoNum]->numParams++;
                        letterNum = 0;
                        word [letterNum] = '\0';
                    }
                }
                else if (n == ',')
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
                    char *idx1 = findLastWord (word);
                    while (*idx1 == '*')
                    {
                        *(idx1 - 1) = '*';
                        *idx1 = ' ';
                        idx1++;
                    }
                    if (*(idx1 - 1) == ' ')
                    {
                        *(idx1 - 1) = '\0';
                    }
                    char* idx2 = findLastWord (word);
                    if (idx2 != word && *(idx2 - 1) == ' ')
                    {
                        *(idx2 - 1) = '\0';
                    }
                    if (idx2 != word && strcmp (idx2, "long") == 0)
                    {
                        if (strcmp (findLastWord (word), "long") == 0)
                        {
                            idx2 = findLastWord (word);
                            *(idx2 + strlen (idx2)) = ' ';
                            if (idx2 != word && *(idx2 - 1) == ' ')
                            {
                                *(idx2 - 1) = '\0';
                            }
                        }
                    }
                    info [infoNum]->parameters [info [infoNum]->numParams]->name = (char*)malloc ((strlen (word) - (word - idx1)) * sizeof (char));
                    info [infoNum]->parameters [info [infoNum]->numParams]->type = (char*)malloc ((idx1 - idx2) * sizeof (char));
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->name, idx1);
                    if (word != idx2)
                    {
                        strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->modifiers, word);
                    }
                    else
                    {
                        info [infoNum]->parameters [info [infoNum]->numParams]->modifiers = "";
                    }
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->type, idx2);
                    info [infoNum]->numParams++;
                    letterNum = 0;
                    word [letterNum] = '\0';
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
                    char *idx1 = findLastWord (word);
                    while (*idx1 == '*')
                    {
                        *(idx1 - 1) = '*';
                        *idx1 = ' ';
                        idx1++;
                    }
                    if (*(idx1 - 1) == ' ')
                    {
                        *(idx1 - 1) = '\0';
                    }
                    char* idx2 = findLastWord (word);
                    if (idx2 != word && *(idx2 - 1) == ' ')
                    {
                        *(idx2 - 1) = '\0';
                    }
                    if (idx2 != word && strcmp (idx2, "long") == 0)
                    {
                        if (strcmp (findLastWord (word), "long") == 0)
                        {
                            idx2 = findLastWord (word);
                            *(idx2 + strlen (idx2)) = ' ';
                            if (idx2 != word && *(idx2 - 1) == ' ')
                            {
                                *(idx2 - 1) = '\0';
                            }
                        }
                    }
                    info [infoNum]->parameters [info [infoNum]->numParams]->name = (char*)malloc ((strlen (word) - (word - idx1)) * sizeof (char));
                    info [infoNum]->parameters [info [infoNum]->numParams]->type = (char*)malloc ((idx1 - idx2) * sizeof (char));
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->name, idx1);
                    if (word != idx2)
                    {
                        strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->modifiers, word);
                    }
                    else
                    {
                        info [infoNum]->parameters [info [infoNum]->numParams]->modifiers = "";
                    }
                    strcpy (info [infoNum]->parameters [info [infoNum]->numParams]->type, idx2);
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
            else if (state == 5)
            {
                if (n == ';')
                {
                    fprintf (cFile, ";\n");
                    state = 7;
                }
                else if (n == '{')
                {
                    functionInfo* inf = info [infoNum];
                    param* cur;
                    fprintf (cFile, "\n{");
                    if (strcmp (inf->name, "main") == 0)
                    {
                    	if(safeMode){
							fprintf (cFile, "\n\tinitPool ();");
                    	}
                    	else {
							fprintf (cFile, "\n\tmakeWorkers ();");
                    	}
                        int i;
                        int j;
                        int k;
                        for (i = 0; i < infoNum; i++)
                        {
                            inf = info [i];
                            fprintf (cFile, "\n\tfunctions [%d] = makeFunction (%s%s, %d);", i, inf->name, extension, inf->numParams);
                        }
                        for (i = 0; i < infoNum; i++)
                        {
                            inf = info [i];
                            for (j = 0; j < inf->numParams; j++)
                            {
                                cur = inf->parameters [j];
                                for (k = 0; k < cur->numFuncs; k++)
                                {
                                    fprintf (cFile, "\n\twaitFor (functions [%d], functions [%d], %d);", searchFor(info, infoNum, cur->functions [k]), i, j);
                                }
                            }
                        }
                    }
                    state = 6;
                }
            }

            else if (state == 6)
            {
                if (startSlash)
                {
                    fprintf (cFile, "/");
                }
                if (n == '/')
                {
                    startSlash = true;
                }
                else if (n != '{' && n != '}')
                {
                    fprintf(cFile, "%c", n);
                }
                else if (n == '{')
                {
                    push (s, '{');
                }
                else if (n == '}')
                {
                    pop (s);
                    if (s->height <= 0)
                    {
                        if (strcmp (info [infoNum]->name, "main") == 0)
                        {
                        	if(safeMode){
								fprintf (cFile, "\tfinish ();\n}");
                        	}
                        	else {
								fprintf (cFile, "\tfinishAllWorkers ();\n}");
							}
                        }
                        state = 7;
                    }
                }
                if (n == '/')
                {
                    startSlash = true;
                }
            }
            else if (state == 7)
            {
                infoNum++;
                fprintf (cFile, "\n");
                state = 0;
            }




            if (state == 4)
            {
                if (n == ')')
                {
                    /*int i;
                    int j;
                    functionInfo* inf = info [infoNum];
                    fprintf (cFile, "%s%s", inf->modifiers, (strcmp (inf->modifiers, "") == 0) ? "" : " ");
                    fprintf (cFile, "%s ", inf->type);
                    for (i = 0; i < strlen (inf->name); i++)
                    {
                        fprintf (cFile, "%c", inf->name [i]);
                    }
                    fprintf (cFile, "_async");
                    fprintf (cFile, " ");
                    fprintf (cFile, "(");
                    for (i = 0; i < inf->numParams; i++)
                    {
                        for (j = 0; j < strlen (inf->parameters [i]->modifiers); j++)
                        {
                            fprintf (cFile, "%c", inf->parameters [i]->modifiers [j]);
                        }
                        fprintf (cFile, "%s ", inf->parameters [i]->type);
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

                    bool new = true;
                    bool same;
                    for (i = 0; i < infoNum; i++)
                    {
                        same = true;
                        if (strlen (info [infoNum]->name) != strlen (info [i]->name))
                        {
                            same = false;
                        }
                        else
                        {
                            for (j = 0; j < strlen (info [i]->name); j++)
                            {
                                if (info [infoNum]->name [j] != info [i]->name [j])
                                {
                                    same = false;
                                }
                            }
                        }
                        if (same == true)
                        {
                            new = false;
                        }
                    }
                    if (!new)
                    {
                        info [infoNum] = NULL;
                        infoNum--;
                    }
                    else
                    {
                        fprintf (cFile, "\n{\n\texecuteFunction (functions [%d]", infoNum);
                        int i;
                        for (i = 0; i < info [infoNum]->numParams; i++)
                        {
                            fprintf (cFile, ", &as%s (%s)", toValue (info [infoNum]->parameters [i]->type), info [infoNum]->parameters [i]->name);
                        }
                        fprintf (cFile, ");\n}");
                    }
                    */
                    int i;
                    int j;
                    functionInfo* inf = info [infoNum];
                    param* cur;

                    bool new = true;
                    bool same;
                    for (i = 0; i < infoNum; i++)
                    {
                        same = true;
                        if (strlen (info [infoNum]->name) != strlen (info [i]->name))
                        {
                            same = false;
                        }
                        else
                        {
                            for (j = 0; j < strlen (info [i]->name); j++)
                            {
                                if (info [infoNum]->name [j] != info [i]->name [j])
                                {
                                    same = false;
                                }
                            }
                        }
                        if (same == true)
                        {
                            new = false;
                        }
                    }

                    if (!new)
                    {
                        info [infoNum] = NULL;
                        infoNum--;
                    }
                    else
                    {
                        fprintf (cFile, "void %s_async (", inf->name);
                        for (i = 0; i < inf->numParams; i++)
                        {
                            cur = inf->parameters [i];
                            fprintf (cFile, "%s%s", (i == 0) ? "" : ", ", cur->modifiers);
                            fprintf (cFile, "%s%s %s", (strcmp (cur->modifiers, "") == 0) ? "" : " ", cur->type, cur->name);
                            if (cur->isArray)
                            {
                                fprintf (cFile, " []");
                            }
                        }
                        fprintf (cFile, ")\n{\n\texecuteFunction (functions [%d]", infoNum);
                        for (i = 0; i < inf->numParams; i++)
                        {
                            cur = inf->parameters [i];
                            fprintf (cFile, ", &as%s (%s)", toValue (cur->type, cur->isArray), cur->name);
                        }
                        fprintf (cFile, ");\n}\n\nValue %s%s (", inf->name, extension);
                        for (i = 0; i < inf->numParams; i++)
                        {
                            cur = inf->parameters [i];
                            fprintf (cFile, "%sValue* %s", i == 0 ? "" : ", ", cur->name);
                        }
                        fprintf (cFile, ")\n{\n\t");
                        if (strcmp (inf->type, "void") != 0)
                        {
                            //cur = inf->parameters [i];
                            fprintf (cFile, "return as%s (", toValue (inf->type, cur->isArray));
                        }
                        fprintf (cFile, "%s (", inf->name);
                        for (i = 0; i < inf->numParams; i++)
                        {
                            cur = inf->parameters [i];
                            fprintf (cFile, "%s%s [%d].as%s", i == 0 ? "" : " ", cur->name, i, toValue (cur->type, cur->isArray));
                        }
                        if (strcmp (inf->type, "void") != 0)
                        {
                            fprintf (cFile, ")");
                        }
                        fprintf (cFile, ");\n}\n\n");
                    }
                    fprintf (cFile, "%s%s%s %s(", inf->modifiers, strcmp (inf->modifiers, "") == 0 ? "" : " ", inf->type, inf->name);

                    for (i = 0; i < inf->numParams; i++)
                    {
                        cur = inf->parameters [i];
                        fprintf (cFile, "%s%s", (i == 0) ? "" : ", ", cur->modifiers);
                        fprintf (cFile, "%s%s %s", (strcmp (cur->modifiers, "") == 0) ? "" : " ", cur->type, cur->name);
                        if (cur->isArray)
                        {
                            fprintf (cFile, " []");
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

        }
        sec = prev;
        prev = n;
    }

    return info;
}