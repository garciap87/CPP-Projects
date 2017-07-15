#include "Emulator.h"

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Test TLM file or Routing Table not able to be read in on start up.\n";
		return -1;
	}

	// Set the Handler to capture control-c, shutdown, etc, events
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)Emulator::CtrlHandler, TRUE))
	{
		cout << ("The Control Handler is installed:\n");
	}
	else
	{
		cout << ("Uanble to create ConsoleCtrlHanler:\n");
		return -1;
	}

	char *TempTlm = argv[1];
	char *TelemetryData = argv[2];

	Emulator* emulator = new Emulator();

	if (!emulator->Initialize(TempTlm, TelemetryData))
	{
		cout << "Initialize failed, program ending.\n";

		delete emulator;
		return -1;
	}

	emulator->setRunning(true);

	emulator->run();

	emulator->setRunning(false);

	delete emulator;
	return 0;
}