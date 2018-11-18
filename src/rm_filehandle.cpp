//
// Created by mektpoy on 2018/11/05.
//

#include "pf.h"
#include "rm.h"
#include "rm_internal.h"
#include <iostream>

RM_FileHandle::RM_FileHandle() {
}

RM_FileHandle::~RM_FileHandle() {
}

RC RM_FileHandle::GetRec (const RID &rid, RM_Record &rec) const {
    return 0;
}

RC RM_FileHandle::InsertRec (const char *pData, RID &rid) {
    return 0;
}

RC RM_FileHandle::DeleteRec (const RID &rid) {
    return 0;
}

RC RM_FileHandle::UpdateRec (const RM_Record &rec) {
    return 0;
}

RC RM_FileHandle::ForcePages (PageNum pageNum) {
    return 0;
}