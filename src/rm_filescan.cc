#include <iostream>
#include <cstring>
#include "rm.h"
using namespace std;


RM_FileScan::RM_FileScan()
{
    this->fileHandle = NULL;
    this->current = RID(0, 0);
}

RM_FileScan::~RM_FileScan()
{
}

RC RM_FileScan::OpenScan  (const 	RM_FileHandle &fileHandle,
									AttrType   attrType,
									int        attrLength,
									int        attrOffset,
									CompOp     compOp,
									void       *value,
									ClientHint pinHint) // Initialize a file scan
{
	if (attrLength < 0 || attrLength > MAXSTRINGLEN) {
		return RM_ERR_ATTRLENGTH;
	}
	this->fileHandle = &fileHandle;
	this->attrType = attrType;
	this->attrLength = attrLength;
	this->attrOffset = attrOffset;
	this->compOp = compOp;
	if (compOp != NO_OP) {
		if (attrType == INT) {
			this->valueINT = *(int *)value;
		} else if (attrType == FLOAT) {
			this->valueFLOAT = *(double *)value;
		} else if (attrType == STRING) {
			this->valueSTRING = (char *)malloc(attrLength);
			memcpy(this->valueSTRING, value, attrLength);
		}
	}
    this->current = RID(0, 0);
	return 0;
}

bool RM_FileScan::Check(char *pData) {
	auto data = pData + this->attrOffset;
	switch (this->attrType) {
		case INT:
			switch (this->compOp) {
				case EQ_OP:
					return *(int *)pData == *data;
				case NE_OP:
					return *(int *)pData != this->valueINT;
				case LT_OP:
					return *(int *)pData < this->valueINT;
				case GT_OP:
					return *(int *)pData > this->valueINT;
				case LE_OP:
					return *(int *)pData <= this->valueINT;
				case GE_OP:
					return *(int *)pData >= this->valueINT;
				case NO_OP:
					return true;
			}
		case FLOAT:
			switch (this->compOp) {
				case EQ_OP:
					return *(double *)data == this->valueFLOAT;
				case NE_OP:
					return *(double *)data != this->valueFLOAT;
				case LT_OP:
					return *(double *)data < this->valueFLOAT;
				case GT_OP:
					return *(double *)data > this->valueFLOAT;
				case LE_OP:
					return *(double *)data <= this->valueFLOAT;
				case GE_OP:
					return *(double *)data >= this->valueFLOAT;
				case NO_OP:
					return true;
			}
		case STRING:
			switch (this->compOp) {
				case EQ_OP:
					return std::strcmp(data, this->valueSTRING) == 0;
				case NE_OP:
					return std::strcmp(data, this->valueSTRING) != 0;
				case LT_OP:
					return std::strcmp(data, this->valueSTRING) < 0;
				case GT_OP:
					return std::strcmp(data, this->valueSTRING) > 0;
				case LE_OP:
					return std::strcmp(data, this->valueSTRING) <= 0;
				case GE_OP:
					return std::strcmp(data, this->valueSTRING) >= 0;
				case NO_OP:
					return true;
			}
	}
}

RC RM_FileScan::GetNextRec(RM_Record &rec)               // Get next matching record
{
	PageNum pageNum;
	SlotNum slotNum;

	TRY(this->current.GetPageNum(pageNum));
	TRY(this->current.GetSlotNum(slotNum));

	while (true) {
		slotNum ++;
		if (slotNum == this->fileHandle->rm_FileHeader.recordNumPerPage) {
			slotNum = 0;
			pageNum ++;
		}
		int rc = this->fileHandle->GetRec(RID(pageNum, slotNum), rec);
		if (rc == 0) {
			char *pData;
			TRY(rec.GetData(pData));
			if (this->Check(pData)) {
				current = RID(pageNum, slotNum);
				return 0;
			}
		}

		if (rc == PF_EOF) {
			current = RID(pageNum, slotNum);
			return RM_WAR_EOF;
		} else if (rc == PF_INVALIDPAGE) {
			pageNum ++;
			slotNum = -1;
		}
	}
}

RC RM_FileScan::CloseScan ()                             // Close the scan
{
	if (this->fileHandle == NULL) return RM_WAR_SCANNOTOPEN;
	this->fileHandle = NULL;
	if (this->attrType == STRING) {
		delete[] valueSTRING;
	}
	return 0;
}