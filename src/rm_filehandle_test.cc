#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"

const char* FILE_NAME = {"test_file_name"};
const int RECORD_SIZE = 8;


class RM_FileHandleTest: public ::testing::Test {
   protected:
    virtual void SetUp() {
    pfManager_.CreateFile(FILE_NAME);
    pfManager_.OpenFile(FILE_NAME, pfFileHandle_);
    PF_PageHandle pfPageHandle;
    // use the first page as rm file hdr
    pfFileHandle_.AllocatePage(pfPageHandle);
    RM_FileHdr fileHdr;
    fileHdr.hdrPageNum = 0;
    fileHdr.recordSize = RECORD_SIZE;
    fileHdr.firstFree = INVALID_PAGE;
    rmFileHandle_ = RM_FileHandle(fileHdr, FALSE, pfFileHandle_);
    }

    virtual void TearDown() {
      pfFileHandle_.ForcePages();
      pfManager_.CloseFile(pfFileHandle_);
      pfManager_.DestroyFile(FILE_NAME);
    }

    void InsertTestRecord(int recordNum) {
      char* pData = new char[RECORD_SIZE];
      RID rid;
      for (int i = 0; i < recordNum; ++i) {
        memset(pData, i % CHAR_BIT_SIZE, RECORD_SIZE);
        RC rc = rmFileHandle_.InsertRec(pData, rid);
        EXPECT_EQ(0, rc);
      }
    }

    PF_Manager pfManager_;
    PF_FileHandle pfFileHandle_;
    RM_FileHandle rmFileHandle_;
};

TEST_F(RM_FileHandleTest, InsertRec) {
  InsertTestRecord(2);
  char* pData = new char[RECORD_SIZE];
  memset(pData, 4, RECORD_SIZE);
  RID rid;
  RC rc = rmFileHandle_.InsertRec(pData, rid);
  EXPECT_EQ(0, rc);
  PageNum pageNum;
  SlotNum slotNum;
  rc = rid.GetPageNum(pageNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(0, pageNum);
  rc = rid.GetSlotNum(slotNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(2, slotNum);
}

TEST_F(RM_FileHandleTest, InsertRec_moreThanAPage) {
  InsertTestRecord(PF_PAGE_SIZE / RECORD_SIZE + 1);
  char* pData = new char[RECORD_SIZE];
  memset(pData, 4, RECORD_SIZE);
  RID rid;
  RC rc = rmFileHandle_.InsertRec(pData, rid);
  EXPECT_EQ(0, rc);
  PageNum pageNum;
  SlotNum slotNum;
  rc = rid.GetPageNum(pageNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(1, pageNum);
  rc = rid.GetSlotNum(slotNum);
  EXPECT_EQ(0, rc);
}

TEST_F(RM_FileHandleTest, GetRec) {
  InsertTestRecord(4);
  RM_Record rmRecord;
  RID rid(0, 2);
  RC rc = rmFileHandle_.GetRec(rid, rmRecord);
  EXPECT_EQ(0, rc);
  char* pData;
  rc = rmRecord.GetData(pData);
  EXPECT_EQ(0, rc);
  for (int i = 0; i < RECORD_SIZE; ++i) {
    EXPECT_EQ(2, pData[i]);
  }
}

TEST_F(RM_FileHandleTest, GetRec_invalid) {
  InsertTestRecord(4);
  RM_Record rmRecord;
  RID rid(0, 10);
  RC rc = rmFileHandle_.GetRec(rid, rmRecord);
  EXPECT_EQ(RM_INVALID_SLOT, rc);
}

TEST_F(RM_FileHandleTest, DeleteRec) {
  InsertTestRecord(4);
  RID rid(0, 2);
  RC rc = rmFileHandle_.DeleteRec(rid);
  EXPECT_EQ(0, rc);
  RM_Record rmRecord;
  rc = rmFileHandle_.GetRec(rid, rmRecord);
  EXPECT_EQ(RM_INVALID_SLOT, rc);
}

TEST_F(RM_FileHandleTest, DeleteRecInFullPage) {
  InsertTestRecord(PF_PAGE_SIZE / RECORD_SIZE);
  RID rid(0, 2);
  RC rc = rmFileHandle_.DeleteRec(rid);
  EXPECT_EQ(0, rc);

  // will reuse the empty slot
  char* pData = new char[RECORD_SIZE];
  memset(pData, 10, RECORD_SIZE);
  RID resRid;
  rc = rmFileHandle_.InsertRec(pData, resRid);
  EXPECT_EQ(0, rc);
  PageNum pageNum;
  SlotNum slotNum;
  rc = resRid.GetPageNum(pageNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(0, pageNum);
  rc = resRid.GetSlotNum(slotNum);
  EXPECT_EQ(0, rc);
  EXPECT_EQ(2, slotNum);
}

TEST_F(RM_FileHandleTest, UpdateRec) {
  InsertTestRecord(4);
  RID rid(0, 2);
  char* pData = new char[RECORD_SIZE];
  memset(pData, 10, RECORD_SIZE);
  RM_Record rmRecord(rid, pData, RECORD_SIZE);
  RC rc = rmFileHandle_.UpdateRec(rmRecord);
  EXPECT_EQ(0, rc);
  RM_Record rmRecord2;
  rc = rmFileHandle_.GetRec(rid, rmRecord2);
  EXPECT_EQ(0, rc);
  for (int i = 0; i < RECORD_SIZE; ++i) {
    EXPECT_EQ(10, pData[i]);
  }
}
