#ifndef __MM_TYPES_H__
#define __MM_TYPES_H__

#ifndef NULL
#define NULL    ((void*)0)
#endif

struct page {
    unsigned long flags;
    long _refcount;
};

#endif
