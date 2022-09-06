#include "gilbert_elliott.h"

gilbert_Elliott::gilbert_Elliott()
{
      p = q = 0;
      bl = 0;
      for (int i=0; i<MAX_BURST_LENGTH_GEC; i++)
        stat[i]=0;
      state = true;
}

void gilbert_Elliott::initGilbert_Elliott(double plr, double lb_avr)
{
      srand( (unsigned)time( NULL ) );
      p = (1 / lb_avr) * 10000;
      q = (plr / (lb_avr * (1 - plr)))*10000;
      state = true;
}

bool gilbert_Elliott::getState()
{
      if (state)
        {
          if (randomNumber(10000) < q)
            {
              state = false;
              bl++;
            }
        }
      else
        {
          if (randomNumber(10000) < p)
            {
              state = true;
              if ((bl)&&(bl < MAX_BURST_LENGTH_GEC))
                {
                  stat[bl]++;
                  bl=0;
                }

            }
          else
            bl++;
        }
      return state;
}

bool gilbert_Elliott::getCurrentState()
{
      if (!state)
        bl++;
      return state;
}

int gilbert_Elliott::randomNumber(int hi)
{
      float scale = rand()/float(RAND_MAX);
      return int((float)scale*hi);
}

int gilbert_Elliott::write_stat_to_file(FILE *file)
{
      fprintf(file, "%s;%s;%s;%s;%s\n","Loss Birst Size","Amount","Packet Count","Probability I","Probability II");
      long total_lost_packets = 0,
          total_loss_bursts = 0;
      int i;
      for (i = 1; i < MAX_BURST_LENGTH_GEC; i++)
        {
          total_loss_bursts += stat[i];
          total_lost_packets += stat[i] * i;
        }
      for (i = 1; i < MAX_BURST_LENGTH_GEC; i++)
        fprintf(file, "%d;%d;%d;%f;%f\n", i, stat[i], stat[i] * i, (float) stat[i] / total_loss_bursts, (float) stat[i] * i / total_lost_packets);
      return EXIT_SUCCESS;
}

