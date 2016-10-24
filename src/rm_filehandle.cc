#include <cstring>
#include "rm.h"
#include "rm_internal.h"
#include <iostream>

RM_FileHandle::RM_FileHandle() {
}

RM_FileHandle::~RM_FileHandle() {
}

RM_FileHandle::RM_FileHandle(const RM_FileHdr& fileHdr,
                             int hdrChange,
                             PF_FileHandle pffh)
:hdr(fileHdr), hdrChange(hdrChange), pffh(pffh) {
}

RM_FileHandle::RM_FileHandle(const RM_FileHandle& other)
:hdr(other.hdr), hdrChange(other.hdrChange), pffh(other.pffh) {

}

RM_FileHandle* RM_FileHandle::operator=(const RM_FileHandle& other) {
  this->hdr = other.hdr;
  this->hdrChange = other.hdrChange;
  this->pffh = other.pffh;
  return this;
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
  if ((rc = this->pffh.UnpinPage(this->GetRealPageNum(pageNum)))) {
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
  while(!(rc = this->pffh.GetNextPage(
      this->GetRealPageNum(pageNum - 1), pfPageHandle))) {
    RM_PageHandle rmPageHandle;
    if ((rc = this->GetPage(pfPageHandle, rmPageHandle))) {
      return rc;
    }
    if ((rc = rmPageHandle.GetNextRecord(slotNum, rec)
         && rc != RM_PAGE_EOF)) {
      return rc;
    }
    if ((rc = this->pffh.UnpinPage(this->GetRealPageNum(pageNum)))) {
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

// insert a record
// in: pData
// out: rid
RC RM_FileHandle::InsertRec(const char *pData, RID &rid) {
  RC rc;
  PageNum pageHasFree = this->hdr.firstFree;
  RM_PageHandle pageHandle;
  if (pageHasFree == INVALID_PAGE) {
    // no free page
    if ((rc = this->NewPage(pageHandle))) {
      RM_PrintError(rc, __LINE__, __FILE__);
      return rc;
    }
    if ((rc = pageHandle.GetPageNum(pageHasFree))) {
      RM_PrintError(rc, __LINE__, __FILE__);
      return rc;
    }
    // insert the new page to the free list
    pageHandle.phdr->nextHasFree = this->hdr.firstFree;
    this->hdr.firstFree = pageHasFree;
    this->hdrChange = TRUE;
  } else {
    if ((rc = this->GetPage(pageHasFree, pageHandle))) {
      RM_PrintError(rc, __LINE__, __FILE__);
      return rc;
    }
  }
  RID newRid(pageHasFree, INVALID_SLOT);
  if ((rc = pageHandle.InsertRecord(pData, newRid))) {
    RM_PrintError(rc, __LINE__, __FILE__);
    return rc;
  }
  rid = newRid;
  if (pageHandle.IsPageFull()) {
    // remove the full page from the has free
    this->hdr.firstFree = pageHandle.phdr->nextHasFree;
    this->hdrChange = TRUE;
    pageHandle.phdr->nextHasFree = INVALID_PAGE;
  }
  if ((rc = this->pffh.MarkDirty(this->GetRealPageNum(pageHasFree)))) {
    RM_PrintError(rc, __LINE__, __FILE__);
    return rc;
  }
  return pffh.UnpinPage(this->GetRealPageNum(pageHasFree));
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
    this->hdrChange = TRUE;
  }
  if ((rc = this->pffh.MarkDirty(this->GetRealPageNum(pageNum)))) {
    return rc;
  }
  return this->pffh.UnpinPage(this->GetRealPageNum(pageNum));
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
  if ((rc = this->pffh.MarkDirty(this->GetRealPageNum(pageNum)))) {
    return rc;
  }
  return this->pffh.UnpinPage(this->GetRealPageNum(pageNum));
}

RC RM_FileHandle::ForcePages(PageNum pageNum) {
  if (pageNum == ALL_PAGES) {
    return this->pffh.ForcePages(ALL_PAGES);
  }
  return this->pffh.ForcePages(this->GetRealPageNum(pageNum));
}

RC RM_FileHandle::GetPage(const PageNum &pageNum, RM_PageHandle& pageHandle) const {
  RC rc;
  PF_PageHandle pfph;
  // get the pf page
  if ((rc = pffh.GetThisPage(this->GetRealPageNum(pageNum), pfph))) {
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
  PageNum realPageNum;
  char* pWholeData;
  if ((rc = pfPageHandle.GetData(pWholeData))) {
    PF_PrintError(rc);
    return rc;
  }
  if ((rc = pfPageHandle.GetPageNum(realPageNum))) {
    return rc;
  }
  rmPageHandle.phdr = (RM_PageHdr*)pWholeData;
  // we save 1 char extra for bitmap, 1 record should have 1 bit
  rmPageHandle.slotsPerPage = (PF_PAGE_SIZE - sizeof(RM_PageHdr) - 1) * CHAR_BIT_SIZE
      / (1 + this->hdr.recordSize * CHAR_BIT_SIZE);

  rmPageHandle.bitmap = pWholeData + sizeof(RM_PageHdr);
  rmPageHandle.bitmapLen =
      (rmPageHandle.slotsPerPage + CHAR_BIT_SIZE - 1) / CHAR_BIT_SIZE;

  rmPageHandle.pData = pWholeData + sizeof(RM_PageHdr) + rmPageHandle.bitmapLen;
  rmPageHandle.pageNum = this->GetLogicPageNum(realPageNum);
  rmPageHandle.recordSize = this->hdr.recordSize;
  return 0;
}

RC RM_FileHandle::NewPage(RM_PageHandle &pageHandle) {
  RC rc;
  PF_PageHandle pfPageHandle;
  if ((rc = this->pffh.AllocatePage(pfPageHandle))) {
    PF_PrintError(rc);
    return rc;
  }
  if ((rc = this->GetPage(pfPageHandle, pageHandle))) {
    PF_PrintError(rc);
    return rc;
  }
  // initialize the new page
  memset((void*)pageHandle.bitmap, 0, pageHandle.bitmapLen);
  pageHandle.phdr->nextHasFree = INVALID_PAGE;
  pageHandle.phdr->slotCount = 0;
  return 0;
}

// the first page to store record starts from head page + 1
PageNum RM_FileHandle::GetRealPageNum(const PageNum pageNum) const {
  return this->hdr.hdrPageNum + 1 + pageNum;
}

PageNum RM_FileHandle::GetLogicPageNum(const PageNum pageNum) const {
  return pageNum - 1 - this->hdr.hdrPageNum;
}
