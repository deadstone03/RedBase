#include "rm.h"
#include "rm_internal.h"
#include<cstring>

RM_PageHandle::RM_PageHandle() {
}

RM_PageHandle::~RM_PageHandle() {
}

RM_PageHandle::RM_PageHandle(
      PageNum pageNum, RM_PageHdr* phdr,
      char* bitmap, int bitmapLen, char* pData,
      SlotNum slotsPerPage, int recordSize)
: pageNum(pageNum), phdr(phdr), bitmap(bitmap),
bitmapLen(bitmapLen), pData(pData), slotsPerPage(slotsPerPage), recordSize(recordSize) {
}

RM_PageHandle::RM_PageHandle(const RM_PageHandle & other)
: pageNum(other.pageNum), phdr(other.phdr), bitmap(other.bitmap),
bitmapLen(other.bitmapLen), pData(other.pData), slotsPerPage(other.slotsPerPage), recordSize(other.recordSize) {
}

RM_PageHandle* RM_PageHandle::operator=(const RM_PageHandle &other) {
  this->pageNum = other.pageNum;
  this->phdr = other.phdr;
  this->bitmap = other.bitmap;
  this->bitmapLen = other.bitmapLen;
  this->pData = other.pData;
  this->slotsPerPage = other.slotsPerPage;
  this->recordSize = other.recordSize;
  return this;
}

RC RM_PageHandle::GetPageNum(PageNum &pageNum) const {
  if (this->pageNum == RM_INVALID_PAGE) {
    return RM_PAGE_NOTINIT;
  }
  pageNum = this->pageNum;
  return (0);
}

RC RM_PageHandle::GetRecord(const SlotNum& slotNum, RM_Record &record) const {

  if (!this->IsValidSlotNum(slotNum)) {
    return RM_INVALID_SLOTNUM;
  }

  if (!this->IsValidSlot(slotNum)) {
    return RM_INVALID_SLOT;
  }

  record.recordSize = this->recordSize;
  // the data will be copied
  record.pData = new char[this->recordSize];
  memcpy(record.pData, this->pData + this->recordSize * slotNum, this->recordSize);
  record.rid = RID(this->pageNum, slotNum);
  return (0);
}

RC RM_PageHandle::GetNextRecord(const SlotNum &slotNum, RM_Record &record) const {
  for (SlotNum i = slotNum + 1; this->IsValidSlotNum(i); ++i) {
    if (this->IsValidSlot(i)) {
      return this->GetRecord(i, record);
    }
  }
  return RM_PAGE_EOF;
}

// Insert a new record data, out rid.
RC RM_PageHandle::InsertRecord(const char *pData, RID &rid) {
  RC rc;
  SlotNum i;
  // find the first usable slot
  for (i = 0; i < this->slotsPerPage; ++i) {
    if (!this->IsValidSlot(i)) {
      break;
    }
  }
  if (!this->IsValidSlotNum(i)) {
    return RM_NOSLOT_INPAGE;
  }

  if ((rc = this->WriteRecord(pData, i))) {
    return rc;
  }
  RID newRid(this->pageNum, i);
  rid = newRid;
  return 0;
}

RC RM_PageHandle::DeleteRecord(const RID &rid) {
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
  // clear the bit
  bitmap[slotNum / CHAR_BIT_SIZE] &= ~(0x1 << slotNum % CHAR_BIT_SIZE);
  this->phdr->slotCount--;
  return 0;
}

RC RM_PageHandle::UpdateRecord(const RM_Record &rec) {
  RC rc;
  RID rid;
  SlotNum slotNum;
  char* pData;
  if ((rc = rec.GetRid(rid))) {
    return rc;
  }
  if ((rc = rid.GetSlotNum(slotNum))) {
    return rc;
  }
  if (!this->IsValidSlotNum(slotNum)) {
    return RM_INVALID_SLOTNUM;
  }
  if (!this->IsValidSlot(slotNum)) {
    return RM_INVALID_SLOT;
  }
  if ((rc = rec.GetData(pData))) {
    return rc;
  }
  // update the data
  memcpy(this->pData + slotNum * this->recordSize, pData, this->recordSize);
  return 0;
}

RC RM_PageHandle::WriteRecord(const char *pData, const SlotNum slotNum) {
  // set the bit
  bitmap[slotNum / CHAR_BIT_SIZE] |= (0x1 << slotNum % CHAR_BIT_SIZE);
  // write the data
  memcpy(this->pData + slotNum * this->recordSize, pData, this->recordSize);
  this->phdr->slotCount++;
  return 0;
}

int RM_PageHandle::IsValidSlot(SlotNum slotNum) const {
  return bitmap[slotNum / CHAR_BIT_SIZE] & (0x1 << slotNum % CHAR_BIT_SIZE);
}

int RM_PageHandle::IsValidSlotNum(SlotNum slotNum) const {
  return 0 <= slotNum && slotNum < this->slotsPerPage;
}

int RM_PageHandle::IsPageFull() const {
  return this->slotsPerPage == this->phdr->slotCount;
}

int RM_PageHandle::IsPageEmpty() const {
  return this->phdr->slotCount == 0;
}
