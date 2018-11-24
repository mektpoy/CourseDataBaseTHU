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
	return (bool)(bitmask[pos >> 3] >> (pos & 0x7) & 1);
}

inline void SetBit(unsigned char *bitmask, int pos, bool value) {
	if (value) {
		bitmask[pos >> 3] |= (unsigned char) (1 << (pos & 0x7));
	} else {
		bitmask[pos >> 3] &= (unsigned char)~(1 << (pos & 0x7));
	}
}

#define RM_PAGE_LIST_END  -1       // end of list of free pages
#define RM_PAGE_USED      -2       // page is being used

#endif
