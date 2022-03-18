/*
 * RelojDespertador.c
 *
 * Created: 23/02/2022 05:03:51 p. m.
 * Author: Daniel Dorantes
 */
#asm
    .equ __lcd_port=0x11  //Puerto F
    .equ __lcd_EN=4
    .equ __lcd_RS=5
    .equ __lcd_D4=0
    .equ __lcd_D5=1
    .equ __lcd_D6=2
    .equ __lcd_D7=3
#endasm

#asm
 .equ __ds1302_port=0x0B //Puerto D
 .equ __ds1302_io=2
 .equ __ds1302_sclk=1
 .equ __ds1302_rst=0
#endasm

#include <90usb1286.h>
#include <display.h>
#include <delay.h>
#include <ds1302.h>
#include <stdio.h>

//Definir string para poner hora en display
char Cadena[17];
unsigned char h, m,s,temp;
eeprom unsigned char HA,MA; //Init horas y minutos de la alarma en Memoria EEPROM
unsigned char tempEnt,tempDec;
float temper;
unsigned char Monito1[8]={0x0E,0x0E,0x0E,0x04,0x0E,0x15,0x0A,0x0A}; //Starting pose
unsigned char Monito2[8]={0x0E,0x0E,0x1F,0x0E,0x04,0x04,0x0A,0x0A}; //Final pose



// Voltage Reference: Int., cap. on AREF
#define ADC_VREF_TYPE ((1<<REFS1) | (1<<REFS0) | (0<<ADLAR))

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

unsigned int i=0,cont=0;
unsigned int suenaAlarma = 0;

flash int du=262,re= 294, ri=312, mi =330,fa=349, fi=370, sol=391,si=416, la=440, li=467, ti=494;
flash int MarioBros[591]={mi*2,mi*2,1,mi*2,1,du*2,mi*2,1,sol*2,1,1,1,sol,1,1,1,du*2,1,1,sol,1,1,mi,1,1,la,1,ti,1,li,la,1,sol,mi*2,1,sol*2,la*2,1,fa*2,sol*2,
1,mi*2,1,du*2,re*2,ti,1,1,du*2,1,1,sol,1,1,mi,1,1,la,1,ti,1,li,la,1,sol,mi*2,1,sol*2,la*2,1,fa*2,sol*2,1,mi*2,1,du*2,re*2,ti,1,1,1,1,sol*2,fi*2,fa*2,ri*2,1,
mi*2,1,si,la,du*2,1,la,du*2,re*2,1,1,sol*2,fi*2,fa*2,ri*2,1,mi*2,1,du*4,1,du*4,du*4,1,1,1,1,1,sol*2,fi*2,fa*2,ri*2,1,mi*2,1,si,la,du*2,1,la,du*2,re*2,1,1,
ri*2,1,1,re*2,1,1,du*2,1,1,1,1,1,1,1,1,1,sol*2,fi*2,fa*2,ri*2,1,mi*2,1,si,la,du*2,1,la,du*2,re*2,1,1,sol*2,fi*2,fa*2,ri*2,1,mi*2,1,du*4,1,du*4,du*4,1,1,1,
1,1,sol*2,fi*2,fa*2,ri*2,1,mi*2,1,si,la,du*2,1,la,du*2,re*2,1,1,ri*2,1,1,re*2,1,1,du*2,1,1,1,1,1,1,1,du*2,du*2,1,du*2,1,du*2,re*2,1,mi*2,du*2,1,la,sol,1,1,1,
du*2,du*2,1,du*2,1,du*2,re*2,1,1,1,1,1,1,1,1,du*2,du*2,1,du*2,1,du*2,re*2,1,mi*2,du*2,1,la,sol,1,1,1,mi*2,mi*2,1,mi*2,1,du*2,mi*2,1,sol*2,1,1,1,sol,1,1,1,du*2,
1,1,sol,1,1,mi,1,1,la,1,ti,1,li,la,1,sol,mi*2,1,sol*2,la*2,1,fa*2,sol*2,1,mi*2,1,du*2,re*2,ti,1,1,du*2,1,1,sol,1,1,mi,1,1,la,1,ti,1,li,la,1,sol,mi*2,1,sol*2,
la*2,1,fa*2,sol*2,1,mi*2,1,du*2,re*2,ti,1,1,mi*2,du*2,1,sol,1,1,si,1,la,fa*2,1,fa*2,la,1,1,1,ti,la*2,1,la*2,la*2,sol*2,1,fa*2,mi*2,du*2,1,la,sol,1,1,1,mi*2,du*2,
1,sol,1,1,si,1,la,fa*2,1,fa*2,la,1,1,1,ti,fa*2,1,fa*2,fa*2,mi*2,1,re*2,sol,mi,1,mi,du,1,1,1,mi*2,du*2,1,sol,1,1,si,1,la,fa*2,1,fa*2,la,1,1,1,ti,la*2,1,la*2,la*2,
sol*2,1,fa*2,mi*2,du*2,1,la,sol,1,1,1,mi*2,du*2,1,sol,1,1,si,1,la,fa*2,1,fa*2,la,1,1,1,ti,fa*2,1,fa*2,fa*2,mi*2,1,re*2,sol,mi,1,mi,du,1,1,1,du*2,du*2,1,du*2,1,
du*2,re*2,1,mi*2,du*2,1,la,sol,1,1,1,du*2,du*2,1,du*2,1,du*2,re*2,1,1,1,1,1,1,1,1,du*2,du*2,1,du*2,1,du*2,re*2,1,mi*2,du*2,1,la,sol,1,1,1,mi*2,mi*2,1,mi*2,1,du*2,
mi*2,1,sol*2,1,1,1,sol,1,1,1,mi*2,du*2,1,sol,1,1,si,1,la,fa*2,1,fa*2,la,1,1,1,ti,la*2,1,la*2,la*2,sol*2,1,fa*2,mi*2,du*2,1,la,sol,1,1,1,mi*2,du*2,1,sol,1,1,si,1,la,
fa*2,1,fa*2,la,1,1,1,ti,fa*2,1,fa*2,fa*2,mi*2,1,re*2,sol,mi,1,mi,du,1,1,1,0}; 

void noTono()
{
    TCCR1B=0x00;
    TCCR1A=0x00; 
    PORTB.1=0;
}

void tono(float frec)
{
    float Cuentas;
    unsigned int CuentasEnt;       // int y no char debido a que el numero de cuentas supera la cuenta de 8bits  
    Cuentas=500000.0/frec;
    CuentasEnt=Cuentas;
    if((Cuentas-CuentasEnt)>=0.5);   //redondear Cuentas 
    CuentasEnt++;   
    TCCR1A=0x40;  //Timer 1 en modo CTC
    TCCR1B= 0x09; //Timer en Ck (sin pre-escalador)
    OCR1AH=(CuentasEnt-1)>>8;
    OCR1AL=(CuentasEnt-1)&0xFF; 
}

void main(void)
{
    
    // ADC initialization
    // ADC Clock frequency: 125.000 kHz
    // ADC Voltage Reference: Int., cap. on AREF
    // ADC High Speed Mode: Off
    // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
    // ADC4: On, ADC5: On, ADC6: On, ADC7: Off
    DIDR0=(1<<ADC7D) | (0<<ADC6D) | (0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D);
    ADMUX=ADC_VREF_TYPE;
    ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);
    ADCSRB=(1<<ADHSM);
    
    if (MA>=60){
        MA=0;
    }
    if (HA>=24){
        HA=0;
    }
    
    SetupLCD();
    CreateChar(0,Monito1);
    CreateChar(1,Monito2);
    PORTC=0x1F; //PC0, PC1, PC2, PC3 pull-ups (push-buttons)
    DDRB.5 = 1; //PB5 de salida (bocina)
    
    // initialize the DS1302
    rtc_init(0,0,0);
    /* read time from the DS1302 RTC */
    rtc_get_time(&h,&m,&s);
    temp=s;
    delay_ms(1100);
    rtc_get_time(&h,&m,&s); //& paso de parámetros por referencia para que pueda modificar h,m y s
    if (temp==s){
        rtc_set_time(18,31,10);
    }
    MoveCursor(0,1);
    StringLCD("Alarma");
    while (1)
    {
        temper=(read_adc(7)*256.0)/1024.0; //Ejemplo con 23.36
        tempEnt=temper; //tempEnt=23
        temper=temper-tempEnt; //temp=23.36-23=0.36
        temper=temper*10;      //temp=3.6
        tempDec=temper;      //tempDec=3
        if((temper-tempDec)>=0.5) //redondear  
        {
            tempDec++;
            if(tempDec==10)
            {
                tempDec=0;
                tempEnt++;
            }
        }
        
        /* read time from the DS1302 RTC */
        rtc_get_time(&h,&m,&s);
        sprintf(Cadena,"%02i:%02i:%02i %i.%i %cC",h,m,s,tempEnt,tempDec,0xDF); //0xDF es el caracter especial para el signo de grados 
        MoveCursor(0,0);
        StringLCDVar(Cadena);
        sprintf(Cadena,"%02i:%02i",HA,MA); //0xDF es el caracter especial para el signo de grados 
        MoveCursor(7,1);
        StringLCDVar(Cadena);
        
        if ((PINC.0==0)&&(suenaAlarma==0)){ //Botón de incrementar las Horas
            h= h+1;
            if (h>=24)
                h=0;
            rtc_set_time(h,m,s);
        }
        
        if ((PINC.1==0)&&(suenaAlarma==0)){ //Botón de incrementar los Minutos
            m= m+1;
            if (m>=60)
                m=0;
            rtc_set_time(h,m,s);
        }
        
        if ((PINC.2==0)&&(suenaAlarma==0)){ //Botón de incrementar las Horas Alarma
            HA= HA+1;
        }
        
        if ((PINC.3==0)&&(suenaAlarma==0)){ //Botón de incrementar las Minutos Alarma
            MA= MA+1;
        }    
        
        //Checar que no se excedan los Minutos y Horas
        if (MA>=60)
            MA=0;
        if (HA>=24)
            HA=0;
            
        if ((HA==h)&&(MA==m)&&(s==0))  //Modo alarma
        {   
            suenaAlarma = 1;
            EraseLCD();
            MoveCursor(4,0);
            StringLCD("DESPIERTA!!");
            while ((PINC.0==1) && (PINC.1==1) && (PINC.2==1) && (PINC.3==1)&&(HA==h)&&(MA==m)&&(cont<560)){  
                
                if(MarioBros[i]!=1) //1 es silencio
                {
                    MoveCursor(7,1);
                    CharLCD(0);
                    tono(MarioBros[i]); //Agregan aporx 7ms de delay
                    delay_ms(100);
                    noTono();
                } 
                else 
                {   
                    MoveCursor(7,1);
                    CharLCD(1);     //Agregan aporx 7ms de delay
                    delay_ms(100); 
                }
                i++;
                cont++; 
                if(i==591)
                    i=0;
            }
            i=0;
            cont=0;            
            if (suenaAlarma == 1){
                delay_ms(500);
                suenaAlarma=0;
            } 
        }
        else{
            suenaAlarma=0;
        }
    }  
}

