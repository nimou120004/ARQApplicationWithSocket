#ifndef ARQ_LINE_SOCKET_H
#define ARQ_LINE_SOCKET_H


#include "simple_c.h"

#define MAX_BURST_LENGTH_AL   100
#define MAX_BURST_COUNT_AL     200
#define MAX_TOTAL_WAITING_TIME_AL 90

using namespace ns3;
namespace ns3
{
  class arq_line_socket
  {
        public:
          arq_line_socket();
          virtual ~arq_line_socket();


        private:
            struct burst {
                unsigned long first_sn; // first serial number in this burst
                unsigned char length; // a number of packets in this burst
            };
            struct waited_group { //a group of lost packets for recovering process
                burst *b; //bursts inside this group
                int amount_of_bursts; //a number of bursts inside this group
                int tt;	//total waiting time of this group
                int t_waiting; //waiting time of this group after sending last NACK
                int nr; //number of requests of lost packets in this group
                bool recalc_nr; //boolean var for re-caclculation of the timeout
            };
            int total_waiting_time,
                min_tr;
            unsigned long ct,
                          rec_ct,
                          ct_isk;
            waited_group *wg;//structures for waiting groups
            bool doNotDrop; //don't drop current packet
            short stat[MAX_BURST_LENGTH_AL]; //statistics
            unsigned char aowg; //an amount of waiting groups
            int rtt,
                srtt,
                dev,
                sdev;


        public:
            int tr;
            unsigned long cur, //p2p pn of current packet
                          prev,//p2p pn of previous packet
                          max_pn, //max pn in this arq_line
                          first_in_transmission; //a number of first packet in this transmission
            unsigned char nt; //tree number of this arq line
            bool isStarted,
                 isActive;//to turn arq line on and off
            int nr;//request number of current packet


        private:
            int clear_wg ();//to clear arq line

        public:

            int reset();
            int check(); //to check current p2p pn (cur) in all waiting groups
            int send_nack(Ipv4Address m_destination_addr, Ipv4Address relay, uint16_t m_port, simple_c *ctrl_c, Ptr<Packet> nack,  Ptr<Socket> m_send_socket, uint32_t nodeId, bool activeMode);
            int send_nack(Ipv4Address m_destination_addr, uint16_t m_port, simple_c *ctrl_c, Ptr<Packet> nack,  Ptr<Socket> m_send_socket, uint32_t nodeId);
            int is_it_first_packet(unsigned char nr);//to check is current packet is first in waiting group
            int do_not_wait(unsigned long sn, unsigned char length);//in case of recieving "do not wait" meessage
            unsigned long GetTickCount();

        };


}

#endif // ARQ_LINE_SOCKET_H
