/*
 * Sensor TH.c
 *
 * Created: 11/26/2020 9:26:31 AM
 * Author : Glauco Filho
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include "LCD16x2_4bit.h"
#define DHT11_PIN 6
uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;

void lcdcommand(unsigned char cmnd)
{
	LCD_DPRT = (LCD_DPRT & 0x0f)|(cmnd & 0xf0);		/* ENVIA COMANDO PARA DATA PORT */
	LCD_DPRT &= ~ (1<<LCD_RS);						/* RS = 0 PARA COMMAND */
	LCD_DPRT |= (1<<LCD_EN);						/* EN = 1 PARA PULSO DE H ATE L */
	_delay_us(1);									
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 PARA PULSO DE H ATE L */
	_delay_us(100);									
	
	LCD_DPRT = (LCD_DPRT & 0x0f)|(cmnd << 4);		/* ENVIA COMANDO PARA DATA PORT */
	LCD_DPRT |= (1<<LCD_EN);						/* EN = 1 PARA PULSO DE H ATE L */
	_delay_us(1);									
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 PARA PULSO DE H ATE L */
	_delay_ms(2);									
}

void lcddata(unsigned char data)
{
	LCD_DPRT = (LCD_DPRT & 0x0f)|(data & 0xf0);		/* ENVIA COMANDO PARA DATA PORT */
	LCD_DPRT |= (1<<LCD_RS);						/* MAKE RS = 1 PARA TER A DATA */
	LCD_DPRT |= (1<<LCD_EN);						/* EN=0 PARA PULSO DE H ATE L */
	_delay_us(1);									
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 PARA PULSO DE H ATE L */
	_delay_us(100);									
	
	LCD_DPRT = (LCD_DPRT & 0x0f)|(data << 4);		
	LCD_DPRT |= (1<<LCD_EN);						/* EN=0 PARA PULSO DE H ATE L*/
	_delay_us(1);									
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 PARA PULSO DE H ATE L*/
	_delay_ms(2);									
}

void lcdinit()
{
	LCD_DDDR = 0xFF;
	_delay_ms(200);									
	lcdcommand(0x33);
	lcdcommand(0x32);								/* ENVIA $32 PARA INIT OT 0X02 */
	lcdcommand(0x28);								/* INIT. LCD 2 LINE, MATRIZ 5 X 7 */
	lcdcommand(0x0C);								/* DISPLAY LIGA */
	lcdcommand(0x01);								/* LIMPA LCD */
	_delay_ms(2);
	lcdcommand(0x82);								/* MUDA O CURSOR PARA ESCRITA */
}

void lcd_gotoxy(unsigned char x, unsigned char y)
{
	unsigned char firstcharadd[]={0x80, 0xC0};
	lcdcommand(firstcharadd[y] + x);
}

void lcd_print(char *str)
{
	unsigned char i=0;
	while (str[i] |= 0)
	{
		lcddata(str[i]);
		i++;
	}
}

void lcd_clear()
{
	lcdcommand(0x01);
	_delay_ms(2);
}
void Request()						/* MICROCONTROLADOR ENIVA PULSO DE INICIO OU PEDIDA*/
{
	DDRD |= (1<<DHT11_PIN);
	PORTD &= ~(1<<DHT11_PIN);		/* NBA */
	_delay_ms(20);					
	PORTD |= (1<<DHT11_PIN);		/* NAA */
}

void Response()						/* RESPOSTA RECEBIDA DO DHT11 */
{
	DDRD &= ~(1<<DHT11_PIN);
	while(PIND & (1<<DHT11_PIN));
	while((PIND & (1<<DHT11_PIN))==0);
	while(PIND & (1<<DHT11_PIN));
}

uint8_t Receive_data()							/* RECEBE A DATA */
{
	for (int q=0; q<8; q++)
	{
		while((PIND & (1<<DHT11_PIN)) == 0);	/* BIT 0 OU 1 */
		_delay_us(30);
		if(PIND & (1<<DHT11_PIN))				/* CASO MAIOR Q 30MS O PULSO */
		c = (c<<1)|(0x01);						/* NAA */
		else									/* NBA */
		c = (c<<1);
		while(PIND & (1<<DHT11_PIN));
	}
	return c;
}

int main(void)
{
	char data[5];
	lcdinit();					/* INICIA O LCD */
	lcd_clear();				/* LIMPA O LCD */
	lcd_gotoxy(0,0);			/* POSICAO DA COLUNA E LINHA */
	lcd_print("Humidity =");
	lcd_gotoxy(0,1);
	lcd_print("Temp = ");
	
	while(1)
	{
		Request();				/* ENIVA PULSO DE INICIO */
		Response();				/* RECEBE A RESPOSTA DO ENVIO */
		I_RH=Receive_data();	/* ARMAZENA PRIMEIROS 8 BITS NO I_RH */
		D_RH=Receive_data();	/* ARMAZENA OS PROXIMOS 8 BITS NO D_RH */
		I_Temp=Receive_data();	/* ARMAZEAN OS PROXIMOS 8 BITS NO I_Temp */
		D_Temp=Receive_data();	/* ARMAZENA OS PROXIMOS 8 BITS NO D_Temp */
		CheckSum=Receive_data();/* ARMAZENA OS PROXIMOS 8 BITS NO CheckSum */
		
		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
		{
			lcd_gotoxy(0,0);
			lcd_print("Error");
		}
		
		else
		{
			itoa(I_RH,data,10);
			lcd_gotoxy(11,0);
			lcd_print(data);
			lcd_print(".");
			
			itoa(D_RH,data,10);
			lcd_print(data);
			lcd_print("%");

			itoa(I_Temp,data,10);
			lcd_gotoxy(6,1);
			lcd_print(data);
			lcd_print(".");
			
			itoa(D_Temp,data,10);
			lcd_print(data);
			lcddata(0xDF);
			lcd_print("C ");
			
			itoa(CheckSum,data,10);
			lcd_print(data);
			lcd_print(" ");
		}
		
		_delay_ms(500);
	}
}
