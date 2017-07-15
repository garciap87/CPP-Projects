#ifndef INCLUDE_GUARD_LOGFILE_H
#define INCLUDE_GUARD_LOGFILE_H

#include <string>
#include <stdio.h>
#include <time.h>
#include <iomanip>
#include <windows.h> 
#include <process.h>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

// An unsigned char can store 1 Bytes (8bits) of data (0-255)
typedef unsigned char Byte;

class LogFile
{
public:
	LogFile();
	~LogFile();

	string hexStr(Byte start, Byte *data, short len);
	void out(string message, string data);
	void messageOut(string message, string data);
	void writeLog();
	void writeMessages();
	string timestamp();
	string timestampForFilename();

private:
	stringstream outBuffer_;
	stringstream messageBuffer_;
};
#endif 