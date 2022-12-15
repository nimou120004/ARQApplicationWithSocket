#ifndef NS3_SOURCE_ARQ_APPLICATION_H
#define NS3_SOURCE_ARQ_APPLICATION_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "packet-data-tag.h"
#include "playback_buffer.h"
#include "gilbert_elliott.h"
#include <algorithm>




using namespace ns3;

namespace ns3
{
  class SourceApplication : public Application
  {
  public:
    SourceApplication ();
    virtual ~SourceApplication ();

    // to integrate with ns3 we should use GetTypeId method
    static TypeId GetTypeId ();
    virtual TypeId GetInstanceTypeId () const;


    /** \brief handles incoming packets on port 9999
       */
    void HandleReadTwo (Ptr<Socket> socket);

    /** \brief Send an outgoing packet. This creates a new socket every time (not the best solution)
      */
    int SendPacket (Ptr<Packet> packet, Ipv4Address to);

    /** \brief handle the transmission and processing of data packets
      */
    int check_udp_socket();

    void print_results ();

    void SetDestinationAddr(Ipv4Address dest_addr);
    Ipv4Address GetDestinationAddr();

    void SetMyAddr(Ipv4Address my_addr);
    Ipv4Address GetMyAddr();

    Time m_random_offset;

    Time m_send_time; /**< How often do you broadcast messages */
    std::list<uint32_t> prevlist; /**< A list of requested packets*/
    uint8_t sourceAddr[4];
    int packetsSend;
    int packetsRetransmitted;
    uint32_t nodeId;
    bool active_mode;
    Ipv4Address m_relay_addr;

  private:


    void SetupReceiveSocket (Ptr<Socket> socket, Ipv4Address myAddr, uint16_t port);
    virtual void StartApplication ();


    Ptr<Socket> m_recv_socket1; /**< A socket to receive on a specific port */
    Ptr<Socket> m_recv_socket2; /**< A socket to receive on a specific port */
    uint16_t m_port1;
    uint16_t m_port2;
    double ploss, lb; /**< packet loss rate and burst loss for Gilbert-Elliott model */

    // Ptr<MyHeader> arqHeader;
    Ipv4Address m_destination_addr;
    Ipv4Address m_my_addr;


    int m_number_of_packets_to_send;

    Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */

    bool isStarted; /**< isStarted==true when first packet has created */
    long starttime; /**< time when first packet generated, it is used also to calculate average PLR */
    uint32_t gal_pn; /**< global sequence number of packet for arq */

    PlaybackBuffer pbb; /**< playback buffer for reordering received packets */
    Socket_io *skt_io;
    Socket_io::Root *root; /**< my root which is the node from which i sending packets */
    gilbert_Elliott g; /**< Gilbert-Elliott model for burst error (loss) */


  };
}

#endif
