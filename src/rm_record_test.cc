#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"

TEST(RM_RecordTest, CreateRecord) {
  RID rid(1, 1);
  int recordSize = 20;
  char* pData = new char[recordSize];
  memset(pData, 1, recordSize);
  RM_Record rmRecord(rid, pData, recordSize);
}

TEST(RM_RecordTest, NotInit) {
  RM_Record rmRecord;
  char* pResData;
  RC rc;
  rc = rmRecord.GetData(pResData);
  EXPECT_EQ(RM_RECORD_NOTINIT, rc);
  RID rid;
  rc = rmRecord.GetRid(rid);
  EXPECT_EQ(RM_RECORD_NOTINIT, rc);
}

TEST(RM_RecordTest, GetRid) {
  RID rid(1, 1);
  int recordSize = 20;
  char* pData = new char[recordSize];
  memset(pData, 1, recordSize);
  RM_Record rmRecord(rid, pData, recordSize);
  RC rc;
  RID ridRes;
  rc = rmRecord.GetRid(ridRes);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(rid, ridRes);
}

TEST(RM_RecordTest, GetData) {
  RID rid(1, 1);
  int recordSize = 20;
  char* pData = new char[recordSize];
  memset(pData, 1, recordSize);
  RM_Record rmRecord(rid, pData, recordSize);
  char* pResData;
  RC rc;
  rc = rmRecord.GetData(pResData);
  EXPECT_EQ(pData, pResData);
  EXPECT_EQ(0, rc);
}

TEST(RM_RecordTest, Assign) {
  RID rid(1, 1);
  int recordSize = 20;
  char* pData = new char[recordSize];
  memset(pData, 1, recordSize);
  RM_Record rmRecord(rid, pData, recordSize);
  RM_Record rmRecord1;
  rmRecord1 = rmRecord;
  EXPECT_EQ(rmRecord, rmRecord1);
}
