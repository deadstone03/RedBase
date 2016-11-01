#include "ix.h"
#include <cstring>

IX_IndexHandle::IX_IndexHandle() {
}

IX_IndexHandle::~IX_IndexHandle() {
}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
  RC rc;
  PageNum pageNum;
  IX_PageHandle ixPageHandle;
  PF_PageHandle pfPageHandle;
  if (this->hdr.root == INVALID_PAGE) {
    // create root and insert in root;
    if ((rc = this->pffh.AllocatePage(pfPageHandle))) {
      return rc;
    }
    if ((rc = this->GetPageHandle(pfPageHandle, ixPageHandle))) {
      return rc;
    }
    ixPageHandle.phdr->parent = INVALID_PAGE;
    ixPageHandle.phdr->sibling = INVALID_PAGE;
    ixPageHandle.phdr->slotCount = 0;
    // initially the node is both leaf and root
    ixPageHandle.phdr->isLeaf = TRUE;
    ixPageHandle.phdr->isRoot = TRUE;
    if ((rc = pfPageHandle.GetPageNum(pageNum))) {
      return rc;
    }
    this->hdr.root = pageNum;
    this->hdrChange = TRUE;
  } else {
    // get the root page
    if ((rc = this->pffh.GetThisPage(this->hdr.root, pfPageHandle))) {
      return rc;
    }
    pageNum = this->hdr.root;
    if ((rc = this->GetPageHandle(pfPageHandle, ixPageHandle))) {
      return rc;
    }
  }
}

RC IX_IndexHandle::DoInsertEntry(void *pData, const RID &rid, PageNum &pageNum, void* pNewData, RID &newRid) {
  RC rc;
  PF_PageHandle pfPageHandle;
  IX_PageHandle ixPageHandle;
  void* pLeftLastData = NULL;
  RID leftLastRid;
  void* pNextData = NULL;
  RID nextRid;
  if ((rc = this->pffh.GetThisPage(pageNum, pfPageHandle))) {
    return rc;
  }
  if ((rc = this->GetPageHandle(pfPageHandle, ixPageHandle))) {
    return rc;
  }
  if (ixPageHandle.phdr->isLeaf) {
    if ((rc = ixPageHandle.InsertEntry(pData, rid))
        && rc != IX_PAGE_FULL) {
      return rc;
    }
    // the page is full
    if (!rc) {
      IX_PageHandle newIxPageHandle;
      PF_PageHandle newPfPageHandle;
      PageNum newPageNum;
      if ((rc = this->pffh.AllocatePage(newPfPageHandle))) {
        return rc;
      }
      if ((rc = newPfPageHandle.GetPageNum(newPageNum))) {
        return rc;
      }
      if ((rc = this->GetPageHandle(newPfPageHandle, newIxPageHandle))) {
        return rc;
      }
      if ((rc = ixPageHandle.Split(newIxPageHandle))) {
        return rc;
      }
      if ((rc = ixPageHandle.GetLastSlot(pLeftLastData, leftLastRid))) {
        goto DoInsertEntryErrorHandle;
      }
      if (compare(pLeftLastData, pData,
                  this->hdr.attrType, this->hdr.attrLength) < 0) {
        if((rc = newIxPageHandle.InsertEntry(pData, rid))) {
          goto DoInsertEntryErrorHandle;
        }
      } else {
        if ((rc = ixPageHandle.InsertEntry(pData, rid))) {
          goto DoInsertEntryErrorHandle;
        }
      }
      if ((rc = newIxPageHandle.GetNextSlot(pLeftLastData, pNextData, nextRid))
           && rc != IX_INVALID_SLOT) {
        goto DoInsertEntryErrorHandle;
      }
      if (!rc) {
        newRid = RID(newPageNum, IX_DUMMY);
      } else {
        newRid = RID(newPageNum, IX_NON_DUMMY);
      }
      pNewData = pNextData;
      rc = 0;
      goto DoInsertEntryErrorHandle;
    }
  }


DoInsertEntryErrorHandle:
  if (pLeftLastData) {
    delete (char*)pLeftLastData;
    pLeftLastData = NULL;
  }
  return rc;

}

RC IX_IndexHandle::GetPageHandle(const PF_PageHandle& pfPageHandle,
                                 IX_PageHandle& ixPageHandle) {
  RC rc;
  char* pData;
  PageNum pageNum;
  if ((rc = pfPageHandle.GetData(pData))) {
    return rc;
  }
  if ((rc = pfPageHandle.GetPageNum(pageNum))) {
    return rc;
  }
  ixPageHandle.pageNum = pageNum;
  ixPageHandle.phdr = (IX_PageHdr*)pData;
  ixPageHandle.m = this->hdr.m;
  ixPageHandle.attrType = this->hdr.attrType;
  ixPageHandle.attrLength = this->hdr.attrLength;
  ixPageHandle.slotSize = this->hdr.slotSize;
  ixPageHandle.pData = pData + sizeof(IX_PageHdr);
  return 0;
}
