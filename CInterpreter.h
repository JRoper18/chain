#ifndef C_INTERPRETER_H
#define C_INTERPRETER_H

typedef struct params params;
typedef struct functionInfo functionInfo;
extern char* findLastWord (char*);
extern struct functionInfo** interpret (char*);

#endif