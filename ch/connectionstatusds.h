#ifndef __CONNECTIONSTATUSDS_H
#define __CONNECTIONSTATUSDS_H

/** Dedicated interface providing data to connection status controls.
*/
class IConnectionStatusDataSource
  {
  public:
    /// Returns number of active connections to the bitshares network.
    virtual unsigned int GetConnectionCount() const = 0;

  protected:
    virtual ~IConnectionStatusDataSource() {}
  };

#endif ///__CONNECTIONSTATUSDS_H


