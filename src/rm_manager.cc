//
// Created by mektpoy on 2018/11/05.
//

#include "pf.h"
#include "rm.h"

RM_Manager::RM_Manager(PF_Manager &pfm) {
    this->pfm = &pfm;
}

RM_Manager::~RM_Manager() {
	delete pfm;
}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
    if (recordSize > PF_PAGE_SIZE) {
        return RM_RECORDSIZEERR;
    }
    pfm->CreateFile(fileName);

    PF_FileHandle fileHandle;
    PF_PageHandle pageHandle;
    RM_FileHeader *fileHeader;
    RM_PageHeader *pageHeader;

    pfm->OpenFile(fileName, fileHandle);
    fileHandle.AllocatePage(pageHandle);

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
