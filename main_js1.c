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

//#include "webside.h"

// Define flags used by the interrupt routines
#define TX BIT0

// Flag register
volatile unsigned char FLAGS = 0;

char buffer[4];
char RXBuffer[15];
char IntDegC;
char TempValues[60];
char index;
char hour=10;
char minute=0;
char second=0;
int position=0;


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
  char vorz = '0'; 
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



#pragma vector=USART0RX_VECTOR
__interrupt void usart0_rx (void)
{
  char r = RXBUF0;
    
  RXBuffer[position++] = RXBUF0;                       // reveive 15 characters from UART
  
  if (position > sizeof RXBuffer-1)
  {
      position = 0;
      for (char i=0; i<15; i++)
      {  
        if (RXBuffer[i] == 'G' & RXBuffer[i+1] == 'E' & RXBuffer[i+2] == 'T')
           {
            ME1 &= ~URXE0;                                       // Disable USART0 RXD
            position = 0;
            
            if (RXBuffer[i+8] =='E')
              P1OUT |= 0x20;
            if (RXBuffer[i+8] =='A')
              P1OUT &= ~0x20;
              
            FLAGS |= TX;                               // Set flag to transmit data
            _BIC_SR_IRQ(LPM3_bits);                   // Clear LPM3 bits from 0(SR)
            _BIC_SR(GIE);
            }
        }
  }
}

// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
  P2OUT ^= 0x04;                            // Toggle P2.2 using exclusive-OR
  
  second++;
  
  if (second == 60) 
  {
    second = 0;
    minute++;
    IntDegC = GetTempVal();
    TempValues[index++] = IntDegC;
  }
  if (minute == 60)
  {
    minute = 0;
    hour++;
    index = 0;
  }
  if (hour==24) hour = 0;
    
}


void main(void)
{
  //WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  WDTCTL = WDT_ADLY_1000;                    // WDT 1000ms, ACLK, interval timer
  
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
  UBR00 = 0x03;                             // 32768/9600 = 3.41, set 3
  UBR10 = 0x00;                             // according to MSP430 User's guide table 13-2
  UMCTL0 = 0x4a;                            // Modulation
  UCTL0 &= ~SWRST;                          // Initialize USART state machine
  IE1 |= URXIE0;                            // Enable USART0 RX/TX interrupt
  IE1 |= WDTIE;                             // Enable WDT interrupt
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
       
        IntDegC =   GetTempVal();              // Get Temperature Value
                 
        printfln("HTTP/1.1 200 OK");
        printfln("Connection: Close");
        printfln("Content-Type: text/html");
       
        printfln("");
        printfln("");
        printfln("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>");
        printfln("<html><head>");
        printfln("<title>MSP430 - Webserver</title>");
                    
        printfln("<script type='text/javascript' src='https://www.google.com/jsapi'></script>");
        printfln("<script type='text/javascript'>");
        printfln("google.load('visualization', '1', {packages:['corechart']});");
        printfln("google.setOnLoadCallback(drawChart);");
        printfln("function drawChart() {\r\n");
        printfln("var data = google.visualization.arrayToDataTable([");
        printfln("['Index', 'Temperatur 1'],");
        // Beispieldaten
    //    printf("['10:00',  22,   -5],\r\n");
//        printf("['10:30',  23,   -4],\r\n");
//        printf("['11:00',  24,   0],\r\n");
//        printf("['11:30',  25,   3],\r\n");
//        printf("['12:00',  21,   5]\r\n");
//        printf("]);\r\n");
        for (char i=0; i<60; i++)
        {
          ltoa_format(buffer, hour, 1,0);
          printf("['");
          printf(buffer);
          ltoa_format(buffer, i, 1,0);
          printf(":");
          printf(buffer);
          printf("',");
          ltoa_format(buffer, TempValues[i], 1,0);
          printf(buffer);
          if (i==59) printfln("]");
          else printfln("],");
        }
        printfln("]);");
        
        printfln("var options = {");
        printf("title: 'Temperaturverlauf',vAxis:{title: 'Temperatur [°C]', maxValue:50, minValue:-20},backgroundColor: {strokeWidth:2, fill:'#CCCCFF'}");
        printfln("};");
        
        printfln("var chart = new google.visualization.LineChart(document.getElementById('chart_div'));");
        printfln("chart.draw(data, options);}");
        
        printfln("</script>");

        // Gauge Chart  Begin      
        
//        printfln("<script type='text/javascript'>");
//        printfln("google.load('visualization', '1', {packages:['gauge']});");
//        printfln("google.setOnLoadCallback(drawChart);");
//        printfln("function drawChart() {");
//        printfln("var data = google.visualization.arrayToDataTable([");
//        printfln("['Label', 'Value'],");
//        printf("['Temp. 1',");
//        ltoa_format(buffer, IntDegC, 1,0);
//        printf(buffer);
//        printfln("]");
//        printfln("]);");
//        printfln("var options = {");
//        printf("width: 200, height: 200, redFrom: 40, redTo: 60, yellowFrom:10, yellowTo: 40, greenFrom: -20, greenTo: 10, min: -20, max: 60, minorTicks: 5");
//        printfln("};");
//        
//        printfln("var chart = new google.visualization.Gauge(document.getElementById('chart_div2'));");
//        printfln("chart.draw(data, options);}");
//        
//        printfln("</script>");
        
        // Gauge Chart End
        
        printfln("</head>");        
        
        printfln("<body bgcolor='#444444'>");
        printfln("<br><hr />");
        printfln("<h2><div align='left'><font color='#2076CD'> Webserver 1.0 </font color></div></h2>");
        printfln("<hr /><br>");
        printfln("<div align='left'><font face='Verdana' color='#FFFFFF'>");
        printfln("<p><b>Aktueller Temperaturwert:");
        ltoa_format(buffer, IntDegC, 1,0);
        printf(buffer);
        printfln(" °C</b></p>");
              
        printfln("<p><b>Aktuelle Systemzeit:");
        ltoa_format(buffer, hour, 1,0);
        printf(buffer);
        printf(":");
        ltoa_format(buffer, minute, 1,0);
        printf(buffer);
        printf(":");
        ltoa_format(buffer, second, 1,0);
        printf(buffer);
        printfln("</b></p>");
        printfln("<br>");
        printfln("<div align='left'><font face='Verdana' color='#FFFFFF'><b>LED schalten:</font></div>");
        
        printf("<br>");
        // HTML-Table
       // printf("<table border='1' width='500' cellpadding='5'>");
       // printf("<tr bgColor='#222222'>");
       // printf("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='3'>LED Rot<br></font></td>");
       // printf("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='ein'></form></td>");
       // printf("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='aus'></form></td>");
        
        //if (P1OUT & 0x20) printf("<td align='center'><font color='red' size='5'>EIN");
        //else printf("<td align='center'><font color='green' size='5'>AUS");

        //printf("</tr>");
        //printfln("</tr></tr></table><br>");
        
        printf("<form method=get><input type=submit name=3 value='EIN'></form>");
        printf("<form method=get><input type=submit name=3 value='AUS'></form>");
        if (P1OUT & 0x20) printf("LED Ausgang: EIN");
        else printf("LED Ausgang: AUS");
        
        printfln("</b><br><br>");
         
        printfln("<div id='chart_div' style='width: 800px; height: 400px;'></div>");
       // printfln("<br>");
       // printfln("<div id='chart_div2'></div>");
        
        printfln("</body></html>");
                    
        FLAGS &= ~TX;
        ME1 |= URXE0;                                       // Enable USART0 RXD
       
        break;
    }
  }
 }