// Line trace with pi camera


// includes for opencv stuff


// includes for the serial communication and other stuff
#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <termios.h>                                                         
#include <stdio.h>
#include <stdlib.h>	                                                     
#include <fcntl.h>                                                                                                               
#include <sys/types.h> 
#include <stdint.h>
#include <sys/signal.h>
#include <time.h>
#include <stdbool.h>	


#define BAUDRATE B115200                                                      
#define MODEMDEVICE "/dev/ttyAMA0"


using namespace std::chrono_literals;
using namespace std;


int uart0_filestream = -1;
void init(){
	// connect to the arduino device
	uart0_filestream = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		exit(-1);
	}
	
	// config stuff
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}	



void line_trace(){
	
	
	
	
}




int main(){
	
	init();
	
	// should be all set by here
	printf("Serial is up and running\n");


	// wait until pico starts up and is ready
	this_thread::sleep_for(1000ms);
	if (uart0_filestream != -1)
	{
		// Read up to 255 characters from the port if they are there
		unsigned char rx_buffer[256];
		int rx_length = 0;
		
		printf("Waiting for pico to start...\n");
		
		while (rx_length <=0)  								//remove the while to make this non-blocking
			rx_length = read(uart0_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
		
		// data recieved, meaning the pico is ready to go
		rx_buffer[rx_length] = '\0';
		printf("Start data recieved: %s\n", rx_buffer);
		
		if (rx_length < 0)
		{
			//An error occured (will occur if there are no bytes)
		}
		else if (rx_length == 0)
		{
			//No data waiting - if we are non-blocking
		}
		else
		{
			//Bytes received
			rx_buffer[rx_length] = '\0';
			printf("%i bytes read : %s", rx_length, rx_buffer);
		}
	}

	this_thread::sleep_for(1000ms);

	int leftmotor, rightmotor;
	
	
	// pico is ready, start line trace
	
	
	
	
	// testing 
	while (true){
		// start command has been recieved
		// start line tracing and sending motor commands to the cytron board
		
		
		/*
		numchars = serialDataAvail(fd);
		if (numchars == 0) {
			cout << "nada "<< numchars<< endl;
			this_thread::sleep_for(10ms);
			continue;
		}
		else{
			cout << "numchars "<< numchars<< endl;
			read(fd,buf, numchars);
			buf[numchars+1] = '\0';
			cout << buf << endl;	
			write(fd,buf, numchars+1);
			buf[0] = 'w';
			this_thread::sleep_for(10ms);
		}
		*/
		
		leftmotor = 42;
		rightmotor = -42;
		char tx_buffer[30];
		sprintf(tx_buffer, "[%d, %d]",leftmotor, rightmotor);
		printf("buffer data = %s\n", tx_buffer);
		if (uart0_filestream != -1)
		{
			int count  = 0;
			count = write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));		//Filestream, bytes to write, number of bytes to write
			if (count < 0)
			{
				printf("UART TX error\n");
			}else{
				printf("UART TX ok\n");
				printf("count: %d\n", count);
			}
		}
	
		printf("Sent the following string: %s\n", tx_buffer);
		this_thread::sleep_for(1000ms);
	
	
	
	}// end while(1)


	

	return 0;
}
