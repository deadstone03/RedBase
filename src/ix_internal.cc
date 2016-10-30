#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ix_internal.h"


char* IndexName(const char *filename, const int indexNo) {
  // 1 for . 1 for end of str
  int indexNameLen = strlen(filename) + sizeof(int) * 8 + 2;
  char* indexName = new char[indexNameLen];
  sprintf(indexName, "%s.%d", filename, indexNo);
  return indexName;
}

int compare(void* pL, void* pR, AttrType attrType, int attrLength) {
  switch(attrType) {
    case INT: {
      int l = *(int*)pL;
      int r = *(int*)pR;
      if (l > r) return 1;
      if (l < r) return -1;
      return 0;
    }
    case FLOAT: {
      float l = *(float*)pL;
      float r = *(float*)pR;
      if (l > r) return 1;
      if (l < r) return -1;
      return 0;
    }
    case STRING: {
      return memcmp(pL, pR, attrLength);
    }
  }
  return 0;
}
