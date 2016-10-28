#include "ix.h"

IX_IndexHandle::IX_IndexHandle() {
}

IX_IndexHandle::~IX_IndexHandle() {
}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
  RC rc;
  if (this->hdr.root == INVALID_PAGE) {
    // create root and insert in root;
    PF_PageHandle pfPageHandle;
    if ((rc = this->pffh.AllocatePage(pfPageHandle))) {
      return rc;
    }
  }
}
