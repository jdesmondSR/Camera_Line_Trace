//   basic serial, sending to cytron board and receiving start and stop commands

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

	std::cout << fd << std::endl;
	int numchars;
	char buf[100]; // just make it more than big enough and it will be fine
	this_thread::sleep_for(1000ms);

	while (true){
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
	}

	cout << numchars<<endl;

	serialClose (fd) ;

	return 0;
}
