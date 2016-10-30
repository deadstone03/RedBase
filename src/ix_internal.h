#ifndef IX_INTERNAL_H
#define IX_INTERNAL_H
#include "redbase.h"
#include "rm_rid.h"

// index's file hdr
struct IX_FileHdr {
  AttrType attrType;
  int attrLength;
  PageNum root;
  int slotSize;  // the size of the slot attrLength + sizeof(RID);
  int m;  // the m value for B tree;
};

struct IX_PageHdr {
  PageNum parent;
  PageNum sibling;
  int isLeaf;
  int isRoot;
  int slotCount;
};

char* IndexName(const char *filename, const int indexNo);
int compare(void* pL, void* pR, AttrType attrType, int attrLength);

#endif
