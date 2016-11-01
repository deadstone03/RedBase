#include "ix.h"
#include "ix_internal.h"
#include <cstring>

IX_PageHandle::IX_PageHandle() {
}

IX_PageHandle::~IX_PageHandle() {
}

RC IX_PageHandle::InsertEntry(void *pData, const RID &rid) {
  if (this->phdr->slotCount == this->m) {
    return IX_PAGE_FULL;
  }
  // insert should be keep the order.
  int i;
  for (i = 0; i < this->phdr->slotCount; ++i) {
    if (compare(pData, this->pData + i * this->slotSize,
                this->attrType, this-> attrLength) <= 0) {
      break;
    }
  }
  memmove(this->pData + (i + 1) * this->slotSize,
          this->pData + i * this->slotSize,
          (this->phdr->slotCount - i) * this->slotSize);
  this->phdr->slotCount++;
  return this->SetThisSlot(i, pData, rid);
}

RC IX_PageHandle::GetPageNum(PageNum &pageNum) {
  pageNum = this->pageNum;
  return 0;
}

RC IX_PageHandle::Split(IX_PageHandle &newIxPageHandle) {
  RC rc;
  PageNum newPageNum;
  if ((rc = newIxPageHandle.GetPageNum(newPageNum))) {
    return rc;
  }
  newIxPageHandle.phdr->isRoot = FALSE;
  newIxPageHandle.phdr->sibling = this->phdr->sibling;
  this->phdr->sibling = newPageNum;
  newIxPageHandle.phdr->isLeaf = this->phdr->isLeaf;

  int leftSize = this->phdr->slotCount / 2;
  int rightSize = this->phdr->slotCount - leftSize;
  memcpy(newIxPageHandle.pData,
         this->pData + leftSize * this->slotSize,
         rightSize * this->slotSize);
  this->phdr->slotCount = leftSize;
  newIxPageHandle.phdr->slotCount = rightSize;
  return 0;
}

RC IX_PageHandle::GetThisSlot(SlotNum slotNum, void* &pData, RID &rid) const {
  if (!this->IsValidSlotNum(slotNum)) {
    return IX_INVALID_SLOTNUM;
  }
  pData = new char[this->attrLength];
  memcpy(pData, this->pData + this->slotSize * slotNum, this->attrLength);
  memcpy(&rid, this->pData + this->slotSize * slotNum + this->attrLength,
         sizeof(RID));
  return 0;
}

RC IX_PageHandle::GetLastSlot(void* &pData, RID &rid) const {
  return this->GetThisSlot(this->phdr->slotCount, pData, rid);
}

RC IX_PageHandle::SetThisSlot(SlotNum slotNum,
                              const void* pData,
                              const RID &rid) {
  if (!this->IsValidSlotNum(slotNum)) {
    return IX_INVALID_SLOTNUM;
  }
  memcpy(this->pData + this->slotSize * slotNum, pData, this->attrLength);
  memcpy(this->pData + this->slotSize * slotNum + this->attrLength,
         &rid,
         sizeof(RID));
  return 0;
}

// To handle duplicate values, there are a few thing we need to do.
// A slot points to a page. the correspond value will be the new value in it's
// child page. The 'new' means the first value in the child page which not equal
// to the previous page's last value.
// If the whole child page is the same values, then current value should be some
// dummy value. But it's hard to set up a dummy value since every possible value
// will be used. But for internal pages, the rid's slot part is actually not
// used, we can use it to mark dummy value. Normall rid's slot will be
// NON_DUMMY, dummy one will be DUMMY.
RC IX_PageHandle::GetNextSlot(const void* pData, void* &pNextData, RID &rid) const {
  int i;
  void* pSlotData;
  RID* pSlotRid;
  RC rc;
  for (i = 0; i < this->phdr->slotCount - 1; ++i) {
    pSlotData = this->pData + this->slotSize * i;
    pSlotRid = (RID*)(this->pData + this->slotSize * i + this->attrLength);
    SlotNum slotNum;
    if ((rc = pSlotRid->GetSlotNum(slotNum)) &&
        rc != RM_RID_NOTINIT) {
      return rc;
    }
    if (slotNum == IX_DUMMY) {
      continue;
    }
    if (compare(pData, pSlotData, this->attrType, this->attrLength) < 0) {
      continue;
  }
}

int IX_PageHandle::IsValidSlotNum(SlotNum slotNum) const {
  return slotNum >=0 && slotNum < this->phdr->slotCount;
}
