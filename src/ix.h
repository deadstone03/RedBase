#ifndef IX_H
#define IX_H
#include "redbase.h"
#include "pf.h"
#include "rm_rid.h"
#include "ix_internal.h"

class IX_IndexHandle;
class IX_PageHandle {
  friend class IX_IndexHandle;
public:
  IX_PageHandle();
  ~IX_PageHandle();
  RC InsertEntry(void *pData, const RID &rid);
  RC DeleteEntry(void *pData, const RID &rid);
  RC Split(IX_PageHandle *ixPageHandle);
  RC GetData(char* &pData);
  RC GetPageNum(PageNum &pageNum);
private:
  int IsValidSlotNum(SlotNum slotNum) const;
  RC SetThisSlot(const SlotNum slotNum, const void* pData, const RID &rid);
  RC GetThisSlot(const SlotNum slotNum, void* &pData, RID &rid) const;
  RC GetLastSlot(void* &pData, RID &rid) const;
  RC GetNextSlot(const void* pData, void* &pNextData, RID &rid) const;
  RC Split(IX_PageHandle &ixPageHandle);
  IX_PageHdr* phdr;
  void* pData;
  int slotSize;
  AttrType attrType;
  int attrLength;
  int m;
  PageNum pageNum;
};


class IX_IndexHandle {
  friend class IX_Manager;
  friend class IX_IndexScan;
public:
  IX_IndexHandle  ();                             // Constructor
  ~IX_IndexHandle ();                             // Destructor
  RC InsertEntry     (void *pData, const RID &rid);  // Insert new index entry
  RC DeleteEntry     (void *pData, const RID &rid);  // Delete index entry
  RC ForcePages      ();                             // Copy index to disk
private:
  RC GetPageHandle(const PF_PageHandle& pfPageHandle,
                   IX_PageHandle& ixPageHandle);
  RC DoInsertEntry(void *pData, const RID &rid, PageNum &pageNum, void* pNewData, RID &newRid);
  IX_FileHdr hdr;
  int hdrChange;
  PF_FileHandle pffh;
};


class IX_IndexScan {
public:
  IX_IndexScan  ();                                 // Constructor
  ~IX_IndexScan ();                                 // Destructor
  RC OpenScan      (const IX_IndexHandle &indexHandle, // Initialize index scan
                    CompOp      compOp,
                    void        *value,
                    ClientHint  pinHint = NO_HINT);
  RC GetNextEntry  (RID &rid);                         // Get next matching entry
  RC CloseScan     ();                                 // Terminate index scan
};


class IX_Manager {
public:
  IX_Manager   (PF_Manager &pfm);              // Constructor
  ~IX_Manager  ();                             // Destructor
  RC CreateIndex  (const char *fileName,          // Create new index
                   int        indexNo,
                   AttrType   attrType,
                   int        attrLength);
  RC DestroyIndex (const char *fileName,          // Destroy index
                   int        indexNo);
  RC OpenIndex    (const char *fileName,          // Open index
                   int        indexNo,
                   IX_IndexHandle &indexHandle);
  RC CloseIndex   (IX_IndexHandle &indexHandle);  // Close index
private:
  RC WriteHdr(const IX_FileHdr &hdr, PF_FileHandle &pfFileHand);
  RC ReadHdr(const PF_FileHandle &pfFileHand, IX_FileHdr &hdr) const;
  PF_Manager* ppfm;
};

#define START_IX_WARN  201
#define IX_PAGE_SPLITTED ((START_IX_WARN) + 1)
#define IX_PAGE_FULL ((START_IX_WARN) + 2)
#define IX_INVALID_SLOTNUM ((START_IX_WARN) + 3)
#define IX_INVALID_SLOT ((START_IX_WARN) + 4)
#define IX_LASTWARN IX_INVALID_SLOT

void IX_PrintError (RC rc, unsigned int line, const char* filename);
#endif
