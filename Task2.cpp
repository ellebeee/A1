//C++ CLASS TO WRAP THE FUNCTIONALITY OF THE ADXL345 CODE
//#includes required for various standard function calls
#include<iostream>
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<unistd.h>
#include <stdlib.h>
#include<math.h>


using namespace std;

// Small macro to display value in hexadecimal with 2 places
#define DEVID       0x00
#define BUFFER_SIZE 40

//a list of the relevant register addresses 
#define OSFX 0x1E
#define OSFY 0x1F
#define OSFZ 0x20
#define BW_RATE 0x2C
#define POWER_CTL 0x2D
#define DATA_FORMAT 0x31
#define DATAX0 0x32
#define DATAX1 0x33
#define DATAY0 0x34
#define DATAY1 0x35
#define DATAZ0 0x36
#define DATAZ1 0x37
#define FIFO_CTL 0x38
#define FIFO_STATUS 0x39

class sensor{
   private:
	

   public:
	int file;
	void automaticActivation(int file);
	//incomplete
};





void sensor::automaticActivation(int file){
	//incomplete
}



int main(){
   int file;
   printf("Starting the ADXL345 test application\n");
   if((file=open("/dev/i2c-1", O_RDWR)) < 0){
      perror("failed to open the bus\n");
      return 1;
   }
        if(ioctl(file, I2C_SLAVE, 0x53) < 0){ //if there's nothing on i2c bus at 0x53
      		perror("Failed to connect to the sensor\n");
      		return 1;
   	}
   	char writeBuffer[1] = {0x00}; //place an address here
   	if(write(file, writeBuffer, 1)!=1){	//if it does not match, the write failed
      		perror("Failed to reset the read address\n");
      		return 1;
   	}
   	char readBuffer[BUFFER_SIZE];
   	if(read(file, readBuffer, BUFFER_SIZE)!=BUFFER_SIZE){
      		perror("Failed to read in the buffer\n");
      		return 1;
   	}
   	printf("The Device ID is: 0x%02x\n", readBuffer[DEVID]); //read current device ID and display in Linux terminal prompt	

	
	void automaticActivation(int file);
   
//____________________________________________________________________
	   //1. DS P14... start device in standby mode and enter measurement mode when required
	   //POWER_CTRL reg 0x2D...set measure bit D3 to enable a measurement to be taken
	   //P26 DS it starts up standby mode automatically, so I put it to measure mode here
	   char config[2] = {0}; 	//create empty config array of size 2 chars
	   config[0] = 0x2D; 		//select power ctrl reg, this is its address
	   config[1] = 0x08; 		//assert the D3 bit, measure bit....00000000 - 00001000 
	   write(file, config, 2);	//write the new hex value to the address specified 
	
	   //select bandwidth rate register and configure it
	   //normal mode - output data rate = 100Hz..default is 100 Hz, P25 of data sheet
	   config[0] = 0x2C; //select register..address of BW_RATE reg
	   config[1] = 0x0A; //value to write in the register...00001010 ...specifies 100Hz data rate and normal mode
	   write(file, config, 2); //write it to file
	
	   //2. SPECIFY THE DATA FORMAT
	   //data format register will change depending on the range and resolution
	   config[0] = 0x31; //select data format...DATA_FORMAT reg address
	
//_____________________________________________________________________
//SPECIFY HIGH RESOLUTION OR NOT - Res = 1 for high resolution, Res = 0 for normal resolution
//SPECIFY RANGE OF PLUS/MINUS 2,4,8,16 g.....	Range = 4 for plus minus 2g
//						Range = 8 for plus minus 4g
//						Range = 16 for plus minus 8g
//						Range = 32 for plus minus 16g
	bool Res = 1;
	int range = 4; //+-2g range	   
	if (Res = 1){ //full resolution chosen
		switch(range){
			case 4: config[1] = 0x0C; break;
			case 8: config[1] = 0x0D; break;
			case 16: config[1] = 0x0E; break;
			case 32: config[1] = 0x0F; break;
			default: config[1] = 0x0C; break;
		}
	}
	else{
		switch(range){
			case 4: config[1] = 0x04; break;
			case 8: config[1] = 0x05; break;
			case 16: config[1] = 0x06; break;
			case 32: config[1] = 0x07; break;
			default: config[1] = 0x04; break;
		}
	}
	write(file, config, 2);	//write appropriate combination of bits to the data format reg based on choice of range and resolution
//____________________________________________________________________
//intermediary vriables to hold previous x y and z values 
	int xAccPrev = 0;
	int yAccPrev = 0;
	int zAccPrev = 0;
	
	//simple counter to update acceleration value output when a change happens
	int i = 0;
	while(i < 50){
	   char reg[1] = {0x32}; 		//starting read at address 0x32
	   write(file,reg,1);
	   char dataReg[6] = {0};		//create an array called dataReg of chars to hold 6 bytes so multi-byte read can be carried out
	   if(read(file,dataReg,6)!=6)	//check is there all 6 bytes present to do a multi-byte read as advised by data sheet
	   {
		printf("Error: not enough outputs \n");	//print error if not
		exit(1); 				//exit statement, with non-zero value to signify an error occurred
	   }	
	   else
	   {
		//convert data in the reg to 10 bits
		int xAcc = ((dataReg[1] & 0x03) * 256 + (dataReg[0] & 0xFF));	//dataReg 1 is X1 which is the MSB, *256
		if(xAcc > 511)	//if it exceeds 2 bytes
		{
			xAcc -= 1024; 	//rollover
		}
		int xChange = xAcc - xAccPrev;
		int yAcc = ((dataReg[3] & 0x03) * 256 + (dataReg[2] & 0xFF));
		if(yAcc > 511)
		{
			yAcc -= 1024;
		}
		int yChange = yAcc - yAccPrev;
		int zAcc = ((dataReg[5] & 0x03) * 256 +(dataReg[4] & 0xFF));
		if(zAcc > 511)
		{
			zAcc -= 1024;
		}
		int zChange = zAcc - zAccPrev;
		if(xChange >=2 || yChange >=2 || zChange >=2){	//print new values if there's been a change greater than or equal to 2 across all axes
			//print data to screen
			printf("Acceleration on x-axis: %d \n", xAcc);
			printf("Acceleration on y-axis: %d \n", yAcc);
			printf("Acceleration on z-axis: %d \n", zAcc);
			xAccPrev = xAcc; //update previous values 
			yAccPrev = yAcc;
			zAccPrev = zAcc;

			//calculate pitch and roll
			//float roll = 180*atan(-xAcc/zAcc)/M_PI;
			float pitch = 180*atan(xAcc/sqrt((yAcc*yAcc)+(zAcc*zAcc)))/M_PI;
			float roll = 180*atan(yAcc/sqrt((xAcc*xAcc)+(zAcc*zAcc)))/M_PI;
			printf("Pitch: %d \n", pitch);
			printf("Roll: %d \n", roll);

			i++; //increment count
		}
   	   }

	}
//_______________________________________________________________________
//			TAP AND  DOUBLE TAP DETECTION
//_______________________________________________________________________
//register configurations required....data sheet advises that the interrupts be mapped first before being enabled , to avoid triggered any accidentally

//SET THRESH_TAP REG...16g as axis accel values are very changeable 
config[0] = 0x1D;	//address of thresh tap reg 		
config[1] = 0xFF;	//16g 		
write(file, config, 2);	//write the value

//SET DUR REG...30ms max for a tap motion 
config[0] = 0x21;	//address of dur reg 		
config[1] = 0x30;	//30ms 		
write(file, config, 2);	//write the value

//SET LATENT REG...80ms to pass before determining that it's a second tap
config[0] = 0x22;	//address of latent reg 		
config[1] = 0x50;	//80ms 		
write(file, config, 2);	//write the value

//SET WINDOW REG...300ms window within which second tap must occur
config[0] = 0x23;	//address of window reg 		
config[1] = 0xF0;	//300ms 		
write(file, config, 2);	//write the value

//SET TAP_AXES REG...enable x y and z axes participation in seeking tap...do not suppress double tap
config[0] = 0x2A;	//address of data_axes reg 		
config[1] = 0x07;	//300ms 		
write(file, config, 2);	//write the value

//SET INT_ENABLE REG...enable single and double tap interrupts
config[0] = 0x2E;	//address of int enable reg 		
config[1] = 0x60;	 		
write(file, config, 2);	//write the value

//SET INT_MAP REG...send single tap to int1 and double tap to int2
config[0] = 0x2F;	//address of int map reg 		
config[1] = 0x27;			
write(file, config, 2);	//write the value

//read from int_source reg at address 0x30 and see if a single or double tap occurred

}