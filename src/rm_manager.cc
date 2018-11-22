//
// Created by mektpoy on 2018/11/05.
//

#include "pf.h"
#include "rm.h"
#include "rm_internal.h"
#include <iostream>

RM_Manager::RM_Manager(PF_Manager &pfm) {
    this->pfm = &pfm;
}

RM_Manager::~RM_Manager() {
}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
    if (recordSize > PF_PAGE_SIZE - (int)sizeof(RM_PageHeader)) {
        return RM_ERR_RECSIZETOOLARGE;
    }
    TRY(pfm->CreateFile(fileName));

    PF_FileHandle fileHandle;
    PF_PageHandle pageHandle;

    char *pageData;

    TRY(pfm->OpenFile(fileName, fileHandle));
    TRY(fileHandle.AllocatePage(pageHandle));
    TRY(pageHandle.GetData(pageData));

//  num * (recordSize) + [(num + 7) / 8] <= (PF_PAGE_SIZE - sizeof(RM_PageHeader) + 1)
//  RM_HEADER_SIZE = sizeof(RM_PageHeader) + [(num + 7) / 8]
    int num = (PF_PAGE_SIZE - sizeof(RM_PageHeader) + 1) / (recordSize + 1.0 / 8.0);
    while (num * recordSize + (num + 7) / 8 > PF_PAGE_SIZE - (int)sizeof(RM_PageHeader) + 1)
        num --;
    auto p = (RM_FileHeader *)pageData;
    p->firstFreePage = RM_PAGE_LIST_END;
    p->recordSize = recordSize;
    p->recordNumPerPage = num;
    p->pageHeaderSize = sizeof(RM_PageHeader) - 1 + (num + 7) / 8;

    TRY(fileHandle.UnpinPage(0));
    TRY(pfm->CloseFile(fileHandle));
    return 0;
}

RC RM_Manager::DestroyFile(const char *fileName) {
    return pfm->DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
    PF_FileHandle pf_FileHandle;
    PF_PageHandle pageHandle;
    char *pData;

    TRY(pfm->OpenFile(fileName, pf_FileHandle));
    TRY(pf_FileHandle.GetFirstPage(pageHandle));
    TRY(pageHandle.GetData(pData));

    fileHandle.bFileOpen = true;
    fileHandle.bHeaderDirty = false;
    fileHandle.rm_FileHeader = *(RM_FileHeader *)pData;

    TRY(pf_FileHandle.UnpinPage(0));

    return 0;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    PF_PageHandle pageHandle;
    char *pData;
    if (fileHandle.bHeaderDirty) {
        TRY(fileHandle.pf_FileHandle->GetFirstPage(pageHandle));
        TRY(fileHandle.pf_FileHandle->GetFirstPage(pageHandle));
        TRY(pageHandle.GetData(pData));
        memcpy(pData, &fileHandle.rm_FileHeader, sizeof(RM_FileHeader));
    }
    pfm->CloseFile(*(fileHandle.pf_FileHandle));
    fileHandle.pf_FileHandle = NULL;
    return 0;
}
