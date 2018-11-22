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
