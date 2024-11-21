/*
 * GPIOController.cpp
 *
 *  Created on: 13.05.2024
 *      Author: joe
 */

#include "GPIOController.h"

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <gpiod.h>

using namespace server_pm_tool;
using namespace std;

// time from press to release for a short button press (100ms)
#define BTN_SHORT_PRESS_TIME_US 100000L

#warning: add as command line
#define GPIO_DEV "/dev/gpiochip0"

#define GPIO_LINE_PWR_SIGNAL		17
#define GPIO_LINE_PWR_SWITCH		9
#define GPIO_LINE_RST_SWITCH		10

GPIOController::GPIOController() :
	statesRequest(NULL),
	btnsRequest(NULL)
{
}

GPIOController::~GPIOController()
{
}

bool GPIOController::Init()
{
	struct gpiod_chip *chip=NULL;

	if (this->statesRequest!=NULL || this->btnsRequest!=NULL)
		this->DeInit();

	if (!this->OpenGPIODevice(&chip))
		return false;

	if (!this->SetupPwrStatusGPIO(chip))
		return false;

	if (!this->SetupSwitchGPIOs(chip))
		return false;

	gpiod_chip_close(chip);

	return true;
}

void GPIOController::DeInit()
{
	if (this->statesRequest!=NULL)
	{
		gpiod_line_request_release(this->statesRequest);
		this->statesRequest=NULL;
	}

	if (this->btnsRequest!=NULL)
	{
		//set btns to inactive as safe state
		gpiod_line_request_set_value(this->btnsRequest, GPIO_LINE_PWR_SWITCH,
			gpiod_line_value::GPIOD_LINE_VALUE_INACTIVE);

		gpiod_line_request_set_value(this->btnsRequest, GPIO_LINE_RST_SWITCH,
			gpiod_line_value::GPIOD_LINE_VALUE_INACTIVE);
			
		gpiod_line_request_release(this->btnsRequest);
		this->btnsRequest=NULL;
	}
}

bool GPIOController::PwrBtnShortPress()
{
	if (!this->PwrBtnPressAndHold()) return false;
	usleep(BTN_SHORT_PRESS_TIME_US);
	return this->PwrBtnRelease();	
}

bool GPIOController::PwrBtnPressAndHold()
{
	if (this->btnsRequest==NULL)
		return false;

	if (gpiod_line_request_set_value(this->btnsRequest, GPIO_LINE_PWR_SWITCH,
		gpiod_line_value::GPIOD_LINE_VALUE_ACTIVE)!=0)
	{
		cout << "Unable to press PWR BTN: "<<strerror(errno)<<endl;
		return false;
	}
    return true;
}

bool GPIOController::PwrBtnRelease()
{
	if (this->btnsRequest==NULL)
		return false;

	if (gpiod_line_request_set_value(this->btnsRequest, GPIO_LINE_PWR_SWITCH,
		gpiod_line_value::GPIOD_LINE_VALUE_INACTIVE)!=0)
	{
		cout << "Unable to release PWR BTN: "<<strerror(errno)<<endl;
		return false;
	}
    return true;
}

bool GPIOController::RstBtnShortPress()
{
	if (this->btnsRequest==NULL)
		return false;
	
	cout << "Pressing RST BTN: "<<std::endl;
	if (gpiod_line_request_set_value(this->btnsRequest, GPIO_LINE_RST_SWITCH,
		gpiod_line_value::GPIOD_LINE_VALUE_ACTIVE)!=0)
	{
		cout << "Unable to press RST BTN: "<<strerror(errno)<<endl;
		return false;
	}
	cout << "Pressed RST BTN: "<<std::endl;
	usleep(BTN_SHORT_PRESS_TIME_US);

	cout << "RELEASING RST BTN: "<<std::endl;
	if (gpiod_line_request_set_value(this->btnsRequest, GPIO_LINE_RST_SWITCH,
		gpiod_line_value::GPIOD_LINE_VALUE_INACTIVE)!=0)
	{
		cout << "Unable to release RST BTN: "<<strerror(errno)<<endl;
		return false;
	}
	cout << "RELEASED RST BTN: "<<std::endl;

    return true;
}

bool GPIOController::GetServerPowerState(bool &state)
{
	gpiod_line_value val;

	if (this->statesRequest==NULL)
		return false;

	val=gpiod_line_request_get_value(this->statesRequest, GPIO_LINE_PWR_SIGNAL);

	if (val==gpiod_line_value::GPIOD_LINE_VALUE_ERROR)
	{
		cout << "Unable to get PWR signal: "<<strerror(errno)<<endl;
		return false;
	}

	state=(val==gpiod_line_value::GPIOD_LINE_VALUE_ACTIVE);

    return true;
}

bool GPIOController::OpenGPIODevice(struct gpiod_chip **chip)
{
	*chip = gpiod_chip_open(GPIO_DEV);
	if (!*chip)
	{
		cout << "Error opening gpio device "<<GPIO_DEV<<": "
			<<strerror(errno)<<endl;
		return false;
	}

    return true;
}

bool GPIOController::SetupPwrStatusGPIO(struct gpiod_chip *chip)
{
	struct gpiod_request_config *req_cfg = NULL;
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;

	unsigned int offset=GPIO_LINE_PWR_SIGNAL;
	
	int ret;

	settings = gpiod_line_settings_new();
	if (!settings)
	{
		cout << "Error creating settings structure: "
			<<strerror(errno)<<endl;
		return false;
	}

	gpiod_line_settings_set_direction(settings,
					  GPIOD_LINE_DIRECTION_INPUT);
	
	line_cfg = gpiod_line_config_new();
	if (!line_cfg)
	{
		cout << "Error creating config structure: "
			<<strerror(errno)<<endl;
		return false;
	}

	if (gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings)!=0)
	{
		cout << "Failed to add line settings to line config: "
			<<strerror(errno)<<endl;
		return false;
	}					  
	
	this->statesRequest = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);
	gpiod_line_config_free(line_cfg);
	gpiod_line_settings_free(settings);

    return true;
}

bool GPIOController::SetupSwitchGPIOs(struct gpiod_chip *chip)
{
	struct gpiod_request_config *req_cfg = NULL;
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;

	unsigned int offsets[]={GPIO_LINE_PWR_SWITCH, GPIO_LINE_RST_SWITCH };
	
	int ret;

	settings = gpiod_line_settings_new();
	if (!settings)
	{
		cout << "Error creating settings structure: "
			<<strerror(errno)<<endl;
		return false;
	}

	gpiod_line_settings_set_direction(settings,
					  GPIOD_LINE_DIRECTION_OUTPUT);
	gpiod_line_settings_set_output_value(settings, gpiod_line_value::GPIOD_LINE_VALUE_INACTIVE);

	line_cfg = gpiod_line_config_new();
	if (!line_cfg)
	{
		cout << "Error creating config structure: "
			<<strerror(errno)<<endl;
		return false;
	}

	if (gpiod_line_config_add_line_settings(line_cfg, offsets, 2, settings)!=0)
	{
		cout << "Failed to add lines settings to line config: "
			<<strerror(errno)<<endl;
		return false;
	}					  
	
	this->btnsRequest = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);
	gpiod_line_config_free(line_cfg);
	gpiod_line_settings_free(settings);

    return true;
}
