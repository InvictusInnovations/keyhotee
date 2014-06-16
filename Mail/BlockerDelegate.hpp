#pragma once

/** Helper callback interface notifying mail window about blocked image.
*/
class IBlockerDelegate
{
  public:
    /// notify when remote image is blocked.
    virtual void onBlockedImage() = 0;
    /// notification send when there was a request to load the blocked images
    virtual void onLoadBlockedImages() = 0;

  protected:
    IBlockerDelegate(){};
    virtual ~IBlockerDelegate(){}
};

