#include "Arduino.h"

#define PARADOX_ZONE_IN_ALARM 24
#define PARADOX_FIRE ALARM 25
#define PARADOX_ZONE_ALARM_RESTOR 26
#define PARADOX_DISARM_AFTER_ALARM 17
#define PARADOX_TROUBLE 36
#define PARADOX_TROUBLE_OK 37

const String PARADOX_TROUBLE_CODES[] = {
  "TLM Trouble"
  "AC failure",
  "Battery failure",
  "Auxiliary current overload",
  "Bell current overload",
  "Bell disconnected",
  "Clock loss",
  "Fire loop trouble"
};

#define Regular A
#define Forced F
#define Stay S
#define Instant I

class Paradox {
public:
	Paradox(HardwareSerial &serial, uint32_t baud);
	void begin();
	int read(byte &EventG, byte &EventN, byte &AreaN);
  void AreaDisarm(byte &AreaN, byte &arm, String &code);
  void AreaArm(byte &AreaN, byte &arm, String &code);

private:
	HardwareSerial *paradoxSerial;
	char serialData[13];
	byte serialIndex = 0;
  uint32_t failedCount = 0;
  uint32_t baudRate;
	void flushBuffer(void);
  void reset(void);
};