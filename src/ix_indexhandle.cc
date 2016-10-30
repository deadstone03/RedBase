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
  if ((rc = this->pffh.GetThisPage(pageNum, pfPageHandle))) {
    return rc;
  }
  if ((rc = this->GetPageHandle(pfPageHandle, ixPageHandle))) {
    return rc;
  }
  if (ixPageHandle.phdr->isLeaf) {
    // this page to insert is leaf
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
      if ((rc = this->pffh.MarkDirty(pageNum))) {
        return rc;
      }
      if ((rc = this->pffh.UnpinPage(pageNum))) {
        return rc;
      }
      return 0;
    } else {
      // leaf but there is no empty slot
      PF_PageHandle newPfPageHandle;
      IX_PageHandle newIxPageHandle;
      if ((rc = this->pffh.AllocatePage(newPfPageHandle))) {
        return rc;
      }
      PageNum newPageNum;
      if ((rc = newPfPageHandle.GetPageNum(newPageNum))) {
        return rc;
      }
      if ((rc = this->GetPageHandle(newPfPageHandle, newIxPageHandle))) {
        return rc;
      }
      newIxPageHandle.phdr->isRoot = FALSE;
      newIxPageHandle.phdr->sibling = ixPageHandle.phdr->sibling;
      newIxPageHandle.phdr->isLeaf = TRUE;
      newIxPageHandle.phdr->isRoot = FALSE;
      int leftSize = (this->hdr.m + 1) / 2;
      int rightSize = this->hdr.m - leftSize;
      char* pLeftPageData;
      char* pRightPageData;
      if ((rc = ixPageHandle.GetData(pLeftPageData))) {
        return rc;
      }
      if ((rc = newIxPageHandle.GetData(pRightPageData))) {
        return rc;
      }
      memcpy(pRightPageData,
             pLeftPageData + leftSize * this->hdr.slotSize,
             rightSize * this->hdr.slotSize);
      ixPageHandle.phdr->slotCount = leftSize;
      ixPageHandle.phdr->sibling = newPageNum;
      newIxPageHandle.phdr->slotCount = rightSize;

      if ((rc = newIxPageHandle.GetData(pRightPageData))) {
        return rc;
      }
      pNewData = new char[this->hdr.attrLength];
      memcpy(pNewData, pRightPageData, this->hdr.attrLength);

      if (compare(pData, pNewData, this->hdr.attrType, this->hdr.attrLength) < 0) {
        ixPageHandle.InsertEntry(pData, rid);
      } else {
        newIxPageHandle.InsertEntry(pData, rid);
      }
      // probably the inserted one is the right smallest
      memcpy(pNewData, pRightPageData, this->hdr.attrLength);
      newRid = RID(newPageNum, INVALID_SLOT);
      if ((rc = this->pffh.MarkDirty(pageNum))) {
        return rc;
      }
      if ((rc = this->pffh.MarkDirty(newPageNum))) {
        return rc;
      }
      if ((rc = this->pffh.UnpinPage(pageNum))) {
        return rc;
      }
      if ((rc = this->pffh.UnpinPage(newPageNum))) {
        return rc;
      }
      return IX_PAGE_SPLITTED;
    }
  } else {
    // the page is internal, maybe root
    char* pPageData;
    if (( rc = ixPageHandle.GetData(pPageData))) {
      return rc;
    }

    PageNum pageToInsert;
    int i;
    for (i = 0; i < ixPageHandle.phdr->slotCount - 1; ++i) {
      char* pSlot = pPageData + this->hdr.slotSize * i;
      if (compare(pData, pSlot, this->hdr.attrType, this->hdr.attrLength) < 0) {
        RID rid = *(RID*)(pSlot + this->hdr.attrLength);
        if ((rc = rid.GetPageNum(pageToInsert))) {
          return rc;
        }
        break;
      }
    }
    // the slot to insert is larger than all slots in this page
    if (i == ixPageHandle.phdr->slotCount - 1) {
      char* pSlot = pPageData + this->hdr.slotSize * i;
      RID rid = *(RID*)(pSlot + this->hdr.attrLength);
      if ((rc = rid.GetPageNum(pageToInsert))) {
        return rc;
      }
    }

    if ((rc = this->DoInsertEntry(
        pData, rid, pageToInsert, pNewData, newRid))
        && rc != IX_PAGE_SPLITTED) {
      return rc;
    }
    if (!rc) {
      return this->pffh.UnpinPage(pageNum);
    }
    // pageSplited
    if (ixPageHandle.phdr->slotCount < this->hdr.m) {
      // node has empty slot
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
      if ((rc = this->pffh.MarkDirty(pageNum))) {
        return rc;
      }
      if ((rc = this->pffh.UnpinPage(pageNum))) {
        return rc;
      }
      return 0;
    } else if (ixPageHandle.phdr->isRoot) {
    }
    else {
      // node but there is no empty slot
      PF_PageHandle newPfPageHandle;
      IX_PageHandle newIxPageHandle;
      if ((rc = this->pffh.AllocatePage(newPfPageHandle))) {
        return rc;
      }
      PageNum newPageNum;
      if ((rc = newPfPageHandle.GetPageNum(newPageNum))) {
        return rc;
      }
      if ((rc = this->GetPageHandle(newPfPageHandle, newIxPageHandle))) {
        return rc;
      }
      newIxPageHandle.phdr->isRoot = FALSE;
      newIxPageHandle.phdr->sibling = ixPageHandle.phdr->sibling;
      newIxPageHandle.phdr->isLeaf = TRUE;
      newIxPageHandle.phdr->isRoot = FALSE;
      int leftSize = (this->hdr.m + 1) / 2;
      int rightSize = this->hdr.m - leftSize;
      char* pLeftPageData;
      char* pRightPageData;
      if ((rc = ixPageHandle.GetData(pLeftPageData))) {
        return rc;
      }
      if ((rc = newIxPageHandle.GetData(pRightPageData))) {
        return rc;
      }
      memcpy(pRightPageData,
             pLeftPageData + leftSize * this->hdr.slotSize,
             rightSize * this->hdr.slotSize);
      ixPageHandle.phdr->slotCount = leftSize;
      ixPageHandle.phdr->sibling = newPageNum;
      newIxPageHandle.phdr->slotCount = rightSize;

      if ((rc = newIxPageHandle.GetData(pRightPageData))) {
        return rc;
      }
      pNewData = new char[this->hdr.attrLength];
      memcpy(pNewData, pRightPageData, this->hdr.attrLength);

      if (compare(pData, pNewData, this->hdr.attrType, this->hdr.attrLength) < 0) {
        ixPageHandle.InsertEntry(pData, rid);
      } else {
        newIxPageHandle.InsertEntry(pData, rid);
      }
      // probably the inserted one is the right smallest
      memcpy(pNewData, pRightPageData, this->hdr.attrLength);
      newRid = RID(newPageNum, INVALID_SLOT);
      if ((rc = this->pffh.MarkDirty(pageNum))) {
        return rc;
      }
      if ((rc = this->pffh.MarkDirty(newPageNum))) {
        return rc;
      }
      if ((rc = this->pffh.UnpinPage(pageNum))) {
        return rc;
      }
      if ((rc = this->pffh.UnpinPage(newPageNum))) {
        return rc;
      }
      return IX_PAGE_SPLITTED;
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
