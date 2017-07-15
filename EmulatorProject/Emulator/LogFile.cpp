#include "LogFile.h"

using namespace std;

LogFile::LogFile()
{

}

LogFile::~LogFile()
{

}

string LogFile::hexStr(Byte start, Byte *data, short len)
{
	stringstream ss;

	for (short i = start; i < len; ++i)
	{
		ss << std::setfill('0') << std::setw(2) << std::hex << (int)data[i] << " ";
	}

	return ss.str();
}

void LogFile::out(string message, string data)
{
	// Write to log buffer
	outBuffer_ << timestamp() << " " << message << " " << data << endl;

	// Write to std out, in case someone watching in real time
	cout << timestamp() << " " << message << " " << data << endl << endl;
}

void LogFile::messageOut(string message, string data)
{
	// Write to message log buffer
	messageBuffer_ << timestamp() << " " << message << " " << data << endl;
}

void LogFile::writeMessages()
{
	string fileLocation = "C:\\SpaceWire_Logs\\OutGoingMessages\\";
	string fileName = "Emulator_Outgoing_Messages_" + timestampForFilename() + ".log";
	try
	{
		ofstream messages;
		messages.open(fileLocation + fileName);
		if (messages.is_open())
		{
			messages << messageBuffer_.str();
			messages.close();
		}
		else
		{
			cout << "Could not open Log File::" << fileName << endl;
		}
	}
	catch (...)
	{
		cout << "Could not open Log File::" << fileName << endl;
	}
}

void LogFile::writeLog()
{
	string fileLocation = "C:\\SpaceWire_Logs\\IncomingMessages\\";
	string fileName = "Emulator_Incoming_Messages_" + timestampForFilename() + ".log";
	try
	{
		ofstream log;
		log.open(fileLocation + fileName);
		if (log.is_open())
		{
			log << outBuffer_.str();
			log.close();
		}
		else
		{
			cout << " Could not open Log File::" << fileName << endl;
		}
	}
	catch (...)
	{
		cout << " Could not open Log File::" << fileName << endl;
	}
}

string LogFile::timestamp()
{
	ostringstream stream;
	time_t rawtime;
	tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	if (timeinfo != NULL)
	{
		stream << (timeinfo->tm_year) + 1900 << "-" << timeinfo->tm_mon + 1
			<< "-" << timeinfo->tm_mday << " " << timeinfo->tm_hour
			<< ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec;
	}
	else
	{
		cout << "timeinfo is NULL" << endl;
	}

	return stream.str();
}

string LogFile::timestampForFilename()
{
	ostringstream stream;
	time_t rawtime;
	tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	if (timeinfo != NULL)
	{
		stream << (timeinfo->tm_year) + 1900 << "_"
			<< timeinfo->tm_mon + 1 << "_" << timeinfo->tm_mday << "-" << timeinfo->tm_hour
			<< timeinfo->tm_min;
	}
	else
	{
		cout << "timeinfo is NULL" << endl;
	}

	return stream.str();
}
