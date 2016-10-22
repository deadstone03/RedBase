#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"

TEST(RIDTest, GetPageNum) {
  RC rc;
  RID rid(1, 1);
  PageNum pageNum;
  rc = rid.GetPageNum(pageNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(1, pageNum);
}

TEST(RIDTest, GetSlotNum) {
  RC rc;
  RID rid(1, 1);
  SlotNum slotNum;
  rc = rid.GetSlotNum(slotNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(1, slotNum);
}

TEST(RIDTest, InvalidRID) {
  RC rc;
  RID rid;
  SlotNum slotNum;
  PageNum pageNum;
  rc = rid.GetSlotNum(slotNum);
  EXPECT_EQ(RM_RID_NOTINIT, rc);
  rc = rid.GetPageNum(pageNum);
  EXPECT_EQ(RM_RID_NOTINIT, rc);
}

TEST(RIDTest, RIDEqual) {
  RID rid1(1, 2);
  RID rid2(1, 2);
  RID rid3(1, 3);
  EXPECT_EQ(rid1, rid2);
  EXPECT_FALSE(rid1 == rid3);
}

TEST(RIDTest, Assign) {
  RID rid1(1, 2);
  RID rid2;
  rid1 = rid2;
  EXPECT_EQ(rid1, rid2);
}
