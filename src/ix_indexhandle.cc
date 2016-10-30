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
    ixPageHandle.phdr->isLeaf = FALSE;
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

RC IX_IndexHandle::DoInsertEntry(void *pData, const RID &rid, IX_PageHandle& ixPageHandle) {
  RC rc;
  if (ixPageHandle.phdr->isLeaf) {
    if (ixPageHandle.phdr->slotCount < this->hdr.m) {
      // leaf has empty slot
      char* pPageData;
      if ((rc = ixPageHandle.GetData(pPageData))) {
        return rc;
      }
      // move to the first empty slot
      pPageData += (this->hdr.slotSize * ixPageHandle.phdr->slotCount);
      memcpy(pPageData, pData, this->hdr.attrLength);
      pPageData += this->hdr.attrLength;
      memcpy(pPageData, &rid, sizeof(RID));
      ixPageHandle.phdr->slotCount++;
    } else {
      // leaf but there is no empty slot
    }
  }
}

RC IX_IndexHandle::GetPageHandle(const PF_PageHandle& pfPageHandle,
                                 IX_PageHandle& ixPageHandle) {
  RC rc;
  char* pData;
  if ((rc = pfPageHandle.GetData(pData))) {
    return rc;
  }
  ixPageHandle.phdr = (IX_PageHdr*)pData;
  ixPageHandle.pData = pData + sizeof(IX_PageHdr);
  return 0;
}
