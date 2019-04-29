#ifndef C_INTERPRETER_H
#define C_INTERPRETER_H

extern struct params;
extern struct functionInfo;
extern char* findLastWord (char*);
extern struct functionInfo** interpret (char*);

#endif