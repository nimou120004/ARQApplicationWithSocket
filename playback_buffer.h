#ifndef PLAYBACK_BUFFER_H
#define PLAYBACK_BUFFER_H

#include "ns3/socket.h"
#include "ns3/application.h"
#include "packet-data-tag.h"
#include "nack-data-tag.h"
#include <list>
#include <algorithm>
#include <iostream>

#define MAX_PBBFR_SIZE	134



using namespace ns3;
namespace ns3
{
  class PlaybackBuffer
  {


  private:
      struct pbb_packet_tag {

                        uint32_t        nodeId;
                        Vector          currentPosition;
                        Time            timestamp;
                        uint32_t        seq_number;
                        Address         sender_addr;
                        uint32_t        packet_id;
                        uint32_t        number_of_repeat;
                        uint8_t   nt;
                        pbb_packet_tag  *next; // a pointer to next pbb_packet (4 b)
      };

      int length; //current length of a buffer

      int copy_packet_tag (pbb_packet_tag *from, pbb_packet_tag *to);

//      int send_first_packet (int s, struct sockaddr_in addr); //send packet
      int delete_first_packet_tag (); //delete only first packet from a buffer
      int show_pn (); //printf packets from pbb
      int clear_pbb (); //delete all packet tags from a buffer

  public:

    PlaybackBuffer();
    virtual ~PlaybackBuffer();
    pbb_packet_tag *first_packet_tag, //first packet in a buffer
               *last_packet_tag, //last packet in a buffer
                new_packet_tag; //a variable for new arrived packet
    int max_length; //maximum length

    bool shift_buffer (); //add packet without sending first packet
//    bool shift_buffer (int s, struct sockaddr_in addr, unsigned long &pn); //add new packet and send first packet
    int get_packet_tag_by_pn (uint32_t pn); //get data from pn packet into new_packet var
    int get_packet_tag_by_p2p_pn (unsigned long p2p_pn, unsigned char nt);//get data from p2p_pn packet into new_packet var
    int get_pn_by_p2p_pn (unsigned long p2p_pn, unsigned char nt, unsigned long &pn);
    int add_packet_tag (pbb_packet_tag *packet_tag);
    int add_packet_tag_source_buffer (pbb_packet_tag *packet_tag);
  };


}

#endif // SOCKET_IO_H


