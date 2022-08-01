#ifndef NETDEVICE_SETUP_H
#define NETDEVICE_SETUP_H
#include "ns3/core-module.h"
#include "ns3/wave-module.h"
#include "ns3/network-module.h"

namespace ns3
{
/** \brief This is a "utility class". It does not an extension to ns3. 
 */
  class NetDeviceSetup
  {
    public:
      NetDeviceSetup ();
      virtual ~NetDeviceSetup ();

      NetDeviceContainer ConfigureDevices (NodeContainer &n);
  };
}

#endif 
