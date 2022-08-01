#ifndef NS3_SINK_ARQ_APPLICATION_H
#define NS3_SINK_ARQ_APPLICATION_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "packet-data-tag.h"



using namespace ns3;

namespace ns3
{
  class SinkApplication : public Application
  {
    public:
      SinkApplication ();
      virtual ~SinkApplication ();

      // to integrate with ns3 we should use GetTypeId method
      static TypeId GetTypeId ();
      virtual TypeId GetInstanceTypeId () const;

      /** \brief handles incoming packets on port 7777
       */
      void HandleReadOne (Ptr<Socket> socket);

      /** \brief handles incoming packets on port 9999
       */
      void HandleReadTwo (Ptr<Socket> socket);

      /** \brief Send an outgoing packet. This creates a new socket every time (not the best solution)
      */
      void SendPacket ();

      /** \brief return a custom packet with Arq header
      */
      // Ptr<Packet> CostumePacket (uint32_t sequenceNumber);

      void SetDestinationAddr(Ipv4Address dest_addr);
      Ipv4Address GetDestinationAddr();

      void SetMyAddr(Ipv4Address my_addr);
      Ipv4Address GetMyAddr();



    private:


      void SetupReceiveSocket (Ptr<Socket> socket,Ipv4Address addr, uint16_t port);
      virtual void StartApplication ();


      Ptr<Socket> m_recv_socket1; /**< A socket to receive on a specific port */
      Ptr<Socket> m_recv_socket2; /**< A socket to receive on a specific port */
      uint16_t m_port1;
      uint16_t m_port2;
      uint32_t m_packet_size;
     // Ptr<MyHeader> arqHeader;
      Ipv4Address m_destination_addr;
      Ipv4Address m_my_addr;



      Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */
  };
}

#endif
