#include "pf.h"
#include "ix.h"
#include "ix_internal.h"
#include <iostream>

IX_Manager::IX_Manager (PF_Manager &pfm) {
	this->pfm = &pfm;
}

IX_Manager::~IX_Manager () {

} 

RC IX_Manager::CreateIndex (const char *fileName, int indexNo, 
					AttrType   attrType, int attrLength) {
	char FileName[20];
	memset(FileName, 0, sizeof(FileName));
	sprintf(FileName, "%s.%d", fileName, indexNo);
	TRY(pfm->CreateFile(FileName));

	PF_FileHandle fileHandle;
	PF_PageHandle pageHandle;

	char *pageData;

	TRY(pfm->OpenFile(FileName, fileHandle));
	TRY(fileHandle.AllocatePage(pageHandle));
	TRY(pageHandle.GetData(pageData));

	auto p = (IX_FileHeader *)pageData;
	p->firstFreePage = IX_PAGE_LIST_END;
	p->attrType = attrType;
	p->attrLength = attrLength;
	p->entrySize = attrLength + sizeof(RID);
	p->entryNumPerPage = (PF_PAGE_SIZE - sizeof(IX_PageHeader)) / p->entrySize;

	PageNum pageNum;
	TRY(pageHandle.GetPageNum(pageNum));
	TRY(fileHandle.UnpinPage(pageNum));
	TRY(pfm->CloseFile(fileHandle));
	return 0;
}

RC IX_Manager::DestroyIndex (const char *fileName, int indexNo) {
	char FileName[20];
	memset(FileName, 0, sizeof(FileName));
	sprintf(FileName, "%s.%d", fileName, indexNo);
	TRY(pfm->CreateFile(FileName));
	return pfm->DestroyFile(FileName);
}

RC IX_Manager::OpenIndex (const char *fileName, int indexNo, 
				IX_IndexHandle &indexHandle) {
	char FileName[20];
	memset(FileName, 0, sizeof(FileName));
	sprintf(FileName, "%s.%d", fileName, indexNo);
	TRY(pfm->CreateFile(FileName));
	indexHandle.fileHandle = new PF_FileHandle();
    PF_PageHandle pageHandle;
    char *pData;

    TRY(pfm->OpenFile(FileName, *indexHandle.fileHandle));
    TRY(indexHandle.fileHandle->GetFirstPage(pageHandle));
    TRY(pageHandle.GetData(pData));

    indexHandle.bFileOpen = true;
    indexHandle.bHeaderDirty = false;
    indexHandle.fileHeader = *(IX_FileHeader *)pData;

    PageNum pageNum;
    TRY(pageHandle.GetPageNum(pageNum));
    TRY(indexHandle.fileHandle->UnpinPage(pageNum));
	return 0;
}

RC IX_Manager::CloseIndex (IX_IndexHandle &indexHandle) {
	PF_PageHandle pageHandle;
    if (indexHandle.bHeaderDirty) {
        char *pData;
        TRY(indexHandle.fileHandle->GetFirstPage(pageHandle));
        TRY(pageHandle.GetData(pData));
        memcpy(pData, &indexHandle.fileHeader, sizeof(IX_FileHeader));
    }
    pfm->CloseFile(*(indexHandle.fileHandle));

    indexHandle.fileHandle = NULL;
	return 0;
}