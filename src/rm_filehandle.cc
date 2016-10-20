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
  PageNum pageNum;
  SlotNum SlotNum;
  if ((rc = rid.GetPageNum(pageNum))) {
    return rc;
  }
  if ((rc = rid.GetSlotNum(SlotNum))) {
    return rc;
  }

  PF_PageHandle pfph;
  if ((rc = pffh.GetThisPage(pageNum, pfph))) {
    return rc;
  }
    char *pData;
    pfph.GetData(pData);

}

RC RM_FileHandle::InsertRec(const char *pData, RID &rid) {
}

