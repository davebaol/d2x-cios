#include "fs_calls.h"

#ifdef DEBUG

#include "syscalls.h"
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "fat.h"
#include "plugin.h"

char mem_cad[32];

void int_char(int num)
{
	int sign = num<0;
	int n,m;

	if(num==0) {
		mem_cad[0] = '0';
		mem_cad[1] = 0;
		return;
	}

	for(n=0;n<10;n++) {
		m = num % 10;
		num /= 10;
		if(m<0) m = -m;
		mem_cad[25-n] = 48+m;
	}

	mem_cad[26] = 0;

	n = 0;
	m = 16;
	if(sign) {
		mem_cad[n] = '-';
		n++;
	}

	while(mem_cad[m]=='0') m++;

	if(mem_cad[m]==0) m--;

	while(mem_cad[m]) {
		mem_cad[n] = mem_cad[m];
		n++;
		m++;
	}
	mem_cad[n]=0;

}

void uint_char(unsigned int num)
{
	int n,m;

	if(num==0) {
		mem_cad[0] = '0';
		mem_cad[1] = 0;
		return;
	}

	for(n=0;n<10;n++) {
		m = num % 10;
		num /= 10;
		mem_cad[25-n] = 48+m;
	}

	mem_cad[26] = 0;

	n=0;
	m=16;

	while(mem_cad[m]=='0') m++;

	if(mem_cad[m]==0) m--;

	while(mem_cad[m]) {
		mem_cad[n] = mem_cad[m];
		n++;
		m++;
	}
	mem_cad[n]=0;

}

void hex_char(u32 num)
{
	int n,m;

	if(num==0) {
		mem_cad[0] = '0';
		mem_cad[1] = 0;
		return;
	}

	for(n=0; n<8; n++) {
		m = num & 15;
		num >>= 4;
		if(m>=10)
			m+=7;
		mem_cad[23-n] = 48+m;
	}

	mem_cad[24]=0;

	n=0;m=16;
    
	mem_cad[n]='0';n++;
	mem_cad[n]='x';n++;

	while(mem_cad[m]=='0') m++;

	if(mem_cad[m]==0) m--;

	while(mem_cad[m]) {
		mem_cad[n] = mem_cad[m];
		n++;
		m++;
	}
	mem_cad[n] = 0;

}

void printf_write(int fd, void *data, int len)
{ 
	os_sync_after_write(data, len);

	FAT_WriteLog(fd, data, len);
}

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00
#define FA__ERROR			0x80

#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20


#define FS_ENOENT	-106

void FS_printf(const char *format, ...)
{
	va_list opt;

	static char out[128] ATTRIBUTE_ALIGN(32)=" ";

	int val;
	char *s;

	va_start(opt, format);

	int fd = -1;
	int message;

	static int first_time = 1;
	static u32 buffer[8];
	static u32 queuehandle2 = -1;

	if(first_time) {
		/* Create the queue only the first time*/
		queuehandle2 = os_message_queue_create(buffer, 8);
		first_time = 0;   
	}
	else
		os_message_queue_receive(queuehandle2, (void *)&message, 0);

	if(config.logfile[0] == '\0')
		goto exit;

	fd = FAT_OpenLog(config.logfile, FA_OPEN_ALWAYS | FA_WRITE);

	if(fd<0) 
		goto exit;

	FAT_SeekLog(fd, 0, SEEK_END); 

	while(format[0]) {
		if(format[0]!='%') {
			out[0] = *format++;
			printf_write(fd, out, strlen(out));
		}
		else {
			format++;
			switch(format[0]) {
				case 'd':
				case 'i':
					val = va_arg(opt,int);
					int_char(val);
					printf_write(fd, mem_cad, strlen(mem_cad));
					break;

				case 'u':
					val = va_arg(opt, unsigned);
					uint_char(val);
					printf_write(fd, mem_cad, strlen(mem_cad));
					break;

				case 'x':
					val = va_arg(opt,int);
					hex_char((u32) val);
					printf_write(fd, mem_cad, strlen(mem_cad));
					break;

				case 's':
					s = va_arg(opt,char *);
					printf_write(fd, s, strlen(s));
					break;

			}
			format++;
		}

	}
   
	va_end(opt);

	FAT_CloseLog(fd);

	exit:
	os_message_queue_send(queuehandle2, 0, 0);
}

#endif 
