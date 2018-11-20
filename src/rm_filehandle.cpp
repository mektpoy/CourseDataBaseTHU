//
// Created by mektpoy on 2018/11/05.
//

#include "pf.h"
#include "rm.h"
#include "rm_internal.h"
#include <iostream>

RM_FileHandle::RM_FileHandle() {
    pf_FileHandle = NULL;
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

	PF_PageHandle pageHandle;
	TRY(this->pf_FileHandle->GetThisPage(pageNum, pageHandle));

	char *pData;
	TRY(pageHandle.GetData(pData));

	unsigned int offset = this->rm_FileHeader.pageHeaderSize + this->rm_FileHeader.recordSize * slotNum;
	rec.size = this->rm_FileHeader.recordSize;
	rec.data = pData + offset;
	rec.rid = rid;
    return 0;
}

RC RM_FileHandle::InsertRec (const char *pData, RID &rid) {
	TRY(IsValid());
	if (pData == NULL) {
		return RM_ERR_NULLRECDATA;
	}

	PF_PageHandle pageHandle;
	PageNum pageNum;
	SlotNum slotNum;

	if (this->rm_FileHeader.firstFreePage == RM_PAGE_LIST_END) {
		TRY(this->pf_FileHandle->AllocatePage(pageHandle));
		TRY(pageHandle.GetPageNum(pageNum));
		char *pageData;
		TRY(pageHandle.GetData(pageData));
		RM_PageHeader *rm_PageHeader = (RM_PageHeader *)pageData;
		memset(rm_PageHeader, 0, (size_t)this->rm_FileHeader.pageHeaderSize);
		rm_PageHeader->firstFreeSlot = 0;
		rm_PageHeader->nextPage = RM_PAGE_USED;
		rm_PageHeader->numRecord = 0;
		this->bHeaderDirty = true;
		this->rm_FileHeader.firstFreePage = pageNum;
		short *p = (short *)(pageData + this->rm_FileHeader.pageHeaderSize);
		for (short i = 0; i < this->rm_FileHeader.recordNumPerPage; i ++) {
			*p = i;
			p += this->rm_FileHeader.recordSize;
		}
		// TRY(this->pf_FileHandle->UnpinPage(pageNum));
	}

	TRY(this->pf_FileHandle->GetThisPage(this->rm_FileHeader.firstFreePage, pageHandle));
	TRY(pageHandle.GetPageNum(pageNum));
	char *pageData;
	TRY(pageHandle.GetData(pageData));
	RM_PageHeader *rm_PageHeader = (RM_PageHeader *)pageData;
	int &tot = rm_PageHeader->numRecord;
	assert (tot < this->rm_FileHeader.recordNumPerPage);
	slotNum = rm_PageHeader->firstFreeSlot;
	char *dest = pageData + this->rm_FileHeader.pageHeaderSize + slotNum * this->rm_FileHeader.recordSize;
	rm_PageHeader->firstFreeSlot = *(short *) dest;
	tot ++;

	if (tot == this->rm_FileHeader.recordNumPerPage) {
		this->bHeaderDirty = true;
		this->rm_FileHeader.firstFreePage = rm_PageHeader->nextPage;
	}
	TRY(this->pf_FileHandle->UnpinPage(pageNum));

    return 0;
}

RC RM_FileHandle::DeleteRec (const RID &rid) {
	TRY(IsValid());
	PageNum pageNum;
	SlotNum slotNum;
	TRY(rid.GetPageNum(pageNum));
	TRY(rid.GetSlotNum(slotNum));
	if (slotNum < 0 || slotNum >= this->rm_FileHeader.recordNumPerPage) return RM_ERR_SLOTNUM;

	PF_PageHandle pageHandle;
	TRY(this->pf_FileHandle->GetThisPage(pageNum, pageHandle));

	char *pageData;
	TRY(pageHandle.GetData(pageData));

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
	TRY(IsValid());
	PageNum pageNum;
	SlotNum slotNum;
	if (rec.size != this->rm_FileHeader.recordSize) {
		return RM_ERR_RECSIZE;
	}
	TRY(rec.rid.GetPageNum(pageNum));
	TRY(rec.rid.GetSlotNum(slotNum));
	if (slotNum < 0 || slotNum >= this->rm_FileHeader.recordNumPerPage) {
		return RM_ERR_SLOTNUM;
	}

	PF_PageHandle pageHandle;
	TRY(this->pf_FileHandle->GetThisPage(pageNum, pageHandle));

	char *pData;
	TRY(pageHandle.GetData(pData));

	unsigned int offset = this->rm_FileHeader.pageHeaderSize + this->rm_FileHeader.recordSize * slotNum;
	memcpy(pData + offset, rec.data, (size_t)rec.size);
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