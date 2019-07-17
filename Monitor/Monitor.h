/*
 * Monitor.h
 *
 *  Created on: 09/01/2017
 *      Author: root
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#include <vector>
#include <string>
#include <queue>

#include "antares.h"


namespace antares {

class Monitor {


public:
	Monitor();
	virtual ~Monitor();

	static Monitor* Initialize();
	void start();

	// Events
	void serviceEvents();
	void pushEvent(event_t evt);

	void serviceBoard();


private:

	std::queue<event_t> events;
	event_t *dummyevt;

	bool wg20_cw;
	bool drum;
	bool vaccum;

	uint8_t wg20pwm;


	bool isDrum();
	void setDrum(bool drum);
	bool isVaccum();
	void setVaccum(bool vaccum);
	bool isWg20Cw();
	void setWg20Cw(bool wg20Cw);
	void setWG20Vel();


};

} /* namespace antares */

#endif /* MONITOR_H_ */
