#include "sm.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include "redbase.h"
#include <set>

using namespace std;

SM_Manager::SM_Manager(IX_Manager &_ixm, RM_Manager &_rmm)
  :rmm(_rmm), ixm(_ixm)
{
  memset(cwd,0,1024);
}
SM_Manager::~SM_Manager()
{
}
RC SM_Manager::OpenDb(const char *dbName){
  if(dbName == NULL){
    return SM_NULLDB;
  }
  if(dbopen){
    return SM_HASOPEN;
  }
  TRY(rmm.OpenFile("attrcat", attrfh))
  TRY(rmm.OpenFile("relcat", relfh))
  dbopen = true;
  return 0;
}

RC SM_Manager::CloseDb(){
  if(!dbopen){
    return SM_HASOPEN;
  }
  TRY(rmm.CloseFile(attrfh))
  TRY(rmm.CloseFile(relfh))
  dbopen = false;
  return 0;
}
RC SM_Manager::CreateTable(const char *relName,int attrCount,AttrInfo *attributes){
  TRY(IsValid());
  if(relName == NULL || attrCount <= 0 || attributes == NULL) {
    return SM_BADTABLE;
  }
  if(strcmp(relName, "relcat") == 0 ||
     strcmp(relName, "attrcat") == 0
    ) {
    return SM_BADTABLE;
  }
  RID rid;
  set<string> uniq;
  DataAttrInfo * d = new DataAttrInfo[attrCount];
  int size = 0;
  for(int i=0;i<attrCount;i++){
    d[i] = DataAttrInfo(attributes[i]);
    d[i].offset = size;
    size += attributes[i].attrLength;
    strcpy (d[i].relName, relName);
    if(uniq.find(string(d[i].attrName)) == uniq.end())
      uniq.insert(string(d[i].attrName));
    else {
      return SM_BADATTR; 
    }
    TRY(attrfh.InsertRec((char*) &d[i], rid));
  }
  TRY(rmm.CreateFile(relName, size));
  DataRelInfo rel;
  strcpy(rel.relName, relName);
  rel.attrCount = attrCount;
  rel.recordSize = size;
  rel.numPages = 1; // initially
  rel.numRecords = 0;
  TRY(relfh.InsertRec((char*) &rel, rid));
  delete []d;
  return (0);
}
RC SM_Manager::GetRelFromCat(const char* relName, DataRelInfo& rel,RID& rid) const{
  TRY(IsValid());
  if(relName == NULL)
    return SM_BADTABLE;
  void * value = const_cast<char*>(relName);
  RM_FileScan rfs;
  TRY(rfs.OpenScan(relfh,STRING,MAXNAME+1,offsetof(DataRelInfo, relName),
                       EQ_OP, value, NO_HINT));
  RM_Record rec;
  RC rc = rfs.GetNextRec(rec);
  if(rc == RM_WAR_EOF)
    return SM_NOSUCHTABLE;
  TRY(rfs.CloseScan());
  DataRelInfo * prel;
  rec.GetData((char *&) prel);
  rel = *prel;
  rec.GetRid(rid);
  return 0;
}

RC SM_Manager::GetAttrFromCat(const char* relName, const char* attrName,DataAttrInfo& attr,RID& rid) const{
  TRY(IsValid());
  if(relName == NULL)
    return SM_BADTABLE;
  if(attrName == NULL)
    return SM_BADATTR;
  void * value = const_cast<char*>(relName);
  RC rc;
  RM_FileScan rfs;
  RM_Record rec;
  DataAttrInfo * data;
  rc = rfs.OpenScan(attrfh,STRING,MAXNAME+1,offsetof(DataRelInfo, relName),EQ_OP, value);
  RM_Record rec;
  bool attrFound = false;
  while(rc){
      rc = rfs.GetNextRec(rec);
      if(rc != RM_WAR_EOF && rc > 0)
        return rc;
      rec.GetData((char*&)data);
      if(strcmp(data->attrName, attrName) == 0) {
        attrFound = true;
        break;
      }
  }
  TRY(rfs.CloseScan());
  if(attrFound){
    return SM_BADATTR;
  }
  else{
    rec.GetRid(rid);
    return 0;
  }
}

RC SM_Manager::DropTable(const char *relName)
{
  TRY(IsValid());
  if(relName == NULL) {
    return SM_BADTABLE;
  }
  if(strcmp(relName, "relcat") == 0 ||
     strcmp(relName, "attrcat") == 0
    ) {
    return SM_BADTABLE;
  }
          
  RM_FileScan rfs;
  RM_Record rec;
  DataRelInfo * data;
  RC rc;
  rc = rfs.OpenScan(relfh,
                         STRING,
                         MAXNAME+1,
                         offsetof(DataRelInfo, relName),
                         EQ_OP,
                         (void*) relName))) 
    return (rc);

  bool attrFound = false;
  while (rc!=RM_WAR_EOF) {
    rc = rfs.GetNextRec(rec);

    if (rc!=0 && rc!=RM_WAR_EOF)
      return (rc);

    if (rc!=RM_WAR_EOF) {
      rec.GetData((char*&)data);
      if(strcmp(data->relName, relName) == 0) {
        attrFound = true;
        break;
      }
    }
  }

  TRY(rfs.CloseScan());
  if(!attrFound)
    return SM_NOSUCHTABLE;

  RID rid;
  rec.GetRid(rid);

  TRY(rmm.DestroyFile(relName));
  TRY(relfh.DeleteRec(rid));
  RM_Record rec;
  DataAttrInfo * adata;
  if ((rc = rfs.OpenScan(attrfh,
                          STRING,
                          MAXNAME+1,
                          offsetof(DataAttrInfo, relName),
                          EQ_OP,
                          (void*) relName))) 
    return (rc);
  
  while (rc!=RM_WAR_EOF) {
    rc = rfs.GetNextRec(rec);
    if (rc!=0 && rc!=RM_WAR_EOF)
    return (rc);
    if (rc!=RM_WAR_EOF) {
      rec.GetData((char*&)adata);
      if(strcmp(adata->relName, relName) == 0) {
        if(adata->indexNo != -1) // drop indexes also
          this->DropIndex(relName, adata->attrName);
        RID rid;
        rec.GetRid(rid);
        if ((rc = attrfh.DeleteRec(rid)) != 0)
          return rc;
      }
    }
  }
  TRY(rfs.CloseScan());
  return (0);
}

RC SM_Manager::CreateIndex(const char *relName,
                           const char *attrName)
{
  TRY(IsValid());
  if(relName == NULL || attrName == NULL) {
    return SM_BADTABLE;
  }
  DataAttrInfo attr;
  DataAttrInfo * data = &attr;
  RC rc;
  RID rid;
  TRY(GetAttrFromCat(relName, attrName, attr, rid));
  if(data->indexNo != -1)
    return SM_INDEXEXISTS;
  // otherwise here is a new one
  data->indexNo = data->offset;
  TRY(ixm.CreateIndex(relName, data->indexNo, 
                          data->attrType, data->attrLength));

  // update attrcat
  RM_Record rec;
  rec.Set((char*)data, DataAttrInfo::size(), rid);
  if ((rc = attrfh.UpdateRec(rec)) != 0)
    return rc;

  // now create index entries
  IX_IndexHandle ixh;
  rc = ixm.OpenIndex(relName, data->indexNo, ixh);
  if(rc !=0) return rc;

  RM_FileHandle rfh;
  rc = rmm.OpenFile(relName, rfh);
  if (rc !=0) return rc;
  RM_FileHandle *prfh = &rfh;

  int attrCount;
  DataAttrInfo * attributes;
  rc = GetFromTable(relName, attrCount, attributes);
  if (rc !=0) return rc;
  RM_FileScan rfs;

  if ((rc = rfs.OpenScan(*prfh, data->attrType, data->attrLength, data->offset, NO_OP, NULL))) 
    return (rc);

  // Index each tuple
  while (rc!=RM_EOF) {
    RM_Record rec;
    rc = rfs.GetNextRec(rec);

    if (rc!=0 && rc!=RM_EOF)
      return (rc);

    if (rc!=RM_EOF) {
      char * pdata;
      rec.GetData(pdata);
      RID rid;
      rec.GetRid(rid);
      // cerr << "SM create index - inserting {" << *(char*)(pdata + data->offset) << "} " << rid << endl;
      ixh.InsertEntry(pdata + data->offset, rid);
    }
  }

  if((rc = rfs.CloseScan()))
    return (rc);
   
  if((0 == rfh.IsValid())) {
    if (rc = rmm.CloseFile(rfh))
      return (rc);
  }

  rc = ixm.CloseIndex(ixh);
  if(rc !=0) return rc;

  delete [] attributes;
  return 0;
}

RC SM_Manager::IsValid () const{
  bool ret = dbopen;
  return ret ? 0 : SM_BADOPEN;
}

