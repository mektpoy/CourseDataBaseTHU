#include "rm_rid.h"
#include "rm.h"
#include <iostream>

using namespace std;

RM_Record::RM_Record()
{
    size = -1;
    data = NULL;
    rid = RID(-1,-1);
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
		return -1; // todo:需要定义一个record为空的错误编号
}

RC RM_Record::GetRid (RID &rid) const 
{
	if (data != NULL && size != -1)
	{
        rid = this->rid;
        return 0;
    }
	else 
		return -1;
}




