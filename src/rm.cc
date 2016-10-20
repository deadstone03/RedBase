#include "rm.h"
#include "rm_internal.h"

RM_Manager::RM_Manager(PF_Manager &pfm)
: pfm(pfm) {
}

RM_Manager::~RM_Manager() {
  pfm.~PF_Manager();
}

RC RM_Manager::CreateFile (const char *fileName, int recordSize) {
  RC rc;
  if ((rc = pfm.CreateFile(fileName))) {
    return rc;
  }
  // The first page in the file should store info about the file
  PF_FileHandle pfh;
  if ((rc = pfm.OpenFile(fileName, pfh))) {
    return rc;
  }

  PF_PageHandle pph;
  if ((rc = pfh.GetFirstPage(pph))) {
    return rc;
  }
  char* pData;
  if ((rc = pph.GetData(pData))) {
    return rc;
  }
  RM_FileHead* fhr = (RM_FileHead*)pData;
  fhr->recordSize = recordSize;
  PageNum pageNum;
  if ((rc = pph.GetPageNum(pageNum))) {
    return rc;
  }
  if ((rc = pfh.UnpinPage(pageNum))) {
    return rc;
  }
  if ((rc = pfm.CloseFile(pfh))) {
      return rc;
  }
  return 0;
}

RC RM_Manager::DestroyFile(const char *fileName) {
  return pfm.DestroyFile(fileName);
}

RC RM_Manager::OpenFile (const char *fileName, RM_FileHandle &fileHandle) {
  pfm.OpenFile(fileName, fileHandle);
}

RC RM_Manager::CloseFile (RM_FileHandle &fileHandle) {
}
