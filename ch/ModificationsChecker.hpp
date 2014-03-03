#ifndef __MODIFICATIONSCHECKER_H
#define __MODIFICATIONSCHECKER_H

/** Dedicated interface checking modifications.
    if there are same changes blocking all mouse, keybord events.
*/
class IModificationsChecker
  {
  public:
    /** Check modification
        returns true if there is no modifications, application can
        go to the next step
    */
    virtual bool canContinue() const = 0;

  protected:
    virtual ~IModificationsChecker() {}
  };

#endif ///__MODIFICATIONSCHECKER_H


