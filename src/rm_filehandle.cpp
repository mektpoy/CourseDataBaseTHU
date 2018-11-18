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
	TRY(IsValid());
	PageNum pageNum;
	SlotNum slotNum;
	TRY(rid.GetPageNum(pageNum));
	TRY(rid.GetSlotNum(slotNum));
	if (slotNum < 0 || slotNum >= this->rm_FileHeader.recordNumPerPage) return RM_ERR_SLOTNUM;

	PF_PageHandle pf_PageHandle;
	TRY(this->pf_FileHandle->GetThisPage(pageNum, pf_PageHandle));

	char *pData;
	TRY(pf_PageHandle.GetData(pData));

	unsigned int offset = this->rm_FileHeader.pageHeaderSize + this->rm_FileHeader.recordSize * slotNum;
	rec.size = this->rm_FileHeader.recordSize;
	rec.data = pData + offset;
	rec.rid = rid;
    return 0;
}

RC RM_FileHandle::InsertRec (const char *pData, RID &rid) {
	TRY(IsValid());
	if (pData == NULL) {
		return RM_ERR_NULLRECORDDATA;
	}

	PF_PageHandle pf_PageHandle;
	// TRY(this->pf_FileHandle->GetThisPage(pageNum, pf_PageHandle));

	// char *pageData;
	// TRY(pf_PageHandle.GetData(pageData));

	// RM_PageHeader *rm_PageHeader = (RM_PageHeader *)pageData;

	// if (GetBit((unsigned char *)(pageData + sizeof(RM_PageHeader) - 1), slotNum) == 0) {
	// 	return RM_WAR_NOSUCHRECORD;
	// }
	// TRY(this->pf_FileHandle->MarkDirty(pageNum));
	// unsigned int offset = this->rm_FileHeader.pageHeaderSize + this->rm_FileHeader.recordSize * slotNum;
	// memset(pageData + offset, 0, this->rm_FileHeader.recordSize);
	// SetBit((unsigned char *)(pageData + sizeof(RM_PageHeader) - 1), slotNum, false);

	// if (rm_PageHeader->numRecord == this->rm_FileHeader.recordNumPerPage) {
	// 	rm_PageHeader->nextPage = this->rm_FileHeader.firstFreePage;
	// 	this->rm_FileHeader.firstFreePage = pageNum;
	// 	this->bHeaderDirty = true;
	// }
	// rm_PageHeader->numRecord --;
	// TRY(this->pf_FileHandle->UnpinPage(pageNum));

    return 0;
}

RC RM_FileHandle::DeleteRec (const RID &rid) {
	TRY(IsValid());
	PageNum pageNum;
	SlotNum slotNum;
	TRY(rid.GetPageNum(pageNum));
	TRY(rid.GetSlotNum(slotNum));
	if (slotNum < 0 || slotNum >= this->rm_FileHeader.recordNumPerPage) return RM_ERR_SLOTNUM;

	PF_PageHandle pf_PageHandle;
	TRY(this->pf_FileHandle->GetThisPage(pageNum, pf_PageHandle));

	char *pageData;
	TRY(pf_PageHandle.GetData(pageData));

	RM_PageHeader *rm_PageHeader = (RM_PageHeader *)pageData;

	if (GetBit((unsigned char *)(pageData + sizeof(RM_PageHeader) - 1), slotNum) == 0) {
		return RM_WAR_NOSUCHRECORD;
	}
	TRY(this->pf_FileHandle->MarkDirty(pageNum));
	unsigned int offset = this->rm_FileHeader.pageHeaderSize + this->rm_FileHeader.recordSize * slotNum;
	memset(pageData + offset, 0, this->rm_FileHeader.recordSize);
	SetBit((unsigned char *)(pageData + sizeof(RM_PageHeader) - 1), slotNum, false);

	if (rm_PageHeader->numRecord == this->rm_FileHeader.recordNumPerPage) {
		rm_PageHeader->nextPage = this->rm_FileHeader.firstFreePage;
		this->rm_FileHeader.firstFreePage = pageNum;
		this->bHeaderDirty = true;
	}
	rm_PageHeader->numRecord --;
	TRY(this->pf_FileHandle->UnpinPage(pageNum));

    return 0;
}

RC RM_FileHandle::UpdateRec (const RM_Record &rec) {
    return 0;
}

RC RM_FileHandle::ForcePages (PageNum pageNum) {
    return this->pf_FileHandle->ForcePages(pageNum);
}

RC RM_FileHandle::IsValid() const {
	if (this->pf_FileHandle == NULL || !this->bFileOpen) {
		return RM_ERR_FILENOTOPEN;
	}
	return 0;
}