#include "plr_counter.h"
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

#include "playback_buffer.h"


namespace ns3
{
  plr_counter::plr_counter()
  {
    cur = 0;
    prev = 0;
    tm = TM_PLRC;
    starttime = 0;
    max_pn = MAX_PACKET_NUMBER;
    isStarted = false;
    plr = 0.0;
    ct = 0;
    loss_ct = 0;
    loss_ct1 = 0;
    ct_isk = 0;
    timer = 0.0;
    justStarted = true;
    measure_number = 0;
    for (int i = 0; i < MAX_BURST_LENGTH_PLRC + 1; i++)
      stat[i] = 0;

  }
  int plr_counter::write_plr_to_file (FILE * file, int active_mode)
  {
    fprintf (file, "%.3lf;%f;%d\n", timer, plr, active_mode);
    return EXIT_SUCCESS;
  }
  int plr_counter::check(FILE *file)
  {

    ct++;
    // enter to if condition only if there was lost packets
    if ((cur != (prev + 1)) && (!justStarted) && (prev != max_pn) && (cur != 0))
      {
        //printf(" check");
        if ((long)(cur - prev - 1) > 0)
          {
            if ((cur - prev - 1) <= MAX_BURST_LENGTH_PLRC)
              stat[cur - prev - 1]++;
            if ((cur - prev - 1) > 1)
              fprintf(file, " PACKET LOSS SN:%lu-SN:%lu\n", prev + 1, cur - 1);
            else
              fprintf(file, " PACKET LOSS SN:%lu lost\n", prev + 1);
            loss_ct = loss_ct + (cur - prev - 1);
            ct = ct + cur - prev - 1;
            prev = cur;
          }//lost packets > 0
        else if ((cur <= prev) && ((long)(prev - cur) < 10000) )
          {
            ct_isk++;
            fprintf(file, "!!!! PREV SN%lu >= CUR SN:%lu\n", prev, cur);
            ct--;
            cur = prev;
          }
        else
          {
            loss_ct = loss_ct + (max_pn - prev) + cur;
            ct = ct + (max_pn - prev) + cur;
            prev = cur;
            if ((max_pn - prev + cur) > 1)
              fprintf(file, "PACKET LOSS SN:%lu-SN:%lu\n", prev + 1, cur - 1);
            else
              fprintf(file, "PACKET LOSS SN:%lu lost\n", prev + 1);
          }
      }  //endif (cur==prev+1)
    else{
       prev = cur;
       //printf("%lu-%lu", cur, prev);
      }


    return EXIT_SUCCESS;
  }
  int plr_counter::calculate()
  {
    //counting PLR every "tm" packets
    double ms = 0;
    if (ct >= tm)
      {
        //printf("=%lu/%lu",ct, tm );
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ms = round(ts.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
        if (ms > 999) {
            timer = (ts.tv_sec + 1) - starttime;
                ms = 0;
            }
        else {
            timer = (ts.tv_sec + ms/1000.0) - starttime;
          }

        justStarted = false;
        if (ct > tm)
          {
            loss_ct1 = ct - tm;
            ct = tm;
            loss_ct = loss_ct - loss_ct1;
          }
        if ((loss_ct + ct) != 0)
          plr = (float) loss_ct / (float) (ct);
        measure_number++;
        if (loss_ct1 == 0)
          {
            ct = 0;
            loss_ct = 0;
          }
        else
          {
            ct = loss_ct = loss_ct1;
            loss_ct1 = 0;
          }
      }//if (ct >= tm)
    else
      return EXIT_FAILURE;
    return EXIT_SUCCESS;
  }
  int plr_counter::write_stat_to_file (FILE *file)
  {
    fprintf(file, "%s;%s;%s;%s;%s\n","Loss Birst Size","Amount","Packet Count","Probability I","Probability II");
    long total_lost_packets = 0,
        total_loss_birsts = 0;
    int i;
    for (i = 1; i < MAX_BURST_LENGTH_PLRC + 1; i++)
      {
        total_loss_birsts += stat[i];
        total_lost_packets += stat[i] * i;
      }
    for (i = 1; i < MAX_BURST_LENGTH_PLRC + 1; i++)
      fprintf(file, "%d;%d;%d;%f;%f\n", i, stat[i], stat[i] * i, (float) stat[i] / total_loss_birsts, (float) stat[i] * i / total_lost_packets);
    return EXIT_SUCCESS;
  }

}
