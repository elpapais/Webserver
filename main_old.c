//******************************************************************************
//   MSP-FET430P140 Demo - USART0, UART 9600 Full-Duplex Transceiver, 32K ACLK
//
//  Description: UART0 communicates continously as fast as possible full-duplex
//  with another device. Normal mode is LPM3, with activity only during RX and
//  TX ISR's. The TX ISR indicates the UART is ready to send another character.
//  The RX ISR indicates the UART has received a character. At 9600 baud, a full
//  character is tranceived ~1ms.
//  The levels on P1.4/5 are TX'ed. RX'ed value is displayed on P1.0/1.1
//  ACLK = UCLK1 = LFXT1 = 32768, MCLK = SMCLK = DCO~ 800k
//  Baud rate divider with 32768hz XTAL @9600 = 32768Hz/9600 = 3.41 (000Dh 4Ah )
//  //* An external watch crystal is required on XIN XOUT for ACLK *//	
//
//                MSP430F149                   MSP430F149
//             -----------------            -----------------
//            |              XIN|-      /|\|              XIN|-
//            |                 | 32KHz  | |                 | 32KHz
//            |             XOUT|-       --|RST          XOUT|-
//            |                 | /|\      |                 |
//            |              RST|---       |                 |
//            |                 |          |                 |
//          ->|P1.4             |          |             P1.0|-> LED
//          ->|P1.5             |          |             P1.1|-> LED
//      LED <-|P1.0             |          |             P1.4|<-
//      LED <-|P1.1             |          |             P1.5|<-
//            |        UTXD/P3.4|--------->|P3.5             |
//            |                 | 9600 8N1 |                 |
//            |        URXD/P3.5|<---------|P3.4             |
//
//
//  M. Buccini / G. Morton
//  Texas Instruments Inc.
//  May 2005
//  Built with Code Composer Essentials Version: 1.0
//******************************************************************************
// Use: http://192.168.0.102:2101/login.htm

#include <msp430x14x.h>

//#include "webside.h"

// Define flags used by the interrupt routines
#define TX BIT0

// Flag register
volatile unsigned char FLAGS = 0;

char buffer[4];
int IntDegC;


/**
* Sends a single byte out through UART
**/
void sendByte(unsigned char byte )
  {
  while (!(IFG1 & UTXIFG0)); // USART0 TX buffer ready?
  TXBUF0 = byte;             // TX -> RXed character
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

int GetTempVal(void)
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



#pragma vector=USART0RX_VECTOR
__interrupt void usart0_rx (void)
{
  char r = RXBUF0;
  P1OUT ^= 0x20;
  
  if (r == 'G')
  {
    FLAGS |= TX; // Set flag to transmit data
    _BIC_SR_IRQ(LPM3_bits);                   // Clear LPM3 bits from 0(SR)
    _BIC_SR(GIE);
  }
}


void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

 // ADC12CTL0 = SHT0_8 + REFON + ADC12ON;
 // ADC12CTL1 = SHP;                          // Enable sample timer
 // ADC12MCTL0 = SREF_1 + INCH_10;
//  ADC12IE = 0x001;
 // ADC12CTL0 |= ENC;  

  P1OUT = 0x00;                             // P1.5 setup for LED output
  P1DIR = 0x20;
  P2OUT = 0x00;
  P2DIR = 0x04;                             // Set P2.2 to output direction
  P3SEL |= 0x30;                            // P3.4,5 = USART0 TXD/RXD
  
  P6SEL = 0x01;                             // use P6.0 for the ADC module
  P6OUT = 0;
  P6DIR = 0x00;                             // all input
  
  ME1 |= UTXE0 + URXE0;                     // Enable USART0 TXD/RXD
  UCTL0 |= CHAR;                            // 8-bit character
  UTCTL0 |= SSEL0;                          // UCLK = ACLK
  UBR00 = 0x03;                             // 32k/9600 = 3.41, set 3
  UBR10 = 0x00;                             //
  UMCTL0 = 0x4a;                            // Modulation
  UCTL0 &= ~SWRST;                          // Initialize USART state machine
  IE1 |= URXIE0;                            // Enable USART0 RX/TX interrupt
  IFG1 &= ~UTXIFG1;                         // initales interrupt-flag loeschen
   _BIS_SR(LPM3_bits + GIE);                // Enter LPM3 w/ interrupt
  
  while(1)
  {
    switch(FLAGS)
    {
        case 0: // No flags set
        _BIS_SR(LPM3_bits + GIE); // Enter LPM3
        break;
        case TX: // Values need to be transmitted
        //printf (WebSide);
        //ADC12CTL0 |= ADC12SC;                   // Sampling and conversion start
        //while (ADC12CTL0 & ADC12BUSY);          // ADC10BUSY?
        //temp = ADC12MEM0; 
       // IntDegC = (temp - 2692) * 423;
       // IntDegC = IntDegC / 4096;
        IntDegC =   GetTempVal();
          
        printf("HTTP/1.1 200 OK\r\n");
        printf("Connection: Close\r\n");
        printf("Content-Type: text/html\r\n");
        //printf("Content-Length:333\r\n");
        printf("\r\n\r\n");
        printf("<html><head>");
        printf("<title>MSP430 - Webserver</title>");
        //printf("<meta http-equiv='refresh' content='5'>\r\n");
        printf("</head>\r\n");
        printf("<body>\r\n");
        printf("<br><hr />\r\n"); 
        printf("<h1>MSP430 Webserver</h1>\r\n");
        printf("<hr />\r\n");
        printf("<p><b>MSP Interner Temperatursensor:\r\n");
        
        ltoa_format(buffer, IntDegC, 1,0);
        printf(buffer);
        printf(" °C</b></p>\r\n");
        printf("<br>\r\n");
        
        printf("<img src='http://chart.apis.google.com/chart?");
        printf("chs=400x225&cht=gom&chd=t:");
        printf(buffer);
        printf("&chco=800080,0000FF,00FFFF,00FF00,FFFF00,FF8040,FF0000&chxt=x,y&chxr=1,-20,50&chds=-20,50&chxl=0:|");
        printf(buffer);
        printf("'><br><br>\r\n");
        printf("<p><b>Temperaturverlauf:</b></p>");
        
        printf("<img src='http://chart.apis.google.com/chart?");
        //printf("chs=200x100&cht=lc&chd=t:27,25,60,31,25,39,25,31,26,28,80,28,27,31,27,29,26,35,70,25");
        printf("chf=bg,lg,45,EFEFEF,0,DBE1ED,1,1&chxl=0:|Time&chxp=0,50&chs=600x400&chxt=x,y&cht=lxy&chd=t:10,20,40,80,90,95,99|20,-30,-40,50,60,70,80|10,20,40,80,90,95,99|5,10,72,-55,65,75,85");
        printf("&chco=3072F3,FF0000&chxl=1:Time:Temperature&chma=40,20,20,30&chdl=Temp1|Temp2&chdlp=b&chls=2,4,1|1&chg=10,10&chds=a");
        printf("'><br><br>\r\n");
        printf("<br><hr />\r\n");        
        
        printf("</body></html>\r\n");
                    
        FLAGS &= ~TX;
        P2DIR ^= 0x04;
        break;
    }
  }
 }