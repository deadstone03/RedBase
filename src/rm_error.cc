#include <iostream>
#include "rm.h"

void RM_PrintError(RC rc, unsigned int line, const char* filename) {
  if ((END_PF_ERR <= rc && rc <= START_PF_ERR) ||
      (START_PF_WARN <= rc && rc <= END_PF_WARN)) {
    PF_PrintError(rc, line, filename);
    return;
  }

  switch(rc) {
    case RM_RID_NOTINIT:
      std::cerr << filename << " " << line << ":" << "RID not init" << std::endl; return;
    case RM_RECORD_NOTINIT:
      std::cerr << filename << " " << line << ":" << "RM record not init." << std::endl; return;
    case RM_PAGE_NOTINIT:
      std::cerr << filename << " " << line << ":" << "RM page not init" << std::endl; return;
    case RM_INVALID_SLOTNUM:
      std::cerr << filename << " " << line << ":" << "SlotNum is invalid." << std::endl; return;
    case RM_INVALID_SLOT:
      std::cerr << filename << " " << line << ":" << "Slot is empty." << std::endl; return;
    case RM_NOSLOT_INPAGE:
      std::cerr << filename << " " << line << ":" << "No empty slot in page." << std::endl; return;
    case RM_PAGE_EOF:
      std::cerr << filename << " " << line << ":" << "Page has no more record." << std::endl; return;
    case RM_EOF:
      std::cerr << filename << " " << line << ":" << "No more record." << std::endl; return;
    default:
      std::cerr << filename << " " << line << ":" << rc << " unknown" << std::endl; return;
  }
}
