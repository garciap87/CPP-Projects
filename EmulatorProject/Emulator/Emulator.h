#ifndef EMULATOR_H
#define EMULATOR_H

#define NUM_PACKETS 1
#define BUFFER_SIZE 400

#include <string>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include "LogFile.h"

using namespace std;

// An unsigned char can store 1 Bytes (8bits) of data (0-255)
typedef unsigned char Byte;

class Emulator
{
public:
	Emulator();
	~Emulator();

	bool Initialize(char *TempTlm, char *TelemetryData);
	void run();
	void messageRouter(Byte pBuffer[], int arraySize);

	void periodicSpacecraft(Byte pBuffer[], int arraySize);
	void aperiodicSpacecraft(Byte pBuffer[], int arraySize);
	void updatePrimarySCEAddress(Byte pBuffer[], int arraySize);
	void externalSourceForward(Byte pBuffer[], int arraySize);
	void acknowledgment(Byte ByteArray[]);
	void periodicTelemetryRespone();
	void aperiodicTelemetryRespone();

	void write(Byte message[], int arraySize);
	long getFileSize(FILE *file);

	static BOOL CtrlHandler(DWORD fdwCtrlType);

	bool getRunning() { return running_; }
	void setRunning(bool set) { running_ = set; }
	bool getShuttingDown() { return shuttingDown_; }
	void setShuttingDown(bool set) { shuttingDown_ = set; }

private:
	bool running_;
	bool shuttingDown_;
	//Address for the SCE field in messages
	Byte sceAddress_;
	//Count for message I
	unsigned short packetCountI_;
	//Count for message h
	unsigned short packetCountH_;
	//count for message j
	unsigned short packetCountJ_;
	Byte startOfFileIndex_;
	int startOfPDU_;
	//Handle for test tlm file
	FILE *tlmFile_;
	FILE *telemetryFile_;
	FILE *configFile_;
	//Pointer to buffered data
	Byte *tlmFileBuf_;
	Byte *configFileBuf_;
	//file pointer to be used to calculate the size
	long tlmFileSize_;
	long configFileSize_;
	LogFile *logFile_;
};
#endif 