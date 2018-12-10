// Used by SM_Manager::CreateTable
#ifndef SM_H
#define SM_H
#include "ix.h"
#include "rm.h"
// Used by SM_Manager::CreateTable
struct AttrInfo {
   char     *attrName;           // Attribute name
   AttrType attrType;            // Type of attribute
   int      attrLength;          // Length of attribute
};

// Used by Printer class
struct DataAttrInfo {
   DataAttrInfo(){
    memset(attrName, 0, MAXNAME + 1);
    memset(relName, 0, MAXNAME + 1);
    indexNo = -1;
    offset = -1;
  };
   DataAttrInfo(const AttrInfo &a ) {
    memset(attrName, 0, MAXNAME + 1);
    strcpy (attrName, a.attrName);
    attrType = a.attrType;
    attrLength = a.attrLength;
    memset(relName, 0, MAXNAME + 1);
    indexNo = -1;
    offset = -1;
  };
   char     relName[MAXNAME+1];  // Relation name
   char     attrName[MAXNAME+1]; // Attribute name
   int      offset;              // Offset of attribute 
   AttrType attrType;            // Type of attribute 
   int      attrLength;          // Length of attribute
   int      indexNo;             // Attribute index number
};

struct DataRelInfo{
   DataRelInfo(){
   recordSize =0;            // Size per row
   attrCount =0;             // # of attributes
   numPages =0;              // # of pages used by relation
   numRecords=0;
   memset(relName, 0, MAXNAME + 1);
   }
   static unsigned int size() { 
      return (MAXNAME+1) + 4*sizeof(int);
   }
   int      recordSize;            // Size per row
   int      attrCount;             // # of attributes
   int      numPages;              // # of pages used by relation
   int      numRecords;            // # of records in relation
   char     relName[MAXNAME+1];    // Relation name
};

class SM_Manager {
  public:
       SM_Manager  (IX_Manager &_ixm, RM_Manager &_rmm);  // Constructor
       ~SM_Manager ();                                  // Destructor
    RC OpenDb      (const char *dbName);                // Open database
    RC CloseDb     ();                                  // Close database
    RC CreateTable (const char *relName,                // Create relation
                    int        attrCount,
                    AttrInfo   *attributes);
    RC DropTable   (const char *relName);               // Destroy relation
    RC CreateIndex (const char *relName,                // Create index
                    const char *attrName);
    RC DropIndex   (const char *relName,                // Destroy index
                    const char *attrName);
    RC Load        (const char *relName,                // Load utility
                    const char *fileName);
    RC Help        ();                                  // Help for database
    RC Help        (const char *relName);               // Help for relation
    RC Print       (const char *relName);               // Print relation
    RC Set         (const char *paramName,              // Set system parameter
                    const char *value);
    
    RC IsValid() const;
    RC GetRelFromCat(const char* relName, DataRelInfo& rel,RID& rid) const;
    RC GetAttrFromCat(const char* relName, const char* attrName,DataAttrInfo& attr,RID& rid) const;



    private:
        RM_Manager& rmm;
        IX_Manager& ixm;
        char cwd[1024];
        bool dbopen = false;
        RM_FileHandle relfh;
        RM_FileHandle attrfh;
};

// Internal errors
#define SM_NULLDB       (START_SM_ERR - 1) 
#define SM_HASOPEN      (START_SM_ERR - 2)
#define SM_BADOPEN      (START_SM_ERR - 3)
#define SM_NOTOPEN      (START_SM_ERR - 4)
#define SM_BADTABLE     (START_SM_ERR - 5) 
#define SM_BADATTR      (START_SM_ERR - 6) //传入的参数有点问题  
#define SM_NOSUCHTABLE  (START_SM_ERR - 7)
#define SM_INDEXEXISTS  (START_SM_ERR - 8)

#endif // SM_H

