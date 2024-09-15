// Line trace with pi camera


// includes for opencv stuff


// includes for the serial communication and other stuff
#include <iostream>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <chrono>
#include <thread>


using namespace std::chrono_literals;
using namespace std;






void line_trace(){
	
	
	
	
}




int main(){
	int fd;
	if ((fd = serialOpen ("/dev/ttyS0", 9600)) < 0)
	{
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiSetup () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
		return 1 ;
	}


	// wait until pico starts up and is ready
	std::cout << fd << std::endl;
	int numchars;
	char buf[100]; // just make it more than big enough and it will be fine
	this_thread::sleep_for(1000ms);
	while( (numchars = serialDataAvail(fd)) == 0){
		printf("Nothing to be read so far\n");
		this_thread::sleep_for(100ms);
	}
	printf("Data recieved\n");
	read(fd, buf, numchars);
	buf[numchars+1] = '\0';
	printf("Data read: %s\n", buf);
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
		
		leftmotor = 100;
		rightmotor = -100;
		sprintf(buf, "[%d, %d, %d, %d]", abs(leftmotor), leftmotor, abs(rightmotor), rightmotor);
		printf("buf.size() = %d\n", strlen(buf));
		write(fd, buf, strlen(buf));
		printf("Sent the following string: %s\n", buf);
		this_thread::sleep_for(1000ms);
	}

	cout << numchars<<endl;

	serialClose (fd) ;

	return 0;
}
