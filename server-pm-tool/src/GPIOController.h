/*
 * GPIOController.h
 *
 *  Created on: 13.05.2024
 *      Author: joe
 */

#ifndef SRC_GPIOCONTROLLER_H_
#define SRC_GPIOCONTROLLER_H_

#include <gpiod.h>

namespace server_pm_tool {

class GPIOController
{
private:
	struct gpiod_line_request *statesRequest;

	struct gpiod_line_request *btnsRequest;
	
	bool OpenGPIODevice(struct gpiod_chip **chip);
	
	bool SetupPwrStatusGPIO(struct gpiod_chip *chip);
	
	bool SetupSwitchGPIOs(struct gpiod_chip *chip);

public:
	GPIOController();

	virtual ~GPIOController();

	bool Init();

	void DeInit();

	bool PwrBtnShortPress();

	bool PwrBtnPressAndHold();

	bool PwrBtnRelease();

	bool RstBtnShortPress();

	bool GetServerPowerState(bool &state);
};

} /* namespace  server_pm_controller  */

#endif /* SRC_GPIOCONTROLLER_H_ */
