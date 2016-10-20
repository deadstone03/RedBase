#include "rm.h"
#include "rm_internal.h"

RM_Record::RM_Record() {
}

RM_Record::~RM_Record() {
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

