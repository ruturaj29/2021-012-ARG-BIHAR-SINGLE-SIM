


/**************************************************************************************************
                           extern all global variables 
***************************************************************************************************/
extern volatile unsigned int numRotateWind,LCDnumRotateWind;
extern char _CurrentWindDir_A[10], _CurrentWindSpd_A[10];

/***************************************************************************************************
                             Function Prototypes
***************************************************************************************************/
void myExtIntrIsr_0(void);
void DisplayWindSensor(void);
void Cup_Read_WindSensor(void);
void Cup_Average_WindSamples(void);
void Cup_average_windDirection(void);
void Cup_Trignometric_Conversion(void);

extern char Cup_Minute_WINDSENSOR_COUNTS;
extern char Cup_DX[10],Cup_DN[10],Cup_Gust[10],_Cup_CurrentWindDir[10];
extern char Cup_SN[10],Cup_MinSpeed[10],Cup_SX[10],_Cup_CurrentWindSpd[10],Cup_SM[10];
extern char Cup_WINDSENSOR_COUNTS,Cup_Tan_Inverse_WindDirection[10];


