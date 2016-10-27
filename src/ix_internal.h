#ifndef IX_INTERNAL_H
#define IX_INTERNAL_H
#include "redbase.h"
#include "rm_rid.h"

// index's file hdr
struct IX_FileHdr {
  AttrType attrType;
  int attrLength;
  PageNum root;
};

struct IX_PageHdr {
};

char* IndexName(const char *filename, const int indexNo);

#endif
