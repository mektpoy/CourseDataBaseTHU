# 数据库课程项目报告

计55 许翰翔 2015011370
计51 金展宇 2015011380

### 概述

本项目基于CS346（RedBase）的框架。
项目git地址：https://github.com/mektpoy/CourseDataBaseTHU

### 实现

#### PF

pf模块使用CS346提供的Paged File Component，稍作了修改，主要是对PF_FileHandle的GetThisPage增加了PF_EOF的返回值，以便在RM阶段发现已扫到文件末尾。

#### RM

RM阶段包括下述文件：

	rm.h
	rm_internal.h
	rm_filehandle.cc
	rm_filescan.cc
	rm_manager.cc
	rm_record.cc
	rid.h
	rid.cc

在RM阶段，使用第0页作为FileHeader，具有如下结构：

	struct RM_FileHeader {
	    int firstFreePage; // 第一个还有空闲Slot的Page
	    int recordSize; // 每条记录的大小（单位是字节）
	    int recordNumPerPage; // 每页最多储存记录数
	    int pageHeaderSize; // 每页的头部占据的字节数（存下来是因为这个数不是一个定值，但指定了recordSize后就能确定了）
	}

而对于除了第0页的每一页有一个PageHeader，具有如下结构：

	struct RM_PageHeader {
	    int nextPage; // 下一个有空闲Slot的Page
	    int firstFreeSlot; // 这一页中第一个空闲的Slot编号
	    int numRecord; // 这一页当前储存的记录数
	    unsigned char bitmask[1]; // Slot的Bitmask，实际大小是(recordNumPerPage + 7) / 8
	};

这里对slot的链表有一个小技巧，空闲的slot可以用来存下一个空闲的slot的位置，这样可以组成一个链表，对slot进行修改的时候只要取链表头部就能快速维护这个结构。

#### IX

IX阶段包含下述文件：

	ix.h
	ix_internal.h
	ix_indexhandle.cc
	ix_indexscan.cc
	ix_manager.cc

在IX阶段，使用第0页作为FileHeader，具有如下结构：

	struct IX_FileHeader {
		int 		rootPage;  // index根节点的PageNum
		AttrType 	attrType;  // 索引值的类型
		int 		attrLength;  // 索引值得长度（单位是字节）
		int			entrySize;  // 索引值在实际储存时的总长（比上个值多了sizeof(RID)和sizeof(PageNum)
		int 		entryNumPerPage;  // 每页总共能储存多少索引值
	};

而除了第0页，每一页有一个PageHeader，具有如下结构：

	struct IX_PageHeader {
		int nextPage;  // 对于叶节点，储存B+树下一跳的PageNum
		int numIndex;  // 这一页已经储存了多少条Index
	};

对于Index中可能出现的值相同的元素，处理方法是，将RID也作为比较的第二关键字，这样可以保证比较Index的时候两两不同。

对于B+树的中间节点，会储存子节点的（双关键字）最大值，并且储存子节点的页号。对于插入操作，当前节点若超过了可以容纳的长度则分裂，调用IX_IndexHandle::SplitAndInsert，首先将当前节点分裂，如果是叶子节点，顺便维护一下单向链表。对于删除操作，如果将当前节点删空了的话，返回的时候打个标记，回溯的时候将对应位置也做删除。

#### SM

SM阶段包含下述文件：
	sm.h
	sm_manager.cc

在SM阶段,使用DataAttrInfo来记录属性的信息。
	char     relName[MAXNAME+1];  
	char     attrName[MAXNAME+1]; 
	int      offset;              
	AttrType attrType;            
	int      attrLength;          
	int      indexNo;             
relName为该属性的表的名字
attrName为该表的名字
offset为偏移
attrType为类的属性
attrLength为这个类需要占的长度
indexNo为所以的序号

在SM阶段,使用DataRealInfo来记录表的信息。
	int      recordSize;            
	int      attrCount;            
	int      numPages;              
	int      numRecords;            
	char     relName[MAXNAME+1];  

recordSize为一个record的大小
attrCount为属性的个数
numPages为该表所占Pages个数
numRecords为该表所占Records个数
relName[MAXNAME+1]为数据库的名称

在SM阶段我们主要使用IX和RM提供的接口，来实现数据库需要实现的种种操作。
在增删改数据，以及创建删除表的时候要注意对DataAttrInfo和DataRealInfo的维护。



