#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

#include "rm_rid.h"

struct RM_FileHdr {
  int recordSize;
  SlotNum slotsPerPage;  //num of slots per page
};

struct RM_PageHdr {
  SlotNum slotCount;  // num of used slots
};

#endif
