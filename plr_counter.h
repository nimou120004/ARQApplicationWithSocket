#ifndef PLR_COUNTER_H
#define PLR_COUNTER_H

#include "ns3/socket.h"
#include "ns3/application.h"
#include "packet-data-tag.h"
#include "nack-data-tag.h"
#include <list>
#include <algorithm>
#include <iostream>

#define MAX_BURST_LENGTH_PLRC 200
#define TM_PLRC               500

using namespace ns3;
namespace ns3
{
  class plr_counter
  {
  public:
    unsigned long cur,
    prev,
    tm, //a number of packets in one measure
    starttime,
    max_pn;
    double plr;
    bool isStarted;
    short stat[MAX_BURST_LENGTH_PLRC];
  private:
    unsigned long ct,
    loss_ct,
    loss_ct1,
    ct_isk;
    double timer;
    bool justStarted; //don't count plr for first measure
    unsigned short measure_number;

  public:
    plr_counter();
    int check(FILE *file);
    int calculate();
    int write_plr_to_file(FILE *file, int active_mode);
    int write_stat_to_file (FILE *file);
  };

}

#endif // PLR_COUNTER_H
