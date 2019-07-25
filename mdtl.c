///////////////////////////////////////////////////////////
//
// PROGRAMA PARA EL MODEM MDTL
//
//        
// Parametros para el compilador: +FM +t
//
// 
////////////////////////////////////////////////////////////

#include <y:\picc\examples\16c63a.h>
#include <y:\picc\examples\ctype.h>
#include <y:\picc\examples\stdlib.h>
#include <y:\picc\examples\string.h>
                                         
#byte PORT_A=5
#byte PORT_B=6
#fuses HS,NOWDT,PROTECT

#use FIXED_IO (b_outputs = PIN_B4)
#use delay (clock=20000000)

#define BUFFER_SIZE 32
//#include "modem.h"
//#include "host.h"

//Configuración EEPROM (9346)
#define CLOCK	PIN_A0
#define bkbhit (next_in!=next_out)

#use FAST_IO (B)

#define PARITY_NONE	0
#define PARITY_ODD	1
#define PARITY_EVEN 2
                  
                  
//Configuración comunicación RS232
#use rs232 ( ERRORS, baud=9600, PARITY = E, BITS=8,xmit=PIN_C6, rcv=PIN_C7)
void SendByte (int data, int par);
void serial_isr();
int bgetc();
void SendMarca ();
byte RxByte ();
int par;

int buffer[BUFFER_SIZE];
byte next_in = 0;
byte next_out = 0;

void main (void)
{
	byte bt;

	enable_interrupts(global);
	enable_interrupts(int_rda);

	set_tris_a (0x2F);
	set_tris_b (0x1F);
	output_bit (PIN_B5,0);

	par=((PORT_B)>>1)&0x3;	//Miramos la paridad

	switch (par)
	{
		case PARITY_NONE:
			#use rs232 (ERRORS,PARITY = N, BITS=8,xmit=PIN_C6, rcv=PIN_C7)
		break;
		
		case PARITY_ODD:
			#use rs232 (ERRORS,PARITY = O, BITS=8,xmit=PIN_C6, rcv=PIN_C7)
		break;			

		case PARITY_EVEN:
			#use rs232 (ERRORS,PARITY = E, BITS=8,xmit=PIN_C6, rcv=PIN_C7)
		break;			
	}

	bt=((PORT_B)>>3)&0x3;	//Miramos la velocidad
	
	switch (bt)
	{
		case 3:
			SET_UART_SPEED (4800);
		break;

		case 2:
			SET_UART_SPEED (19200);
		break;
		
		case 1:
			SET_UART_SPEED (9600);
		break;
			
		case 0:
			SET_UART_SPEED (38400);
		break;
				
	}
		
	while (TRUE)
	{

		SendMarca ();	//Mientras no hayan datos, enviamos 0xff para indicar
						//señal de marca.

		while (input (PIN_B0))	//Generamos test
		{
			SendByte ('t',par);	
			SendByte ('e',par);	
			SendByte ('s',par);	
			SendByte ('t',par);	
		}

	    while(bkbhit)
	        SendByte(bgetc(),par);
	}
}
         

byte RxByte ()
{    
	int n;
	char bt;
	
	for (n=0;n<8;n++)
	{
		while (!input (PIN_A2));
		shift_right (&bt,1,input (PIN_A3));
		while (input (PIN_A2));
	}
	while (!input (PIN_A2));
	while (input (PIN_A2));
	return bt;
}

void SendMarca ()
{
	int i;
	for(i=0;i<8;i++)
	{
		while (!input(PIN_A0));
		output_bit(PIN_A1, 1);
		while (input(PIN_A0));
	}
}
void SendByte (int data, int par)
{
	int i;
	byte shf;
	int paridad;

	while (!input(PIN_A0));
	output_bit(PIN_A1, 0);			//Generamos bit de START
	while (input(PIN_A0));
             
	paridad=1;
			
	for(i=0;i<8;i++)
	{
		while (!input(PIN_A0));
		shf=shift_right (&data,1,0);	//Enviamos bit de datos
		output_bit(PIN_A1,shf);
		
		if (shf)
			paridad = !paridad;		//Calculamos paridad
		while (input(PIN_A0));
	}

//	if (par)	//Si hay paridad
//	{
		while (!input(PIN_A0));
		output_bit(PIN_A1, shift_right (&paridad,1,0));	//Enviamos paridad
		while (input(PIN_A0));
//	}

	while (!input(PIN_A0));	
	output_bit(PIN_A1, 1);		//Enviamos bit de STOP
	while (input(PIN_A0));
}

#int_rda
void serial_isr() {
	int t;
   buffer[next_in]=getc();
   t=next_in;
   next_in=(next_in+1) % BUFFER_SIZE;
   if(next_in==next_out)
     next_in=t;           // Buffer full !!
}

int bgetc() {
   int c;

   while(!bkbhit) ;
   c=buffer[next_out];
   next_out=(next_out+1) % BUFFER_SIZE;
   return(c);
}

