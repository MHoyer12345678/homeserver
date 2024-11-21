/*
 * main.cpp
 *
 *  Created on: 13.05.2024
 *      Author: joe
 */
#include "ServerPMTool.h"

using namespace server_pm_tool;

int main (int argc, char **argv)
{
	int result=0;

	ServerPMTool *serverPMTool=new ServerPMTool();

	if (serverPMTool->Init(argc,argv))
		serverPMTool->ExecuteCommand();

	result=serverPMTool->GetReturnCode();

	serverPMTool->DeInit();

	return result;
}




