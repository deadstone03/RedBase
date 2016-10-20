#include<cstring>
#include "rm.h"
#include "rm_internal.h"

RM_FileHandle::RM_FileHandle() {
}

RM_FileHandle::~RM_FileHandle() {
}

// Get a record from the file.
// The record will have a copy of data.
RC RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const {
  RC rc;
  RM_PageHandle pageHandle;
  PageNum pageNum;
  SlotNum slotNum;
  if ((rc = rid.GetPageNum(pageNum))) {
    return rc;
  }
  if ((rc = rid.GetSlotNum(slotNum))) {
    return rc;
  }
  if ((rc = this->GetPage(pageNum, pageHandle))) {
    return rc;
  }
  if ((rc = pageHandle.GetRecord(slotNum, rec))) {
    return rc;
  }
  if ((rc = this->pffh.UnpinPage(pageNum))) {
    return rc;
  }
  return 0;
}

RC RM_FileHandle::GetNextRec(const RID &rid, RM_Record &rec) const {
  PageNum pageNum;
  SlotNum slotNum;
  RC rc;
  if ((rc = rid.GetPageNum(pageNum))) {
    return rc;
  }
  if ((rc = rid.GetSlotNum(slotNum))) {
    return rc;
  }
  PF_PageHandle pfPageHandle;
  while(!(rc = this->pffh.GetNextPage(pageNum - 1, pfPageHandle))) {
    RM_PageHandle rmPageHandle;
    if ((rc = this->GetPage(pfPageHandle, rmPageHandle))) {
      return rc;
    }
    if ((rc = rmPageHandle.GetNextRecord(slotNum, rec)
         && rc != RM_PAGE_EOF)) {
      return rc;
    }
    if (!rc) {
      // find one;
      return 0;
    }
    pageNum += 1;
  }
  if (rc == PF_EOF) {
    return RM_EOF;
  }
  return rc;
}

RC RM_FileHandle::InsertRec(const char *pData, RID &rid) {
  RC rc;
  PageNum pageHasFree = this->hdr.firstFree;
  RM_PageHandle pageHandle;
  if (pageHasFree == INVALID_PAGE) {
    // no free page
    if ((rc = this->NewPage(pageHandle))) {
      return rc;
    }
    if ((rc = pageHandle.GetPageNum(pageHasFree))) {
      return rc;
    }
    // insert the new page to the free list
    pageHandle.phdr->nextHasFree = this->hdr.firstFree;
    this->hdr.firstFree = pageHasFree;
  } else {
    if ((rc = this->GetPage(pageHasFree, pageHandle))) {
      return rc;
    }
  }
  RID newRid(pageHasFree, INVALID_SLOT);
  if ((rc = pageHandle.InsertRecord(pData, newRid))) {
    return rc;
  }
  rid = newRid;
  if (pageHandle.IsPageFull()) {
    // remove the full page from the has free
    this->hdr.firstFree = pageHandle.phdr->nextHasFree;
    pageHandle.phdr->nextHasFree = INVALID_PAGE;
  }
  if ((rc = this->pffh.MarkDirty(pageHasFree))) {
    return rc;
  }
  return pffh.UnpinPage(pageHasFree);
}

RC RM_FileHandle::DeleteRec(const RID &rid) {
  RC rc;
  RM_PageHandle pageHandle;
  PageNum pageNum;
  if ((rc = rid.GetPageNum(pageNum))) {
    return rc;
  }
  if ((rc = this->GetPage(pageNum, pageHandle))) {
    return rc;
  }

  int wasFull = pageHandle.IsPageFull();
  if ((rc = pageHandle.DeleteRecord(rid))) {
    return rc;
  }
  if (wasFull) {
    // insert the page to has free
    pageHandle.phdr->nextHasFree = this->hdr.firstFree;
    this->hdr.firstFree = pageNum;
  }
  if ((rc = this->pffh.MarkDirty(pageNum))) {
    return rc;
  }
  return this->pffh.UnpinPage(pageNum);
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec) {
  RC rc;
  RID rid;
  RM_PageHandle pageHandle;
  PageNum pageNum;
  if ((rc = rec.GetRid(rid))) {
    return rc;
  }
  if ((rc = rid.GetPageNum(pageNum))) {
    return rc;
  }
  if ((rc = this->GetPage(pageNum, pageHandle))) {
    return rc;
  }
  if ((rc = pageHandle.UpdateRecord(rec))) {
    return rc;
  }
  if ((rc = this->pffh.MarkDirty(pageNum))) {
    return rc;
  }
  return this->pffh.UnpinPage(pageNum);
}

RC RM_FileHandle::ForcePages(PageNum pageNum) {
  return this->pffh.ForcePages(pageNum);
}

RC RM_FileHandle::GetPage(const PageNum &pageNum, RM_PageHandle& pageHandle) const {
  RC rc;
  PF_PageHandle pfph;
  // get the pf page
  if ((rc = pffh.GetThisPage(pageNum, pfph))) {
    return rc;
  }
  if ((rc = this->GetPage(pfph, pageHandle))) {
    return rc;
  }
  return 0;
}

//get a rm pagehandle from a pf page handle
RC RM_FileHandle::GetPage(const PF_PageHandle& pfPageHandle, RM_PageHandle &rmPageHandle) const {
  RC rc;
  PageNum pageNum;
  char* pWholeData;
  if ((rc = pfPageHandle.GetData(pWholeData))) {
    return rc;
  }
  if ((rc = pfPageHandle.GetPageNum(pageNum))) {
    return rc;
  }
  rmPageHandle.phdr = (RM_PageHdr*)pWholeData;
  // we save 1 char extra for bitmap, 1 record should have 1 bit
  rmPageHandle.slotsPerPage = (PF_PAGE_SIZE - sizeof(RM_PageHdr) - 1)  * CHAR_BYTE_SIZE
      / (1 + this->hdr.recordSize * CHAR_BYTE_SIZE);
  rmPageHandle.bitmap = pWholeData + sizeof(RM_PageHdr);
  rmPageHandle.bitmapLen =
      (rmPageHandle.slotsPerPage + CHAR_BYTE_SIZE - 1) / CHAR_BYTE_SIZE;
  rmPageHandle.pData = pWholeData + sizeof(RM_PageHdr) + rmPageHandle.bitmapLen;
  rmPageHandle.pageNum = pageNum;
  rmPageHandle.recordSize = this->hdr.recordSize;
  return 0;
}

RC RM_FileHandle::NewPage(RM_PageHandle &pageHandle) {
  RC rc;
  PF_PageHandle pfPageHandle;
  if ((rc = this->pffh.AllocatePage(pfPageHandle))) {
    return rc;
  }
  RM_PageHandle rmPageHandle;
  if ((rc = this->GetPage(pfPageHandle, rmPageHandle))) {
    return rc;
  }
  // initialize the new page
  memset((void*)pageHandle.bitmap, 0, pageHandle.bitmapLen);
  pageHandle.phdr->nextHasFree = INVALID_PAGE;
  pageHandle.phdr->slotCount = 0;
  return 0;
}
