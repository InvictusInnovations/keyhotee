//#include <miniupnpc/miniwget.h>
extern "C" {
#define STATICLIB
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>
}

#include <bts/net/upnp.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>


namespace bts { namespace net {

  namespace detail 
  {
     class upnp_service_impl
     {
        public:
          upnp_service_impl()
          :upnp_thread("upnp"),done(false)
          {
          }

          /** upnp runs in its own thread because it makes
           *  blocking network calls 
           */
          fc::thread       upnp_thread;
          bool             done;
          fc::future<void> map_port_complete;
          fc::ip::address  external_ip;
     };

  }

upnp_service::upnp_service()
:my( new detail::upnp_service_impl() )
{
}

fc::ip::address upnp_service::external_ip()const
{
    return my->external_ip;
}

upnp_service::~upnp_service()
{
  try {
      my->done = true;
      my->upnp_thread.quit();
      if( my->map_port_complete.valid() )
      {
         my->map_port_complete.wait();
      }
  } 
  catch ( const fc::canceled_exception& )
  {} // expected
  catch ( const fc::exception& e )
  {
      wlog( "unexpected exception\n ${e}", ("e", e.to_detail_string() ) );
  }
}


void upnp_service::map_port( uint16_t local_port )
{
  std::string port = fc::variant(local_port).as_string();

  my->map_port_complete = my->upnp_thread.async( [=]() {
      const char * multicastif = 0;
      const char * minissdpdpath = 0;
      struct UPNPDev * devlist = 0;
      char lanaddr[64];
      
      /* miniupnpc 1.6 */
      int error = 0;
      devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, &error);

      struct UPNPUrls urls;
      memset( &urls, 0, sizeof(urls) );
      struct IGDdatas data;
      memset( &data, 0, sizeof(data) );
      int r;
      
      r = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));

      bool port_mapping_added = false;


       if (r == 1)
       {
           if (true) //  TODO  config this ?  fDiscover) 
           {
               char externalIPAddress[40];
               r = UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, externalIPAddress);
               if(r != UPNPCOMMAND_SUCCESS)
                   printf("UPnP: GetExternalIPAddress() returned %d\n", r);
               else
               {
                   if(externalIPAddress[0])
                   {
                       printf("UPnP: ExternalIPAddress = %s\n", externalIPAddress);
                       my->external_ip = fc::ip::address( std::string(externalIPAddress) );
                       // AddLocal(CNetAddr(externalIPAddress), LOCAL_UPNP);
                   }
                   else
                       printf("UPnP: GetExternalIPAddress failed.\n");
               }
           }
       
           std::string strDesc = "BitShares 0.0"; // TODO  + FormatFullVersion();
       
     //      try 
           {
               while(!my->done)  // TODO provide way to exit cleanly
               {
                   /* miniupnpc 1.6 */
                   r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                                       port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0, "0");
       
                   port_mapping_added = true;
                   if(r!=UPNPCOMMAND_SUCCESS)
                       printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
                           port.c_str(), port.c_str(), lanaddr, r, strupnperror(r));
                   else
                       printf("UPnP Port Mapping successful.\n");;
       
                   fc::usleep( fc::seconds(60*20) ); // Refresh every 20 minutes
               }
           }
     //      catch (boost::thread_interrupted)
           {
               if( port_mapping_added )
               {
                 r = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, port.c_str(), "TCP", 0);
                 ilog("UPNP_DeletePortMapping() returned : ${r}", ("r",r));
                 freeUPNPDevlist(devlist); devlist = 0;
                 FreeUPNPUrls(&urls);
               }
      //         throw;
           }
       } else {
           //printf("No valid UPnP IGDs found\n");
           wlog("No valid UPnP IGDs found");
           freeUPNPDevlist(devlist); devlist = 0;
           if (r != 0)
           {
               FreeUPNPUrls(&urls);
           }
       }
  });
}

} } // namespace bts::net
