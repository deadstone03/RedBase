#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"
#include <iostream>

const char* FILE_NAME = {"test_file_name"};
const int RECORD_SIZE = 8;


class RM_ManagerTest: public ::testing::Test {
   protected:
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }

    RM_FileHdr GetFileHdr() {
      RC rc;
      PF_FileHandle pfFilehandle;
      rc = pfManager_.OpenFile(FILE_NAME, pfFilehandle);
      EXPECT_EQ(0, rc);
      PF_PageHandle pfPageHandle;
      // this first pf page should have rm file head info
      rc = pfFilehandle.GetFirstPage(pfPageHandle);
      EXPECT_EQ(0, rc);
      char* pData;
      rc = pfPageHandle.GetData(pData);
      EXPECT_EQ(0, rc);
      RM_FileHdr* fileHdr = (RM_FileHdr*)pData;
      RM_FileHdr rmFileHdr;
      rmFileHdr.recordSize = fileHdr->recordSize;
      rmFileHdr.hdrPageNum = fileHdr->hdrPageNum;
      rmFileHdr.firstFree = fileHdr->firstFree;
      PageNum pageNum;
      EXPECT_EQ(0, pfPageHandle.GetPageNum(pageNum));
      EXPECT_EQ(0, pfFilehandle.UnpinPage(pageNum));
      EXPECT_EQ(0, pfManager_.CloseFile(pfFilehandle));
      return rmFileHdr;
    }

    void InsertTestRec(RM_Manager& rmManager, int recordNum) {
      RM_FileHandle rmFileHandle;
      EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));
      char* pData = new char[RECORD_SIZE];
      for (int i = 0; i < recordNum; i++) {
        memset(pData, i, RECORD_SIZE);
        RID rid;
        EXPECT_EQ(0, rmFileHandle.InsertRec(pData, rid));
      }
      EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
    }
    PF_Manager pfManager_;
};

TEST_F(RM_ManagerTest, CreateFile) {
  RM_Manager rmManager(pfManager_);
  EXPECT_EQ(0, rmManager.CreateFile(FILE_NAME, RECORD_SIZE));

  RM_FileHdr fileHdr = this->GetFileHdr();
  EXPECT_EQ(0, fileHdr.hdrPageNum);
  EXPECT_EQ(RECORD_SIZE, fileHdr.recordSize);
  EXPECT_EQ(RM_INVALID_PAGE, fileHdr.firstFree);

  EXPECT_EQ(0, rmManager.DestroyFile(FILE_NAME));
}

TEST_F(RM_ManagerTest, DestroyFile) {
  RM_Manager rmManager(pfManager_);
  RC rc = rmManager.CreateFile(FILE_NAME, RECORD_SIZE);
  rc = rmManager.DestroyFile(FILE_NAME);
  EXPECT_EQ(0, rc);
  RM_FileHandle rmFileHandle;
  rc = rmManager.OpenFile(FILE_NAME, rmFileHandle);
  EXPECT_EQ(PF_UNIX, rc);
}

TEST_F(RM_ManagerTest, OpenFileAndCloseFile) {
  RM_Manager rmManager(pfManager_);
  RC rc = rmManager.CreateFile(FILE_NAME, RECORD_SIZE);
  EXPECT_EQ(0, rc);

  RM_FileHandle rmFileHandle;
  EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));

  EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
  EXPECT_EQ(0, rmManager.DestroyFile(FILE_NAME));
}

TEST_F(RM_ManagerTest, InsertRecAndDeleteRec) {
  RM_Manager rmManager(pfManager_);
  RC rc = rmManager.CreateFile(FILE_NAME, RECORD_SIZE);
  EXPECT_EQ(0, rc);

  this->InsertTestRec(rmManager, 3);
  RM_FileHdr hdr = this->GetFileHdr();
  EXPECT_EQ(0, hdr.firstFree);

  this->InsertTestRec(rmManager, PF_PAGE_SIZE / RECORD_SIZE);
  hdr = this->GetFileHdr();
  EXPECT_EQ(1, hdr.firstFree);

  RM_FileHandle rmFileHandle;
  EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));
  RID rid(0, 3);
  EXPECT_EQ(0, rmFileHandle.DeleteRec(rid));
  EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
  hdr = this->GetFileHdr();
  EXPECT_EQ(0, hdr.firstFree);

  EXPECT_EQ(0, rmManager.DestroyFile(FILE_NAME));
}
