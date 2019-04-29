#ifndef _GO_H_
#define _GO_H_

#include <stdint.h>
#include <stdbool.h>
#include "value.h"

struct Channel;
typedef struct Channel Channel;

struct Stream;
typedef struct Stream Stream;

typedef void (*Func)(void);

typedef void (*StreamFunc)(Value v);

extern Channel* go(Func func);
extern Channel* me(void);
extern Channel* channel(void);
extern void poison(Channel* ch);
extern bool isPoisoned(Channel* ch);
extern void send(Channel* ch, Value v);
extern Value receive(Channel* ch);
extern void again(void);
extern Stream* stream(StreamFunc f, Value v);
extern bool endOfStream(Stream*);
extern Value next(Stream*);
extern void yield(Value value);

#endif