#ifndef NS3_SINK_ARQ_APPLICATION_H
#define NS3_SINK_ARQ_APPLICATION_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "packet-data-tag.h"
#include "nack-data-tag.h"
#include <algorithm>




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



      /** \brief Send nack packet. This creates a new socket every time (not the best solution)
      */
      void SendNack (uint32_t seq_number);


      bool findPrev(uint32_t prev);



      void SetDestinationAddr(Ipv4Address dest_addr);
      Ipv4Address GetDestinationAddr();

      void SetMyAddr(Ipv4Address my_addr);
      Ipv4Address GetMyAddr();

      Time m_random_offset;

      Time m_send_time; /**< How often do you broadcast messages */
      std::list<uint32_t> prevlist; /**< A list of requested packets*/


      uint32_t exp; /**< Sequence number of expected packet */

    private:


      void SetupReceiveSocket (Ptr<Socket> socket, Ipv4Address myAddr, uint16_t port);
      virtual void StartApplication ();


      Ptr<Socket> m_recv_socket1; /**< A socket to receive on a specific port */
      Ptr<Socket> m_recv_socket2; /**< A socket to receive on a specific port */
      uint16_t m_port1;
      uint16_t m_port2;
      uint32_t m_packet_size;
     // Ptr<MyHeader> arqHeader;
      Ipv4Address m_destination_addr;
      Ipv4Address m_my_addr;

      uint32_t m_number_of_packets_to_send;




      uint32_t prev; /**< Sequence number of the previous received packet */

      Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */
  };
}

#endif
