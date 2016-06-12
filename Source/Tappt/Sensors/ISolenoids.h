#ifndef ISolenoids_h
#define ISolenoids_h

#include "ITap.h"

class ISolenoids {
public:
  virtual void OpenForTap(ITap &tap) = 0;
  virtual void CloseForTap(ITap &tap) = 0;
  virtual ~ISolenoids() {}
};

#endif
