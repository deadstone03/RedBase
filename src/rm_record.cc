#include <cstring>
#include "rm.h"
#include "rm_internal.h"

RM_Record::RM_Record()
:rid(INVALID_RID), pData(NULL), recordSize(-1) {
}

RM_Record::RM_Record(RID rid, char* pData, int recordSize)
:rid(rid), pData(pData), recordSize(recordSize) {
}

RM_Record::RM_Record(const RM_Record & other)
:rid(other.rid), recordSize(other.recordSize) {
  this->pData = new char[recordSize];
  memcpy(this->pData, other.pData, this->recordSize);
}

RM_Record::~RM_Record() {
  if (this->pData != NULL) {
    delete pData;
  }
}

RM_Record* RM_Record::operator=(const RM_Record& other) {
  this->rid = other.rid;
  this->recordSize = other.recordSize;
  if (this->pData == NULL) {
    this->pData = new char[this->recordSize];
  }
  memcpy(this->pData, other.pData, recordSize);
  return this;
}

int RM_Record::operator==(const RM_Record& other) const {
  return this->rid == other.rid && this->recordSize == other.recordSize
      && memcmp(this->pData, other.pData, this->recordSize) == 0;
}

RC RM_Record::GetData(char *&pData) const {
    if (this->rid == INVALID_RID) {
        return RM_RECORD_NOTINIT;
    }
    pData = this->pData;
    return 0;
}

RC RM_Record::GetRid(RID &rid) const {
    if (this->rid == INVALID_RID) {
        return RM_RECORD_NOTINIT;
    }
    rid = this->rid;
    return 0;
}

