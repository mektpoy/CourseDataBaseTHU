//
// File:        rm_internal.h
// Description: Declarations internal to the record manager component
// Authors:     mektpoy
//

#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

#include <cstdlib>
#include <cstring>

//
// Constants and defines
//

inline bool GetBit(unsigned char *bitmask, int pos) {
	return bitmask[pos >> 3] >> (pos & 0x7) & 1;
}

inline void SetBit(unsigned char *bitmask, int pos, bool value) {
	int c = bitmask[pos >> 3] >> (pos & 0x7) & 1;
	bitmask[pos >> 3] += -(c << (pos & 0x7)) + (value << (pos & 0x7));
}

#define CREATION_MASK      0600    // r/w privileges to owner only
#define RM_PAGE_LIST_END  -1       // end of list of free pages
#define RM_PAGE_USED      -2       // page is being used

#endif
