#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"

TEST(RIDTest, GetPageNum) {
  RC rc;
  PF_Manager pfManager;
  rc = pfManager.CreateFile(FILE_NAME);
  EXPECT_EQ(0, rc);
  PF_FileHandle pfFileHandle;
  rc = pfManager.OpenFile(FILE_NAME, pfFileHandle);
  EXPECT_EQ(0, rc);
  PF_PageHandle pfPageHandle;
  rc = pfFileHandle.GetFirstPage(pfPageHandle);
  EXPECT_EQ(0, rc);
}

