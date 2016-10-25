#include<cstring>
#include "rm.h"
#include "rm_internal.h"

RM_FileScan::RM_FileScan() {
}

RM_FileScan::~RM_FileScan() {
}

RC RM_FileScan::OpenScan(const RM_FileHandle &fileHandle,
                         AttrType   attrType,
                         int        attrLength,
                         int        attrOffset,
                         CompOp     compOp,
                         void       *value,
                         ClientHint pinHint) {
  this->fileHandle = fileHandle; // this is right but very bad style.
  this->attrType = attrType;
  this->attrLength = attrLength;
  this->attrOffset = attrOffset;
  this->compOp = compOp;
  if (value == NULL) {
    this->value = NULL;
  } else {
    this->value = new char[attrLength];
    memcpy(this->value, value, attrLength);
  }

  this->pinHint = pinHint;
  this->curRID = RID(0, INVALID_SLOT);
  return 0; }


RC RM_FileScan::GetNextRec(RM_Record &rec) {
  RC rc;
  while (!(rc = this->fileHandle.GetNextRec(this->curRID, rec))) {
    int check;
    if ((rc = rec.GetRid(this->curRID))) {
      return rc;
    }
    if ((rc = this->CheckRecord(rec, check))) {
      RM_PrintError(rc, __LINE__, __FILE__);
      return rc;
    }
    if (check) {
      return 0;
    }
  }
  return rc;
}

RC RM_FileScan::CheckRecord(const RM_Record &rec, int &check) const {
  if (this->value == NULL) {
    check = TRUE;
    return 0;
  }
  RC rc;
  char* pData;
  if (( rc = rec.GetData(pData))) {
    return rc;
  }
  void* pVal = (void*)(pData + this->attrOffset);
  switch(this->compOp) {
    case EQ_OP: check = (this->Compare(pVal, this->value) == 0); break;
    case LT_OP: check = (this->Compare(pVal, this->value) < 0); break;
    case GT_OP: check = (this->Compare(pVal, this->value) > 0); break;
    case LE_OP: check = (this->Compare(pVal, this->value) <= 0); break;
    case GE_OP: check = (this->Compare(pVal, this->value) >= 0); break;
    case NE_OP: check = (this->Compare(pVal, this->value) != 0); break;
    case NO_OP: check = TRUE; break;
  }
  return 0;
}

int RM_FileScan::Compare(const void* pCond, const void* pVal) const {
  switch(this->attrType) {
    case INT:
      {
        int *pLeft = (int*)pCond;
        int *pRight = (int*)pVal;
        if (*pLeft < *pRight) {
          return -1;
        } else if (*pLeft > *pRight) {
          return 1;
        }
        return 0;
      }
    case FLOAT:
      {
        float *pLeft = (float*)pCond;
        float *pRight = (float*)pVal;
        if (*pLeft < *pRight) {
          return -1;
        } else if (*pLeft > *pRight) {
          return 1;
        }
        return 0;
      }
    case STRING:
      {
        char *pLeft = (char*)pCond;
        char *pRight = (char*)pVal;
        for (int i = 0; i < this->attrLength; ++i) {
          if (pLeft[i] < pRight[i]) {
            return -1;
          } else if (pLeft[i] > pRight[i]) {
            return 1;
          }
        }
        return 0;
      }
  }
  return 0;
}

RC RM_FileScan::CloseScan () {
  if (this->value != NULL) {
    delete (char*)this->value;
  }

  return 0;
}
