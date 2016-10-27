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
  char* pData;
  PageNum pageNum;
  rc = this->ppfm->CreateFile(indexName);
  if (rc) {
    goto CreateIndexErrorHandle;
  }
  // use the first page as ix file head
  if ((rc = this->ppfm->OpenFile(indexName, pfFileHandle))) {
    goto CreateIndexErrorHandle;
  }
  if ((rc = pfFileHandle.AllocatePage(pfPageHandle))) {
    goto CreateIndexErrorHandle;
  }
  fileHdr.attrType = attrType;
  fileHdr.attrLength = attrLength;
  fileHdr.root = INVALID_PAGE;
  if ((rc = pfPageHandle.GetData(pData))) {
    goto CreateIndexErrorHandle;
  }
  memcpy(pData, &fileHdr, sizeof(fileHdr));
  if ((rc = pfPageHandle.GetPageNum(pageNum))) {
    goto CreateIndexErrorHandle;
  }
  if ((rc = pfFileHandle.MarkDirty(pageNum))) {
    goto CreateIndexErrorHandle;
  }

  if ((rc = pfFileHandle.UnpinPage(pageNum))) {
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
  PF_FileHandle pfFilehandle;
  PF_PageHandle pfPageHandle;
  char* pData;
  if ((rc = this->ppfm->OpenFile(indexName, pfFilehandle))) {
    goto OpenIndexErrorHandle;
  }
  if ((rc = pfFilehandle.GetFirstPage(pfPageHandle))) {
    goto OpenIndexErrorHandle;
  }
  

OpenIndexErrorHandle:
  delete indexName;
  return rc;
}

RC CloseIndex(IX_IndexHandle *indexHandle) {
}
