//
// rm_rid.h
//
//   The Record Id interface
//

#ifndef RID_H
#define RID_H

// We separate the interface of RID from the rest of RM because some
// components will require the use of RID but not the rest of RM.

#include "redbase.h"
#include "pf.h"

typedef int SlotNum;

//
// RID: Record id interface
//
class RID {
public:
    RID();                                         // Default constructor
    RID(PageNum pageNum, SlotNum slotNum);
    ~RID();                                        // Destructor

    RC GetPageNum(PageNum &pageNum) const;         // Return page number
    RC GetSlotNum(SlotNum &slotNum) const;         // Return slot number

private:
    PageNum page;
    SlotNum slot;
};

#endif
