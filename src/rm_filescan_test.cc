#include <limits.h>
#include "gtest/gtest.h"
#include "rm.h"

#define STRLEN 29  // length of string in testrec
#ifndef offsetof
#       define offsetof(type, field)   ((size_t)&(((type *)0) -> field))
#endif
//
// Structure of the records we will be using for the tests
//
struct TestRec {
    char  str[STRLEN];
    int   num;
    float r;
};

const char* FILE_NAME = {"test_file_name"};
const int RECORD_SIZE = sizeof(TestRec);


class RM_FileScanTest: public ::testing::Test {
   protected:
    virtual void SetUp() {
      RM_Manager rmManager(pfManager_);
      EXPECT_EQ(0, rmManager.CreateFile(FILE_NAME, RECORD_SIZE));
    }
    virtual void TearDown() {
      RM_Manager rmManager(pfManager_);
      EXPECT_EQ(0, rmManager.DestroyFile(FILE_NAME));
    }

    void InsertTestRec(int recordNum) {
      RM_Manager rmManager(pfManager_);
      RM_FileHandle rmFileHandle;
      EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));
      TestRec* testRec = new TestRec();
      for (int i = 0; i < recordNum; i++) {
        testRec->num = i;
        testRec->r = i * i;
        sprintf(testRec->str, "%d", i);
        RID rid;
        EXPECT_EQ(0, rmFileHandle.InsertRec((char*)testRec, rid));
      }
      EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
    }
    PF_Manager pfManager_;
};

TEST_F(RM_FileScanTest, FunctionTest) {
  this->InsertTestRec(200);
  RM_Manager rmManager(pfManager_);
  RM_FileHandle rmFileHandle;
  EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));
  RM_FileScan rmFileScan;
  int value = 3;
  RC rc = rmFileScan.OpenScan(rmFileHandle,
                              INT,
                              sizeof(int),
                              offsetof(TestRec, num),
                              EQ_OP,
                              &value);
  EXPECT_EQ(0, rc);
  RM_Record rec;
  rc = rmFileScan.GetNextRec(rec);
  EXPECT_EQ(0, rc);
  char* pData;
  EXPECT_EQ(0, rec.GetData(pData));
  TestRec* pTestRec = (TestRec*)pData;
  EXPECT_EQ(3, pTestRec->num);
  EXPECT_FLOAT_EQ(9.0, pTestRec->r);
  EXPECT_STREQ("3", pTestRec->str);

  EXPECT_EQ(0, rmFileScan.CloseScan());
  EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
}
