#include <limits.h>
#include "gtest/gtest.h"
#include "ix.h"
#include "ix_internal.h"
#include <iostream>

const int INDEX_NO = 12;
const char* FILE_NAME = {"test_file_name"};
const char* IX_FILE_NAME = {"test_file_name.12"};
const AttrType ATTR_TYPE = INT;
const int ATTR_LENGTH = sizeof(int);

class IX_ManagerTest: public ::testing::Test {
   protected:
    virtual void SetUp() {
      IX_Manager ixManager(pfManager_);
      EXPECT_EQ(0, ixManager.CreateIndex(FILE_NAME, INDEX_NO, ATTR_TYPE, ATTR_LENGTH));
    }
    virtual void TearDown() {
      IX_Manager ixManager(pfManager_);
      EXPECT_EQ(0, ixManager.DestroyIndex(FILE_NAME, INDEX_NO));
    }

    PF_Manager pfManager_;

    IX_FileHdr GetFileHdr() {
      PF_FileHandle pfFileHand;
      EXPECT_EQ(0, pfManager_.OpenFile(IX_FILE_NAME, pfFileHand));
      PF_PageHandle pfPageHandle;
      EXPECT_EQ(0, pfFileHand.GetFirstPage(pfPageHandle));
      char* pData;
      EXPECT_EQ(0, pfPageHandle.GetData(pData));
      IX_FileHdr fileHdr;
      memcpy(&fileHdr, pData, sizeof(IX_FileHdr));

      EXPECT_EQ(0, pfFileHand.UnpinPage(0));
      EXPECT_EQ(0, pfManager_.CloseFile(pfFileHand));
      return fileHdr;
    }
};

TEST_F(IX_ManagerTest, CreateIndex) {
  IX_FileHdr fileHdr = GetFileHdr();
  EXPECT_EQ(INT, fileHdr.attrType);
  EXPECT_EQ(ATTR_LENGTH, fileHdr.attrLength);
  EXPECT_EQ(INVALID_PAGE, fileHdr.root);
}
