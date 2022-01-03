#ifndef __MM_TYPES_H__
#define __MM_TYPES_H__

#define NULL    ((void*)0)

struct page {
    unsigned long flags;
    long _refcount;
};

#endif
