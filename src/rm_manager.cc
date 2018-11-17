//
// Created by mektpoy on 2018/11/05.
//

#include "pf.h"
#include "rm.h"
#include "rm_internal.cpp"
#include <iostream>

RM_Manager::RM_Manager(PF_Manager &pfm) {
    this->pfm = &pfm;
}

RM_Manager::~RM_Manager() {
}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
    if (recordSize > PF_PAGE_SIZE - sizeof(RM_PageHeader)) {
        return RM_RECORDSIZEERR;
    }
    TRY(pfm->CreateFile(fileName));

    PF_FileHandle fileHandle;
    PF_PageHandle pageHandle;
    RM_FileHeader *fileHeader;
    RM_PageHeader *pageHeader;

    char *pageData;

    TRY(pfm->OpenFile(fileName, fileHandle));
    TRY(fileHandle.AllocatePage(pageHandle));
    TRY(pageHandle.GetData(pageData));

//  num * (recordSize) + [(num + 7) / 8] <= (PF_PAGE_SIZE - sizeof(RM_PageHeader) + 1)
//  RM_HEADER_SIZE = sizeof(RM_PageHeader) + [(num + 7) / 8]
    int num = (PF_PAGE_SIZE - sizeof(RM_PageHeader) + 1) / (recordSize + 1.0 / 8.0);
    while (num * recordSize + (num + 7) / 8 > PF_PAGE_SIZE - sizeof(RM_PageHeader) + 1)
        num --;
    //int numPage    int recordSize;
    int recordNumPerPage;
    int pageHeaderSize;
    auto p = (RM_FileHeader *)pageData;
    p->firstFreePage = RM_PAGE_LIST_END;


    return 0;
}

RC RM_Manager::DestroyFile(const char *fileName) {
    return pfm->DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
    
    return 0;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    
    return 0;
}
