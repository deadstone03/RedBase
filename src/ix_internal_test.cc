#include <limits.h>
#include "gtest/gtest.h"
#include "ix_internal.h"
#include <iostream>

TEST(IndexName, Get) {
  char* indexName = IndexName("filename", 1234);
  EXPECT_STREQ("filename.1234", indexName);
  delete indexName;
}
