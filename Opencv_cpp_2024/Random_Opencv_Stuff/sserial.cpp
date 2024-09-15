//   build 
//   gcc -Wall -o sserial sserial.cpp -lwiringPi

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
char buf[12];
this_thread::sleep_for(1000ms);

while (true){
	numchars = serialDataAvail(fd);
	if (numchars < 6) {
		cout << "nada "<< numchars<< endl;
		this_thread::sleep_for(1000ms);
		continue;
	}
	cout << "numchars "<< numchars<< endl;
	read(fd,buf, 6);
	buf[7] = '\0';
	cout << buf << endl;	
	write(fd,buf,6);
	buf[0] = 'w';
	this_thread::sleep_for(100ms);
	
}

cout << numchars<<endl;

serialClose (fd) ;

return 0;
}
