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
#include <list>
#include <algorithm>
#include <iostream>
#include <time.h>
#include "socket_io.h"
#include "inttypes.h"

#include "playback_buffer.h"


namespace ns3
{

  PlaybackBuffer::PlaybackBuffer()
  {
    //ctor
    //socket_io = new Socket_io();
    length = 0; //current length of playback buffer
    max_length = MAX_PBBFR_SIZE; //maximum length of playback buffer = 134
    max_time = MAX_PBBFR_TIME; //maximum time of playback buffer before play packet
    last_packet_tag = first_packet_tag = NULL;
    isReadyToPlay = false;
    ts0 = 0;
    ts_lp = 0;


  }

  PlaybackBuffer::~PlaybackBuffer()
  {
    clear_pbb ();
  }

  int PlaybackBuffer::copy_packet_tag(PlaybackBuffer::pbb_packet_tag *from, PlaybackBuffer::pbb_packet_tag *to)
  {
    to->next = from->next;
    to->number_of_repeat = from->number_of_repeat;
    to->currentPosition = from->currentPosition;
    to->nodeId = from->nodeId;
    to->packet_id = from->packet_id;
    to->sender_addr = from->sender_addr;
    to->seq_number = from->seq_number;
    to->timestamp = from->timestamp;
    to->nt = from->nt;
    std::memcpy(to->sourceAddr, from->sourceAddr, sizeof (to->sourceAddr));

    return EXIT_SUCCESS;
  }

  bool PlaybackBuffer::shift_buffer()
  {
    if(length >= max_length){
        add_packet_tag_source_buffer (&new_packet_tag);
        delete_first_packet_tag ();
        return true;
      }
    else{
        add_packet_tag_source_buffer (&new_packet_tag);
        return false;
      }
  }

  //  bool PlaybackBuffer::shift_buffer(int s, sockaddr_in addr, unsigned long &pn)
  //  {
  //     return false;
  //  }

  int PlaybackBuffer::delete_first_packet_tag()
  {
    pbb_packet_tag *temp;
    if (first_packet_tag == NULL)
      {
        return EXIT_SUCCESS;
      }
    else if (first_packet_tag->next == NULL)
      {
        delete (first_packet_tag);
        isReadyToPlay=false;
        ts0=0;
        ts_lp=0;
        length=0;
        return EXIT_SUCCESS;
      }
    else
      {
        temp = first_packet_tag->next;
        delete (first_packet_tag);
        first_packet_tag = NULL;
        first_packet_tag = temp;
        length--;
        return EXIT_SUCCESS;
      }
    return EXIT_FAILURE;
  }

  int PlaybackBuffer::send_first_packet()
  {
    delete_first_packet_tag ();
    return EXIT_SUCCESS;
  }

  /* for multi-source */
  int PlaybackBuffer::add_packet_tag (pbb_packet_tag *packet)
  {
    pbb_packet_tag 	*temp = new pbb_packet_tag,
        *cur,
        *prev;

    if (!length)
      {
        copy_packet_tag (&new_packet_tag, temp);
        first_packet_tag = temp;
        last_packet_tag = temp;
        ts0 = socket_io.GetTickCount ();
      }
    else if ( (addrEquals (first_packet_tag->sourceAddr, packet->sourceAddr)) && (first_packet_tag->seq_number > packet->seq_number))
      {
        copy_packet_tag (first_packet_tag, temp);
        copy_packet_tag (&new_packet_tag, first_packet_tag);
        first_packet_tag->next = temp;
        //ts0 = socket_io.GetTickCount ();
        //return EXIT_SUCCESS;
      }
    else if ((addrEquals (last_packet_tag->sourceAddr, packet->sourceAddr)) && (last_packet_tag->seq_number < packet->seq_number))
      {
        copy_packet_tag (&new_packet_tag, temp);
        last_packet_tag->next = temp;
        last_packet_tag = temp;
        //ts_lp = socket_io.GetTickCount ();
        //return EXIT_SUCCESS;
      }
    else
      {
        copy_packet_tag (&new_packet_tag, temp);
        prev = first_packet_tag;
        cur = first_packet_tag->next;
        while (cur != NULL)
          {
            if(!addrEquals (cur->sourceAddr, packet->sourceAddr))
              {
                prev = cur;
                cur = prev->next;
              }
            else if((addrEquals (cur->sourceAddr, packet->sourceAddr)) && (cur->seq_number < packet->seq_number))
              {
                prev = cur;
                cur = prev->next;
              }
            else
              break;
          }
        if ((cur!=NULL)&&(cur->seq_number == packet->seq_number) && (addrEquals (cur->sourceAddr, packet->sourceAddr)))
          {
            return EXIT_FAILURE;
          }
        prev->next = temp;
        temp->next = cur;
      }
    length++;
    ts_lp = socket_io.GetTickCount ();
    return EXIT_SUCCESS;
  }

  /* for one source
  int PlaybackBuffer::add_packet_tag(PlaybackBuffer::pbb_packet_tag *packet_tag)
  {
   // NS_LOG_INFO("packet added ");
    pbb_packet_tag    *temp = new pbb_packet_tag,
        *cur,
        *prev;
    if(length == 0)
      {
        copy_packet_tag (&new_packet_tag, temp);
        first_packet_tag = temp;
        last_packet_tag = temp;
      }
    else if (first_packet_tag->seq_number > packet_tag->seq_number)
      {
        copy_packet_tag (first_packet_tag, temp);
        copy_packet_tag (&new_packet_tag, first_packet_tag);
        first_packet_tag->next = temp;
      }
    else if (last_packet_tag->seq_number < packet_tag->seq_number)
      {
        copy_packet_tag (&new_packet_tag, temp);
        last_packet_tag->next = temp;
        last_packet_tag = temp;
      }
    else
      {
        copy_packet_tag (&new_packet_tag, temp);
        prev = first_packet_tag;
        cur = first_packet_tag->next;
        while ((cur != NULL) && (cur->seq_number < packet_tag->seq_number))
          {
            prev = cur;
            cur = prev->next;
          }
        if (cur->seq_number == packet_tag->seq_number || prev->seq_number == packet_tag->seq_number)
          return EXIT_FAILURE;
        prev->next = temp;
        temp->next = cur;

      }
    length++;
    return EXIT_SUCCESS;

  }
  */


  int PlaybackBuffer::add_packet_tag_source_buffer (PlaybackBuffer::pbb_packet_tag *packet_tag)
  {
    // NS_LOG_INFO("packet added ");
    pbb_packet_tag    *temp = new pbb_packet_tag,
        *cur,
        *prev;
    if(length == 0)
      {
        copy_packet_tag (&new_packet_tag, temp);
        first_packet_tag = temp;
        last_packet_tag = temp;
      }
    else if (first_packet_tag->seq_number > packet_tag->seq_number)
      {
        copy_packet_tag (first_packet_tag, temp);
        copy_packet_tag (&new_packet_tag, first_packet_tag);
        first_packet_tag->next = temp;
      }
    else if (last_packet_tag->seq_number < packet_tag->seq_number)
      {
        copy_packet_tag (&new_packet_tag, temp);
        last_packet_tag->next = temp;
        last_packet_tag = temp;
      }
    else
      {
        copy_packet_tag (&new_packet_tag, temp);
        prev = first_packet_tag;
        cur = first_packet_tag->next;
        while ((cur != NULL) && (cur->seq_number < packet_tag->seq_number))
          {
            prev = cur;
            cur = prev->next;
          }
        if (cur->seq_number == packet_tag->seq_number)
          return EXIT_FAILURE;
        prev->next = temp;
        temp->next = cur;

      }
    length++;
    return EXIT_SUCCESS;

  }
  //  int PlaybackBuffer::get_packet_tag_by_p2p_pn(unsigned long p2p_pn, unsigned char nt)
  //  {
  //    return EXIT_SUCCESS;
  //  }

  //  int PlaybackBuffer::get_pn_by_p2p_pn(unsigned long p2p_pn, unsigned char nt, unsigned long &pn)
  //  {
  //    return EXIT_SUCCESS;
  //  }

  int PlaybackBuffer::get_packet_tag_by_pn(uint32_t pn)
  {
    pbb_packet_tag	*cur,
        *prev;
    if (length == 0 || first_packet_tag == NULL)
      {
        return EXIT_FAILURE;
      }
    else if (first_packet_tag->seq_number > pn)
      {
        return EXIT_FAILURE;
      }
    else if (first_packet_tag->seq_number == pn)
      {
        copy_packet_tag (first_packet_tag, &new_packet_tag);
        return EXIT_SUCCESS;
      }
    else if ((new_packet_tag.next != NULL) && (new_packet_tag.next->seq_number == pn))
      {
        copy_packet_tag (new_packet_tag.next, &new_packet_tag);
        return EXIT_SUCCESS;
      }
    else if (last_packet_tag->seq_number < pn)
      {
        return EXIT_FAILURE;
      }
    else if ((new_packet_tag.next != NULL) && (new_packet_tag.next->seq_number < pn))
      {
        prev = new_packet_tag.next;
        cur = new_packet_tag.next->next;
        while ((cur != NULL) && (cur->seq_number < pn))
          {
            prev = cur;
            cur = prev->next;
          }
        if ((cur != NULL) && (cur->seq_number == pn))
          {
            copy_packet_tag (cur, &new_packet_tag);
          }
        else
          {
            return EXIT_FAILURE;
          }
        return EXIT_SUCCESS;
      }
    else
      {
        prev = first_packet_tag;
        cur = first_packet_tag->next;
        while ((cur != NULL) && (cur->seq_number < pn))
          {
            prev = cur;
            cur = prev->next;
          }
        if ((cur != NULL) && (cur->seq_number == pn))
          {
            copy_packet_tag (cur, &new_packet_tag);
          }
        else
          {
            return EXIT_FAILURE;
          }
        return EXIT_SUCCESS;
      }
  }

  int PlaybackBuffer::get_packet_tag_by_p2p_pn (unsigned long p2p_pn, unsigned char nt)
  {
    pbb_packet_tag	*cur,
        *prev;
    if (length == 0 || first_packet_tag == NULL)
      {
        return EXIT_FAILURE;
      }
    else if ((first_packet_tag->nt == nt) && (first_packet_tag->seq_number == p2p_pn))
      {
        copy_packet_tag (first_packet_tag, &new_packet_tag);
        return EXIT_SUCCESS;
      }
    else
      {
        prev = first_packet_tag;
        cur = first_packet_tag->next;
        while (cur != NULL)
          {
            if ((cur->nt == nt) && (cur->seq_number == p2p_pn))
              break;
            prev = cur;
            cur = prev->next;
          }
        if (cur != NULL)
          {
            copy_packet_tag (cur, &new_packet_tag);
            return EXIT_SUCCESS;
          }
        else
          {
            return EXIT_FAILURE;
          }
      }
  }

  int PlaybackBuffer::clear_pbb()
  {
    while(length != 0)
      delete_first_packet_tag ();
    length = 0;
    return EXIT_SUCCESS;

  }

  int PlaybackBuffer::show_pn()
  {

    return EXIT_SUCCESS;
  }

  bool PlaybackBuffer::addrEquals(uint8_t addr1[], uint8_t addr2[])
  {
    for (int i = 0; i < 4; i++)
      {
        if(addr1[i] != addr2[i]){
            return false;
          }
      }
    return true;
  }

  bool PlaybackBuffer::play(uint32_t &pn, uint32_t &nr)
  {

    if (!isReadyToPlay)
      {
        //printf("OKKKKKK");
        if ((ts_lp-ts0) > max_time)
          {
            isReadyToPlay = true;
          }

        return false;
      }
    else
      {
        //printf("OKKKKKK");
        if (first_packet_tag!=NULL)
          {
            pn = first_packet_tag->seq_number; //first packet number for statistics
            //printf(" %" PRIu32, pn);
            nr = first_packet_tag->number_of_repeat; //first packet repeat numer for statistics
          }
        else
          {
            printf("[NULL PBB]");
          }
         send_first_packet ();
      }

    return true;
  }

}

