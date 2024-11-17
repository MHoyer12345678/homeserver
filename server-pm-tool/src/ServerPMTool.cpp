/*
 * ServerPMTool.cpp
 *
 *  Created on: 13.05.2024
 *      Author: joe
 */

#include "ServerPMTool.h"
#include <string.h>
#include <iostream>
#include <cassert>
#include <unistd.h>

/** -------------------  parameters for power on sequence -----------------------------
Sequence to power on server:
- (shortly) press power btn
- wait PWR_ON_WAIT_TIME_INTV_TO_CHK_US until checking pwr state
- repeat up PWR_ON_MAX_RETRY_CHECK_PWR_STATE time: check pwr state
- if pwr on -> sucess
- repeat up PWR_ON_MAX_RETRY_BTN_PRESS times whole sequence
- fail if still no power on signal 
**/
//tools trys up to this value times pressing the power button to power up server
#define PWR_ON_MAX_RETRY_BTN_PRESS          5
//waits for this time until checking the power status after pressing the power btn
#define PWR_ON_WAIT_TIME_INTV_TO_CHK_US     100000L //100ms
//checks up to this value times for power up signal until giving up
#define PWR_ON_MAX_RETRY_CHECK_PWR_STATE    10      //10x100ms -> 1s

/** -------------------  parameters for power off sequence -----------------------------
Sequence to power off server:
- press power btn and hold
- wait PWR_OFF_WAIT_TIME_INTV_TO_CHK_US until checking pwr state
- repeat up PWR_OFF_MAX_RETRY_CHECK_PWR_STATE time: check pwr state
- if pwr off -> sucess 
- release pwr btn
- fail if still power on signal 
**/
//waits for this time until checking the power status after pressing the power btn
#define PWR_OFF_WAIT_TIME_INTV_TO_CHK_US     100000L //100ms
//checks up to this value times for power up signal until giving up
#define PWR_OFF_MAX_RETRY_CHECK_PWR_STATE    50      //50x100ms -> 5s

using namespace server_pm_tool;
using namespace std;

ServerPMTool::ServerPMTool() :
    returnCode(0),
    command(CMD_PRINT_HELP)
{
    this->gpioController=new GPIOController();
}

ServerPMTool::~ServerPMTool() 
{
    delete this->gpioController;
}

bool ServerPMTool::Init(int argc, char *argv[])
{
    if (argc==1)
        this->command=CMD_PRINT_SRV_STATUS;
    else if (!this->ParseCommand(argv[1]))
        this->returnCode=1;

    if (this->command==CMD_PRINT_HELP)
    {
        this->Usage();
        return false;
    }

    if (this->returnCode==0)
    {
        if (!this->gpioController->Init())
            this->returnCode=1;
    }
    
    return this->returnCode==0;
}

void ServerPMTool::DeInit()
{
    this->gpioController->DeInit();
}

bool ServerPMTool::ParseCommand(const char *cmdString)
{
    if (strcmp(cmdString,"help")==0 || strcmp(cmdString,"-h")==0 ||
        strcmp(cmdString,"--help")==0 || strcmp(cmdString,"-?")==0)
        this->command=CMD_PRINT_HELP;
    else if (strcmp(cmdString,"poweron")==0)
        this->command=CMD_SRV_POWER_ON;
    else if (strcmp(cmdString,"poweroff")==0)
        this->command=CMD_SRV_POWER_OFF;
    else if (strcmp(cmdString,"reset")==0)
        this->command=CMD_SRV_RESET;
    else
    {
        cout << "Unknown command: " << cmdString << endl << endl;
        return false;
    }

    return true;
}

void ServerPMTool::Usage()
{
    cout << "usage: server-pm-tool [--help|-h|-?] [<command>]" << endl;
    cout << "Powers server on or off or resets server as specified by <command>. The servers "
        "power status is printed when calling without a command given." << endl << endl;
    cout << "Commands:" << endl;
    cout << "     help - prints this usage screen" << endl;
    cout << "     poweron - powers server on" << endl;
    cout << "     poweroff - powers server off" << endl;
    cout << "     reset - resets server" << endl << endl;
    cout << "Options:" << endl;
    cout << "     -?, -h, --help - prints this usage screen" << endl << endl; 
}

void ServerPMTool::ExecuteCommand()
{
    switch(this->command)
    {
        case CMD_PRINT_HELP:
            assert(1==0 && "Internal error.");
            break;
        case CMD_PRINT_SRV_STATUS:
            this->CommandPrintStatus();
            break;
        case CMD_SRV_POWER_ON:
            this->CommandPowerOn();
            break;
        case CMD_SRV_POWER_OFF:
            this->CommandPowerOff();
            break;
        case CMD_SRV_RESET:
            this->CommandReset();
            break;
        default:
            assert(1==0 && "Internal error.");
    }
}

void ServerPMTool::CommandPrintStatus()
{
    bool state;
    state=this->ReadPowerState();

    if (this->returnCode==0)
    {
        if (state)
            std::cout << "Server is powered on." << endl;
        else
            std::cout << "Server is powered off." << endl;
    }
}

void ServerPMTool::CommandPowerOn()
{
    bool pwrState;
    int tryBtnPressCntr=0;

    std::cout << "Powering server on ..." << endl;
    pwrState=this->ReadPowerState();
    if (this->returnCode!=0) return;

    if (pwrState)
    {
        std::cout << "Server already powered on." << endl;
        return;
    }

    while (tryBtnPressCntr<PWR_ON_MAX_RETRY_BTN_PRESS)
    {
        tryBtnPressCntr++;

        //press power btn
        std::cout << "Pressing power button (try: " << tryBtnPressCntr << " of "
            << PWR_ON_MAX_RETRY_BTN_PRESS << ")" << endl;
        this->PwrBtnShortPress();
        if (this->returnCode!=0) return;

        //check for power state
        std::cout << "Waiting for server power on signal"<<endl;
        pwrState=this->CheckPowerState(1, PWR_ON_WAIT_TIME_INTV_TO_CHK_US, PWR_ON_MAX_RETRY_CHECK_PWR_STATE);
        if (this->returnCode !=0) return;

        if (pwrState)
        {
            std::cout << "Server power on signal received. Server powered on successfully."<<endl;
            return;
        }

        std::cout << "Did not receive power on signal in time.";
        if (tryBtnPressCntr<PWR_ON_MAX_RETRY_BTN_PRESS) 
            std:cout << " Trying to press power button again.";
        std::cout << endl;
    }
    std::cout << "Maximum retries exceeded. Failed to power up server."<<endl;
    this->returnCode=1;
}

void ServerPMTool::CommandPowerOff()
{
    bool pwrState;
    int tryBtnPressCntr=0;

    std::cout << "Powering server off ..." << endl;
    pwrState=this->ReadPowerState();
    if (this->returnCode!=0) return;

    if (!pwrState)
    {
        std::cout << "Server already powered off." << endl;
        return;
    }

    //press & hold power btn
    std::cout << "Pressing power button ..."<< endl;
    this->gpioController->PwrBtnPressAndHold();
    if (this->returnCode!=0) return;

    //check for power state
    std::cout << "Waiting for server power off signal"<<endl;
    pwrState=this->CheckPowerState(0, PWR_OFF_WAIT_TIME_INTV_TO_CHK_US, PWR_OFF_MAX_RETRY_CHECK_PWR_STATE);
    this->gpioController->PwrBtnRelease();

    if (this->returnCode !=0) return;

    if (!pwrState)
        std::cout << "Server power off signal received. Server powered off successfully."<<endl;
    else
        std::cout << "Did not receive power off signal in time. Failed powering off the server."<<endl;
}

void ServerPMTool::CommandReset()
{
    if (!this->gpioController->RstBtnShortPress())
        this->returnCode=1;
}

void ServerPMTool::PwrBtnShortPress()
{
    if (!this->gpioController->PwrBtnShortPress())
        this->returnCode=1;
}

bool ServerPMTool::ReadPowerState()
{
    bool state;
    if (!this->gpioController->GetServerPowerState(state))
    {
        this->returnCode=1;
        return false;
    }
    return state;
}

bool ServerPMTool::CheckPowerState(bool expectedState, long reCheckIntervalUS, int maxReChecksToFail)
{
    int reCheckCntr=0;
    while (reCheckCntr<maxReChecksToFail)
    {
        usleep(reCheckIntervalUS);
        if (this->ReadPowerState()==expectedState || this->returnCode!=0)
            return expectedState;
        reCheckCntr++;
    }
    return !expectedState;
}

int ServerPMTool::GetReturnCode()
{
    return this->returnCode;
}
