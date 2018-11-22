#include "rid.h"
#include "rm.h"

RID::RID()
{
    page = -1;
    slot = -1;
}

RID::RID(PageNum pageNum, SlotNum slotNum){
    page = pageNum;
    slot = slotNum;
}

RID::~RID(){

}

RC RID::GetPageNum(PageNum &pageNum) const{
    if(page >= 0){
        pageNum = page;
        return 0;
    }
    else{
        return RM_ERR_PAGENUM;
    }
}

RC RID::GetSlotNum(SlotNum &slotNum) const{
    if(slot >= 0){
        slotNum = slot;
        return 0;
    }
    else{
        return RM_ERR_SLOTNUM;
    }
}


