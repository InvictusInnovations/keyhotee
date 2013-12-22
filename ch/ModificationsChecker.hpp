#ifndef __MODIFICATIONSCHECKER_H
#define __MODIFICATIONSCHECKER_H

/** Dedicated interface checking modifications.
if there are same changes blocking all mouse, keybord events.
*/
class IModificationsChecker
  {
  public:
    virtual bool canContinue() const = 0;

  protected:
    virtual ~IModificationsChecker() {}
  };

#endif ///__MODIFICATIONSCHECKER_H


