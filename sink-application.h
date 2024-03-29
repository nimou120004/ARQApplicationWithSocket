#ifndef NS3_SINK_ARQ_APPLICATION_H
#define NS3_SINK_ARQ_APPLICATION_H
#include "ns3/socket.h"
#include "ns3/application.h"
#include "packet-data-tag.h"
#include "nack-data-tag.h"
#include "arq_line_socket.h"
#include <algorithm>
#include "playback_buffer.h"
#include "socket_io.h"
#include "gilbert_elliott.h"
#include "plr_counter.h"
//#include "vector"




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

    void print_results ();
    //void SetDestinationAddr(Ipv4Address dest_addr);
    int check_pbb();
    //Ipv4Address GetDestinationAddr();

    void SetMyAddr(Ipv4Address my_addr);

    Ipv4Address GetMyAddr();

    Time m_random_offset;

    Time m_send_time; /**< How often do you broadcast messages */
    std::list<uint32_t> prevlist; /**< A list of requested packets*/


    uint32_t exp; /**< Sequence number of expected packet */
    bool isArqEnabled;
    std::string path_temp; /**< log file path string */
    std::string Dir; /**< directory name where to create log files */
    Socket_io *skt_io;
    Socket_io::MyPeer *my_peer;

    int packets_received;
    int packets_recovered;
    Ipv4Address m_relay_addr;
    bool active_mode;

    double plr_corr, plr_pure; // plr with ARQ and without ARQ measured for each portion (1000 pckts)
    std::vector<Ipv4Address> m_destination_addrs;
    Ptr<Socket> m_recv_socket1; /**< A socket to receive on a specific port */


    std::string getTime();
    int switch_trasmission();
    int send_ping(std::vector<Ipv4Address> addrs);
  private:


    FILE    *plr_f_corr,    //corrected packet loss ratio file
            *plr_f_pure,    //pure packet loss ratio file
            *log_file;
    void SetupReceiveSocket (Ptr<Socket> socket, Ipv4Address myAddr, uint16_t port);
    virtual void StartApplication ();

    plr_counter plr_c_corr, //packet loss counter after ARQ recovery
    plr_c_pure; //packet loss counter without any correction method

    Ptr<Socket> m_recv_socket2; /**< A socket to receive on a specific port */
    uint16_t m_port1;
    uint16_t m_port2;
    uint32_t m_packet_size;
    unsigned long tmi;//calculation interval for plr (tm = lost + recieved -> plr = lost/tm)
    // Ptr<MyHeader> arqHeader;

    Ipv4Address m_my_addr;
    uint32_t m_number_of_packets_to_send;
    uint32_t prev; /**< Sequence number of the previous received packet */
    Ptr<Socket> m_send_socket; /**< A socket to listen on a specific port */
    arq_line_socket al[MTR], al2[MTR], al3[MTR], al4[MTR], al5[MTR], al6[MTR], al7[MTR], al8[MTR], al9[MTR], al10[MTR]; /**< arq lines for each parent peer */
    PlaybackBuffer pbb; /**< playback buffer for reordering received packets */
    gilbert_Elliott g[MTR];  /** < Gilbert model for packet loss simulation */
    double ploss, lb; /** < packet loss rate and burst loss for Gilbert-Elliott model */
    simple_c ctrl_c; /** < control channel model */
  };
}

#endif
