/*
 Name:		ArduinoMotor01.ino
 Created:	31.12.2016 08:18:26
 Author:	Christian Daumeter
*/

/*
Motor Steuerung  0.2

*/
#include "MotorControl.h"
#include <LiquidCrystal.h>
#include <Wire.h>
#include <string.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// I2C Bus 
#define SLAVE_ADDRESS 0x04

// RC Empfänger

static const int RC_KANAL2 = A0;
static const int RC_KANAL3 = A1;
static const int RC_KANAL4 = A2;
static const int RC_KANAL5 = A3;

// Anzeige Puffer für Ausgabe
String aktZeile1 = "";
String aktZeile2 = "";

bool MotorenAktiv = false;

// Laufvariabeln
int aktPower1 = 0;
int aktPower2 = 0;

int newPower1 = 0;
int newPower2 = 0;

int newSteering = 50;

int PowerOut1 = 0;
int PowerOut2 = 0;

String laufrichtung = "forward";

String receiveText = "Wait ...";
int receivePower1 = 0;
int receivePower2 = 0;
motor_direction receiveDirection = FORWARD;

// the setup function runs once when you press reset or power the board
void setup() {

	// Serial Output ( für ggf. Debug )
	Serial.begin(115200); // start serial for output
	Serial.println("Start Setup");

	//Display init
	lcd.begin(16, 2);
	lcd.clear();

	//I2C Init
	Wire.begin(SLAVE_ADDRESS);

	Wire.onReceive(receiveData);
	Wire.onRequest(sendData);

	// Vorw?rts als default
	MotorControl01.init();
	MotorControl01.setM1(FORWARD);
	MotorControl01.setM2(FORWARD);

	MotorControl01.setPower(0);

	laufrichtung = "forward";

	pinMode(RC_KANAL2, INPUT);
	pinMode(RC_KANAL3, INPUT);
	pinMode(RC_KANAL5, INPUT);
}

// the loop function runs over and over again forever
void loop() {
	//newPower = readNewPower();
	if (aktPower1 != receivePower1 || aktPower2 != receivePower2) {
		newPower1 = receivePower1;
		newPower2 = receivePower2;
	}

	changeDirection();
	newPower1 = abs(newPower1);
	newPower2 = abs(newPower2);
	//newSteering = readNewSterring();

	if (!calcSteeringn()) {
		calcPower();
	}

	if (MotorenAktiv) {
		MotorControl01.setM1Power(PowerOut1);
		MotorControl01.setM2Power(PowerOut2);
	}
	else
	{
		MotorControl01.setM1Power(0);
		MotorControl01.setM2Power(0);
	}
	writeValue2Display();
	delay(10);
}

int readNewPower() {
	int value = pulseIn(RC_KANAL2, HIGH) - 1500;
	if (value < -600) {
		value = 0;
	};
	if (value < MINSPEED && value > -MINSPEED) {
		value = 0;
	};
	value = 0.5 * value;
	return value;
}

int readNewSterring() {
	int value = pulseIn(RC_KANAL3, HIGH) - 1000;
	if (value < 1) {
		value = 500;
	};
	value = 0.1 * value;
	return value;
}

boolean readNewDirection() {
	int value = pulseIn(RC_KANAL5, HIGH) - 1000;
	return value < 500;
}

boolean calcSteeringn() {
	boolean buf = false;
	if (newSteering > 60) {
		PowerOut1 = MAXSPEED;
		PowerOut2 = 0;
		buf = true;
	};

	if (newSteering < 40) {
		PowerOut1 = 0;
		PowerOut2 = MAXSPEED;
		buf = true;
	};
	return buf;
}

boolean calcPower() {
	if (newPower1 > MAXSPEED) { newPower1 = MAXSPEED; }
	if (newPower2 > MAXSPEED) { newPower2 = MAXSPEED; }
	if (newPower1 < MINSPEED) { newPower1 = 0; }
	if (newPower2 < MINSPEED) { newPower2 = 0; }
	PowerOut1 = newPower1;
	PowerOut2 = newPower2;
	return true;
}

void changeDirection() {
	if ((newPower1 < 1) && (newPower2 < 1)) {
		MotorControl01.setM1(BACKWARD);
		MotorControl01.setM2(BACKWARD);
		laufrichtung = "backward";
	}
	else
	{
		MotorControl01.setM1(FORWARD);
		MotorControl01.setM2(FORWARD);
		laufrichtung = "forward ";
	}
}


void writeValue2Display() {
	String newZeile1 = "";
	newZeile1.concat("M1: " + String(PowerOut1) + " ");
	newZeile1.concat("M2: " + String(PowerOut2) + "   ");
	if (!newZeile1.equalsIgnoreCase(aktZeile1)) {
		aktZeile1 = newZeile1;
		lcd.setCursor(0, 0);
		lcd.print(aktZeile1);
		lcd.setCursor(1, 1);
		if (aktZeile2.length() > 0)
			lcd.print(aktZeile2);
	}

	if (receiveText.length() > 0) {
		if (!aktZeile2.equalsIgnoreCase(receiveText)) {
			aktZeile2 = receiveText;
			lcd.setCursor(0, 0);
			lcd.print(aktZeile1);
			lcd.setCursor(1, 1);
			lcd.print(aktZeile2);
		}
	}
}

// callback for received data
void receiveData(int byteCount) {
	Serial.println("Receive Data");
	receiveText = "";
	char inChar;
	String command = "";
	int size = 0;
	int readcount;
	char readdata[16];

	// Den Command Befehl einlesen. Muss ein "T" sein
	if (Wire.available())
		inChar = (char)Wire.read();
	command += inChar;
	Serial.println("Receive data (Command): " + command);

	// Die Länge einlesen, es müssen 16 Byte sein
	if (Wire.available())
		size = (int)Wire.read();
	String bytecount(size, DEC);
	Serial.println("Receive data (Size): " + bytecount);

	// Wenn alles stimmt, den Datenblock einlesen
	if (command.equals("T") && (size = 16)) {
		readcount = 0;
		readdata[0] = 0;
		while (Wire.available() && (readcount < 16)) {
			inChar = (char)Wire.read();
			readdata[readcount] = (char)inChar;
			readcount++;
		}
		char token1;
		int token2;
		int token3;

		//sscanf(readdata, "%c-%03d-%03d", token1, token2, token3);
		//char charbuf[10];
		//sprintf(charbuf, "Test %c-%03d-%03d", token1, token2, token3);
		//Serial.println(charbuf);
		
		if (readcount = 16)
			splitReadData(readdata);
	}
}

void splitReadData(char readbytes[16]) {
	String buf = "";
	String textout = "";
	int bufPower1 = 0;
	int bufPower2 = 0;
	
	for (int i = 0; i < 9; i++) {
		String inChar(readbytes[i], DEC);
		textout += inChar + '-';
	}
	Serial.println("Receive data (Data): " + textout);

	// Die Richtung einlesen
	buf += readbytes[0];
	if (buf.equals("F"))
		receiveDirection = FORWARD;
	if (buf.equals("B"))
		receiveDirection = BACKWARD;
	Serial.println("Receive data (Direction): " + buf);
	receiveText += buf;

	// Power1 einlesen
	buf = "";
	buf += readbytes[2];
	buf += readbytes[3];
	buf += readbytes[4];
	Serial.println("Receive data (Power1): " + buf);
	bufPower1 = buf.toInt();
	receiveText += '-' + buf;

	// Power2 einlesen
	buf = "";
	buf += readbytes[6];
	buf += readbytes[7];
	buf += readbytes[8];
	Serial.println("Receive data (Power2): " + buf);
	bufPower2 = buf.toInt();
	receiveText += '-' + buf;
	
	if ((bufPower1 == 999) && (bufPower2 == 999)) {
		Serial.println("Receive data : Stop Motor");
		MotorenAktiv = false;
	}
	if ((bufPower1 == 888) && (bufPower2 == 888)) {
		Serial.println("Receive data : Start Motor");
		MotorenAktiv = true;
	}

	if ((bufPower1 < MAXSPEED) && (bufPower2 < MAXSPEED)) {
		Serial.println("Receive data : Write to Engine"); 
		receivePower1 = bufPower1;
		receivePower2 = bufPower2; 
	};
	Serial.println("Receive data (Text): " + receiveText);
}

void sendData() {
	if (Wire.available()) {
		/*
		//Wire.beginTransmission(0);
		int writecount = 0;
		char writedata[16];

		Wire.write("R");
		Wire.write(16);
		while (Wire.available() && (writecount < 16)) {
			if (writecount < aktZeile2.length()) {
				Wire.write((char)aktZeile2[writecount]);
			}
			else {
				Wire.write(32);
			}
			
			writecount++;
		}
		//Wire.endTransmission();
		*/
	}
	char writedata[32];
	writedata[0] = 'R';
	writedata[1] = 16;
	writedata[2] = '1';
	writedata[3] = '2';
	writedata[4] = '3';
	writedata[5] = '4';
	writedata[6] = '5';
	writedata[7] = '6';
	writedata[8] = '7';
	writedata[8] = '8';
	Wire.write((byte*) &writedata[0], sizeof(writedata));

	Serial.println("Send data (Text): " + aktZeile2);
}
