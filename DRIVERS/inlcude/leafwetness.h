#include"stdutils.h"

typedef struct
{
  unsigned short int current;
  unsigned short int min;
  unsigned short int max;
  unsigned short int average;  
}leafWetness_t;

extern leafWetness_t leafWetness;

void Read_LeafWetness(void);
void averageof_leafWetness(void);
void ClearleafWetnessFlags(void);
