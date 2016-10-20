#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

#include "rm_rid.h"

struct RM_FileHdr {
  int recordSize;
  PageNum firstFree;
  PageNum firstData;
  //SlotNum slotsPerPage;  //num of slots per page
};

struct RM_PageHdr {
  SlotNum slotCount;  // num of used slots
  PageNum nextHasFree;  // double link
};

#endif
