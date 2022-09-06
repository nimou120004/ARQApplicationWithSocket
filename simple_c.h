#ifndef SIMPLE_C_H
#define SIMPLE_C_H

#include "socket_io.h"


#define TM_SIMPLE_C          100

using namespace ns3;
namespace ns3 {

  class simple_c
  {
  public:simple_c();

  public:
    int p;
    double q;
    int c;//packet counter
    int l;//packet loss


  public:
    bool error();
    int randomNumber(int hi);
    void init_c(double plr);

  };

}

#endif // SIMPLE_C_H
