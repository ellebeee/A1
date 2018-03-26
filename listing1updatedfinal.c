//author: Lisa Byrne
//listing1updated.c is a c code which was extended from the listing1.c code provided by Derek Molloy in the assignment brief. listing1updated.c 
//- reads current device ID and displays it in the Linux terminal prompt
//- writes to the device to activate it to measure automatically
//- displays the current x, y, and z axes values as a single numeric value
//code is commented. 'DS' referenced through the comments is for 'data sheet'

//#includes remain the same from listing1.c
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<unistd.h>

//included to use exit function below
#include <stdlib.h>

//following code section kept from listing1.c by Derek Molloy
//________________________________________________________________________________________________________
// Small macro to display value in hexadecimal with 2 places
#define DEVID       0x00
#define BUFFER_SIZE 40

int main(){
   //create I2C bus
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
//________________________________________________________________________________________________________

   //added in from ref 3
   //get I2C device, ADXL345 I2C address is 0x53(83)
   //ioctl(file, I2C_SLAVE, 0x53);

   //TASK: WRITE TO DEVICE TO ACTIVATE IT AUTOMATICALLY

   // 3 steps to initialise ADXL345
   //1. enabling measurement mode in register POWER_CTRL
   //2. specifying the data format
   //3. setting offset registers - OFSX, OSFY, OSFZ

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
   //no self test, 4 wire spi, active high interrupts, full res, right-justified, +-16g range
   config[0] = 0x31; //select data format...DATA_FORMAT reg address
   config[1] = 0x0B; //specified by the assertions in the data format reg..00001011
   write(file, config, 2);
   
   //3. set offset registers...P30 DS recommends doing this after system assembly
   
   //Read from data regs..2s comp data stored in 8 bits
   //DATAX0...lsb
   //DATAX1...msb....and same for Y and Z
   
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
	int yAcc = ((dataReg[3] & 0x03) * 256 + (dataReg[2] & 0xFF));
	if(yAcc > 511)
	{
		yAcc -= 1024;
	}
	int zAcc = ((dataReg[5] & 0x03) * 256 +(dataReg[4] & 0xFF));
	if(zAcc > 511)
	{
		zAcc -= 1024;
	}

	//print data to screen
	printf("Acceleration on x-axis: %d \n", xAcc);
	printf("Acceleration on y-axis: %d \n", yAcc);
	printf("Acceleration on z-axis: %d \n", zAcc);
   }
}
   



