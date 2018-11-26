#include "pf.h"
#include "ix.h"
#include "ix_internal.h"
#include <iostream>

IX_IndexHandle::IX_IndexHandle(){
	this->bFileOpen = false;
	this->fileHandle = NULL;
	this->bHeaderDirty = false;
}

IX_IndexHandle::~IX_IndexHandle(){
	free(this->Data);
	free(this->indexData);
}

int IX_IndexHandle::Compare(char *str) {
	std::memcpy(indexData, str, this->fileHeader.attrLength);
	RID *rid = (RID *)(str + this->fileHeader.attrLength);
	int ret;
	if ((ret = strcmp(Data, indexData))){
		return ret;
	}
	if (DataRid.GetPage() == rid->GetPage()){
		if (DataRid.GetSlot() <  rid->GetSlot()) return -1;
		if (DataRid.GetSlot() == rid->GetSlot()) return 0;
		return 1;
	} else {
		if (DataRid.GetPage() < rid->GetPage()) return -1;
		return 1;
	}
}

RC IX_IndexHandle::GetEntry(const char *pData, const int pos, 
	char *&data, RID *&rid, PageNum *&pageNum) const{
	data = (char *)pData + pos * this->fileHeader.entrySize;
	rid = (RID *)(pData + pos * this->fileHeader.entrySize + 
		this->fileHeader.attrLength);
	pageNum = (PageNum *)(pData + pos * this->fileHeader.entrySize + 
		this->fileHeader.attrLength + sizeof(RID));
	return 0;
}

RC IX_IndexHandle::SetEntry(char *pData, const int pos, 
	const char *data, const RID &rid, PageNum pageNum){
	std::memcpy(pData + pos * this->fileHeader.entrySize,
		data, (size_t)this->fileHeader.attrLength);
	*(RID *)(pData + pos * this->fileHeader.entrySize + 
		this->fileHeader.attrLength) = rid;
	*(PageNum *)(pData + pos * this->fileHeader.entrySize + 
		this->fileHeader.attrLength + sizeof(RID)) = pageNum;
	return 0;
}

RC IX_IndexHandle::SplitAndInsert(PF_PageHandle &pageHandle, IX_PageHeader *pageHeader,
	char *pData, int pos, bool isLeaf) {
	if (pageHeader->numIndex == this->fileHeader.entryNumPerPage){
		PF_PageHandle newPageHandle;
		this->fileHandle->AllocatePage(newPageHandle);
		PageNum newPageNum;
		char *newPageData;
		TRY(newPageHandle.GetPageNum(newPageNum));
		TRY(newPageHandle.GetData(newPageData));
		IX_PageHeader *newPageHeader = (IX_PageHeader *)newPageData;
		newPageData = newPageData + sizeof(IX_PageHeader);
		if (isLeaf){
			newPageHeader->nextPage = pageHeader->nextPage;
			pageHeader->nextPage = newPageNum;
		} else {
			newPageHeader->nextPage = IX_PAGE_NOT_LEAF;
			pageHeader->nextPage = IX_PAGE_NOT_LEAF;
		}
		int mid = pageHeader->numIndex / 2;
		newPageHeader->numIndex = pageHeader->numIndex - mid;
		memmove(newPageData, pData + mid * this->fileHeader.entrySize, pageHeader->numIndex - mid);
		pageHeader->numIndex = mid;
		if (pos < mid) {
			memmove(pData + (pos + 1) * this->fileHeader.entrySize, 
				pData + pos * this->fileHeader.entrySize,
				(size_t)(pageHeader->numIndex - pos) * this->fileHeader.entrySize);
			TRY(SetEntry(pData, pos, this->Data, this->DataRid, this->newSonPageNum));
			pageHeader->numIndex ++;
		} else {
			pos -= mid;
			memmove(newPageData + (pos + 1) * this->fileHeader.entrySize, 
				newPageData + pos * this->fileHeader.entrySize,
				(size_t)(newPageHeader->numIndex - pos) * this->fileHeader.entrySize);
			TRY(SetEntry(newPageData, pos, this->Data, this->DataRid, this->newSonPageNum));
			newPageHeader->numIndex ++;
			this->bSonSplited = true;
			char *dest = newPageData + (newPageHeader->numIndex - 1) * this->fileHeader.entrySize;
			memcpy(this->Data, dest, this->fileHeader.attrLength);
			this->DataRid = *(RID *)(dest + this->fileHeader.attrLength);
			this->newSonPageNum = newPageNum;
		}
		TRY(this->fileHandle->UnpinPage(newPageNum));
	} else {
		memmove(pData + (pos + 1) * this->fileHeader.entrySize, 
			pData + pos * this->fileHeader.entrySize,
			(size_t)(pageHeader->numIndex - pos) * this->fileHeader.entrySize);
		TRY(SetEntry(pData, pos, this->Data, this->DataRid, this->newSonPageNum));
		pageHeader->numIndex ++;
		this->bSonSplited = false;
	}
	PageNum pageNum;
	pageHandle.GetPageNum(pageNum);
	TRY(this->fileHandle->UnpinPage(pageNum));
	return 0;
}

RC IX_IndexHandle::Insert(PF_PageHandle pageHandle) {
	char *pData;
	PageNum pageNum;

	TRY(pageHandle.GetPageNum(pageNum));
	TRY(pageHandle.GetData(pData));
	IX_PageHeader *pageHeader = (IX_PageHeader *)pData;

	pData = pData + sizeof(IX_PageHeader);
	int pos = -1;
	char *p = pData;
	for (int i = 0; i < pageHeader->numIndex; i ++, p = p + this->fileHeader.entrySize) {
		int res = Compare(p);
		if (res == -1) pos = i;
		if (res == 0) return IX_WAR_DUPLICATEDIX;
		break;
	}
	if (pos == -1){
		pos = pageHeader->numIndex - 1;
	}

	if (pageHeader->nextPage == IX_PAGE_NOT_LEAF){
		// NOT LEAF
		PageNum *sonPageNum = (PageNum *)(pData + pos * this->fileHeader.entrySize
			+ this->fileHeader.attrLength + sizeof(RID));
		PF_PageHandle sonPageHandle;
		this->fileHandle->GetThisPage(*sonPageNum, sonPageHandle);
		int rc = Insert(sonPageHandle);
		if (rc == IX_WAR_DUPLICATEDIX || rc < 0) {
			return rc;
		}
		pos = -1;
		p = pData;
		for (int i = 0; i < pageHeader->numIndex; i ++, p = p + this->fileHeader.entrySize) {
			int res = Compare(p);
			if (res == -1) pos = i;
			if (res == 0) return IX_WAR_DUPLICATEDIX;
			break;
		}
		if (pos == -1){
			pos = pageHeader->numIndex - 1;
		}
		TRY(SplitAndInsert(pageHandle, pageHeader, pData, pos, false));
	} else {
		// LEAF
		TRY(SplitAndInsert(pageHandle, pageHeader, pData, pos, true));
	}
	return 0;
}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid){
	TRY(IsValid());
	if (pData == NULL) {
		return IX_ERR_NULLENTRY;
	}
	if (this->Data == NULL) {
		this->Data = (char *)malloc(fileHeader.attrLength);
	}
	std::strcpy(this->Data, (char *)pData);
	this->DataRid = rid;

	PF_PageHandle pageHandle;

	TRY(this->fileHandle->GetThisPage(this->fileHeader.rootPage, pageHandle));
	int rc = Insert(pageHandle);
	if (rc == IX_WAR_DUPLICATEDIX || rc < 0) {
		return rc;
	}
	if (bSonSplited){
		PF_PageHandle newPageHandle;
		this->fileHandle->AllocatePage(newPageHandle);
		PageNum newPageNum;
		char *newPageData;
		TRY(newPageHandle.GetPageNum(newPageNum));
		TRY(newPageHandle.GetData(newPageData));
		IX_PageHeader *newPageHeader = (IX_PageHeader *)newPageData;
		newPageHeader->nextPage = IX_PAGE_NOT_LEAF;
		newPageHeader->numIndex = 2;
		newPageData = newPageData + sizeof(IX_PageHeader);
		TRY(SetEntry(newPageData, 0, Data, DataRid, this->fileHeader.rootPage));
		TRY(SetEntry(newPageData, 1, Data, DataRid, newSonPageNum));
		this->bHeaderDirty = true;
		this->fileHeader.rootPage = newPageNum;
		TRY(this->fileHandle->UnpinPage(newPageNum));
	}
	return 0;
}

RC IX_IndexHandle::Remove(PF_PageHandle pageHandle) {
	// char *pData;
	// PageNum pageNum;

	// TRY(pageHandle.GetPageNum(pageNum));
	// TRY(pageHandle.GetData(pData));
	// IX_PageHeader *pageHeader = (IX_PageHeader *)pData;

	// pData = pData + sizeof(IX_PageHeader);
	// int pos = -1;
	// char *p = pData;
	// for (int i = 0; i < pageHeader->numIndex; i ++, p = p + this->fileHeader.entrySize) {
	// 	int res = Compare(p);
	// 	if (res == -1) pos = i;
	// 	if (res == 0) return IX_WAR_DUPLICATEDIX;
	// 	break;
	// }
	// if (pos == -1){
	// 	pos = pageHeader->numIndex - 1;
	// }

	// if (pageHeader->nextPage == IX_PAGE_NOT_LEAF){
	// 	// NOT LEAF
	// 	PageNum *sonPageNum = (PageNum *)(pData + pos * this->fileHeader.entrySize
	// 		+ this->fileHeader.attrLength + sizeof(RID));
	// 	PF_PageHandle sonPageHandle;
	// 	this->fileHandle->GetThisPage(*sonPageNum, sonPageHandle);
	// 	int rc = Insert(sonPageHandle);
	// 	if (rc == IX_WAR_DUPLICATEDIX || rc < 0) {
	// 		return rc;
	// 	}
	// 	pos = -1;
	// 	p = pData;
	// 	for (int i = 0; i < pageHeader->numIndex; i ++, p = p + this->fileHeader.entrySize) {
	// 		int res = Compare(p);
	// 		if (res == -1) pos = i;
	// 		if (res == 0) return IX_WAR_DUPLICATEDIX;
	// 		break;
	// 	}
	// 	if (pos == -1){
	// 		pos = pageHeader->numIndex - 1;
	// 	}
	// 	TRY(SplitAndInsert(pageHandle, pageHeader, pData, pos, false));
	// } else {
	// 	// LEAF
	// 	TRY(SplitAndInsert(pageHandle, pageHeader, pData, pos, true));
	// }
	return 0;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid){
	// TRY(IsValid());
	// if (pData == NULL) {
	// 	return IX_ERR_NULLENTRY;
	// }
	// if (this->Data == NULL) {
	// 	this->Data = (char *)malloc(fileHeader.attrLength);
	// }
	// std::strcpy(this->Data, (char *)pData);
	// this->DataRid = rid;

	// PF_PageHandle pageHandle;

	// TRY(this->fileHandle->GetThisPage(this->fileHeader.rootPage, pageHandle));
	// TRY(Remove(pageHandle));
	return 0;
}

RC IX_IndexHandle::ForcePages(){
    return this->fileHandle->ForcePages();
}  

RC IX_IndexHandle::IsValid() const {
	if (this->fileHandle == NULL || !this->bFileOpen) {
		return IX_ERR_FILENOTOPEN;
	}
	return 0;
}