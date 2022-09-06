#ifndef GILBERT_ELLIOTT_H
#define GILBERT_ELLIOTT_H

#include "socket_io.h"

#define MAX_BURST_LENGTH_GEC 1000

class gilbert_Elliott
{
        int p,q;
        bool state;
    public:
        short stat[MAX_BURST_LENGTH_GEC];
        unsigned char bl; //burst length
    public:
        gilbert_Elliott();
        void initGilbert_Elliott(double plr, double lb_avr);
        bool getState(void);
        bool getCurrentState(void);
        int randomNumber(int hi);
        int write_stat_to_file(FILE *file);
};

#endif // GILBERT_ELLIOTT_H
