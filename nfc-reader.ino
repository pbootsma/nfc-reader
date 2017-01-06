#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>

#define UUID_LENGTH 4
#define TAG_DETECT_TIMEOUT 500

unsigned long lastTagDetect = 0;

PN532_I2C pn532_i2c(Wire);
PN532 nfc(pn532_i2c);

#define KEY_COUNT 2
byte validKeys[KEY_COUNT][UUID_LENGTH] = {
	{ 0xA6, 0x1C, 0x2E, 0x03 }, // blue peter
	{ 0x3B, 0x4E, 0xF1, 0xC5 } // white card
};

void setup()
{
	Serial.begin(115200);
	nfc.begin();
	
	uint32_t versiondata = nfc.getFirmwareVersion();
	if (!versiondata) {
		Serial.print("Didn't find PN53x board");
		while (1); // halt
	}
	// Got ok data, print it out!
	Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
	Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
	Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
	
	// configure board to read RFID tags
	nfc.SAMConfig();
	
	Serial.println("Setup finished");
}

void loop()
{
	byte uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
	byte uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
	
	// Blocks for a couple of seconds
	byte success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
	
	if (success) {
		unsigned long now = millis();
		if ((now - lastTagDetect) > TAG_DETECT_TIMEOUT) {
			if (isAuthorized(uid, uidLength)) {
				Serial.print("Authorized tag, toggle armed: ");
			} else {
				Serial.print("Unauthorized tag: ");
			}
			nfc.PrintHex(uid, uidLength);
		}
		lastTagDetect = now;
	}
}

bool isAuthorized(byte *uid, int uidLength) {
	bool isAuthorized = false;
	if (uidLength == 4) {
	    for (int keyNum=0; keyNum<KEY_COUNT; keyNum++) {
    		bool keyValid = true;
    		for (int byteNum=0; byteNum<UUID_LENGTH; byteNum++) {
    			if (uid[byteNum] != validKeys[keyNum][byteNum]) {
    				keyValid = false;
    			}
    		}
    		if (keyValid) isAuthorized = true;
    	}
	}
	return isAuthorized;
}
