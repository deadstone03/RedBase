#include "rm_internal.h"

#define RM_INVALID_PAGE (-1)
#define CHAR_BYTE_SIZE (sizeof(char))
#define CHAR_BIT_SIZE (CHAR_BYTE_SIZE * 8)

RM_PageHandle::RM_PageHandle()
: pageNum(RM_INVALID_PAGE) {
}

RM_PageHandle::~RM_PageHandle() {
}

RC RM_PageHandle::GetPageNum(PageNum &pageNum) const {
  if (pageNum == RM_INVALID_PAGE) {
    return RM_PAGE_NOTINIT;
  }
  pageNum = this->pageNum;
  return (0);
}

int RM_PageHandle::IsValidSlotNum(SlotNum slotNum) const {
  return 0 <= slotNum && slotNum < this->slotsPerPage;
}

RC RM_PageHandle::GetRecord(RID rid, RM_Record &record) const {

  RC rc;
  SlotNum slotNum;
  if ((rc = rid.GetSlotNum(slotNum))) {
    return rc;
  }

  if (!this->IsValidSlotNum(slotNum)) {
    return RM_INVALID_SLOTNUM;
  }
  
  if (!this->IsValidSlot(slotNum)) {
    return RM_INVALID_SLOT;
  }

  record.recordSize = recordSize;
  record.pData = pData + this->recordSize * slotNum;
  record.rid = rid;

}

int RM_PageHandle::IsValidRecord(SlotNum slotNum) const {
  return bitmap[slotNum / CHAR_BIT_SIZE] & (0x1 << slotNum % CHAR_BIT_SIZE);
}
