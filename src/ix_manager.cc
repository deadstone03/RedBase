#include "rm_rid.h"
#include "ix.h"
#include "ix_internal.h"
#include <cstring>


IX_Manager::IX_Manager(PF_Manager &pfm) {
  this->ppfm = &pfm;
}


IX_Manager::~IX_Manager() {
}

RC IX_Manager::CreateIndex(const char *filename,
                           int indexNo,
                           AttrType attrType,
                           int attrLength) {
  char* indexName = IndexName(filename, indexNo);
  RC rc;
  PF_PageHandle pfPageHandle;
  PF_FileHandle pfFileHandle;
  IX_FileHdr fileHdr;
  PageNum pageNum;
  if ((rc = this->ppfm->CreateFile(indexName))) {
    goto CreateIndexErrorHandle;
  }
  // use the first page as ix file head
  if ((rc = this->ppfm->OpenFile(indexName, pfFileHandle))) {
    goto CreateIndexErrorHandle;
  }
  if ((rc = pfFileHandle.AllocatePage(pfPageHandle))) {
    goto CreateIndexErrorHandle;
  }
  if ((rc = pfPageHandle.GetPageNum(pageNum))) {
    goto CreateIndexErrorHandle;
  }
  if ((rc = pfFileHandle.UnpinPage(pageNum))) {
    goto CreateIndexErrorHandle;
  }

  // init file head
  fileHdr.attrType = attrType;
  fileHdr.attrLength = attrLength;
  fileHdr.root = INVALID_PAGE;
  fileHdr.slotSize = attrLength + sizeof(RID);
  fileHdr.m = (PF_PAGE_SIZE - sizeof(IX_PageHdr)) / fileHdr.slotSize;

  if ((rc = this->WriteHdr(fileHdr, pfFileHandle))) {
    goto CreateIndexErrorHandle;
  }

  if ((rc = this->ppfm->CloseFile(pfFileHandle))) {
    goto CreateIndexErrorHandle;
  }

CreateIndexErrorHandle:
  delete indexName;
  return rc;
}

RC IX_Manager::DestroyIndex(const char* filename, int indexNo) {
  char* indexName = IndexName(filename, indexNo);
  RC rc = this->ppfm->DestroyFile(indexName);
  delete indexName;
  return rc;
}

RC IX_Manager::OpenIndex(const char *fileName,
                         int indexNo,
                         IX_IndexHandle &indexHandle) {
  RC rc;
  char* indexName = IndexName(fileName, indexNo);
  PF_FileHandle pfFileHand;
  if ((rc = this->ppfm->OpenFile(indexName, pfFileHand))) {
    goto OpenIndexErrorHandle;
  }
  if ((rc = this->ReadHdr(pfFileHand, indexHandle.hdr))) {
    goto OpenIndexErrorHandle;
  }
  indexHandle.hdrChange = FALSE;
  indexHandle.pffh = pfFileHand;

OpenIndexErrorHandle:
  delete indexName;
  return rc;
}

RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
  if (indexHandle.hdrChange) {
    RC rc = this->WriteHdr(indexHandle.hdr, indexHandle.pffh);
    if (rc) {
      return rc;
    }
  }
  return this->ppfm->CloseFile(indexHandle.pffh);
}

RC IX_Manager::ReadHdr(const PF_FileHandle &pfFileHand, IX_FileHdr &hdr) const {
  RC rc;
  char* pData;
  PageNum pageNum;
  PF_PageHandle pfPageHandle;
  if ((rc = pfFileHand.GetFirstPage(pfPageHandle))) {
    return rc;
  }
  if ((rc = pfPageHandle.GetData(pData))) {
    return rc;
  }
  memcpy(&hdr, pData, sizeof(IX_FileHdr));

  if ((rc = pfPageHandle.GetPageNum(pageNum))) {
    return rc;
  }
  if ((rc = pfFileHand.UnpinPage(pageNum))) {
    return rc;
  }
  return 0;
}

RC IX_Manager::WriteHdr(const IX_FileHdr &hdr, PF_FileHandle &pfFileHand) {
    PF_PageHandle pfPageHandle;
    char* pData;
    RC rc;
    PageNum pageNum;
    if ((rc = pfFileHand.GetFirstPage(pfPageHandle))) {
      return rc;
    }
    if ((rc = pfPageHandle.GetData(pData))) {
      return rc;
    }

    memcpy(pData, &hdr, sizeof(IX_FileHdr));

    if ((rc = pfPageHandle.GetPageNum(pageNum))) {
      return rc;
    }
    if ((rc = pfFileHand.MarkDirty(pageNum))) {
      return rc;
    }
    if ((rc = pfFileHand.UnpinPage(pageNum))) {
      return rc;
    }
    return 0;
}
