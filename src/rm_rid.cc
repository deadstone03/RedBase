#includes "rm_rid.h"

RID::RID()
    : pageNum(INVALID_PAGE), slotNum(INVALID_SLOT) {
}

RID:RID(PageNum pageNum, SlotNum slotNum)
    : pageNum(pageNum), slotNum(slotNum) {

}
RID::~RID()

RC RID::GetPageNum(PageNum &pageNum) {
    if (this.pageNum == INVALID_PAGE) {
        return RM_RID_NOTINIT;
    }

    pageNum = this.pageNum;
    return (0);

}

RC RID::GetSlotNum(SlotNum &slotNum) {
    if (this.slotNum == INVALID_SLOT) {
        return RM_RID_NOTINIT;
    }

    slotnum = this.slotNum;
    return (0);
}
