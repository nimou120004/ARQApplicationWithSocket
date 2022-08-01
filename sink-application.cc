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
  NS_LOG_COMPONENT_DEFINE("SinkApplication");
  NS_OBJECT_ENSURE_REGISTERED(SinkApplication);

  TypeId
  SinkApplication::GetTypeId()
  {
    static TypeId tid = TypeId("ns3::SinkApplication")
        .AddConstructor<SinkApplication>()
        .SetParent<Application>();
    return tid;
  }

  TypeId
  SinkApplication::GetInstanceTypeId() const
  {
    return SinkApplication::GetTypeId();
  }

  //Constructor
  SinkApplication::SinkApplication()
  {
    m_port1 = 7777;
    m_port2 = 9999;
    m_packet_size = 1000;

  }

  //Destructor
  SinkApplication::~SinkApplication()
  {
  }

  void SinkApplication::SetupReceiveSocket(Ptr<Socket> socket, Ipv4Address addr, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(addr, port);
    if (socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
  }

  void SinkApplication::SetDestinationAddr (Ipv4Address dest_addr){
    m_destination_addr = dest_addr;
  }
  Ipv4Address SinkApplication::GetDestinationAddr (){
    return m_destination_addr;
  }

  void SinkApplication::SetMyAddr (Ipv4Address my_addr){
    m_my_addr = my_addr;
  }
  Ipv4Address SinkApplication::GetMyAddr (){
    return m_my_addr;
  }

  void SinkApplication::StartApplication()
  {
     NS_LOG_FUNCTION("Start sink application  ...");
    //Receive sockets

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recv_socket1 = Socket::CreateSocket(GetNode(), tid);
    //m_recv_socket2 = Socket::CreateSocket(GetNode(), tid);

    SetupReceiveSocket(m_recv_socket1, m_my_addr, m_port1);
    //SetupReceiveSocket(m_recv_socket2, "1.0.0.1", m_port2);

    //Send Socket
    m_send_socket = Socket::CreateSocket(GetNode(), tid);

    //Simulator::Schedule(Seconds (1), &SinkApplication::SendPacket, this);

    m_recv_socket1->SetRecvCallback(MakeCallback(&SinkApplication::HandleReadOne, this));
    //m_recv_socket2->SetRecvCallback(MakeCallback(&SinkApplication::HandleReadTwo, this));



  }

  void SinkApplication::HandleReadOne(Ptr<Socket> socket)
  {
    //NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;

    Address from;
    Address localAddress;
    PacketDataTag tag;

    while ((packet = socket->RecvFrom(from)))
      {
        if(packet->PeekPacketTag (tag)){

            NS_LOG_INFO(TEAL_CODE << "HandleReadOne: node " << GetNode ()->GetId ()<< " Received a Packet of size: " << packet->GetSize()
                    << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                    << " port " <<InetSocketAddress::ConvertFrom (from).GetPort () << " seq-number: " << tag.GetSeqNumber () << END_CODE);
        }else {
            NS_LOG_INFO(PURPLE_CODE << "HandleReadOne: node " << GetNode ()->GetId ()<< " Received a Packet of size: " << packet->GetSize()
                  << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                  << " port " <<InetSocketAddress::ConvertFrom (from).GetPort ()
                  << END_CODE);
            }
      }
  }

  void SinkApplication::HandleReadTwo(Ptr<Socket> socket)
  {
    //NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    PacketDataTag tag;
    while ((packet = socket->RecvFrom(from)))
      {
        if(packet->PeekPacketTag (tag)){

              NS_LOG_INFO(PURPLE_CODE << "HandleReadTwo: node " << GetNode ()->GetId ()<< " Received a Packet of size: " << packet->GetSize()
                    << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                    << " port " <<InetSocketAddress::ConvertFrom (from).GetPort () << " seq-number: " << tag.GetSeqNumber ()
                    << END_CODE);
        }else {
          NS_LOG_INFO(PURPLE_CODE << "HandleReadTwo: node " << GetNode ()->GetId ()<< " Received a Packet of size: " << packet->GetSize()
                << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                << " port " <<InetSocketAddress::ConvertFrom (from).GetPort ()
                << END_CODE);
          }

      }
  }

  void SinkApplication::SendPacket()
  {

    //NS_LOG_FUNCTION (this << packet << destination << port << packet->PeekPacketTag (tag));

    for(int i = 0; i< 50 ; i++){
        Ptr <Packet> packet = Create <Packet> (m_packet_size);
        PacketDataTag tag;
        tag.SetSeqNumber (i);
        packet->AddPacketTag (tag);

        m_send_socket->Connect(InetSocketAddress(m_destination_addr, m_port1));
        m_send_socket->Send(packet);

      }
    Simulator::Schedule(Seconds (3), &SinkApplication::SendPacket, this);

  }

} // namespace ns3
