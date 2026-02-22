#include "Paradox.h"

Paradox::Paradox(HardwareSerial &serial, uint32_t baud): paradoxSerial(&serial)
{
  baudRate = baud;
}

void Paradox::begin() {
	paradoxSerial->begin(baudRate);
  paradoxSerial->flush();
}

int Paradox::read(byte &EventG, byte &EventN, byte &AreaN) {
	if (paradoxSerial->available() < 12) {
		return -1;
	}
    serialIndex = 0;
    while(serialIndex < 12) { // Paradox PRT3 packet is 12 bytes 
    
        serialData[serialIndex++] = paradoxSerial->read();
    }
    serialData[++serialIndex] = 0x00; // Make it print-friendly

    bool isData = (serialData[0] & 0xF0) == 0xE0;
    if (!isData) {
      failedCount++;
      if (failedCount > 3) {
        reset();
      } else {
        flushBuffer();
      }
    } else {
      if(serialData[0] == 'G' && serialData[4] == 'N')
      {
        EventG=serialData[1];
        EventN=serialData[4];
        AreaN=serialData[9];
      }
    }
    return isData ? 0 : 1;
}

void Paradox::flushBuffer() {
	while (paradoxSerial->available()) {
    paradoxSerial->read();
	}
}


void Paradox::reset() {
  paradoxSerial->end();
  delay(200);
  paradoxSerial->begin(baudRate);
  delay(200);
  paradoxSerial->flush();
}

void Paradox::AreaArm(byte &AreaN, byte &arm, String &code)
{
  Serial.println("Arming");
  paradoxSerial->print("AA"+AreaN+arm+code);
  Serial.println("AA"+AreaN+arm+code);
}

void Paradox::AreaDisarm(byte &AreaN, byte &arm, String &code)
{
  Serial.println("Disarming");
  paradoxSerial->print("AD"+AreaN+arm+code);
  Serial.println("AD"+AreaN+arm+code);
}