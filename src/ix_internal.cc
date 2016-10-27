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
