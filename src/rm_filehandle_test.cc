#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"

const char* FILE_NAME = {"test_file_name"};
const int RECORD_SIZE = 8;


class RM_FileHandleTest: public ::testing::Test {
   protected:
    virtual void SetUp() {
      pfManager_ = PF_Manager();
      pfManager_.CreateFile(FILE_NAME);
      pfManager_.OpenFile(FILE_NAME, pfFileHandle_);
      RM_FileHdr fileHdr;
      fileHdr.hdrPageNum = 0;
      fileHdr.recordSize = RECORD_SIZE;
      fileHdr.firstFree = INVALID_PAGE;
      rmFileHandle_ = RM_FileHandle(fileHdr, FALSE, pfFileHandle_);
    }

    virtual void TearDown() {
      pfFileHandle_.ForcePages();
    }

    PF_Manager pfManager_;
    PF_FileHandle pfFileHandle_;
    RM_FileHandle rmFileHandle_;
};

TEST_F(RM_FileHandleTest, InsertRec) {
  char* pData = new char[RECORD_SIZE];
  memset(pData, 2, RECORD_SIZE);
  RID rid;
  RC rc = rmFileHandle_.InsertRec(pData, rid);
  EXPECT_EQ(0, rc);
  PageNum pageNum;
  SlotNum slotNum;
  rc = rid.GetPageNum(pageNum);
}
