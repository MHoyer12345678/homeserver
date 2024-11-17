/*
 * ServerPMTool.h
 *
 *  Created on: 13.05.2024
 *      Author: joe
 */

#ifndef SRC_SERVERPMTOOL_H_
#define SRC_SERVERPMTOOL_H_

#include "GPIOController.h"

namespace server_pm_tool
{

class ServerPMTool
{

private:
	typedef enum
	{
		CMD_PRINT_HELP,
		CMD_PRINT_SRV_STATUS,
		CMD_SRV_POWER_ON,
		CMD_SRV_POWER_OFF,
		CMD_SRV_RESET
	} Command;

	GPIOController *gpioController;

	Command command;

	int returnCode;

	bool ParseCommand(const char *cmdString);

	void Usage();

	void CommandPrintStatus();

	void CommandPowerOn();

	void CommandPowerOff();

	void CommandReset();

	void PwrBtnShortPress();

	bool CheckPowerState(bool expectedState, long reCheckIntervalUS, int maxReChecksToFail);

	bool ReadPowerState();

public:
	ServerPMTool();

	virtual ~ServerPMTool();

	bool Init(int argc, char *argv[]);

	void DeInit();

	void ExecuteCommand();

	int GetReturnCode();
};

} //namespace server_pm_tool

#endif /* SRC_SERVERPMTOOL_H_ */
