#include "ns3/log.h"
#include "source-application.h"
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




#define PURPLE_CODE "\033[95m"
#define CYAN_CODE "\033[96m"
#define TEAL_CODE "\033[36m"
#define BLUE_CODE "\033[94m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define LIGHT_YELLOW_CODE "\033[93m"
#define RED_CODE "\033[91m"
#define BOLD_CODE "\033[1m"
#define END_CODE "\033[0m"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("SourceApplication");
  NS_OBJECT_ENSURE_REGISTERED(SourceApplication);

  TypeId
  SourceApplication::GetTypeId()
  {
    static TypeId tid = TypeId("ns3::SourceApplication")
        .AddConstructor<SourceApplication>()
        .SetParent<Application>();
    return tid;
  }

  TypeId
  SourceApplication::GetInstanceTypeId() const
  {
    return SourceApplication::GetTypeId();
  }

  //Constructor
  SourceApplication::SourceApplication()
  {
    m_port1 = 7777;
    m_port2 = 9999;
    m_packet_size = 1000;
    m_number_of_packets_to_send = 50;
    prev = exp = 0;

  }

  //Destructor
  SourceApplication::~SourceApplication()
  {
  }

  void SourceApplication::SetupReceiveSocket(Ptr<Socket> socket, Ipv4Address myAddr, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(myAddr, port);
    if (socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
  }

  void SourceApplication::SetDestinationAddr (Ipv4Address dest_addr){
    m_destination_addr = dest_addr;
  }
  Ipv4Address SourceApplication::GetDestinationAddr (){
    return m_destination_addr;
  }

  void SourceApplication::SetMyAddr (Ipv4Address my_addr){
    m_my_addr = my_addr;
  }
  Ipv4Address SourceApplication::GetMyAddr (){
    return m_my_addr;
  }

  void SourceApplication::StartApplication()
  {

    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
    m_random_offset = MicroSeconds (rand->GetValue(2,10));

    NS_LOG_FUNCTION("Start application ... " << m_my_addr);

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
    m_recv_socket2 = Socket::CreateSocket(GetNode(), tid);

    SetupReceiveSocket(m_recv_socket1, m_my_addr, m_port1);
    SetupReceiveSocket(m_recv_socket2, m_my_addr, m_port2);

    //Send Socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);

    //If the application of node source, then sendPacket will be scheduled
    if(this->m_my_addr == ("10.1.1.2")){
        for(int i = 1, j = 2; i <= 3; i+=j){
            Ptr <Packet> packet = Create <Packet> (m_packet_size);
            PacketDataTag tag;
            tag.SetSeqNumber (i);
            tag.SetTimestamp (Now ());
            packet->AddPacketTag (tag);
            this->SendPacket (packet);
           // Simulator::Schedule(Seconds (2), &SourceApplication::SendPacket, this, packet); //, dest_ip, 7777);

          }


      }

    if(this->m_my_addr == ("10.1.1.1")){
        //If the application of node sink, then the Handle packet method will be called

        m_recv_socket1->SetRecvCallback(MakeCallback(&SourceApplication::HandleReadOne, this));

        //m_recv_socket2->SetRecvCallback(MakeCallback(&SourceApplication::HandleReadTwo, this));


      }

    if(this->m_my_addr == ("10.1.1.2")){
        //If the application of node sink, then the Handle packet method will be called

        //m_recv_socket1->SetRecvCallback(MakeCallback(&SourceApplication::HandleReadTwo, this));

        m_recv_socket2->SetRecvCallback(MakeCallback(&SourceApplication::HandleReadTwo, this));


      }






  }

  void SourceApplication::HandleReadOne(Ptr<Socket> socket)
  {
    //NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;

    Address from;
    Address localAddress;
    PacketDataTag tag;


    while ((packet = socket->RecvFrom(from)))
      {
        if(packet->PeekPacketTag (tag)){

            NS_LOG_INFO(TEAL_CODE << "HandleReadOne: node " << GetNode ()->GetId ()<< " Received " << packet->GetSize() << " bytes"
                        << " at time " << Now().GetSeconds ()<< " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                        << " port " <<InetSocketAddress::ConvertFrom (from).GetPort () << " seq-number: " << tag.GetSeqNumber () << END_CODE);

            if(tag.GetSeqNumber () != prev + 1)
              {

                if (!findPrev (tag.GetSeqNumber ()))  //loss event
                  {
                    exp = prev + 1;
                    NS_LOG_INFO (TEAL_CODE << "Packetloss : expected packet number " << prev + 1 << " but received packet number " << tag.GetSeqNumber ()
                                 << " nack was sent");
                    prev = tag.GetSeqNumber ();
                    prevlist.push_back (exp);
                    //sqNack = prev + 1;
                    this->SendNack (exp);

                  }
                else //this is a requested packet
                  {
                    NS_LOG_INFO (TEAL_CODE << "Packet recovery : recovered packet n " << tag.GetSeqNumber () );
                    prevlist.remove (tag.GetSeqNumber ());

                  }

              }
            else
              {
                prev++;
              }
          }
        else
          {
            NS_LOG_INFO(PURPLE_CODE << "HandleReadOne: node " << GetNode ()->GetId ()<< " Received a Packet of size: " << packet->GetSize()
                        << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                        << " port " <<InetSocketAddress::ConvertFrom (from).GetPort ()
                        << END_CODE);
          }
      }
  }

  void SourceApplication::HandleReadTwo(Ptr<Socket> socket)
  {
    //NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    NackDataTag tag;
    while ((packet = socket->RecvFrom(from)))
      {
        if(packet->PeekPacketTag (tag)){

            NS_LOG_INFO(PURPLE_CODE << "HandleReadTwo: " << " node " << GetNode ()->GetId () << " Nack received"
                        << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                        << " port " <<InetSocketAddress::ConvertFrom (from).GetPort () << END_CODE);

            Ptr <Packet> packet = Create <Packet> (m_packet_size);
            PacketDataTag packet_tag;
            packet_tag.SetSeqNumber (tag.GetSeqNumber ());
            packet_tag.SetTimestamp (Now ());
            packet->AddPacketTag (packet_tag);
            this->SendPacket (packet);

          }else {
            NS_LOG_INFO(PURPLE_CODE << "HandleReadTwo: node " << GetNode ()->GetId ()<< " Received a Packet of size: " << packet->GetSize()
                        << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                        << " port " <<InetSocketAddress::ConvertFrom (from).GetPort ()
                        << END_CODE);
          }

      }
  }

  void SourceApplication::SendPacket(Ptr<Packet> packet)
  {

    //NS_LOG_FUNCTION (this << m_my_addr << m_port1 );

    m_send_socket->Connect(InetSocketAddress(m_destination_addr, m_port1));
    m_send_socket->Send(packet);
    //Simulator::Schedule(Seconds (3), &SourceApplication::SendPacket, this, packet); //, dest_ip, 7777);
  }


  void SourceApplication::SendNack (uint32_t seq_number)
  {

    //NS_LOG_FUNCTION (this << m_my_addr << m_port1 );
    Ptr<Packet> nack = Create<Packet>(80);
    NackDataTag tag;
    tag.SetNodeId (this->GetNode ()->GetId ());
    tag.SetTimestamp (Now ());
    tag.SetSeqNumber (seq_number);
    nack->AddPacketTag (tag);

    m_send_socket->Connect(InetSocketAddress(m_destination_addr, m_port2));
    m_send_socket->Send(nack);
    //Simulator::Schedule(Seconds (3), &SourceApplication::SendPacket, this, packet); //, dest_ip, 7777);
  }

  bool SourceApplication::findPrev(uint32_t prev){

    std::list<uint32_t>::iterator findIter = std::find (prevlist.begin(), prevlist.end(), prev);
    if(findIter != prevlist.end())
      {
        return 1;
      }else{
        return 0;
      }

  }


} // namespace ns3
