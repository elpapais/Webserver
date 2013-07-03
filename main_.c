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

#include <msp430x14x.h>
#include "webside.h"

static char string1[8];
char string2[]="HTTP/1.1 200 OK\r\n";
char string3[]="Connection: Close\r\n";
char string4[]="Content-Type: text/html\r\n";
char string5[]="Content-Length:150\r\n\r\n\r\n";
char string6[]="<HTML>\r\n";
char string7[]="<BODY>\r\n";
char string8[]="<H2>Webserver mit DigiConnect von Kai</H2>\r\n";
char string9[]="<p>Website von MSP430 erzeugt...</p>\r\n";
char string10[]="</BODY>\r\n";
char string11[]="</HTML>\r\n";

char m, j, k, l, n, p, o, q, r;
int i;

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1OUT = 0x00;                             // P1.5 setup for LED output
  P1DIR = 0x20;
 // P2DIR |= 0x04;                            // Set P2.2 to output direction
  P3SEL |= 0x30;                            // P3.4,5 = USART0 TXD/RXD
  ME1 |= UTXE0 + URXE0;                     // Enable USART0 TXD/RXD
  UCTL0 |= CHAR;                            // 8-bit character
  UTCTL0 |= SSEL0;                          // UCLK = ACLK
  UBR00 = 0x03;                             // 32k/9600 - 3.41
  UBR10 = 0x00;                             //
  UMCTL0 = 0x4a;                            // Modulation
  UCTL0 &= ~SWRST;                          // Initialize USART state machine
  IE1 |= URXIE0 +  UTXIE0;                  // Enable USART0 RX/TX interrupt
  IFG1 &= ~UTXIFG1;  // initales interrupt-flag loeschen
  _BIS_SR(LPM3_bits + GIE);                 // Enter LPM3 w/ interrupt
  
 }

#pragma vector=USART0RX_VECTOR
__interrupt void usart0_rx (void)
{
//  string1[j++] = RXBUF0;
//  if (j > sizeof string1-1)
//  {
//    i = 0;
//    j = 0;
//    TXBUF0  = string1[i++];
//  }
//  P1OUT = RXBUF0 << 4;                           // RXBUF0 to TXBUF0
  string1[0] = RXBUF0;
  P1OUT ^= 0x20;
  
  if (string1[0] == 'G')
  {
    i = 0;
 /*   j = 0;
    k = 0;
    l = 0;
    m = 0;
    n = 0;
    p = 0;
    q = 0;
    o = 0;
    r = 0;*/
   // TXBUF0  = string2[i++];
   TXBUF0  = WebSide[i++]; 
  }
   
}

#pragma vector=USART0TX_VECTOR
__interrupt void usart0_tx (void)
{
 /* if (i < sizeof string2-1)
    TXBUF0  = string2[i++];                             // Transmit string
  else if (j < sizeof string3-1)
    TXBUF0  = string3[j++];                             // Transmit string
  else if (k < sizeof string4-1)
    TXBUF0  = string4[k++];                             // Transmit string   
  else if (l < sizeof string5-1)
    TXBUF0  = string5[l++];                             // Transmit string  
  else if (m < sizeof string6-1)
    TXBUF0  = string6[m++];                             // Transmit string  
  else if (n < sizeof string7-1)
    TXBUF0  = string7[n++];                             // Transmit string  
  else if (o < sizeof string8-1)
    TXBUF0  = string8[o++];                             // Transmit string  
  else if (p < sizeof string9-1)
    TXBUF0  = string9[p++];                             // Transmit string  
  else if (q < sizeof string10-1)
    TXBUF0  = string10[q++];                             // Transmit string  
   else if (r < sizeof string11-1)
    TXBUF0  = string11[r++];                             // Transmit string  
  */
   if (i < sizeof WebSide-1)
    TXBUF0  = WebSide[i++];                             // Transmit string
}
