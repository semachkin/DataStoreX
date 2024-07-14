#pragma once

#include "stdint.h"
#include "minwindef.h"

typedef unsigned char ENUMT;

typedef struct STRVAL_S {
    char *p; 
    size_t len;
} STRVAL;

#pragma pack(push, 1)
typedef struct TYPEOBJECT_S {
    void *data;
    ENUMT type;
} TYPEOBJECT;
#pragma pack(pop)

typedef struct HASHSTRVAL_S {
    struct HASHSTRVAL_S *last;
    uint16_t hash;
    TYPEOBJECT obj;
} HASHSTRVAL;

typedef struct HASHLIST_S {
    HASHSTRVAL **buff;
    uint16_t size;
    uint16_t nuse;
} HASHLIST;

#define HashIndexing(l, s, p) \
    { STRVAL key = stro(s, strlen(s)); p = HashGet(l, key); }

#define hashfunc(k, str) \
    for (uint16_t l = 0; l < str.len; l++) k ^= k + cast(BYTE, str.p[l]);

#ifndef ARRAlloc
#define ARRAlloc(t, s) (cast(t*, malloc(s*sizeof(t))))
#endif

#define STRPrint(pt, sv) pt = ARRAlloc(char, sv.len+1); memcpy(pt, sv.p, sv.len); pt[sv.len] = '\0'

#define stro(p,l) {p, cast(size_t, l)}
#define typeo(p,t) {p, cast(ENUMT, t)}

#ifndef MEMFree
#define MEMFree(p) \
    if (p != NULL) free(p); \
    p = NULL;
#endif

#ifndef cast
#define cast(t, a) ((t)(a))
#endif

#ifndef InlineApi
#define InlineApi static inline
#endif

#ifndef MAX_NUM
#define MAX_NUM(t) ((~(t)0)-2)
#endif

#define HASHLIST_STARTSIZE 16
#define MAX_HASHLIST_SIZE MAX_NUM(uint16_t)

#define HashListCreate(p) { memset(p, 0, sizeof(HASHLIST)); HashListRealloc(p, HASHLIST_STARTSIZE); }

void HashSetVal(HASHLIST *list, STRVAL key, TYPEOBJECT val);
void HashListRealloc(HASHLIST *list, uint16_t newsize);
void HashListClean(HASHLIST *list);
HASHSTRVAL *HashGet(HASHLIST *list, STRVAL key);