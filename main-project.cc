#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "source-application.h"
#include "sink-application.h"
#include "nack-data-tag.h"
#include "packet-data-tag.h"
#include "netDevice-setup.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-channel.h"


#define PACKET_SIZE  1000;
using namespace ns3;

/**
This example illustrates the use of udp ARQ Applications (client/server). It should work regardless of the device you have.
*/

int main (int argc, char *argv[])
{
  CommandLine cmd;
  //LogComponentEnable ("SourceApplication", LOG_LEVEL_INFO);

  uint32_t nNodes = 7;
  double simTime = 60; //4 seconds
  double distance = 37.0;
  bool enablePcap = false;
  cmd.AddValue ("t","Simulation Time", simTime);
  cmd.AddValue ("n", "Number of nodes", nNodes);
  cmd.AddValue ("pcap", "Enable PCAP", enablePcap);
  cmd.AddValue ("d", "distance", distance);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create(nNodes);

  //LogComponentEnable ("SourceApplication", LOG_LEVEL_FUNCTION);
  /*
    You must setup Mobility. Any mobility will work. Use one suitable for your work
  */
  /* example 1 using ConstantVelocityMobilityModel
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install(nodes);
  for (uint32_t i=0 ; i<nodes.GetN(); i++)
  {
    //set initial positions, and velocities
    //NS_LOG_LOGIC ("Setting up mobility for node " << i);
    //NS_LOG_ERROR ("An error happened :(");
    Ptr<ConstantVelocityMobilityModel> cvmm = DynamicCast<ConstantVelocityMobilityModel> (nodes.Get(i)->GetObject<MobilityModel>());
    cvmm->SetPosition ( Vector (20+i*5, 20+(i%2)*5, 0));
    cvmm->SetVelocity ( Vector (10+((i+1)%2)*5,0,0) );
  }
  */
  /*example 2 using ConstantPositionMobilityModel*/
  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  for (int i=0 ; i<(int)nodes.GetN (); i++)
    {
      mobility.Install (nodes.Get (i));
    }
  /*end example 2*/

  NetDeviceSetup setup;

  NetDeviceContainer devices = setup.ConfigureDevices (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifaces;
  ifaces = address.Assign (devices);


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Packet::EnablePrinting ();

  // Create source application
  std::vector<Ipv4Address> srcAddresses;
  Ipv4Address dest_ip ("10.1.1.1");
  for (int i = 1; i < (int)nNodes; i++)
    {
      Ptr <SourceApplication> appSource = CreateObject <SourceApplication> ();
      std:: string str = "10.1.1.";
      appSource->SetDestinationAddr (dest_ip);
      str = str + std::to_string (i+1);
      Ipv4Address my_addr (str.c_str ());
      appSource->SetMyAddr (my_addr);
      srcAddresses.insert (srcAddresses.end (), my_addr);
      appSource->SetStartTime (Seconds(1 + 0.2*i));
      appSource->SetStopTime (Seconds (simTime));
      nodes.Get(i)->AddApplication (appSource);
    }

  //Create sink application
  Ipv4Address my_addr ("10.1.1.2");
  Ptr <SinkApplication> appSink = CreateObject <SinkApplication> ();
  appSink->SetStartTime (Seconds(1));
  appSink->SetStopTime (Seconds (simTime));
  Ipv4Address my_ip("10.1.1.1");
  appSink->SetMyAddr (my_ip);
  for (uint32_t i = 0; i < srcAddresses.size (); i++)
    {
      appSink->m_destination_addrs.insert (appSink->m_destination_addrs.end (), srcAddresses[i]);
    }
  nodes.Get(0)->AddApplication (appSink);


  LogComponentEnable ("SourceApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("SinkApplication", LOG_LEVEL_ALL);

  AnimationInterface anim("animARQ.xml");
  anim.SetConstantPosition (nodes.Get (0), 0.0, 0.0);
  for (int i = 1; i < (int)nodes.GetN (); i++)
    {
      anim.SetConstantPosition (nodes.Get (i), distance, 0.0);
    }


  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  //appSource->print_results ();

  appSink->print_results ();

  Simulator::Destroy ();

}
