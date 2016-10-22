#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"

const PageNum PAGE_NUM = 1;
const int SLOTS_PER_PAGE = 10;
const int RECORD_SIZE = 8;

RM_PageHandle CreateTestRM_PageHandle() {
  PageNum pageNum = PAGE_NUM;
  int slotsPerPage = SLOTS_PER_PAGE;
  int bitmapLen = (slotsPerPage + CHAR_BYTE_SIZE - 1) / CHAR_BYTE_SIZE * CHAR_BYTE_SIZE;
  int recordSize = RECORD_SIZE;
  int wholeDataLen = sizeof(RM_PageHdr) + bitmapLen + slotsPerPage * recordSize;
  char* pWholeData = new char[wholeDataLen];
  memset(pWholeData, 0, wholeDataLen);
  RM_PageHdr* phdr = (RM_PageHdr*)pWholeData;
  char* bitmap = pWholeData + sizeof(RM_PageHdr);
  char* pData = bitmap + bitmapLen;
  return RM_PageHandle(
      pageNum, phdr, bitmap, bitmapLen,
      pData, slotsPerPage, recordSize);
}

TEST(RM_PageHandle, CreateRM_PageHandle) {
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
}

TEST(RM_PageHandle, GetPageNum) {
  PageNum pageNum;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  RC rc = rmPageHandle.GetPageNum(pageNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(PAGE_NUM, pageNum);
}

void InsertTestRecord(RM_PageHandle & pageHandle, int recordNum) {
  char* pData = new char[RECORD_SIZE];
  RID rid;
  // insert multiple times.
  for (int i = 0; i < recordNum; ++i) {
    memset(pData, (char)i, RECORD_SIZE);
    RC rc = pageHandle.InsertRecord(pData, rid);
    EXPECT_EQ(0, rc);
  }
}

TEST(RM_PageHandle, InsertRecord) {
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 9);

  RID rid;
  RC rc;
  char* pData = new char[RECORD_SIZE];
  memset(pData, 1, RECORD_SIZE);
  PageNum pageNum;
  SlotNum slotNum;
  rc = rmPageHandle.InsertRecord(pData, rid);
  EXPECT_EQ(0, rc);
  rc = rid.GetPageNum(pageNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(PAGE_NUM, pageNum);
  rc = rid.GetSlotNum(slotNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(9, slotNum);
}

TEST(RM_PageHandle, InsertRecord_noSlot) {
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle(); InsertTestRecord(rmPageHandle, SLOTS_PER_PAGE);
  RID rid;
  RC rc;
  char* pData = new char[RECORD_SIZE];
  memset(pData, 1, RECORD_SIZE);
  rc = rmPageHandle.InsertRecord(pData, rid);
  EXPECT_EQ(RM_NOSLOT_INPAGE, rc);
}

TEST(RM_PageHandle, GetRecord) {
  RC rc;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 5);

  RM_Record record;
  rc = rmPageHandle.GetRecord(2, record);
  EXPECT_EQ(0, rc);

  RID rid;
  rc = record.GetRid(rid);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(RID(PAGE_NUM, 2), rid);
  char* pData;
  rc = record.GetData(pData);
  EXPECT_EQ(0, rc);
  for (int i = 0; i < RECORD_SIZE; ++i) {
    EXPECT_EQ(2, pData[i]);
  }
}

TEST(RM_PageHandle, GetRecord_invalid) {
  RC rc;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 5);

  RM_Record record;
  rc = rmPageHandle.GetRecord(6, record);
  EXPECT_EQ(RM_INVALID_SLOT, rc);
}

TEST(RM_PageHandle, GetRecord_invalidSlotNum) {
  RC rc;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 5);

  RM_Record record;
  rc = rmPageHandle.GetRecord(SLOTS_PER_PAGE + 1, record);
  EXPECT_EQ(RM_INVALID_SLOTNUM, rc);
}

TEST(RM_PageHandle, DeleteRecord) {
  RC rc;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 5);

  RM_Record record;
  rc = rmPageHandle.GetRecord(2, record);
  EXPECT_EQ(0, rc);
  rc = rmPageHandle.DeleteRecord(RID(PAGE_NUM, 2));
  EXPECT_EQ(0, rc);

  rc = rmPageHandle.GetRecord(2, record);
  EXPECT_EQ(RM_INVALID_SLOT, rc);
}

TEST(RM_PageHandle, DeleteRecord_invalidSlot) {
  RC rc;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 5);

  RM_Record record;
  rc = rmPageHandle.GetRecord(6, record);
  EXPECT_EQ(RM_INVALID_SLOT, rc);
}

TEST(RM_PageHandle, UpdateRecord) {
  RC rc;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 5);

  char* pData = new char[RECORD_SIZE];
  memset(pData, 4, RECORD_SIZE);
  RM_Record record(RID(PAGE_NUM, 2), pData, RECORD_SIZE);
  rc = rmPageHandle.UpdateRecord(record);

  RM_Record record1;
  rc = rmPageHandle.GetRecord(2, record1);
  char* pResData;
  rc = record1.GetData(pResData);
  EXPECT_EQ(0, rc);
  for (int i = 0; i < RECORD_SIZE; ++i) {
    EXPECT_EQ(4, pResData[i]);
  }
}

TEST(RM_PageHandle, UpdateRecord_invalid) {
  RC rc;
  RM_PageHandle rmPageHandle = CreateTestRM_PageHandle();
  InsertTestRecord(rmPageHandle, 5);

  char* pData = new char[RECORD_SIZE];
  memset(pData, 4, RECORD_SIZE);
  RM_Record record(RID(PAGE_NUM, 6), pData, RECORD_SIZE);
  rc = rmPageHandle.UpdateRecord(record);
  EXPECT_EQ(RM_INVALID_SLOT, rc);
}
