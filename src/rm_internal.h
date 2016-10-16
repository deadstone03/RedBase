#include "rm.h"

struct RM_FileHdr {
  int recordSize;
  SlotNum slotsPerPage;  //num of slots per page 
};

struct RM_PageHdr {
  SlotNum slotCount;  // num of used slots
};
