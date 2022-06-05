#include <MySensor.h>
#include <SPI.h>

#define ID 0

enum WashingMachineState {
	Off,
	Running,
	Idle
};

// these constants won't change:
const int sensorPin = 7;    // the pin that the Light Sensor is attached to
const int interval = 200;
const int threshold = 300;

const int triggerOnOffCount = 80;
const int triggerIdleCount = 10;

// Variables will change:
int FlashValue = 0;
int flashCounter = 0;   // counter for the number of flashes
int offCounter = 0;
int onCounter = 0;
int lightState = 0;         // current state of the light
int lastlightState = 0;     // previous state of the light

WashingMachineState machineState;
WashingMachineState lastMachineState;

MySensor gw;
MyMessage msg(ID, V_VAR1);

void setup() {
	gw.begin();
	gw.present(ID, S_CUSTOM);

	// initialize the button pin as a input:
	pinMode(sensorPin, INPUT);

	// Set default state
	machineState = Off;
	lastMachineState = Off;
}


void loop() {
	delay(interval);

	readLightStatus();

	if (machineState == Idle && machineState != lastMachineState) {
		lastMachineState = machineState;
		// send notification
		gw.send(msg.set("Idle"));
	}

	if (machineState == Off && machineState != lastMachineState) {
		lastMachineState = machineState;
		// send notification
		gw.send(msg.set("Off"));
	}

	if (machineState == Running && machineState != lastMachineState) {
		lastMachineState = machineState;
		// send notification
		gw.send(msg.set("Running"));
	}
}

void readLightStatus() {
	// read the Light sensor input pin:
	FlashValue = analogRead(sensorPin);

	if (FlashValue > threshold)
	{
		// light On
		lightState = 1;
	}
	else
	{
		// light off
		lightState = 0;
	}

	incrementCounters();

	// save the current state as the last state, 
	lastlightState = lightState;
}

void incrementCounters(){
	if (lightState == 1)
	{
		// light On
		onCounter += 1;
		offCounter = 0;
                if (onCounter > triggerOnOffCount) { onCounter = triggerOnOffCount; }
	}
	else
	{
		// light off
		offCounter += 1;
		onCounter = 0;
                if (offCounter > triggerOnOffCount) { offCounter = triggerOnOffCount; }
	}

	// compare the lightState to its previous state
	if (lightState != lastlightState && lightState == 1) {
		// if the state has changed to On, increment the flash counter
		flashCounter++;
                if (flashCounter > triggerIdleCount) { flashCounter = triggerIdleCount; }
	}

	// 10 Flashes counted, machine is now idle
	if (flashCounter >= triggerIdleCount && machineState != Idle) {
		machineState = Idle;
		offCounter = 0;
		onCounter = 0;
	}

	if (onCounter >= triggerOnOffCount && machineState != Running)
	{
		machineState = Running;
		offCounter = 0;
		flashCounter = 0;
	}

	if (offCounter >= triggerOnOffCount && machineState != Off)
	{
		machineState = Off;
		onCounter = 0;
		flashCounter = 0;
		offCounter = triggerOnOffCount; // don't let the number increment any further
	}

	//Serial.println("flashCounter: " + String(flashCounter));
	//Serial.println("onCounter:    " + String(onCounter));
	//Serial.println("offCounter:   " + String(offCounter));

}