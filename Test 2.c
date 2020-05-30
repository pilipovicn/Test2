#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>

//Velicina prijemnog bafera (mora biti 2^n)
#define USART_RX_BUFFER_SIZE 64

char Rx_Buffer[USART_RX_BUFFER_SIZE];			//prijemni FIFO bafer
volatile unsigned char Rx_Buffer_Size = 0;	//broj karaktera u prijemnom baferu
volatile unsigned char Rx_Buffer_First = 0;
volatile unsigned char Rx_Buffer_Last = 0;

ISR(USART_RX_vect)
{
  	Rx_Buffer[Rx_Buffer_Last++] = UDR0;		//ucitavanje primljenog karaktera
	Rx_Buffer_Last &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	if (Rx_Buffer_Size < USART_RX_BUFFER_SIZE)
		Rx_Buffer_Size++;					//inkrement brojaca primljenih karaktera
}

void usartInit(unsigned long baud)
{
	UCSR0A = 0x00;	//inicijalizacija indikatora
					//U2Xn = 0: onemogucena dvostruka brzina
					//MPCMn = 0: onemogucen multiprocesorski rezim

	UCSR0B = 0x98;	//RXCIEn = 1: dozvola prekida izavanog okoncanjem prijema
					//RXENn = 1: dozvola prijema
					//TXENn = 1: dozvola slanja

	UCSR0C = 0x06;	//UMSELn[1:0] = 00: asinroni rezim
					//UPMn[1:0] = 00: bit pariteta se ne koristi
					//USBSn = 0: koristi se jedan stop bit
					//UCSzn[2:0] = 011: 8bitni prenos

	UBRR0 = F_CPU / (16 * baud) - 1;

	sei();	//I = 1 (dozvola prekida)
}

unsigned char usartAvailable()
{
	return Rx_Buffer_Size;		//ocitavanje broja karaktera u prijemnom baferu
}

void usartPutChar(char c)
{
	while(!(UCSR0A & 0x20));	//ceka da se setuje UDREn (indikacija da je predajni bafer prazan)
	UDR0 = c;					//upis karaktera u predajni bafer
}

void usartPutString(char *s)
{
	while(*s != 0)				//petlja se izvrsava do nailaska na nul-terminator
	{
		usartPutChar(*s);		//slanje tekuceg karaktera
		s++;					//azuriranje pokazivaca na tekuci karakter
	}
}

void usartPutString_P(const char *s)
{
	while (1)
	{
		char c = pgm_read_byte(s++);	//citanje sledeceg karaktera iz programske memorije
		if (c == '\0')					//izlazak iz petlje u slucaju
			return;						//nailaska na terminator
		usartPutChar(c);				//slanje karaktera
	}
}

char usartGetChar()
{
	char c;

	if (!Rx_Buffer_Size)						//bafer je prazan?
		return -1;
  	c = Rx_Buffer[Rx_Buffer_First++];			//citanje karaktera iz prijemnog bafera
	Rx_Buffer_First &= USART_RX_BUFFER_SIZE - 1;	//povratak na pocetak u slucaju prekoracenja
	Rx_Buffer_Size--;							//dekrement brojaca karaktera u prijemnom baferu

	return c;
}

unsigned char usartGetString(char *s)
{
	unsigned char len = 0;

	while(Rx_Buffer_Size) 			//ima karaktera u faferu?
		s[len++] = usartGetChar();	//ucitavanje novog karaktera

	s[len] = 0;						//terminacija stringa
	return len;						//vraca broj ocitanih karaktera
}

char korisnici[10][32] =

{

    "Sundjer Bob Kockalone",

    "Dijego Armando Maradona",

    "Bond. Dzejms bond.",

    "Zoran Kostic Cane",

    "Kim Dzong Un",
      
    "Marko Markovic",
  
    "Milica Milicevic",
      
    "Milan Milanovic",
  
    "Zoran Zoranovic",
  
    "Jelena Jelenic"

};

char PINCODE[10][5] = {"5346", "2133", "7445", "8756", "7435", "1234", "0666", "0120", "0542", "2510"};

int main()
{
	usartInit(9600);
  	
	while(1)
	{
      int i=0;
      int id=-1;
      char ime[32];
      char pin[4];
      int attempt=3;
      
      usartPutString("Dobar dan, unesite ime i prezime:\r\n");
      
      while(!usartAvailable());
      _delay_ms(50);
      
      usartGetString(ime);
      
      for(i=0; i<10; i++)
       if(!strcmp(ime,korisnici[i])) id=i;
        
      if(id<0){
        usartPutString("Nepostojeci korisnik!\r\n");
        continue;
      }
      
      usartPutString("Dobrodosli ");
      usartPutString(ime);
      usartPutString("! \r\nUnesite PIN: ");
      
      while(1){
        
        while(!usartAvailable());
        _delay_ms(50);

        usartGetString(pin);

        if(!strcmp(pin,PINCODE[id])) break;
        
        attempt--;
        usartPutString("Pogresan PIN! ");
        
        if (!attempt){
          usartPutString("3 puta ste pogresili PIN! Dovidjenja!\r\n");
          break;
        }
        usartPutString("Imate ukupno 3 pokusaja:\r\n");
      }
      if (!attempt) continue;
      
      usartPutString("****\r\n");
      usartPutString("Uspesno! Na racunu imate 100 evra! \r\n");
      usartPutString("Za novog korisnika upisite bilo sta. \r\n");
      
      while(!usartAvailable());
      usartGetString(ime); //flush
    }

	return 0;
}