#include <iostream>
#include "rm.h"
using namespace std;


RM_FileScan::RM_FileScan()
{
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
	return 0;
}
RC RM_FileScan::GetNextRec(RM_Record &rec)               // Get next matching record
{
	return 0;
}

RC RM_FileScan::CloseScan ()                             // Close the scan
{
	return 0;
}