//
// rm.h
//
//   Record Manager component interface
//
// This file does not include the interface for the RID class.  This is
// found in rm_rid.h
//

#ifndef RM_H
#define RM_H

// Please DO NOT include any files other than redbase.h and pf.h in this
// file.  When you submit your code, the test program will be compiled
// with your rm.h and your redbase.h, along with the standard pf.h that
// was given to you.  Your rm.h, your redbase.h, and the standard pf.h
// should therefore be self-contained (i.e., should not depend upon
// declarations in any other file).

// Do not change the following includes
#include "redbase.h"
#include "rm_rid.h"
#include "pf.h"
#include "rm_internal.h"

#define CHAR_BYTE_SIZE (sizeof(char))
#define CHAR_BIT_SIZE (CHAR_BYTE_SIZE * 8)

const PageNum RM_INVALID_PAGE = -1;
const RID INVALID_RID;
const int INVALID_RECORDSIZE = -1;
//
// RM_Record: RM Record interface
//
class RM_PageHandle;

class RM_Record {
  friend class RM_PageHandle;
public:
    RM_Record();
    ~RM_Record();

    // Return the data corresponding to the record.  Sets *pData to the
    // record contents.
    RC GetData(char *&pData) const;

    // Return the RID associated with the record
    RC GetRid (RID &rid) const;
private:
    RID rid;
    char* pData;
    int recordSize;
};

// RM_PageHandle: RM Page interface
class RM_PageHandle {
  friend class RM_FileHandle;
public:
  RM_PageHandle();
  ~RM_PageHandle();
  RC GetPageNum(PageNum &pageNum) const;
  RC GetRecord(const SlotNum &slotNum, RM_Record &record) const;
  RC InsertRecord(const char *pData, RID &rid);
  RC DeleteRecord(const RID &rid);
  RC UpdateRecord(const RM_Record &rec);
private:
  PageNum pageNum;
  RM_PageHdr* phdr;
  char* bitmap;
  int bitmapLen;
  char* pData;
  SlotNum slotsPerPage;
  int recordSize;
  RC GetNextRecord(const SlotNum &slotNum, RM_Record &record) const;
  int IsValidSlotNum(SlotNum slotNum) const;
  int IsValidSlot(SlotNum slotNum) const;
  RC WriteRecord(const char *pData, SlotNum slotNum);
  int IsPageEmpty() const;
  int IsPageFull() const;
};

//
// RM_FileHandle: RM File interface
//
class RM_FileScan;
class RM_Manager;
class RM_FileHandle {
  friend class RM_FileScan;
  friend class RM_Manager;
public:
    RM_FileHandle ();
    ~RM_FileHandle();

    // Given a RID, return the record
    RC GetRec     (const RID &rid, RM_Record &rec) const;

    RC InsertRec  (const char *pData, RID &rid);       // Insert a new record

    RC DeleteRec  (const RID &rid);                    // Delete a record
    RC UpdateRec  (const RM_Record &rec);              // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages (PageNum pageNum = ALL_PAGES);
private:
    RC GetPage(const PageNum &pageNum, RM_PageHandle &pageHandle) const;
    RC GetPage(const PF_PageHandle &pfPageHandle, RM_PageHandle &pageHandle) const;
    RC NewPage(RM_PageHandle &pageHandle);
    RC GetNextRec(const RID &rid, RM_Record &rec) const;
    PageNum GetRealPageNum(const PageNum pageNum) const;
    RM_FileHdr hdr;
    int hdrChange;
    PF_FileHandle pffh;
};

//
// RM_FileScan: condition-based scan of records in the file
//
class RM_FileScan {
public:
    RM_FileScan  ();
    ~RM_FileScan ();

    RC OpenScan  (const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan ();                             // Close the scan
private:
    RC CheckRecord(const RM_Record &rec, int &check) const;
    int Compare(const void* pCond, const void* pVal) const;
    RM_FileHandle fileHandle;
    RID curRID;
    AttrType attrType;
    int attrLength;
    int attrOffset;
    CompOp compOp;
    void *value;
    ClientHint pinHint;
};

//
// RM_Manager: provides RM file management
//
class RM_Manager {
public:
    RM_Manager    (PF_Manager &pfm);
    ~RM_Manager   ();

    RC CreateFile (const char *fileName, int recordSize);
    RC DestroyFile(const char *fileName);
    RC OpenFile   (const char *fileName, RM_FileHandle &fileHandle);

    RC CloseFile  (RM_FileHandle &fileHandle);

private:
    RC GetFileHdr(const PF_FileHandle& pfFileHandle,
                  RM_FileHdr &rmFileHdr) const;
    RC WriteFileHdr(const PF_FileHandle& pfFileHandle,
                    const RM_FileHdr &rmFileHdr) const;
    PF_Manager pfm;
};

//
// Print-error function
//
void RM_PrintError(RC rc);

#define RM_RECORD_NOTINIT (RM_RID_LASTWARN + 1)
#define RM_PAGE_NOTINIT (RM_RID_LASTWARN + 2)
#define RM_INVALID_SLOTNUM (RM_RID_LASTWARN + 3)
#define RM_INVALID_SLOT (RM_RID_LASTWARN + 4)
#define RM_NOSLOT_INPAGE (RM_RID_LASTWARN + 5)
#define RM_PAGE_EOF (RM_RID_LASTWARN + 6)  // no more records in page
#define RM_EOF (RM_RID_LASTWARN + 7)  // no more records in file
#define RM_LASTWARN RM_EOF

#endif
