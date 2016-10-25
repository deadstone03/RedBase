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
  this->InsertTestRec(PF_PAGE_SIZE / RECORD_SIZE * 2);
  RM_Manager rmManager(pfManager_);
  RM_FileHandle rmFileHandle;
  EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));
  RM_FileScan rmFileScan;
  RC rc = rmFileScan.OpenScan(rmFileHandle,
                              INT,
                              sizeof(int),
                              offsetof(TestRec, num),
                              NO_OP,
                              NULL);
  EXPECT_EQ(0, rc);
  RM_Record rec;
  char* pData;
  TestRec* pTestRec;
  int i = 0;
  while(!(rc = rmFileScan.GetNextRec(rec))) {
    EXPECT_EQ(0, rec.GetData(pData));
    pTestRec = (TestRec*)pData;
    EXPECT_EQ(i, pTestRec->num);
    EXPECT_FLOAT_EQ(i * i, pTestRec->r);
    EXPECT_EQ(i, atoi(pTestRec->str));
    i++;
  }
  EXPECT_EQ(RM_EOF, rc);
  EXPECT_EQ(0, rmFileScan.CloseScan());
  EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
}

TEST_F(RM_FileScanTest, FunctionTestWithFilterEQ) {
  this->InsertTestRec(PF_PAGE_SIZE / RECORD_SIZE * 2);
  RM_Manager rmManager(pfManager_);
  RM_FileHandle rmFileHandle;
  EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));
  RM_FileScan rmFileScan;
  int value = 20;
  RC rc = rmFileScan.OpenScan(rmFileHandle,
                              INT,
                              sizeof(int),
                              offsetof(TestRec, num),
                              EQ_OP,
                              &value);
  EXPECT_EQ(0, rc);
  RM_Record rec;
  char* pData;
  TestRec* pTestRec;
  EXPECT_EQ(0, rmFileScan.GetNextRec(rec));
  EXPECT_EQ(0, rec.GetData(pData));
  pTestRec = (TestRec*)pData;
  EXPECT_EQ(20, pTestRec->num);
  EXPECT_FLOAT_EQ(20 * 20, pTestRec->r);
  EXPECT_EQ(20, atoi(pTestRec->str));

  EXPECT_EQ(RM_EOF, rmFileScan.GetNextRec(rec));

  EXPECT_EQ(0, rmFileScan.CloseScan());
  EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
}

TEST_F(RM_FileScanTest, FunctionTestWithFilterLT) {
  this->InsertTestRec(PF_PAGE_SIZE / RECORD_SIZE * 2);
  RM_Manager rmManager(pfManager_);
  RM_FileHandle rmFileHandle;
  EXPECT_EQ(0, rmManager.OpenFile(FILE_NAME, rmFileHandle));
  RM_FileScan rmFileScan;
  int value = 20;
  RC rc = rmFileScan.OpenScan(rmFileHandle,
                              INT,
                              sizeof(int),
                              offsetof(TestRec, num),
                              LT_OP,
                              &value);
  EXPECT_EQ(0, rc);
  RM_Record rec;
  char* pData;
  TestRec* pTestRec;
  int i = 0;
  while(!(rc = rmFileScan.GetNextRec(rec))) {
    EXPECT_EQ(0, rec.GetData(pData));
    pTestRec = (TestRec*)pData;
    EXPECT_EQ(i, pTestRec->num);
    EXPECT_FLOAT_EQ(i * i, pTestRec->r);
    EXPECT_EQ(i, atoi(pTestRec->str));
    i++;
  }
  EXPECT_EQ(20, i);
  EXPECT_EQ(RM_EOF, rc);

  EXPECT_EQ(0, rmFileScan.CloseScan());
  EXPECT_EQ(0, rmManager.CloseFile(rmFileHandle));
}
