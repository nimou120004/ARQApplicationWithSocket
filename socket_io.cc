#include "ns3/log.h"
#include "sink-application.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/csma-net-device.h"
#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "packet-data-tag.h"
#include "nack-data-tag.h"
#include <list>
#include <algorithm>
#include <iostream>
#include <time.h>

#include "socket_io.h"

namespace ns3
{

  Socket_io::Socket_io()
  {

  }

  unsigned long Socket_io::GetTickCount ()
  {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_nsec;
  }

}

