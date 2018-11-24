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
    int 		firstFreePage;
    AttrType 	attrType;
    int 		attrLength;
    int			entrySize;
    int 		entryNumPerPage;
};

struct IX_PageHeader {
    int nextPage;
    int numRecord;
    unsigned char bitmask[1];
};

//
// IX_IndexHandle: IX File interface
//
class IX_IndexHandle {
	friend class IX_Manager;
public:
	IX_IndexHandle  ();                             
	~IX_IndexHandle ();                            
	RC InsertEntry 	(void *pData, const RID &rid);  
	RC DeleteEntry 	(void *pData, const RID &rid);  
	RC ForcePages 	();    
private:
	PF_FileHandle *fileHandle;
	bool bFileOpen;
	bool bHeaderDirty;
	IX_FileHeader fileHeader;                         
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

#endif



