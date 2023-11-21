
#define STCTRL *((volatile long *) 0xE000E010)    // control and status
#define STRELOAD *((volatile long *) 0xE000E014)    // reload value
#define STCURRENT *((volatile long *) 0xE000E018)    // current value

#define RED_LED 1
#define BLUE_LED 2
#define GREEN_LED 3

#define ENABLE      (1 << 0)    // bit 0 of CSR to enable the timer
#define CLKINT      (1 << 2)    // bit 2 of CSR to specify CPU clock
#define INTEN       (1 << 1)    //bit 1 of CSR to enable Interrupt bit

#include <stdint.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include <string.h>

#define MASK_BITS 0x11 // pins for which interrupts should be enabled

uint8_t lap_count=0;
uint32_t timestamps[10]={0,0,0,0,0,0,0,0,0,0};
uint32_t current_timestamp;
uint32_t current_laptime;
uint8_t enable_status=0;

uint8_t Buffer;//to flush UDR

#define LAP_NUMBER_WORD "n="
#define ORIG_LAP_NUMBER_WORD LAP_NUMBER_WORD
const char *lap_number_word = LAP_NUMBER_WORD;
const char *orig_lap_number_word = ORIG_LAP_NUMBER_WORD;

#define CURRENT_TIME_WORD "c_t="
#define ORIG_CURRENT_TIME_WORD CURRENT_TIME_WORD
const char *current_time_word = CURRENT_TIME_WORD;
const char *orig_current_time_word = CURRENT_TIME_WORD;

#define TIME_PRESSED_AT_WORD "t="
#define ORIG_TIME_PRESSED_AT_WORD TIME_PRESSED_AT_WORD
const char *time_pressed_at_word = TIME_PRESSED_AT_WORD;
const char *orig_time_pressed_at_word = TIME_PRESSED_AT_WORD;

#define CURRENT_LAPTIME_WORD "lap="
#define ORIG_CURRENT_LAPTIME_WORD CURRENT_LAPTIME_WORD
const char *current_laptime_word = CURRENT_LAPTIME_WORD;
const char *orig_current_laptime_word = ORIG_CURRENT_LAPTIME_WORD;

#define NULL_STRING "\n"
#define ORIG_NULL_STRING NULL_STRING
const char *null_string = NULL_STRING;
const char *orig_null_string= ORIG_NULL_STRING;

#define MS_STRING "s"
#define ORIG_MS_STRING MS_STRING
const char *ms_string = MS_STRING;
const char *orig_ms_string = ORIG_MS_STRING;

#define DS_STRING "*0.1s"
#define ORIG_DS_STRING DS_STRING
const char *ds_string = DS_STRING;
const char *orig_ds_string = ORIG_DS_STRING;

//const char *lap_number_word = "lap number=";
//const char *current_time_word = "current time=";
//const char *time_pressed_at_word = "time pressed at=";
//const char *current_laptime_word = "current laptime=";
//const char *null_string = " \n";
//const char *ms_string = "ms";

//const char *orig_lap_number_word = lap_number_word;
//const char *orig_current_time_word = current_time_word ;
//const char *orig_time_pressed_at_word = time_pressed_at_word;
//const char *orig_current_laptime_word = current_laptime_word;
//const char *orig_null_string = null_string;
//const char *orig_ms_string = ms_string;


uint32_t time_count=0;

void UART_Init()
{

    SYSCTL_RCGCUART_R |=(1<<2);//Enable UART Module 2
    UART2_ICR_R|=(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<1);//Clear Interrupts
    UART2_IM_R|=(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<1);//Unmask interrupts
    //Baud Calc: for 9600: Integer part: 104=0x68 and frac part=11= 0xB
    UART2_IBRD_R=0x68;//Integer Divider for baud rate
    UART2_FBRD_R=0xB;//Fraction Divider for baud rate

    UART2_LCRH_R|=(1<<6)|(1<<5);//8-bit Word length
    UART2_LCRH_R|=(1<<4);//FIFO Enable

    UART2_IFLS_R|=0x24;//FIFO level

    UART2_LCRH_R&=~(1<<2);//Odd parity
    UART2_LCRH_R|=(1<<1);//parity enable
    //UART2_CTL_R|=(1<<9);//Rx Enable
    UART2_CTL_R|=(1<<0);//UART Enable
}

void UART_Transmit(uint8_t TxWordF)
{
        UART2_CTL_R&=~(1<<0);//UART Disable
        Buffer=UART2_DR_R;//To flush the data_register
          //UART2_ICR_R=(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4);//Clear Interrupts
          //UART2_IM_R=(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4);//Unmask interrupts
          UART2_DR_R=TxWordF;//  // Send the current character
          UART2_CTL_R|=(1<<8);//Tx Enable
          UART2_CTL_R|=(1<<0);//UART Enable
}
void SysTickHandler()
{
       uint8_t wait=0;
    STCTRL=0x00;//to stop timer and clear Interrupt bit
    time_count++;
    if(time_count%10==0)
    {
               char systick_temp[20];
               sprintf(systick_temp, "%u", time_count/10);
               char *current_time_string=systick_temp;

               while (*current_time_word) {
                                       // Send the current character
                                       UART_Transmit(*current_time_word);
                                       // Move to the next character in the string
                                       current_time_word++;}
               current_time_word = orig_current_time_word;
               while (*current_time_string)
               {
                              // Send the current character
                              UART_Transmit(*current_time_string);
                              // Move to the next character in the string
                              current_time_string++;
               }
               while (*ms_string )
                                                           {
                                                                          // Send the current character
                                                                          UART_Transmit(*ms_string);
                                                                          // Move to the next character in the string
                                                                          ms_string++;
                                                           }
                              ms_string = orig_ms_string;
               while (*null_string )
                              {
                                             // Send the current character
                                             UART_Transmit(*null_string);
                                             // Move to the next character in the string
                                             null_string++;
                              }
               null_string=orig_null_string;

               while(wait);

    }
     STRELOAD=100*16000;//100ms delay
     STCURRENT=0;
     STCTRL |= (CLKINT | ENABLE |INTEN);//start systick


}
void GPIO_PortF_Handler()//change name in startup.ccs
{
    GPIO_PORTF_ICR_R |= MASK_BITS; // clear the GPIO interrupt

       if(GPIO_PORTF_DATA_R & 0x01)//SW1 pressed
           {
           int i;
           for(i=0;i<1000;i++){GPIO_PORTF_IM_R &= ~(MASK_BITS);} // mask interrupt by setting bits}//to account for debouncing
           GPIO_PORTF_IM_R |= MASK_BITS; // unmask interrupt by setting bits
           enable_status++;
           if(enable_status%2==1)
           {
           STRELOAD=100*16000;//100ms delay
              STCURRENT=0;
              STCTRL |= (CLKINT | ENABLE |INTEN);//start systick
              GPIO_PORTF_DATA_R |= (1<<GREEN_LED);//Turn ON GREEN LED
                     GPIO_PORTF_DATA_R &= ~(1<<RED_LED);//Turn OFF RED LED
                     GPIO_PORTF_DATA_R &= ~(1<<BLUE_LED);//Turn OFF BLUE LED
           }
           else
                      {

                         STCURRENT=0;
                         STCTRL=0x00;//stop systick
                         //resetting to 0:
                        lap_count=0;
                        int i;
                        for(i=0;i<10;i++)
                        {
                          timestamps[i]=0;}
                         time_count=0;
                         GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
                                 GPIO_PORTF_DATA_R &= ~(1<<GREEN_LED);//Turn OFF GREEN LED
                                 GPIO_PORTF_DATA_R &= ~(1<<BLUE_LED);//Turn OFF BLUE LED
                      }

           }
       if(GPIO_PORTF_DATA_R & 0x10)//SW2 pressed
       {
               GPIO_PORTF_DATA_R |= (1<<BLUE_LED);//Turn ON BLUE LED
               int i=0;
               for(i=0;i<1000;i++){GPIO_PORTF_IM_R &= ~(MASK_BITS);} // mask interrupt by setting bits}//to account for debouncing
                          GPIO_PORTF_IM_R |= MASK_BITS; // unmask interrupt by setting bits
               lap_count++;

               timestamps[lap_count]=time_count;//It is a 32 bit value
               current_laptime=timestamps[lap_count]-timestamps[lap_count-1];

               //char time_pressed_temp[20];
               //sprintf(time_pressed_temp, "%u", time_count*10);
               //char *time_pressed_at_string=time_pressed_temp;

               char lap_number_temp[20];
               sprintf(lap_number_temp, "%u", lap_count);
               char *lap_number_string=lap_number_temp;

               char current_laptime_temp[20];
               sprintf(current_laptime_temp, "%u", current_laptime);
               char *current_laptime_string=current_laptime_temp;

/*
               while (*time_pressed_at_word ) {
                                   // Send the current character
                                   UART_Transmit(*time_pressed_at_word);
                                   // Move to the next character in the string
                                   time_pressed_at_word++;}
               time_pressed_at_word = orig_time_pressed_at_word;
               while (*time_pressed_at_string ) {
                                       // Send the current character
                                       UART_Transmit(*time_pressed_at_string);
                                       // Move to the next character in the string
                                       time_pressed_at_string++;}
               while (*ms_string )
                                                                         {
                                                                                        // Send the current character
                                                                                        UART_Transmit(*ms_string);
                                                                                        // Move to the next character in the string
                                                                                        ms_string++;
                                                                         }
                                            ms_string = orig_ms_string;
               while (*null_string )
                                            {
                                                           // Send the current character
                                                           UART_Transmit(*null_string);
                                                           // Move to the next character in the string
                                                           null_string++;
                                            }
               null_string = orig_null_string;
*/
               while (*lap_number_word ) {
                                   // Send the current character
                                   UART_Transmit(*lap_number_word);
                                   // Move to the next character in the string
                                   lap_number_word++;}
               lap_number_word = orig_lap_number_word;
               while (*lap_number_string) {
                                          // Send the current character
                                          UART_Transmit(*lap_number_string);
                                          // Move to the next character in the string
                                          lap_number_string++;}
               while (*null_string )
                                            {
                                                           // Send the current character
                                                           UART_Transmit(*null_string);
                                                           // Move to the next character in the string
                                                           null_string++;
                                            }
               null_string=orig_null_string;

               while (*current_laptime_word ) {
                                   // Send the current character
                                   UART_Transmit(*current_laptime_word);
                                   // Move to the next character in the string
                                   current_laptime_word++;}
               current_laptime_word =orig_current_laptime_word ;
               while (*current_laptime_string) {
                                       // Send the current character
                                       UART_Transmit(*current_laptime_string);
                                       // Move to the next character in the string
                                       current_laptime_string++;}
               while (*ds_string )
                                                                         {
                                                                                        // Send the current character
                                                                                        UART_Transmit(*ds_string);
                                                                                        // Move to the next character in the string
                                                                                        ds_string++;
                                                                         }
                                            ds_string = orig_ds_string;
               while (*null_string )
                                            {
                                                           // Send the current character
                                                           UART_Transmit(*null_string);
                                                           // Move to the next character in the string
                                                           null_string++;
                                            }
               null_string=orig_null_string;

               GPIO_PORTF_DATA_R &= ~(1<<BLUE_LED);//Turn OFF BLUE LED


       }

}

void UART_Interrupt_Handler()//change name in startup.ccs
{
    //UART2_ICR_R|=(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<1);//Clear Interrupts
    if((UART2_RIS_R & (1<<10))==(1<<10))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    if((UART2_RIS_R & (1<<9))==(1<<9))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    if((UART2_RIS_R & (1<<8))==(1<<8))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    if((UART2_RIS_R & (1<<7))==(1<<7))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    if((UART2_RIS_R & (1<<6))==(1<<6))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    if((UART2_RIS_R & (1<<5))==(1<<5))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    if((UART2_RIS_R & (1<<4))==(1<<4))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    if((UART2_RIS_R & (1<<1))==(1<<1))
      {
          GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
      }
    UART2_ICR_R|=(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<1);//Clear Interrupts
    UART2_CTL_R&=~(1<<0);//UART Disable
}



int main()
{
      //Deciding to Toggle on PORTF
        SYSCTL_RCGC2_R |= 0x00000020;      // enable clock to GPIOF
        SYSCTL_RCGC2_R |= 0x00000008;      // enable clock to GPIOD
        SYSCTL_RCGCGPIO_R |= (1<<5)|(1<<2);    // Enable system clock to PORTF and PORTD and PORTC //PORT D for UART, PORTF for switch, PORTC for timer
        SYSCTL_RCGCUART_R |=(1<<2);//Enable UART Module 2

        GPIO_PORTF_LOCK_R = 0x4C4F434B;     // unlock commit register
        GPIO_PORTF_CR_R = 0xFF;             // make PORTF configurable
        GPIO_PORTF_DIR_R = 0xEE;            // set PORTF pins 0,4 as input, remaining output
        GPIO_PORTF_DEN_R = 0xFF;            // set PORTF pins as digital pins
        GPIO_PORTF_PUR_R |= 0x11;            // enable pull-up for pins 0 and 4

        GPIO_PORTF_IM_R &= ~MASK_BITS; // mask interrupt by clearing bits
        GPIO_PORTF_IS_R &= ~MASK_BITS; // edge sensitive interrupts
        GPIO_PORTF_IBE_R &= ~MASK_BITS; // interrupt NOT on both edges
        GPIO_PORTF_IEV_R &= ~MASK_BITS; // falling edge trigger

        // Prioritize and enable interrupts in NVIC
        NVIC_PRI7_R = (NVIC_PRI7_R & 0xF1FFFFFF) | (3 << 21);
         // interrupt priority register 7
         // bits 21-23 for interrupt 30 (port F)

        NVIC_EN0_R  |= 1 << 30; // enable interrupt 30 (port F)

        // Enable interrupt generation in GPIO
        GPIO_PORTF_ICR_R |= MASK_BITS; // clear any prior interrupt
        GPIO_PORTF_IM_R |= MASK_BITS; // unmask interrupt by setting bits

        GPIO_PORTF_DATA_R &= ~(1<<GREEN_LED);//Turn OFF Green LED
        GPIO_PORTF_DATA_R &= ~(1<<BLUE_LED);//Turn OFF BLUE LED
        GPIO_PORTF_DATA_R &= ~(1<<RED_LED);//Turn OFF RED LED



        //PORTD for UART
        GPIO_PORTD_LOCK_R = 0x4C4F434B;     // unlock commit register
        GPIO_PORTD_CR_R = 0xFF;             // make PORTD configurable
        GPIO_PORTD_DIR_R &= ~0x40; // Set Pin 6 as input
        GPIO_PORTD_DIR_R |= 0x80; // Set Pin 7 as output
        GPIO_PORTD_PUR_R |= 0x40;  // Enable the internal pull-up resistor for Pin 6
        GPIO_PORTD_DEN_R |= (0x80)|(0x40);  // Digital enable Pin 6 and 7

        //PortD Alternate function settings:
        GPIO_PORTD_AFSEL_R |= (1<<6)|(1<<7); // PD6 and PD7 set for alternate function
        GPIO_PORTD_PCTL_R |= 0x11000000;    // make PD7 and PD6 for UART2 Rx and Tx
        GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
        GPIO_PORTF_DATA_R &= ~(1<<GREEN_LED);//Turn OFF GREEN LED
        GPIO_PORTF_DATA_R &= ~(1<<BLUE_LED);//Turn OFF BLUE LED
    UART_Init();

    while(time_count<6000)//limit of 600 seconds(10 min)
        {

        }
  STCURRENT=0;
                         STCTRL=0x00;//stop systick
                         //resetting to 0:
                        lap_count=0;
                        int i;
                        for(i=0;i<10;i++)
                        {
                          timestamps[i]=0;}
                         time_count=0;
                         GPIO_PORTF_DATA_R |= (1<<RED_LED);//Turn ON RED LED
                                 GPIO_PORTF_DATA_R &= ~(1<<GREEN_LED);//Turn OFF GREEN LED
                                 GPIO_PORTF_DATA_R &= ~(1<<BLUE_LED);//Turn OFF BLUE LED

}


