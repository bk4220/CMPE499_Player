/*
 * File:   main.c
 * Author: Brett
 *
 * Created on January 4, 2024, 2:07 PM
 */



#include "config.h"

void system_init(void);
void delay(unsigned long milliseconds);

void lcd_init(void);
void i2c_start_and_addr(unsigned char address);
void lcd_clear(void);
void i2c_data_tx(char data);
void i2c_stop(void);
void lcd_message(char *message);
void lcd_char(char letter);
void lcd_command(char data, char lt, char rw, char rs);
void lcd_move_cursor(char line, char position);
void lcd_backspace(void);
char check_strings();

char has_printed_message = 0;
char current_character = 0;
char last_character = 0;
char has_prior_check = 0;
char new_input = 0;
char character_match_cnt = 0;
char Receiver_buffer[TRANSMISSION_REPEATS + 1][11];
char Buffer_col_index = 0;
char Buffer_row_index = 0;
char Time_out = 0;
char screen_has_text = 0;

void main(void) 
{
    
    
    system_init();
    
    lcd_init();    
    
    //Enables timer0 for interrupt every .25 seconds
    T0CON = 0b00000111;
    INTCONbits.TMR0IF = 0;    
    TMR0H = 0x00;
    TMR0L = 0x00;
    INTCONbits.TMR0IE = 1;
    T0CONbits.TMR0ON = 1;
    
    while(1)
    {
       if(new_input)
       {
           if(has_prior_check == 0)
           {
               has_prior_check = 1;
               new_input = 0;
           }
           else
           {
               character_match_cnt = check_strings();
               new_input = 0;
           }
           
           if(character_match_cnt >= 1 && !has_printed_message)
           {
               char i = 0;
                //lcd_message(Receiver_buffer + (Buffer_index - 1));
                //while(Receiver_buffer[Buffer_row_index -1][i] != '\0')
                //{
                   //lcd_message(Receiver_buffer + (((Buffer_row_index - 1) * 11) + i));
                    //i++;
                //}
               
               
               
               
                //BLOCKED FOR TESTING
                //lcd_message(Receiver_buffer + (Buffer_row_index - 1) * 11);
               
               
               //testing
                lcd_move_cursor(1,0);
                lcd_message(Receiver_buffer + (Buffer_row_index - 1) * 11);
               
                screen_has_text = 1;
                has_printed_message = 1;
                new_input = 0;
                
           }
       }
    }

    return;
}

void __interrupt() ISR()
{
    if(INTCONbits.INT0IF == 1)
    {
        //turn timeout timer off
        //T0CONbits.TMR0ON = 0;
        //TMR0H = 0x00;
        //TMR0L = 0x00;
        
        last_character = current_character;
        current_character = (PORTB & 0x1F) >> 1;
        while(PORTBbits.RB0);
        INTCONbits.INT0IF = 0;
        while(!INTCONbits.INT0IF)
        {
         if(INTCONbits.TMR0IF == 1)
         {
             INTCONbits.TMR0IF = 0;
            Time_out++;
            if(Time_out == TIMEOUT_MAX && !new_input && screen_has_text)
            {
             lcd_clear();
             screen_has_text = 0;
             Time_out = 0;
                new_input = 0;
             has_prior_check = 0;
                character_match_cnt = 0;
                Buffer_row_index = 0;
                Buffer_col_index = 0;
            
              return;  
            }
            
         }
        }
        current_character = current_character | ((PORTB & 0x1E) << 3);
        
        INTCONbits.INT0IF = 0;
        
        //T0CONbits.TMR0ON = 1;
        if(current_character != 0xAA && current_character != 0xBB && current_character != 'B')
        {
            Receiver_buffer[Buffer_row_index][Buffer_col_index] = current_character;
            Buffer_col_index++;
            
        }
        else if(current_character == 0xBB && last_character != 0xBB)
        {
            //test print
            lcd_message(Receiver_buffer + (Buffer_row_index) * 11);
            
            Receiver_buffer[Buffer_row_index][Buffer_col_index] = '\0';
            Buffer_row_index++;
            Buffer_col_index = 0;
            new_input = 1;
        }
        else if(current_character == 'B' && last_character != 'B')
        {
            lcd_clear();
            has_printed_message = 0;
            new_input = 0;
            character_match_cnt = 0;
            current_character = 0;
            has_prior_check = 0;
            Time_out = 0;
            Buffer_row_index = 0;
            Buffer_col_index = 0;
            
        }
        else if ((current_character == 0xAA && (has_printed_message || Buffer_row_index == 0)) || Buffer_row_index >= 7)
        {
            has_printed_message = 0;
            new_input = 0;
            character_match_cnt = 0;
            current_character = 0;
            has_prior_check = 0;
            Time_out = 0;
            Buffer_row_index = 0;
            Buffer_col_index = 0;
            
        }
    }
    if(INTCONbits.TMR0IF == 1)
    {
        //timer and variable init
        INTCONbits.TMR0IF = 0;
        Time_out++;
        if(Time_out >= TIMEOUT_MAX && !new_input && screen_has_text)
        {
            lcd_clear();
            screen_has_text = 0;
            Time_out = 0;
            new_input = 0;
            has_prior_check = 0;
            character_match_cnt = 0;
            Buffer_row_index = 0;
            Buffer_col_index = 0;
            for(int i = 0; i < TRANSMISSION_REPEATS;i++)
            {
                Receiver_buffer[i][8] = '\0';

            }
        }
    
    }
    return;
}

char check_strings()
{
    char match_cnt = 0;
    char j = 0;
    char currently_match = 1;
    for(int i = 0; i < Buffer_row_index - 1; i++)
    {
        while(currently_match && Receiver_buffer[Buffer_row_index - 1][j] != '\0')
        {
            if(Receiver_buffer[Buffer_row_index - 1][j] == Receiver_buffer[i][j])
            {
                j++;
            }
            else
            {
               currently_match = 0; 
            }
        }
        if(currently_match)
        {
            match_cnt++;
        }
        j = 0;
    }
    return match_cnt;
}

void delay(unsigned long milliseconds)
{
    T1CON = 0b10110000;
    PIE1bits.TMR1IE = 0;
    unsigned long long timer_value = (milliseconds) * 1000 ;
    while(timer_value > 65535)
    {
        timer_value -= 65535;
        TMR1H = 0;
        TMR1L = 0;
        T1CONbits.TMR1ON = 1;
        while(!PIR1bits.TMR1IF);
        T1CONbits.TMR1ON = 0;
        PIR1bits.TMR1IF = 0;
    }
    timer_value = 65535 - timer_value + 1;
    TMR1H = (unsigned char)(0xFF00 & timer_value);
    TMR1L = (unsigned char)(0xFF & timer_value);
    T1CONbits.TMR1ON = 1;
    while(!PIR1bits.TMR1IF);
    T1CONbits.TMR1ON = 0;
    PIR1bits.TMR1IF = 0;
}

void lcd_init()
{
    //i2c module data format: P7-P0: 0b 7 6 5 4 lt E Rw Rs
    //                                  0 0 0 0  0 0  0  0
    
    
    //sets lcd into 4 bit mode
    lcd_command(0x02, 1, 0, 0);
    lcd_command(0x28, 1, 0, 0);
    
    lcd_clear();
    
    //turns cursor on
    lcd_command(0x0F, 1, 0, 0);
    
    
    
    //prints out capstone splash screen
    lcd_move_cursor(0, 5);
    lcd_message("Senior");
    lcd_move_cursor(1, 4);
    lcd_message("Capstone");
    //delay(500);
    lcd_clear();
    //lcd_message("By Aidan, Brett,");
    //lcd_move_cursor(1, 0);
    //lcd_message("Chris, and Gabe");
    //delay(1000);
    //lcd_clear();
    
    
    
}

void lcd_backspace()
{
    lcd_command(0x10, 1, 0, 0);
    lcd_message(" ");
    lcd_command(0x10, 1, 0, 0);
}

void lcd_move_cursor(char line, char position)
{
    if(!line)
    {
        lcd_command(0x80 | position, 1, 0, 0);
    }
    else
    {
        lcd_command(0xC0 | position, 1, 0, 0);
    }
    
    
}

void lcd_command(char data, char lt, char rw, char rs)
{
    i2c_start_and_addr(LCD_ADDR);
    
    i2c_data_tx((data & 0xF0) | (lt << 3) | 4 | (rw <<1) | rs);
    delay(1);
    i2c_data_tx(lt<<3);
    
    
    i2c_data_tx(((data & 0x0F) <<4) | (lt << 3) | 4 | (rw <<1) | rs);
    delay(1);
    i2c_data_tx(lt<<3);
    
    i2c_stop();
}

void lcd_message(char* message)
{
    i2c_start_and_addr(LCD_ADDR);
    while(*message != 0)
    {
        lcd_char(*message);
        message++;
    }
    i2c_stop();
}

void lcd_char(char letter)
{
    i2c_data_tx((letter & 0xF0) | 0b1101);
    i2c_data_tx((letter & 0xF0) | 8);
    
    
    
    i2c_data_tx(((letter & 0x0F) << 4) | 0b1101);
    i2c_data_tx(((letter & 0x0F) << 4) | 8);
    
}

void lcd_clear()
{
    i2c_start_and_addr(LCD_ADDR);
    
    //clears lcd
    i2c_data_tx(0b00001100);
    delay(1);
    i2c_data_tx(0b00001000);
    
    
    i2c_data_tx(0b00011100);
    delay(1);
    i2c_data_tx(0b00011000);
    delay(1);

    i2c_stop();
}

void i2c_data_tx(char data)
{
    do
    {
        SSPBUF = data;
        while(!PIR1bits.SSPIF);
        PIR1bits.SSPIF = 0;
        
    }while(SSPCON2bits.ACKSTAT);
}

void i2c_stop()
{
   //stops i2c transmission
    SSPCON2bits.PEN = 1;
    while(!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0; 
}

void i2c_start_and_addr(unsigned char address)
{
    //start condition and addressing the i2c
    SSPCON2bits.SEN = 1;
    while(!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;
    do
    {
        SSPBUF = address;
        while(!PIR1bits.SSPIF);
        PIR1bits.SSPIF = 0;
    }while(SSPCON2bits.ACKSTAT);
    
}

void system_init()
{
    //sets internal clock and pll to 32MHz
    OSCCONbits.SCS = 0;
    OSCCONbits.IDLEN = 1;
    OSCCONbits.IRCF = 7;
    OSCTUNEbits.PLLEN = 1;
    
    //enables interrupts and priorities
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    INTCON2bits.RBPU = 0;
    
    //makes I/O digital
    ADCON1bits.PCFG = 15;
    
    //change to one if we were to want priority levels
    RCONbits.IPEN = 0;
    
    //Receiver init
    TRISB = TRISB | 0x1F;
    PORTB = PORTB & 0xE0;
    INTCONbits.INT0IE = 1;
    INTCON2bits.INTEDG0 = 1;
    
    //i2c init
    SSPSTATbits.SMP = 1;
    SSPSTATbits.CKE = 0;
    SSPADD = 0x50;
    SSPCON1bits.SSPEN = 1;
    SSPCON1bits.SSPM = 0b1000;
    TRISCbits.RC3 = 1;
    TRISCbits.RC4 = 1;
    
    //empty buffer array
    for(int i = 0; i < TRANSMISSION_REPEATS;i++)
    {
        
        for(int j = 0; j < 11;j++)
        {
            Receiver_buffer[i][j] = '\0';
        }
    }
}
