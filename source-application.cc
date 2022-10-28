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
#include "socket_io.h"
#include "inttypes.h"




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
    m_number_of_packets_to_send = 10;
    isStarted = false;
    starttime = 0;
    gal_pn = 0;
    ploss = 0.0;
    lb = 0;
    packetsSend = 0;
    packetsRetransmitted = 0;
    g.initGilbert_Elliott (ploss, lb);
    pbb.max_length = MAX_PBBFR_SIZE;
    root = new Socket_io::Root;
    for (int i = 0; i < MTR; i++)
      {
        root->child[i] = NULL;
      }

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

    m_recv_socket1->SetRecvCallback(MakeCallback(&SourceApplication::HandleReadTwo, this));
    //Simulator::Schedule(Seconds (3), &SourceApplication::check_udp_socket, this);
    //this->check_udp_socket ();

  }

  int SourceApplication::check_udp_socket ()
  {
    if(isStarted == false)
      {
        if (starttime == 0)
          {
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC, &tp);
            starttime = tp.tv_nsec;
          }
        isStarted = true;
      }
   // for (int i = 0; i< m_number_of_packets_to_send; i++)
    //  {
        Ptr<Packet> packet = Create<Packet>(MTU_SIZE);
        PacketDataTag tag;
        tag.SetNumberOfRepeat (0);
        if(gal_pn == MAX_PN) gal_pn = 0; else gal_pn++;
        tag.SetSeqNumber (gal_pn);
        tag.SetNodeId (GetNode ()->GetId ());
        tag.SetPacketId (IDM_UDP_ARQ_VIDEO);
        tag.SetTimestamp (Simulator::Now ());
        tag.SetTreeNumber (0);
        packet->AddPacketTag (tag);

        pbb.new_packet_tag.number_of_repeat = tag.GetNumberOfRepeat ();
        pbb.new_packet_tag.seq_number = tag.GetSeqNumber ();
        pbb.new_packet_tag.nodeId = tag.GetNodeId ();
        pbb.new_packet_tag.packet_id = tag.GetpacketId ();
        pbb.new_packet_tag.timestamp = tag.GetTimestamp ();
        pbb.new_packet_tag.nt = tag.GetTreeNumber ();
        pbb.new_packet_tag.next = NULL;
        pbb.shift_buffer ();
        if (g.getState ())
          {
            if(this->SendPacket (packet) == EXIT_SUCCESS)
              {

               /* NS_LOG_INFO(TEAL_CODE << "SendPacket: node " << GetNode ()->GetId ()<< " Send " << packet->GetSize() << " bytes"
                            << " at time " << Now().GetSeconds()<< " seq-number: " << tag.GetSeqNumber () << END_CODE);
                */
                packetsSend++;
                //printf (".");

                printf (PURPLE_CODE);
                printf (" %" PRIu32, tag.GetSeqNumber ());
                printf (END_CODE);

              }

          }
        else
          {
            printf (" l");
          }

     // }
    //Simulator::Schedule(Seconds (4), &SourceApplication::check_udp_socket, this);


    return EXIT_SUCCESS;
  }

  void SourceApplication::print_results()
  {
    printf ("\nPackets sent: %d \n", packetsSend);
    printf ("Packets retransmitted: %d \n", packetsRetransmitted);
  }

  void SourceApplication::HandleReadTwo(Ptr<Socket> socket)
  {
    //NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    NackDataTag nack_tag;
    while ((packet = socket->RecvFrom(from)))
      {
        if(packet->PeekPacketTag (nack_tag))
          {
            if (nack_tag.GetPacketId () == IDM_UDP_ARQ_NACK_AL)
              {
               // printf(" recvNack");
                /*
                NS_LOG_INFO(PURPLE_CODE << "HandleReadTwo: " << " node " << GetNode ()->GetId () << " Nack received"
                            << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                            << " port " <<InetSocketAddress::ConvertFrom (from).GetPort () << "" <<  END_CODE);

                */
                uint32_t nack_n = nack_tag.GetNumberOfRepeat ();
                int nack_bc = nack_tag.GetAmountOfBurst ();
                unsigned char nack_nt = nack_tag.GetTreeNumber ();
                for (int i = 0; i < nack_bc; i++)
                  {
                    int nack_pn = nack_tag.get_uint (nack_tag.burst_first_sn, i);
                    int nack_bl = nack_tag.get_uint (nack_tag.bursts_length, i);
                    //printf("nack_pn= %d", nack_pn );
                    //printf("nack_bl= %d", nack_bl );

                    unsigned char nack_dwbl = 0;
                    //unsigned long nack_fpn = nack_pn;

                    while (nack_bl > 0)
                      {
                        if (pbb.get_packet_tag_by_p2p_pn (nack_pn, nack_nt) == EXIT_SUCCESS)
                          {

                            //fprintf (log_file," p2p_pn=%lu",pbb.new_packet.p2p_pn);
                            Ptr<Packet> req_packet = Create<Packet>(MTU_SIZE);
                            PacketDataTag tag;
                            tag.SetNumberOfRepeat (nack_n);
                            tag.SetSeqNumber (pbb.new_packet_tag.seq_number);
                            tag.SetNodeId (pbb.new_packet_tag.nodeId);
                            tag.SetPacketId (pbb.new_packet_tag.packet_id);
                            tag.SetTimestamp (pbb.new_packet_tag.timestamp);
                            tag.SetTreeNumber (pbb.new_packet_tag.nt);
                            req_packet->AddPacketTag (tag);
                            if (this->SendPacket (req_packet) == EXIT_SUCCESS)
                              {
                                packetsRetransmitted++;
                                printf(" r%lu ", (unsigned long)tag.GetSeqNumber ());
                              }

                          }
                        else
                          nack_dwbl++;
                        nack_pn++;
                        nack_bl--;
                      }//while (nack_bl>0)

                    /*
                    if (nack_dwbl > 0)
                      {
                        put_uc (bfr_out, 0, IDM_UDP_ARQ_DNWM_AL);
                        put_uc (bfr_out, 1, nack_nt);
                        put_ul (bfr_out, 2, nack_fpn);
                        put_uc (bfr_out, 6, nack_dwbl);
                        if ((bytes_out = sendto(s, bfr_out, 7, 0, (struct sockaddr *) &addr, sizeof(struct sockaddr))) < 0)
                          fprintf(log_file,"Error %d: can't send message %d to my child.\n", errno, IDM_UDP_ARQ_DNWM_AL);
                        else
                          fprintf(log_file,"SENT DNWM sn=%lu fpn=%lu dwbl=%d\n",nack_pn,nack_fpn,nack_dwbl);

                      }
                    */
                  }//for (nack.bc)

              }
          }
        else {
            // some code
          }

      }
  }

  int SourceApplication::SendPacket(Ptr<Packet> packet)
  {

    //NS_LOG_FUNCTION (this << m_my_addr << m_port1 );

    m_send_socket->Connect(InetSocketAddress(m_destination_addr, m_port1));
    if(m_send_socket->Send(packet)> 0)
      return EXIT_SUCCESS;
    else return EXIT_FAILURE;
    //Simulator::Schedule(Seconds (3), &SourceApplication::SendPacket, this, packet); //, dest_ip, 7777);
  }



} // namespace ns3
