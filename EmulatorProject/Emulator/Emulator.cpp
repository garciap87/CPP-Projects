#include "Emulator.h"
#include "LogFile.h"

static Emulator* emulator_;

Emulator::Emulator()
{
	emulator_ = this;
	logFile_ = new LogFile();
	tlmFile_ = NULL;
	packetCountI_ = 0x0000;
	packetCountH_ = 0x0000;
	packetCountJ_ = 0x0000;
	startOfFileIndex_ = 0;
	startOfPDU_ = 0;
	running_ = false;
	shuttingDown_ = false;
}

Emulator::~Emulator()
{
	delete[]tlmFileBuf_;
	fclose(tlmFile_);

	if (logFile_ != NULL)
	{
		delete logFile_;
		logFile_ = NULL;
	}
}

bool Emulator::Initialize(char *TempTlm, char *TelemetryData)
{
	// Open Test TLM file in binary mode using the "rb" format string. This also checks if the file exists and/or can be opened for reading correctly 
	if ((tlmFile_ = fopen(TempTlm, "rb")) == NULL)
	{
		cout << "Could not open Test TLM file\n";
		return false;
	}
	else
	{
		cout << "Test TLM File opened successfully!!\n";
	}

	// Get the size of the file in Bytes  
	tlmFileSize_ = getFileSize(tlmFile_);

	// Allocate space in the buffer for the whole file 
	tlmFileBuf_ = new Byte[tlmFileSize_];

	// Read the file in to the buffer  
	fread(tlmFileBuf_, tlmFileSize_, 1, tlmFile_);

	// ..............................Open aperiodic file...................................
	if ((configFile_ = fopen(TelemetryData, "rb")) == NULL)
	{
		cout << "Could not open Telemetry Data file\n";
		return false;
	}
	else
	{
		cout << "Telemetry Data File opened successfully!!\n";
	}
	// Get the size of the file in Bytes  
	configFileSize_ = getFileSize(configFile_);

	// Allocate space in the buffer for the whole file 
	configFileBuf_ = new Byte[configFileSize_];

	// Read the file into the buffer  
	fread(configFileBuf_, configFileSize_, 1, configFile_);
	//..................................END...................................................

	cout << "Router registered to receive on all ports!\n\n";

	return true;
}

void Emulator::run()
{
	bool result;
	bool USBSpaceWire_ReadPackets;
	bool USBSpaceWire_WaitOnReadPacketAvailable;
	bool TRANSFER_SUCCESS = true;

	Byte pBuffer[BUFFER_SIZE];

	cout << ("Waiting for incoming messages.....\n\n");

	while (!getShuttingDown())
	{
		// Check if there are any packets available
		if (USBSpaceWire_WaitOnReadPacketAvailable)
		{
			result = USBSpaceWire_ReadPackets;

			// Check the result for success
			if (result != TRANSFER_SUCCESS)
			{
				cout << ("Error: Could not receive the packet on SCU1\n");
			}
			else
			{
				Byte arraySize = 20;

				messageRouter(pBuffer, arraySize);
			}
		}
	}
}

void Emulator::messageRouter(Byte pBuffer[], int arraySize)
{
	//Variables from the CCSDS Space Packet Protocol. Field names are packet version number, packet type, secondary header flag, and APID
	Byte byteFour = pBuffer[4];
	Byte byteFive = pBuffer[5];

	if (byteFour == 0x1b && byteFive == 0xd9)
	{
		//Used to determine what type of message received, D through G. Gets the packet type out of the message which are bits 80-87 or Byte 10.
		Byte messageType = pBuffer[10];

		switch (messageType)
		{
			//Letter B in ICD
		case 0x01:
			logFile_->out("(Time_Message)", "Emulator doesn't handle this message");
			break;

			//Letter C in ICD
		case 0x02:
			logFile_->out("(ADACS_Message)", "Emulator doesn't handle this message");
			break;

			//Letter D in ICD
		case 0x04:
			cout << "Periodic Spacecraft Telemetry Status Data Request message received\n";
			periodicSpacecraft(pBuffer, arraySize);
			break;

			//Letter E in ICD
		case 0x06:
			cout << "Aperiodic Spacecraft Telemetry Status Data Request message received\n";
			aperiodicSpacecraft(pBuffer, arraySize);
			break;

			//Letter F in ICD
		case 0x08:
			cout << "Update Primary SCE Address message received\n";
			updatePrimarySCEAddress(pBuffer, arraySize);
			break;

		default:
			cout << "No matching messages found!!!\n\n";
			break;
		}
	}

	else
	{
		//Letter G in ICD
		cout << "External Source Forward message received\n";
		externalSourceForward(pBuffer, arraySize);
	}
}

//Letter D in ICD
void Emulator::periodicSpacecraft(Byte pBuffer[], int messageSize)
{
	if (messageSize == 14)
	{
		Byte ByteOne = pBuffer[0];

		if (ByteOne == 0x39)
		{
			cout << "Target logical address is SCU1\n";
		}
		else if (ByteOne == 0x59)
		{
			cout << "Target logical address is SCU2\n";
		}
		else
		{
			cout << "Can not determine Logical Address\n";
			return;
		}

		string data = logFile_->hexStr(0, pBuffer, messageSize);

		logFile_->out("(Periodic_Data_Request)", data);

		periodicTelemetryRespone();
	}
	else
	{
		cout << "Incoming Periodic Spacecraft message is corrupt\n\n";
		return;
	}
}

//Letter E in ICD
void Emulator::aperiodicSpacecraft(Byte pBuffer[], int messageSize)
{
	if (messageSize == 22)
	{
		Byte ByteOne = pBuffer[0];

		if (ByteOne == 0x39)
		{
			cout << "Target logical address is SCU1\n";
		}
		else if (ByteOne == 0x59)
		{
			cout << "Target logical address is SCU2\n";
		}
		else
		{
			cout << "Can not determine Logical Address\n";
			return;
		}

		string data = logFile_->hexStr(0, pBuffer, messageSize);

		logFile_->out("(Aperiodic_Request)", data);

		aperiodicTelemetryRespone();
	}

	else
	{
		cout << "Incoming Aperiodic Spacecraft message is corrupt\n";
		return;
	}
}

//Letter F in ICD
void Emulator::updatePrimarySCEAddress(Byte pBuffer[], int messageSize)
{
	if (messageSize == 15)
	{
		//Set the SCE field used by the other messages
		sceAddress_ = pBuffer[12];

		Byte ByteOne = pBuffer[0];

		if (ByteOne == 0x39)
		{
			cout << "Target logical address is SCU1\n";
		}
		else if (ByteOne == 0x59)
		{
			cout << "Target logical address is SCU2\n";
		}
		else
		{
			cout << "Can not determine Logical Address\n";
			return;
		}

		string data = logFile_->hexStr(0, pBuffer, messageSize);

		logFile_->out("(Update_SCE_Message)", data);

		acknowledgment(pBuffer);
	}

	else
	{
		cout << "Incoming Update Primary SCE Address message is corrupt\n";
		return;
	}
}

//Letter G in ICD
void Emulator::externalSourceForward(Byte pBuffer[], int messageSize)
{
	if (messageSize >= 10)
	{
		Byte ByteOne = pBuffer[0];

		if (ByteOne == 0x39)
		{
			cout << "Target logical address is SCU1\n";
		}
		else if (ByteOne == 0x59)
		{
			cout << "Target logical address is SCU2\n";
		}
		else
		{
			cout << "Can not determine Logical Address\n";
			return;
		}

		string data = logFile_->hexStr(10, pBuffer, messageSize);

		logFile_->out("(External_Forward_Message)", data);

		//Goes to CFDP Engine
	}

	else
	{
		cout << "Incoming External Source Forward message is corrupt\n";
		return;
	}
}

//Letter H in ICD
void Emulator::acknowledgment(Byte pBuffer[])
{
	Byte messageH[26];

	if (sceAddress_ == 0x3e)
	{
		messageH[0] = 0x3e;
	}
	else if (sceAddress_ == 0x5e)
	{
		messageH[0] = 0x5e;
	}
	else
	{
		cout << "SCE Address could not be set with proper values\n";
		return;
	}

	messageH[1] = 0x02;
	messageH[2] = 0x00;
	messageH[3] = 0x00;
	messageH[4] = 0x0b;
	messageH[5] = 0x22;

	if (packetCountH_ > 0x3fff)
	{
		packetCountH_ = 0x0000;
	}

	unsigned short tempShort = 0xC000 | (packetCountH_ & 0x3FFF);
	messageH[6] = ((tempShort >> 8) & 0xFF);
	messageH[7] = ((tempShort) & 0xFF);
	packetCountH_++;

	messageH[8] = 0x00;
	messageH[9] = 0x0f;

	// get current time
	time_t timer;
	struct tm y2k = { 0 };

	y2k.tm_hour = 12;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

	//Convert to J2000 time
	time(&timer);
	double time = difftime(timer, mktime(&y2k));
	int seconds = difftime(timer, mktime(&y2k));

	messageH[10] = (seconds >> 24) & 0xff;
	messageH[11] = (seconds >> 16) & 0xFF;
	messageH[12] = (seconds >> 8) & 0xFF;
	messageH[13] = seconds & 0xFF;

	int smallSecond = seconds / 65536;
	messageH[14] = (smallSecond >> 8) & 0xff;
	messageH[15] = smallSecond & 0xFF;

	//Dont care field
	for (Byte m = 16; m < 20; m++)
	{
		messageH[m] = 0x00;
	}

	messageH[20] = 0x08;

	//Sequence count of command packet being acknowledged.
	unsigned short sequenceAcknowledgedPacket = (((pBuffer[6] << 8) | pBuffer[7])) & 0x3FFF;
	sequenceAcknowledgedPacket = ((sequenceAcknowledgedPacket << 2) & 0xFFFC);
	messageH[21] = ((sequenceAcknowledgedPacket >> 8) & 0xFF);
	messageH[22] = ((sequenceAcknowledgedPacket) & 0xFF);
	messageH[23] = 0x00;

	cout << "Created Acknowledgment Message\n";

	string data = logFile_->hexStr(0, messageH, 26);

	logFile_->messageOut("(Acknowledgment)", data);

	write(messageH, 26);
}

//Letter I in ICD
void Emulator::periodicTelemetryRespone()
{
	Byte messageI[39];

	if (sceAddress_ == 0x3e)
	{
		messageI[0] = 0x3e;
	}
	else if (sceAddress_ == 0x5e)
	{
		messageI[0] = 0x5e;
	}
	else
	{
		cout << "ERROR!!: SCE Address could not be set for Periodic Telemetry Response!\n\n";
		return;
	}

	messageI[1] = 0x02;
	messageI[2] = 0x00;
	messageI[3] = 0xff;
	messageI[4] = 0x1b;
	messageI[5] = 0x3e;

	if (packetCountI_ > 0x3fff)
	{
		packetCountI_ = 0x0000;
	}

	unsigned short tempShort = 0xC000 | (packetCountI_ & 0x3FFF);
	messageI[6] = ((tempShort >> 8) & 0xFF);
	messageI[7] = ((tempShort) & 0xFF);
	packetCountI_++;

	messageI[8] = 0x00;
	messageI[9] = 0x1c;
	messageI[10] = 0x05;
	messageI[11] = 0x00;

	if ((tlmFileSize_ - startOfFileIndex_) <  25)
	{
		startOfFileIndex_ = 0;
	}

	//25 Bytes per periodic request
	for (Byte j = 12; j< 37; j++)
	{
		messageI[j] = tlmFileBuf_[startOfFileIndex_];
		startOfFileIndex_++;
	}

	cout << "Created Periodic Telemetry Response Message\n";

	string data = logFile_->hexStr(0, messageI, 39);

	logFile_->messageOut("(Periodic_Response)", data);

	write(messageI, 39);
}

//Letter J in ICD
void Emulator::aperiodicTelemetryRespone()
{
	Byte messageJ[254];

	if (sceAddress_ == 0x3e)
	{
		messageJ[0] = 0x3e;
	}
	else if (sceAddress_ == 0x5e)
	{
		messageJ[0] = 0x5e;
	}
	else
	{
		cout << "SCE Address could not be set with proper values\n";
		return;
	}

	messageJ[1] = 0x02;
	messageJ[2] = 0x00;
	messageJ[3] = 0xff;
	messageJ[4] = 0x1b;
	messageJ[5] = 0x3e;

	if (packetCountJ_ > 0x3fff)
	{
		packetCountJ_ = 0x0000;
	}

	unsigned short tempShort = 0xC000 | (packetCountJ_ & 0x3FFF);
	messageJ[6] = ((tempShort >> 8) & 0xFF);
	messageJ[7] = ((tempShort) & 0xFF);
	packetCountJ_++;

	messageJ[8] = 0x00;
	messageJ[9] = 0xF3;
	messageJ[10] = 0x07;
	messageJ[11] = 0x00;

	//Enter PDU data here from CFDP engine

	if ((configFileSize_ - startOfPDU_) <  240)
	{
		startOfPDU_ = 0;
	}

	//Loop to read from made up file. Only for testing
	for (short j = 12; j<252; j++)
	{
		messageJ[j] = configFileBuf_[startOfPDU_];
		startOfPDU_++;
	}

	cout << "Created Aperiodic Telemetry Response Message\n";

	string data = logFile_->hexStr(0, messageJ, 254);

	logFile_->messageOut("(Aperiodic_Response)", data);

	write(messageJ, 254);
}

void Emulator::write(Byte message[], int arraySize)
{
	int address;
	string result;
	string TRANSFER_SUCCESS;

	Byte byteOne = message[0];

	if (byteOne == 0x3e)
	{
		//Address for port sending message to
		address = 1;
	}
	else if (byteOne == 0x5e)
	{
		//Address for port sending message to
		address = 3;
	}
	else
	{
		cout << "Can not determine Logical Address\n";
		return;
	}

	if (result == TRANSFER_SUCCESS)
	{
		cout << ("Transmit of outgoing packet successful.\n\n");
	}

	else
	{
		cout << ("Error: Sending packet!\n\n");
	}

	logFile_->writeLog();

	logFile_->writeMessages();
}

BOOL Emulator::CtrlHandler(DWORD fdwCtrlType)
{
	if (emulator_->getRunning())
	{
		emulator_->setShuttingDown(true);

		// Sleep until Terminate completes and application can quit.
		while (emulator_->getRunning())
		{
			Sleep(1);
		}
	}

	return true;
}

long Emulator::getFileSize(FILE *file)
{
	long lCurPos, lEndPos;
	lCurPos = ftell(file);
	fseek(file, 0, 2);
	lEndPos = ftell(file);
	fseek(file, lCurPos, 0);
	return lEndPos;
}