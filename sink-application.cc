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
#include "nack-data-tag.h"
#include "socket_io.h"
#include "inttypes.h"
#include "sys/stat.h"
#include "iostream"
#include "string.h"
#include "cstdio"
#include "ctime"
#include "time.h"



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
    //tmi = 500;
    //plr_c_corr.tm = tmi;
    //plr_c_pure.tm = tmi;
    active_mode = false;
    m_port1 = 7777;
    m_port2 = 9999;
    m_packet_size = 1000;
    m_number_of_packets_to_send = 50;
    prev = exp = 0;
    isArqEnabled = true;
    ploss = 0;
    lb = 0;
    packets_received = 0;
    packets_recovered = 0;
    skt_io = new Socket_io;
    for (int i = 0; i < MTR; i++)
      {
        if (!isArqEnabled)
          {
            al[i].isActive=false;
            al2[i].isActive=false;
            al3[i].isActive=false;
            al4[i].isActive=false;
            al5[i].isActive=false;
            al6[i].isActive=false;
            al7[i].isActive=false;
            al8[i].isActive=false;
            al9[i].isActive=false;
            al10[i].isActive=false;

          }
        al[i].nt = i;
        al2[i].nt = i;
        al3[i].nt = i;
        al4[i].nt = i;
        al5[i].nt = i;
        al6[i].nt = i;
        al7[i].nt = i;
        al8[i].nt = i;
        al9[i].nt = i;
        al10[i].nt = i;
      }
    my_peer = new Socket_io::MyPeer;
    my_peer->n = 0;
    for (int i = 0; i < MTR; i++)
      {
        my_peer->child[i] = NULL;
        my_peer->parent[i] = new Socket_io::MyPeer;
        my_peer->parent[i]->ping_n = 0;
        my_peer->parent[i]->ping_t = skt_io->GetTickCount ();
      }
    for (int i = 0; i < MTR; i++)
      {
        g[i].initGilbert_Elliott (ploss, lb);
      }
    ctrl_c.init_c (ploss);
    //creation of log files
    Dir = "Res";
    mkdir(Dir.c_str (), 0777);
    path_temp = Dir + "//test_" + getTime () + "_log.txt";
    log_file = std::fopen(path_temp.c_str (),"w");
    path_temp = Dir + "//test_" + getTime () + "_plr_corr.csv";
    plr_f_corr = fopen(path_temp.c_str(),"w");
    path_temp = Dir + "//test_" + getTime () + "_plr_pure.csv";
    plr_f_pure = fopen(path_temp.c_str(),"w");



  }

  //Destructor
  SinkApplication::~SinkApplication()
  {
  }

  void SinkApplication::SetupReceiveSocket(Ptr<Socket> socket, Ipv4Address myAddr, uint16_t port)
  {
    InetSocketAddress local = InetSocketAddress(myAddr, port);
    if (socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
  }

  /*
  void SinkApplication::SetDestinationAddr (Ipv4Address dest_addr){
    m_destination_addr = dest_addr;
  }
  Ipv4Address SinkApplication::GetDestinationAddr (){
    return m_destination_addr;
  }
  */

  void SinkApplication::SetMyAddr (Ipv4Address my_addr){
    m_my_addr = my_addr;
  }
  Ipv4Address SinkApplication::GetMyAddr (){
    return m_my_addr;
  }

  void SinkApplication::StartApplication()
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

    m_recv_socket1->SetRecvCallback(MakeCallback(&SinkApplication::HandleReadOne, this));

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
        if(packet->PeekPacketTag (tag))
          {
            if (tag.GetpacketId () == IDM_UDP_ARQ_VIDEO && packet != NULL)
              {
                //printf("g");
                pbb.new_packet_tag.number_of_repeat = tag.GetNumberOfRepeat ();
                pbb.new_packet_tag.seq_number = tag.GetSeqNumber ();
                pbb.new_packet_tag.nodeId = tag.GetNodeId ();
                pbb.new_packet_tag.packet_id = tag.GetpacketId ();
                pbb.new_packet_tag.timestamp = tag.GetTimestamp ();
                pbb.new_packet_tag.nt = tag.GetTreeNumber ();
                int nt = pbb.new_packet_tag.nt; // ree number of current packet for this peer. in this case, always equals to 1
                std::memcpy(pbb.new_packet_tag.sourceAddr, tag.sourceAddr, sizeof(pbb.new_packet_tag.sourceAddr));
                pbb.new_packet_tag.next = NULL;
                /*
                NS_LOG_INFO(TEAL_CODE << "HandleReadOne: node " << GetNode ()->GetId ()<< " Received " << packet->GetSize() << " bytes"
                            << " at time " << Now().GetSeconds ()<< " from " <<InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                            << " port " <<InetSocketAddress::ConvertFrom (from).GetPort () << " seq-number: " << tag.GetSeqNumber () << END_CODE);
                */
                if(pbb.add_packet_tag (&pbb.new_packet_tag) == EXIT_SUCCESS)
                  {
                    printf (CYAN_CODE);
                    printf (" %" PRIu32, pbb.new_packet_tag.seq_number);
                    printf (END_CODE);


                    if(pbb.new_packet_tag.number_of_repeat == 0)
                      {
                        packets_received ++;
                      }
                    else
                      {
                        packets_recovered ++;
                      }
                    if(my_peer->child[nt] != NULL)
                      {
                        if (g[nt].getState ())
                          {
                            // some code to forward to packet to the next peer in case of tree topology
                          }
                      }
                  }
                if (tag.GetpacketId () == IDM_UDP_ARQ_VIDEO)
                  {

                    //printf("the node id is: %" PRIu32, pbb.new_packet_tag.nodeId);
                    if (pbb.new_packet_tag.nodeId == 1)
                      {
                        if ((al[nt].isActive) && (!al[nt].isStarted))
                          {
                            al[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 2)
                      {
                        if ((al2[nt].isActive) && (!al2[nt].isStarted))
                          {
                            al2[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al2[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 3)
                      {
                        if ((al3[nt].isActive) && (!al3[nt].isStarted))
                          {
                            al3[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al3[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 4)
                      {
                        if ((al4[nt].isActive) && (!al4[nt].isStarted))
                          {
                            al4[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al4[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 5)
                      {
                        if ((al5[nt].isActive) && (!al5[nt].isStarted))
                          {
                            al5[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al5[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 6)
                      {
                        if ((al6[nt].isActive) && (!al6[nt].isStarted))
                          {
                            al6[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al6[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 7)
                      {
                        if ((al7[nt].isActive) && (!al7[nt].isStarted))
                          {
                            al7[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al7[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 8)
                      {
                        if ((al8[nt].isActive) && (!al8[nt].isStarted))
                          {
                            al8[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al8[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 9)
                      {
                        if ((al9[nt].isActive) && (!al9[nt].isStarted))
                          {
                            al9[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al9[nt].isStarted = true;
                          }
                      }
                    if (pbb.new_packet_tag.nodeId == 10)
                      {
                        if ((al10[nt].isActive) && (!al10[nt].isStarted))
                          {
                            al10[nt].first_in_transmission = pbb.new_packet_tag.seq_number ;
                            al10[nt].isStarted = true;
                          }
                      }


                    //my_peer->parent[MTR]->ping_n = 0;//a number of activity requests (if ping_n=0 then OK)
                    //my_peer->parent[MTR]->ping_t = skt_io->GetTickCount();

                    if ((al[nt].isActive) && pbb.new_packet_tag.nodeId == 1)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al[i].isActive)
                            {
                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              al[i].send_nack(m_destination_addrs[0] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);

                            }
                      }

                    if ((al2[nt].isActive) && pbb.new_packet_tag.nodeId == 2)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al2[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al2[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al2[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al2[i].isActive)
                            {
                              //printf("isFirstPacket\n");

                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              //NackDataTag nack_tag;
                              al2[i].send_nack(m_destination_addrs[1] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                            }
                      }
                    if ((al3[nt].isActive) && pbb.new_packet_tag.nodeId == 3)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al3[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al3[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al3[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al3[i].isActive)
                            {
                              //printf("isFirstPacket\n");

                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              //NackDataTag nack_tag;
                              al3[i].send_nack(m_destination_addrs[2] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                            }
                      }
                    if ((al4[nt].isActive) && pbb.new_packet_tag.nodeId == 4)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al4[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al4[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al4[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al4[i].isActive)
                            {
                              //printf("isFirstPacket\n");

                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              //NackDataTag nack_tag;
                              al4[i].send_nack(m_destination_addrs[3] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                            }
                      }
                    if ((al5[nt].isActive) && pbb.new_packet_tag.nodeId == 5)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al5[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al5[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al5[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al5[i].isActive)
                            {
                              //printf("isFirstPacket\n");

                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              //NackDataTag nack_tag;
                              al5[i].send_nack(m_destination_addrs[4] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                            }
                      }
                    if ((al6[nt].isActive) && pbb.new_packet_tag.nodeId == 6)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al6[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al6[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al6[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al6[i].isActive)
                            {
                              //printf("isFirstPacket\n");

                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              //NackDataTag nack_tag;
                              al6[i].send_nack(m_destination_addrs[5] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                            }
                      }
                    if ((al7[nt].isActive) && pbb.new_packet_tag.nodeId == 7)
                      {
                        //arq lines for packets with "nt" from 0 to mtratio-1
                        al7[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                        if (al7[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                          {
                            al7[nt].check();
                          }
                        for (int i=0 ;i < MTR; i++)
                          if (al7[i].isActive)
                            {
                              //printf("isFirstPacket\n");

                              Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                              //NackDataTag nack_tag;
                              al7[i].send_nack(m_destination_addrs[6] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                            }
                        if ((al8[nt].isActive) && pbb.new_packet_tag.nodeId == 8)
                          {
                            //arq lines for packets with "nt" from 0 to mtratio-1
                            al8[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                            if (al8[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                              {
                                al8[nt].check();
                              }
                            for (int i=0 ;i < MTR; i++)
                              if (al8[i].isActive)
                                {
                                  //printf("isFirstPacket\n");

                                  Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                                  //NackDataTag nack_tag;
                                  al8[i].send_nack(m_destination_addrs[7] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                                }
                          }
                        if ((al9[nt].isActive) && pbb.new_packet_tag.nodeId == 9)
                          {
                            //arq lines for packets with "nt" from 0 to mtratio-1
                            al9[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                            if (al9[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                              {
                                al9[nt].check();
                              }
                            for (int i=0 ;i < MTR; i++)
                              if (al9[i].isActive)
                                {
                                  //printf("isFirstPacket\n");

                                  Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                                  //NackDataTag nack_tag;
                                  al9[i].send_nack(m_destination_addrs[8] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                                }
                            if ((al10[nt].isActive) && pbb.new_packet_tag.nodeId == 10)
                              {
                                //arq lines for packets with "nt" from 0 to mtratio-1
                                al10[nt].cur = tag.GetSeqNumber (); //get_ul (bfr_in, 7); //old p2p packet number
                                if (al10[nt].is_it_first_packet(pbb.new_packet_tag.number_of_repeat) == EXIT_FAILURE)
                                  {
                                    al10[nt].check();
                                  }
                                for (int i=0 ;i < MTR; i++)
                                  if (al10[i].isActive)
                                    {
                                      //printf("isFirstPacket\n");

                                      Ptr<Packet> nack = Create<Packet>(MTU_NACK_SIZE);
                                      //NackDataTag nack_tag;
                                      al10[i].send_nack(m_destination_addrs[9] ,m_relay_addr, m_port1, &ctrl_c, nack, m_send_socket, pbb.new_packet_tag.nodeId, active_mode);
                                    }
                              }
                            timespec ts;
                            if (!plr_c_corr.isStarted)
                              {

                                clock_gettime(CLOCK_MONOTONIC, &ts);
                                plr_c_corr.starttime = ts.tv_sec;
                                plr_c_corr.isStarted = true;
                              }
                            if (!plr_c_pure.isStarted)
                              {
                                clock_gettime(CLOCK_MONOTONIC, &ts);
                                plr_c_pure.starttime = ts.tv_sec;
                                plr_c_pure.isStarted = true;
                              }
                          }


                      }
                  }
                else
                  {
                    //some code
                  }

              }

          }
        check_pbb ();
        //switch_trasmission();
      }

  }

  int SinkApplication::switch_trasmission()
  {
    if(plr_pure > 0.03)
      {
        active_mode = true;
        //switch to relay mode (send ping packet) by sending 7 ping packets
        for (int i= 0; i < 7; i++){
            send_ping(m_destination_addrs);
          }
        return EXIT_SUCCESS;
      }
    else
      {
        return EXIT_FAILURE;
      }

  }

  int SinkApplication::send_ping(std::vector<Ipv4Address> addrs)
  {
    Ptr<Packet> packet_ping = Create<Packet>(MTU_SIZE);
    PacketDataTag tag;
    tag.SetNumberOfRepeat (0);
    tag.SetSeqNumber (0);
    tag.SetNodeId (GetNode ()->GetId ());
    tag.SetPacketId (IDM_UDP_PING);
    tag.SetTimestamp (Simulator::Now ());
    tag.SetTreeNumber (0);
    m_my_addr.Serialize (tag.sourceAddr);
    packet_ping->AddPacketTag (tag);
    for (int i=0; i < (int)addrs.size(); i++){
        m_send_socket->Connect (InetSocketAddress(addrs[i], m_port1));
        if (m_send_socket-> Send (packet_ping) > 0)
          {
            printf (YELLOW_CODE);
            printf (" D_PING");
            printf (END_CODE);
            //return EXIT_SUCCESS;
          }

        //else return EXIT_FAILURE;
      }
    return EXIT_SUCCESS;

  }

  void SinkApplication::print_results ()
  {
    printf ("\nPackets received: %d \n", packets_received);
    printf ("Packets recovered: %d \n", packets_recovered);
  }

  int SinkApplication::check_pbb()
  {
    uint32_t sn; //temporary sequantial number
    uint32_t nr; //temporary packet repeat number
    if (pbb.play(sn,nr))
      {
        //calculate corrected plr if playback buffer was shifted
        fprintf(log_file,"plr_c_corr sn= %" PRIu32, sn);
        fprintf(log_file, "nr= %" PRIu32, nr);
        fprintf(log_file, "\n");
        plr_c_corr.cur = sn; //input for plr counter
        plr_c_corr.check(log_file);
        if (plr_c_corr.calculate() == EXIT_SUCCESS)
          {
            plr_corr = plr_c_corr.plr;
            if(active_mode){
                plr_c_corr.write_plr_to_file(plr_f_corr, 1);
              }
            else
              {
                plr_c_corr.write_plr_to_file(plr_f_corr, 0);
              }

          }

        //calculate pure plr if current outcoming packet isn't recovered one
        if (nr == 0)
          {
            fprintf(log_file,"plr_c_pure sn= %" PRIu32, sn);
            fprintf(log_file, "nr= %" PRIu32, nr);
            fprintf(log_file, "\n");
            plr_c_pure.cur = sn; //input for plr counter
            plr_c_pure.check(log_file);
            if (plr_c_pure.calculate() == EXIT_SUCCESS)
              {
                plr_pure = plr_c_pure.plr;
                if(active_mode)
                  {
                    plr_c_pure.write_plr_to_file(plr_f_pure, 1);
                  }
                else {
                    plr_c_pure.write_plr_to_file(plr_f_pure, 0);
                  }

                switch_trasmission ();
              }

          }
      }
    else
      return EXIT_FAILURE;
    return EXIT_SUCCESS;
  }

  std::string SinkApplication::getTime()
  {
    time_t curr_time;
    tm * curr_tm;
    char time_string[20];
    time(&curr_time);
    curr_tm = localtime(&curr_time);
    std::strftime(time_string, 20, "%T", curr_tm);

    return std::string(time_string);
  }


} // namespace ns3
