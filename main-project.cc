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


#define PACKET_SIZE  1000;
using namespace ns3;

/**
This example illustrates the use of udp ARQ Applications (client/server). It should work regardless of the device you have.
*/

int main (int argc, char *argv[])
{
  /*
  CommandLine cmd;
  cmd.Parse (argc, argv);
  NodeContainer nodes;
  nodes.Create (4);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("1Gbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds(6560)));
  */
  CommandLine cmd;
  //LogComponentEnable ("SourceApplication", LOG_LEVEL_INFO);

  //Number of nodes
  uint32_t nNodes = 2;
  double simTime = 60; //4 seconds
  double interval = 0.5;
  bool enablePcap = false;
  cmd.AddValue ("t","Simulation Time", simTime);
  cmd.AddValue ("i", "Broadcast interval in seconds", interval);
  cmd.AddValue ("n", "Number of nodes", nNodes);
  cmd.AddValue ("pcap", "Enable PCAP", enablePcap);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create(nNodes);

  //LogComponentEnable ("SourceApplication", LOG_LEVEL_FUNCTION);
  /*
    You must setup Mobility. Any mobility will work. Use one suitable for your work
  */
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


  //Create our Two UDP applications sink and source
  Ptr <SinkApplication> appSink = CreateObject <SinkApplication> ();
  Ptr <SourceApplication> appSource = CreateObject <SourceApplication> ();


  //Set up sink application
  appSink->SetStartTime (Seconds(1));
  appSink->SetStopTime (Seconds (simTime));
  Ipv4Address dest_ip ("10.1.1.2");
  Ipv4Address my_ip("10.1.1.1");
  appSink->SetDestinationAddr (dest_ip);
  appSink->SetMyAddr (my_ip);

  // Set up source application
  appSource->SetStartTime (Seconds(2));
  appSource->SetStopTime (Seconds (simTime));
  Ipv4Address dest_ip2 ("10.1.1.1");
  appSource->SetDestinationAddr (dest_ip2);
  Ipv4Address my_addr2 ("10.1.1.2");
  appSource->SetMyAddr (my_addr2);


  //install one application at node 0, and the other at node 1
  nodes.Get(0)->AddApplication (appSink);
  nodes.Get(1)->AddApplication (appSource);



  LogComponentEnable ("SourceApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("SinkApplication", LOG_LEVEL_ALL);



  //Simulator::Schedule (Seconds (2), &SinkApplication::HandleReadOne, appSink->m_recv_socket1, appSink);

  for (int i = 0; i < 10; i++)
    {
      Simulator::Schedule (Seconds (3), &SourceApplication::check_udp_socket, appSource);
    }




  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  appSource->print_results ();
  appSink->print_results ();

  Simulator::Destroy ();

}
