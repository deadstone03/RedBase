#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

#include "rm_rid.h"

struct RM_FileHdr {
  int recordSize;
  PageNum hdrPageNum; // this should always be 0;
  PageNum firstFree;
  //SlotNum slotsPerPage;  //num of slots per page
};

struct RM_PageHdr {
  SlotNum slotCount;  // num of used slots
  PageNum nextHasFree;  // double link
};

#endif
