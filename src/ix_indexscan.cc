#include "pf.h"
#include "ix.h"
#include "ix_internal.h"
#include <iostream>

IX_IndexScan::IX_IndexScan(){

}

IX_IndexScan::~IX_IndexScan(){

}

RC Compare (char *str) {
	
}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle, CompOp compOp,
					void *value, ClientHint pinHint){
	this->indexHandle = &indexHandle;
	if (indexHandle.bFileOpen == false || indexHandle.fileHandle == NULL) {
		return IX_ERR_FILENOTOPEN;
	}
	if (value == NULL) {
		return IX_ERR_NULLENTRY;
	}
	if (compOp == NE_OP) {
		return IX_ERR_NEOP;
	}
	if (compOp == NO_OP || compOp == LT_OP || compOp == LE_OP) {
		this->getLeft = true;
	} else {
		this->getLeft = false;
	}
	this->compOp = compOp;
	this->value = value;
	PF_PageHandle pageHandle;
	PF_FileHandle *fileHandle = indexHandle.fileHandle;
	IX_FileHeader fileHeader = indexHandle.fileHeader;
	TRY(fileHandle->GetThisPage(fileHeader.rootPage, pageHandle));
	while(true) {
		char *pData;
		TRY(pageHandle.GetData(pData));
		IX_PageHeader *pageHeader = (IX_PageHeader *)pData;
		pData = pData + sizeof(IX_PageHeader);
		int pos = pageHeader->numIndex - 1;
		if (pageHeader->nextPage == IX_PAGE_NOT_LEAF) {
			for (int i = 0; i < pageHeader->numIndex; i ++) {
				char *dest = pData + i * fileHeader.entrySize;
				if (Compare(dest)) pos = i;
			}
		} else {
			TRY(pageHandle.GetPageNum(currentPage));
			currentSlot = 0;
			break;
		}
	}
	return 0;
}

RC IX_IndexScan::GetNextEntry(RID &rid){
	return 0;
}

RC IX_IndexScan::CloseScan(){
	return 0;
}