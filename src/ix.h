//
// ix.h
//
//   Indexing component interface
//

#ifndef IX_H
#define IX_H

// Do not change the following includes
#include "redbase.h"
#include "rid.h"
#include "pf.h"

struct IX_FileHeader {
    int 		rootPage;
    AttrType 	attrType;
    int 		attrLength;
    int			entrySize;
    int 		entryNumPerPage;
};

struct IX_PageHeader {
    int nextPage;
    int numIndex;
};

//
// IX_IndexHandle: IX File interface
//
class IX_IndexHandle {
	friend class IX_Manager;
	friend class IX_IndexScan;
public:
	IX_IndexHandle  ();
	~IX_IndexHandle ();
	RC InsertEntry 	(void *pData, const RID &rid);
	RC DeleteEntry 	(void *pData, const RID &rid);
	RC ForcePages 	();
	RC IsValid		()const;
	RC Insert 		(PF_PageHandle pageHandle);
	RC Remove		(PF_PageHandle pageHandle);
	RC GetEntry 	(const char *pData, const int pos, 
		char *&data, RID *&rid, PageNum *&pageNum) const;
	RC SetEntry		(char *pData, const int pos, 
		const char *data, const RID &rid, PageNum pageNum);
	RC SplitAndInsert(PF_PageHandle &pageHandle, IX_PageHeader *pageHeader,
		char *pData, int pos, bool isLeaf);
	int Compare	(char *str);
private:
	PF_FileHandle *fileHandle;
	bool bFileOpen;
	char *Data;
	RID DataRid;
	PageNum newSonPageNum;
	IX_FileHeader fileHeader;
	char *indexData;
	bool bSonSplited;
	bool bHeaderDirty;
	bool bMaxModified;
};

//
// IX_IndexScan: condition-based scan of index in the file
//
class IX_IndexScan {
public:
	IX_IndexScan  	();
	~IX_IndexScan 	();
	RC OpenScan 	(const IX_IndexHandle &indexHandle, CompOp compOp,
						void *value, ClientHint pinHint = NO_HINT);           
    RC GetNextEntry (RID &rid);
    RC CloseScan 	();
    RC Compare		(char *str);
    RC Check 		(char *str);
    const IX_IndexHandle *indexHandle;
    PageNum currentPage;
    int currentSlot;
    CompOp compOp;
    void *value;
    bool getLeft;
    char *pDataBuf;
    char *indexData;
};

//
// IX_Manager: provides IX file management
//
class IX_Manager {
public:
	IX_Manager 		(PF_Manager &pfm);              
	~IX_Manager 	(); 
	RC CreateIndex  (const char *fileName, int indexNo, 
						AttrType   attrType, int attrLength);
	RC DestroyIndex (const char *fileName, int indexNo);
	RC OpenIndex    (const char *fileName, int indexNo, 
					IX_IndexHandle &indexHandle);
	RC CloseIndex   (IX_IndexHandle &indexHandle);  
private:
    PF_Manager *pfm;
};

//
// Print-error function
//
void IX_PrintError(RC rc);

#define IX_WAR_DUPLICATEDIX (START_IX_WARN + 0)
#define IX_WAR_NOSUCHINDEX	(START_IX_WARN + 1)
#define IX_EOF 				(START_IX_WARN + 2)

#define IX_ERR_FILENOTOPEN 	(START_IX_ERR - 0)
#define IX_ERR_NULLENTRY	(START_IX_ERR - 1)
#define IX_ERR_NEOP 		(START_IX_ERR - 2)

#endif



