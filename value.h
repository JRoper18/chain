#ifndef _VALUE_H_
#define _VALUE_H_

#include <stdint.h>
#include <stdbool.h>

typedef union Value {
    char asChar;
    short asShort;
    int asInt;
    long asLong;
    long long asLongLong;
    uint64_t asU64;
    uint32_t asU32;
    uint16_t asU16;
    uint8_t asU8;
    int64_t asI64;
    int32_t asI32;
    int16_t asI16;
    int8_t asI8;
    void* asPointer;
    char* asString;
} Value;
typedef struct ValueQueueMem {
	Value val;
	struct ValueQueueMem* next;
} ValueQueueMem;
typedef struct ValueQueue {
	struct ValueQueueMem* head;
	struct ValueQueueMem* tail;
} ValueQueue;
void addValueQ(ValueQueue* q, Value v);
bool isEmptyQ(ValueQueue* q);
Value removeValueQ(ValueQueue* q);

/////////////
// Helpers //
/////////////

// C Doesn't doesn't have function overloading so we do this instead :-(
// The good news is that it has 0 run-time overhead, the compiler could've
// done it all.
//
// These are tradeoffs between convenience and compiler magic. C minimizes magic

static inline Value asPointer(void* v) {
    Value x;
    x.asPointer = v;
    return x;
}

static inline Value asString(char* v) {
    Value x;
    x.asString = v;
    return x;
}

static inline Value asChar(char v) {
    Value x;
    x.asChar = v;
    return x;
}

static inline Value asShort(short v) {
    Value x;
    x.asShort = v;
    return x;
}

static inline Value asInt(int v) {
    Value x;
    x.asInt = v;
    return x;
}

static inline Value asLong(long v) {
    Value x;
    x.asLong = v;
    return x;
}

static inline Value asLongLong(long long v) {
    Value x;
    x.asLongLong = v;
    return x;
}

static inline Value asU64(uint64_t v) {
    Value x;
    x.asU64 = v;
    return x;
}

static inline Value asU32(uint32_t v) {
    Value x;
    x.asU32 = v;
    return x;
}

static inline Value asU16(uint16_t v) {
    Value x;
    x.asU16 = v;
    return x;
}

static inline Value asU8(uint8_t v) {
    Value x;
    x.asU8 = v;
    return x;
}

#endif