#include "rm_rid.h"

RID::RID()
: pageNum(INVALID_PAGE), slotNum(INVALID_SLOT) {}

RID::RID(PageNum pageNum, SlotNum slotNum)
: pageNum(pageNum), slotNum(slotNum) {}

RID::RID(const RID& rid)
: pageNum(rid.pageNum), slotNum(rid.slotNum) {}

RID::~RID() {}

bool RID::operator==(const RID &other) const {
   return other.slotNum == this->slotNum &&
     other.pageNum == this->pageNum;
}

RID& RID::operator=(const RID &other) {
  if (this != &other) {
    this->slotNum = other.slotNum;
    this->pageNum = other.pageNum;
  }
  return *this;
}

RC RID::GetPageNum(PageNum &pageNum) const {
    if (this->pageNum == INVALID_PAGE) {
      pageNum = INVALID_PAGE;
      return RM_RID_NOTINIT;
    }

    pageNum = this->pageNum;
    return (0);

}

RC RID::GetSlotNum(SlotNum &slotNum) const {
    if (this->slotNum == INVALID_SLOT) {
      slotNum = INVALID_SLOT;
      return RM_RID_NOTINIT;
    }

    slotNum = this->slotNum;
    return (0);
}
