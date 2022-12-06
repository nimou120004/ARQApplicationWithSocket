#include "relay_application.h"

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
  NS_LOG_COMPONENT_DEFINE("RelayApplication");
  NS_OBJECT_ENSURE_REGISTERED(RelayApplication);

  TypeId
  RelayApplication::GetTypeId()
  {
    static TypeId tid = TypeId("ns3::RelayApplication")
        .AddConstructor<RelayApplication>()
        .SetParent<Application>();
    return tid;
  }

  TypeId
  RelayApplication::GetInstanceTypeId() const
  {
    return RelayApplication::GetTypeId();
  }

  //Constructor
  RelayApplication::RelayApplication()
  {
    isArqEnabled = true;
    for (int i = 0; i < MTR; i++)
      {
        if (!isArqEnabled)
          {
            al[i].isActive=false;
          }
        al[i].nt = i;
      }
    packets_received = 0;
    packets_recovered = 0;
    ploss = 0;
    lb = 0;
    ctrl_c.init_c (ploss);
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
  RelayApplication::~RelayApplication()
  {
  }

  void RelayApplication::SetupReceiveSocket(Ptr<Socket> socket, Ipv4Address myAddr, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(myAddr, port);
    if (socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
  }

  void RelayApplication::SetDestinationAddr (Ipv4Address dest_addr){
    m_destination_addr = dest_addr;
  }
  Ipv4Address RelayApplication::GetDestinationAddr (){
    return m_destination_addr;
  }

  void RelayApplication::SetMyAddr (Ipv4Address my_addr){
    m_my_addr = my_addr;
  }
  Ipv4Address RelayApplication::GetMyAddr (){
    return m_my_addr;
  }

  void RelayApplication::SetSourceAddress(Ipv4Address src_addr)
  {
    m_source_add = src_addr;
  }

  Ipv4Address RelayApplication::GetSourceAddress()
  {
    return m_source_add;
  }

  void RelayApplication::StartApplication()
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

    m_recv_socket1->SetRecvCallback(MakeCallback(&RelayApplication::HandleReadTwo, this));
    /*
    for (int i = 0; i < 10; i++)
      {
        Simulator::Schedule (Seconds (3 + (i * 0.01)), &RelayApplication::check_udp_socket, this);
      }
    */
  }


  void RelayApplication::print_results()
  {
    printf ("\nPackets sent: %d \n", packetsSend);
    printf ("Packets retransmitted: %d \n", packetsRetransmitted);
  }

  void RelayApplication::HandleReadTwo(Ptr<Socket> socket)
  {
    //NS_LOG_FUNCTION(this << socket);
    //printf(" recvNack");

    Ptr<Packet> packet;
    Address from;
    //NackDataTag nack_tag;
    PacketDataTag pckt_tag;
    while ((packet = socket->RecvFrom(from)))
      {

        /* if(packet->PeekPacketTag (nack_tag))
          {
            if (nack_tag.GetPacketId () == IDM_UDP_ARQ_NACK_AL)
              {
                // printf(" recvNack");

                NS_LOG_INFO(PURPLE_CODE << "HandleReadTwo: " << " node " << GetNode ()->GetId () << " Nack received"
                            << " at time " << Now().GetSeconds() << " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                            << " port " <<InetSocketAddress::ConvertFrom (from).GetPort () << "" <<  END_CODE);


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
                            std::memcpy(tag.sourceAddr, pbb.new_packet_tag.sourceAddr, sizeof(pbb.new_packet_tag.sourceAddr));
                            req_packet->AddPacketTag (tag);
                            if (this->SendPacket (req_packet, m_destination_addr) == EXIT_SUCCESS)
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

                  }//for (nack.bc)

              }
          }
        else*/
        if (packet->PeekPacketTag (pckt_tag))
          {
            printf("\n");
            if (pckt_tag.GetpacketId () == IDM_UDP_PING)
              {

                Ptr<Packet> packet = Create<Packet>(MTU_SIZE);

                PacketDataTag tag;
                tag.SetNumberOfRepeat (0);
                tag.SetNodeId (pckt_tag.GetNodeId ());
                tag.SetPacketId (pckt_tag.GetpacketId ());
                tag.SetTimestamp (pckt_tag.GetTimestamp ());
                tag.SetTreeNumber (0);
                std::memcpy(pckt_tag.sourceAddr, tag.sourceAddr, sizeof(tag.sourceAddr));

                packet->AddPacketTag (tag);
                if(this->SendPacket (packet, m_source_add) == EXIT_SUCCESS)
                  {
                    printf (GREEN_CODE);
                    printf ("R_PING ");
                    printf (END_CODE);
                  }
              }
            else if (pckt_tag.GetpacketId () == IDM_UDP_ARQ_VIDEO && packet != NULL)
              {
                //printf("g");
                pbb.new_packet_tag.number_of_repeat = pckt_tag.GetNumberOfRepeat ();
                pbb.new_packet_tag.seq_number = pckt_tag.GetSeqNumber ();
                pbb.new_packet_tag.nodeId = pckt_tag.GetNodeId ();
                pbb.new_packet_tag.packet_id = pckt_tag.GetpacketId ();
                pbb.new_packet_tag.timestamp = pckt_tag.GetTimestamp ();
                pbb.new_packet_tag.nt = pckt_tag.GetTreeNumber ();
                int nt = pbb.new_packet_tag.nt; // ree number of current packet for this peer. in this case, always equals to 1
                std::memcpy(pbb.new_packet_tag.sourceAddr, pckt_tag.sourceAddr, sizeof(pbb.new_packet_tag.sourceAddr));
                pbb.new_packet_tag.next = NULL;

                if(pbb.add_packet_tag (&pbb.new_packet_tag) == EXIT_SUCCESS)
                  {
                    if (SendPacket (packet, m_destination_addr) == EXIT_SUCCESS)
                      {
                        printf (GREEN_CODE);
                        printf (" %" PRIu32, pbb.new_packet_tag.seq_number);
                        printf (END_CODE);
                      }

                    if(pbb.new_packet_tag.number_of_repeat == 0)
                      {
                        packets_received ++;
                      }
                    else
                      {
                        packets_recovered ++;
                      }
                    /*
                    if(my_peer->child[nt] != NULL)
                      {
                        if (g[nt].getState ())
                          {
                            // some code to forward to packet to the next peer in case of tree topology
                          }
                      }
                    */
                  }
                if (pckt_tag.GetpacketId () == IDM_UDP_ARQ_VIDEO)
                  {
                    //printf("the node id is: %" PRIu32, pbb.new_packet_tag.nodeId);
                        if ((al[nt].isActive) && (!al[nt].isStarted))
                          {
                            al[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al[nt].isStarted = true;
                          }

                    //my_peer->parent[MTR]->ping_n = 0;//a number of activity requests (if ping_n=0 then OK)
                    //my_peer->parent[MTR]->ping_t = skt_io->GetTickCount();

                    if (al[nt].isActive)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al[nt].cur = pckt_tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al[i].isActive)
                            {
                              //printf("isFirstPacket\n");
                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              //NackDataTag nack_tag;
                              al[i].send_nack(m_source_add , m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId);
                            }
                      }

                  }

              }

          }
      }
  }

    int RelayApplication::SendPacket(Ptr<Packet> packet, Ipv4Address to)
    {

      //NS_LOG_FUNCTION (this << m_my_addr << m_port1 );

      m_send_socket->Connect(InetSocketAddress(to, m_port1));
      if(m_send_socket->Send(packet)> 0)
        return EXIT_SUCCESS;
      else return EXIT_FAILURE;
      //Simulator::Schedule(Seconds (3), &RelayApplication::SendPacket, this, packet); //, dest_ip, 7777);
    }



  } // namespace ns3


