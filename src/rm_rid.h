//
// rm_rid.h
//
//   The Record Id interface
//

#ifndef RM_RID_H
#define RM_RID_H

// We separate the interface of RID from the rest of RM because some
// components will require the use of RID but not the rest of RM.

#include "redbase.h"

//
// PageNum: uniquely identifies a page in a file
//
typedef int PageNum;
#define INVALID_PAGE (-1)
//
// SlotNum: uniquely identifies a record in a page
//
typedef int SlotNum;
#define INVALID_SLOT (-1)

//
// RID: Record id interface
//
class RID {
    friend class RID;
public:
    RID();                                         // Default constructor
    RID(PageNum pageNum, SlotNum slotNum);
    RID(const RID& rid);
    ~RID();                                        // Destructor

    bool operator== (const RID &other) const;

    RC GetPageNum(PageNum &pageNum) const;         // Return page number
    RC GetSlotNum(SlotNum &slotNum) const;         // Return slot number

private:
    PageNum pageNum;
    SlotNum slotNum;
};

#define RM_RID_NOTINIT (START_RM_WARN + 0) //RID is not initialized
#define RM_RID_LASTWARN RM_RID_NOTINIT

//
// Print-error function
//
void RM_RID_PrintError(RC rc);

#endif
