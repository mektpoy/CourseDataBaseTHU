<<<<<<< HEAD
#include "rid.h"
#include "rm.h"
#include "rm_error.h"
#include <iostream>

#define INIT_REC_SIZE  -1
#define INIT_REC_PAGE  -1 
#define INIT_REC_SLOT  -1

=======
#include "rm_rid.h"
#include "rm.h"
#include <iostream>

>>>>>>> 1a202fac15c28d4232e5bc80f8bd9d650fdb164a
using namespace std;

RM_Record::RM_Record()
{
<<<<<<< HEAD
    size = INIT_REC_SIZE;
    data = NULL;
    rid = RID(INIT_REC_PAGE,INIT_REC_SLOT);
=======
    size = -1;
    data = NULL;
    rid = RID(-1,-1);
>>>>>>> 1a202fac15c28d4232e5bc80f8bd9d650fdb164a
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
<<<<<<< HEAD
		return RM_NULLRECORD; // todo:需要定义一个record为空的错误编号
=======
		return -1; // todo:需要定义一个record为空的错误编号
>>>>>>> 1a202fac15c28d4232e5bc80f8bd9d650fdb164a
}

RC RM_Record::GetRid (RID &rid) const 
{
	if (data != NULL && size != -1)
	{
        rid = this->rid;
        return 0;
    }
	else 
<<<<<<< HEAD
		return RM_NULLRECORD;
=======
		return -1;
>>>>>>> 1a202fac15c28d4232e5bc80f8bd9d650fdb164a
}




