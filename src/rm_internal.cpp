//
// File:        rm_internal.h
// Description: Declarations internal to the record manager component
// Authors:     mektpoy
//

#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

#include <cstdlib>
#include <cstring>
#include "rm.h"

//
// Constants and defines
//

#define CREATION_MASK      0600    // r/w privileges to owner only
#define RM_PAGE_LIST_END  -1       // end of list of free pages
#define RM_PAGE_USED      -2       // page is being used

struct RM_FileHeader {
    int firstFreePage;
    //int numPage;
    int recordSize;
    int recordNumPerPage;
    int pageHeaderSize;
};

struct RM_PageHeader {
	int nextPage;
	int numRecord;
	unsigned char bitmask[1];
};


#endif
