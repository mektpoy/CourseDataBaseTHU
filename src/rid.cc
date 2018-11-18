#include "rm_rid.h"
#include "rm_error.h"

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
        return RM_RID_PAGENUM_ERR
    }
}

RC RID::GetSlotNum(SlotNum &slotNum) const{
    if(slow >=0 ){
        slotNum = slot;
        return 0;
    }
    else{
        return RM_RID_SLOTNUM_ERR
    }
}


