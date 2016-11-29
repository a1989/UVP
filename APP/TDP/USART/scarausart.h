#ifndef __SCARAUSART_H
#define __SCARAUSART_H

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stdint.h"

#define BAUDRATE 115200

#define MSG_OK 	"ok"
#define MSG_FILE_SAVED	"Done saving file"
#define MSG_M104_INVALID_EXTRUDER 	"M104 Invalid extruder"
#define MSG_M105_INVALID_EXTRUDER 	"M105 Invalid extruder"
#define MSG_M109_INVALID_EXTRUDER 	"M109 Invalid extruder"

#define RX_BUFFER_SIZE 128

struct ring_buffer
{
		unsigned char buffer[RX_BUFFER_SIZE];
		int head;
		int tail;
};

void Usart_Init(void);
void init_usart(void);
char usart_getch(void);
char usart_getchar(void);
void usart_putstr(char *p);
void usart_putstrln(char *p);
void USART_Put_I2A(uint32_t n);
void USART_Put_F2A(double n);
uint32_t Serial_Available(void);
char read_cmd(void);
void buffer_flush(void);

#endif
