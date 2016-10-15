#includes "rm.h"

RM_Record::RM_Record()
: rid(INVALID_RECORDSIZE), pData(NULL), recordSize(INVALID_RECORDSIZE) {
}

RM_Record::RM_Record(RID rid, char* pData, int recordSize)
: rid(rid), pData(pData), recordSize(recordSize) {
}

RM_Record::~RM_Record() {
}

RC RM_Record::GetData(char *&pData) const {
    if (this.rid == INVALID_RID) {
        return RM_RECORD_NOTINIT;
    }
    pData = this.pData;
    return 0;
}

RC RM_Record::GetRid(RID &rid) const {
    if (this.rid == INVALID_RID) {
        return RM_RECORD_NOTINIT;
    }
    rid = this.rid;
    return 0;
}

RM_FileHandle::RM_FileHandle() {
    //TODO(xingdai)
}

RM_FileHandle::~RM_FileHandle() {
    //TODO(xingdai)
}

RC RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const {
    //TODO(xingdai)
}

RM_MA