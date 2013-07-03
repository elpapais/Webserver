//******************************************************************************
//   MSP-FET430P140 Demo - USART0, UART 9600 Full-Duplex Transceiver, 32K ACLK
//
//  Description: UART0 communicates continously as fast as possible full-duplex
//  with another device. Normal mode is LPM3, with activity only during RX and
//  TX or WDT ISR's. The TX ISR indicates the UART is ready to send another character.
//  The RX ISR indicates the UART has received a character. At 9600 baud, a full
//  character is tranceived ~1ms.
//  RX ISR is displayed on P1.5
//  ACLK = UCLK1 = LFXT1 = 32768, MCLK = SMCLK = DCO~ 800k
//  Baud rate divider with 32768hz XTAL @9600 = 32768Hz/9600 = 3.41 (000Dh 4Ah )
//  //* An external watch crystal is required on XIN XOUT for ACLK *//	
//
//                MSP430F149                  
//             -----------------           
//            |              XIN|-      
//            |                 | 32KHz  
//            |             XOUT|-       
//            |                 |      
//            |              RST|---     
//            |                 |         
//   Testpad->|P1.4             |         
//          ->|P1.5             |         
//   LED Red<-|P1.5             |        
// LED Green<-|P2.2             |         
//            |        UTXD/P3.4|--------->|DigiConnect RXD (Pin 7)
//            |                 | 9600 8N1 |                 |
//            |        URXD/P3.5|<---------|DigiConnect TXD (Pin 8)
//
//
//  K. Riedel
//  Built with IAR 5.51.1
//******************************************************************************
// Use: http://192.168.0.102:2101/login.htm

#include <msp430x14x.h>

// Define flags used by the interrupt routines
#define TX BIT0
#define API_KEY "t5oM-cXVCGkP-Rbb3m8xa8Avwc-SAKxJV0l1bVUvaEdoTT0g" // your Cosm API key
#define FEED_ID "116164" // Cosm feed ID
#define USER_AGENT "Datalogger_One"
#define TEMP_MIN_TEMP -40
#define TEMP_MAX_TEMP 79
#define TEMP_TABLE_SIZE 120

// Flag register
volatile unsigned char FLAGS = 0;

char buffer[4];
//char RXBuffer[15];
char IntDegC;
//char TempValues[60];
long index;
//char hour=10;
//char minute=0;
int second=0;
int position=0;

static const unsigned int Temperature_Lookup[] = {
  0x3E9, 0x3E7, 0x3E5, 0x3E4, 0x3E2, 0x3DF, 0x3DD, 0x3DB, 0x3D8, 0x3D6, 0x3D3,
  0x3D0, 0x3CD, 0x3C9, 0x3C6, 0x3C2, 0x3BE, 0x3BA, 0x3B6, 0x3B2, 0x3AD, 0x3A8,
  0x3A3, 0x39D, 0x398, 0x392, 0x38C, 0x385, 0x37F, 0x378, 0x371, 0x36A, 0x362,
  0x35A, 0x352, 0x34A, 0x341, 0x338, 0x32F, 0x326, 0x31C, 0x312, 0x308, 0x2FE,
  0x2F4, 0x2E9, 0x2DE, 0x2D3, 0x2C8, 0x2BD, 0x2B1, 0x2A6, 0x29A, 0x28F, 0x283,
  0x277, 0x26B, 0x25F, 0x253, 0x247, 0x23B, 0x22F, 0x223, 0x217, 0x20B, 0x1FF,
  0x1F3, 0x1E8, 0x1DC, 0x1D1, 0x1C5, 0x1BA, 0x1AF, 0x1A4, 0x199, 0x18F, 0x184,
  0x17A, 0x170, 0x166, 0x15C, 0x153, 0x14A, 0x140, 0x137, 0x12F, 0x126, 0x11E,
  0x116, 0x10E, 0x106, 0x0FE, 0x0F7, 0x0F0, 0x0E9, 0x0E2, 0x0DB, 0x0D5, 0x0CF,
  0x0C9, 0x0C3, 0x0BD, 0x0B7, 0x0B2, 0x0AD, 0x0A8, 0x0A3, 0x09E, 0x099, 0x095,
  0x090, 0x08C, 0x088, 0x084, 0x080, 0x07C, 0x079, 0x075, 0x072, 0x06E, 0x06B
   };

signed char Temperature_GetTemperature(unsigned int Temp_ADC)
{
	if (Temp_ADC > Temperature_Lookup[0])
	  return TEMP_MIN_TEMP;

	for (char Index = 0; Index < TEMP_TABLE_SIZE; Index++)
	{
		if (Temp_ADC > Temperature_Lookup[Index])
		  return (Index + TEMP_MIN_TEMP);
	}

	return TEMP_MAX_TEMP;
}

/**
* Sends a single byte out through UART
**/
void sendByte(unsigned char byte )
  {
  while (!(IFG1 & UTXIFG0)); // USART0 TX buffer ready?
  TXBUF0 = byte;             // send character
  }

void putc(unsigned b)
  {
  sendByte(b);
  }

void printf(char *s)
  {
  char c;
  
  // Loops through each character in string 's'
  while (c = *s++) {
  sendByte(c);
  }
  }

void printfln(char *s)          //send with \r\n
  {
  char c;
  
  // Loops through each character in string 's'
  while (c = *s++) {
  sendByte(c);
  }
  printf("\r\n");
  }

int ltoa_format (char *erg, long zahl, unsigned int vk, unsigned int nk)
{ // Out-String, input long, pre-decimal digits, decimal digits, sign '+' or ' ' 
  char vorz = ' '; 
  long temp;
   int  i;
   i = vk + nk + 1;                   	// string length 
   erg[i--] = 0;                      	// string end
   if ( zahl == 0 )						// special case input = 0  
   {
      while( i >= 0 && 
	       ( zahl > 0 || i+2*( nk != 0 ) >= vk) ) // vk contain sign
	   	{
	      if (i==vk&&nk!=0) erg[i--]='.';  // decimal point, if nk is executed
	      else erg[i--] = '0'; // detach digit, value = 0
	  	}
      if ( i >= 0 ) erg[i--] = vorz;      	// write sign
    } 
   else 
   {
      if ( zahl < 0 )
      {
      vorz  = '-';                     // sign = '-'
      zahl *=  -1;                     // calculate further with positive value
      }
      while( i >= 0 && 
	       ( zahl > 0 || i+2*( nk != 0 ) > vk) ) // vk contain sign  
      {
      if (i==vk&&nk!=0) erg[i--]='.';  // decimal point, if nk is executed  
      else 
      {
      temp     =  zahl / 10;           	// integer division  
      erg[i--] = (zahl - temp*10) + 48; // detach digit, assign ASCII-value
      zahl     =  temp;                	// for next pass reduce digit
      }
   	  }
   if ( i >= 0 ) erg[i--] = vorz;      	// write sign
   }
   while( i >= 0 ) erg[i--] = ' ';     	// fill begin with spaces
   return  vk + nk + 1;                	// return string length
}

// samples and returns AD-converter value of channel 10
// (MSP430's internal temperature reference diode)
// NOTE: to get a more exact value, 8-times oversampling is used

char GetTempVal(void)
{
  unsigned long ReturnValue;

  ADC12CTL0 = ADC12ON | SHT0_15 | MSH | REFON;   // ADC on, int. ref. on (1,5 V),
                                                 // multiple sample & conversion
  ADC12CTL1 = ADC12SSEL_2 | ADC12DIV_7 | CSTARTADD_0 | CONSEQ_1 | SHP;   // MCLK / 8 = 1 MHz

  ADC12MCTL0 = SREF_1 | INCH_10;                 // int. ref., channel 10
  ADC12MCTL1 = SREF_1 | INCH_10;                 // int. ref., channel 10
  ADC12MCTL2 = SREF_1 | INCH_10;                 // int. ref., channel 10
  ADC12MCTL3 = SREF_1 | INCH_10;                 // int. ref., channel 10
  ADC12MCTL4 = SREF_1 | INCH_10;                 // int. ref., channel 10
  ADC12MCTL5 = SREF_1 | INCH_10;                 // int. ref., channel 10
  ADC12MCTL6 = SREF_1 | INCH_10;                 // int. ref., channel 10
  ADC12MCTL7 = EOS | SREF_1 | INCH_10;           // int. ref., channel 10, last seg.
  
  ADC12CTL0 |= ENC;                              // enable conversion
  ADC12CTL0 |= ADC12SC;                          // sample & convert
  
  while (ADC12CTL0 & ADC12SC);                   // wait until conversion is complete
  
  ADC12CTL0 &= ~ENC;                             // disable conversion

  ReturnValue = ADC12MEM0;                       // sum up values...
  ReturnValue += ADC12MEM1;
  ReturnValue += ADC12MEM2;
  ReturnValue += ADC12MEM3;
  ReturnValue += ADC12MEM4;
  ReturnValue += ADC12MEM5;
  ReturnValue += ADC12MEM6;
  ReturnValue += ADC12MEM7;

  ReturnValue >>= 3;                             // ... and divide by 8
  
  ReturnValue = (ReturnValue - 2692) * 423;
  ReturnValue = ReturnValue / 4096;

  return ReturnValue;
}

// samples and returns AD-converter value of channel 0
// external NTC on channel 0
char GetTempVal2(void)
{
  unsigned long temp;

  ADC12CTL0 = ADC12ON | SHT0_15 | MSH;   // ADC on, int. ref. off,
                                                 // multiple sample & conversion
  ADC12CTL1 = ADC12SSEL_2 | ADC12DIV_7 | CSTARTADD_0 | CONSEQ_1 | SHP;   // MCLK / 8 = 1 MHz

  ADC12MCTL0 = SREF_0 | INCH_0;                 // int. ref., channel 0
  ADC12MCTL1 = SREF_0 | INCH_0;                 // int. ref., channel 0
  ADC12MCTL2 = SREF_0 | INCH_0;                 // int. ref., channel 0
  ADC12MCTL3 = SREF_0 | INCH_0;                 // int. ref., channel 0
  ADC12MCTL4 = SREF_0 | INCH_0;                 // int. ref., channel 0
  ADC12MCTL5 = SREF_0 | INCH_0;                 // int. ref., channel 0
  ADC12MCTL6 = SREF_0 | INCH_0;                 // int. ref., channel 0
  ADC12MCTL7 = EOS | SREF_0 | INCH_0;           // int. ref., channel 0, last seg.
  
  ADC12CTL0 |= ENC;                              // enable conversion
  ADC12CTL0 |= ADC12SC;                          // sample & convert
  
  while (ADC12CTL0 & ADC12SC);                   // wait until conversion is complete
  
  ADC12CTL0 &= ~ENC;                             // disable conversion

  temp = ADC12MEM0;                       // sum up values...
  temp += ADC12MEM1;
  temp += ADC12MEM2;
  temp += ADC12MEM3;
  temp += ADC12MEM4;
  temp += ADC12MEM5;
  temp += ADC12MEM6;
  temp += ADC12MEM7;

  temp >>= 5;                             // ... and divide by 32
  
  
  return Temperature_GetTemperature (temp); // calculate temperature, external NTC
}

// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
  P2OUT ^= 0x04;                            // Toggle P2.2 
    
  second++;
  
  if (second == 60) 
  {
    second = 0;
    
    printfln("POST /service/point HTTP/1.1"); // create point
    
    printfln("Host:nimbits1.appspot.com");
    printfln("Connection:close");
    printfln("Cache-Control:max-age=0");
    printfln("Content-Type: application/x-www-form-urlencoded");
    printfln("Content-Length: 3");
    
    printfln("");
    printfln("");
    
    IntDegC = GetTempVal();              // Get Temperature Value internal sensor
    ltoa_format(buffer, IntDegC, 2,0);
    
    printfln("email=kairiedel66@gmail.com&key=a23br420&action=create&point=Data");
    
    //printfln(buffer);

    // here's the actual content of the PUT request:
    
    
    //IntDegC = GetTempVal2();              // Get Temperature Value external NTC
    //ltoa_format(buffer, IntDegC, 2,0);
    
    
     //for (index = 0; index < 200000; index++);  // delay
     //P3OUT &= ~0x01;                         // deactivate ME9210
    _BIS_SR(LPM3_bits + GIE);                // Enter LPM3 w/ interrupt
  }
 }


void main(void)
{
  //WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  WDTCTL = WDT_ADLY_1000;                    // WDT 1000ms, ACLK, interval timer
  
  P1OUT = 0x00;                             // P1.5 setup for LED output
  P1DIR = 0x20;
  P2OUT = 0x00;
  P2DIR = 0x04;                             // Set P2.2 to output direction
  P3SEL |= 0x30;                            // P3.4,5 = USART0 TXD/RXD
  P3OUT = 0x00;
  P3DIR = 0x01;                             // P3.0 output for ME9210 Reset
  
  P3OUT |= 0x01;                           // activate ME9210
  
  P6SEL = 0x01;                             // use P6.0 for the ADC module
  P6OUT = 0;
  P6DIR = 0x00;                             // all input
        
  ME1 |= UTXE0;                             // Enable USART0 TXD
  UCTL0 |= CHAR;                            // 8-bit character
  UTCTL0 |= SSEL0;                          // UCLK = ACLK
  UBR00 = 0x03;                             // 32768/9600 = 3.41, set 3
  UBR10 = 0x00;                             // according to MSP430 User's guide table 13-2
  UMCTL0 = 0x4a;                            // Modulation
  UCTL0 &= ~SWRST;                          // Initialize USART state machine
  IE1 |= WDTIE;                             // Enable WDT interrupt
  IFG1 &= ~UTXIFG1;                         // initales interrupt-flag loeschen
   _BIS_SR(LPM3_bits + GIE);                // Enter LPM3 w/ interrupt
  
  while(1)
  {
    
  }
 }