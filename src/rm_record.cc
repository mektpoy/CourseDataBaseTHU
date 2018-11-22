
#include "rid.h"
#include "rm.h"
#include <iostream>

#define INIT_REC_SIZE  -1
#define INIT_REC_PAGE  -1 
#define INIT_REC_SLOT  -1

using namespace std;

RM_Record::RM_Record()
{
    size = INIT_REC_SIZE;
    data = NULL;
    rid = RID(INIT_REC_PAGE,INIT_REC_SLOT);

}

RM_Record::~RM_Record()
{
	if (data != NULL) {
		delete [] data;
	}
}

RC RM_Record::GetData(char *&pData) const{
    if (data != NULL && size != -1)
    {
		pData = data;
		return 0;
	}
	else 
		return RM_ERR_NULLRECORD; 

}

RC RM_Record::GetRid (RID &rid) const 
{
	if (data != NULL && size != -1)
	{
        rid = this->rid;
        return 0;
    }
	else 
		return RM_ERR_NULLRECORD;
}




