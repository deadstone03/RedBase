#include<cstring>
#include "rm.h"
#include "rm_internal.h"

RM_Manager::RM_Manager(PF_Manager &pfm) {
  this->pfm = pfm;
}

RM_Manager::~RM_Manager() {
}

RC RM_Manager::CreateFile(const char *fileName, int recordSize) {
  RC rc;
  if ((rc = this->pfm.CreateFile(fileName))) {
    return rc;
  }
  // Set the first page to rm file head
  PF_FileHandle pfFileHandle;
  if ((rc = this->pfm.OpenFile(fileName, pfFileHandle))) {
    return rc;
  }
  RM_FileHdr rmFileHdr;
  if ((rc = this->GetFileHdr(pfFileHandle, rmFileHdr))) {
    return rc;
  }
  rmFileHdr.recordSize = recordSize;
  rmFileHdr.firstFree = INVALID_PAGE;
  if ((rc = this->WriteFileHdr(pfFileHandle, rmFileHdr))) {
    return rc;
  }
  return this->pfm.CloseFile(pfFileHandle);
}

RC RM_Manager::DestroyFile(const char *fileName) {
  return this->pfm.DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
  RC rc;
  PF_FileHandle pfFileHandle;
  if ((rc = this->pfm.OpenFile(fileName, pfFileHandle))) {
    return rc;
  }
  if ((rc = this->GetFileHdr(pfFileHandle,fileHandle.hdr))) {
    return rc;
  }
  fileHandle.pffh = pfFileHandle; //this is not right?
  fileHandle.hdrChange = 0;
  return 0;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
  RC rc;
  PF_FileHandle pfFileHandle = fileHandle.pffh;
  if (fileHandle.hdrChange) {
    if ((rc = this->WriteFileHdr(pfFileHandle, fileHandle.hdr))) {
      return rc;
    }
  }
  return this->pfm.CloseFile(pfFileHandle);
}

RC RM_Manager::GetFileHdr(const PF_FileHandle& pfFileHandle,
                          RM_FileHdr &rmFileHdr) const {
  RC rc;
  PF_PageHandle pfPageHandle;
  if ((rc = pfFileHandle.GetFirstPage(pfPageHandle))) {
    return rc;
  }
  char *pData;
  if ((rc = pfPageHandle.GetData(pData))) {
    return rc;
  }
  RM_FileHdr* pRmFileHdr = (RM_FileHdr*)pData;
  rmFileHdr.firstFree = pRmFileHdr->firstFree;
  rmFileHdr.recordSize = pRmFileHdr->recordSize;
  if ((rc = pfPageHandle.GetPageNum(rmFileHdr.hdrPageNum))) {
    return rc;
  }
  // unpin the page
  return pfFileHandle.UnpinPage(rmFileHdr.hdrPageNum);
}

RC RM_Manager::WriteFileHdr(const PF_FileHandle& pfFileHandle,
                            const RM_FileHdr &rmFileHdr) const {
  RC rc;
  PF_PageHandle pfPageHandle;
  if ((rc = pfFileHandle.GetFirstPage(pfPageHandle))) {
    return rc;
  }
  char *pData;
  if ((rc = pfPageHandle.GetData(pData))) {
    return rc;
  }
  RM_FileHdr* pRmFileHdr = (RM_FileHdr*)pData;
  memcpy(pRmFileHdr, &rmFileHdr, sizeof(RM_FileHdr));
  if ((rc = pfFileHandle.MarkDirty(rmFileHdr.hdrPageNum))) {
    return rc;
  }
  return pfFileHandle.UnpinPage(rmFileHdr.hdrPageNum);
}
