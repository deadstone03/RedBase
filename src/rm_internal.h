#include "rm.h"

struct RM_FileHdr {
  int recordSize;
  SlotNum slotsPerPage;  //num of slots per page 
};

struct RM_PageHdr {
  SlotNum slotCount;  // num of used slots
};


class RM_PageHandle {
  friend class RM_FileHandle;
public:
  RM_PageHandle();
  ~RM_PageHandle();
  RC GetData(char *&pData) const;
  RC GetPageNum(PageNum &pageNum) const;
  RC GetRecord(RID rid, RM_Record &record) const;
private:
  PageNum pageNum;
  PF_PageHandle pfph;
  RM_PageHdr *phdr;
  char* bitmap;
  char* pData;
  SlotNum slotsPerPage;
  int recordSize;
  int IsValidSlotNum(SlotNum slotNum) const;
  int IsValidSlot(SlotNum slotNum) const;
};
