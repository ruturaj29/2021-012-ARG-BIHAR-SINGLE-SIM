

#include "stdutils.h"
/******************* LCD Scroll **********************/
extern volatile uint32_t KeyInterrupt, scrollScreen; // variable for external intrpt


/***************************************************************************************************
                             Function Prototypes
***************************************************************************************************/
void myExtIntrIsr_1(void);
void LCD_ScrollDisplay(void);
void sampleTestlog(void);
void USB (void);
