/*
      Scale Puzzle

  2019 Shawn Yates
  www.cluecontrol.com
  feedback@cluecontrol.com
	

	Driver for Nano:  http://www.wch-ic.com/download/list.asp?id=126 OR http://www.5v.ru/ch340g.htm
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts
#include <Adafruit_NeoPixel.h>
#include <EEPROMVar.h>
#include <EEPROMex.h>
#include <HX711.h>


/*
// pin assignments
*
* Pin   Nano Signal			Usage
* -----------------------------
* 0   	D1-Serial RX		. none
* 1   	D0-Serial TX		. none
* 2		D2					. 
* 3		D3					. 
* 4   	D4					. 
* 5		D5					. 
* 6   	D6					. 
* 7		D7					. 

* 8		B0					. 
* 9		B1					. 
* 10	B2 /SS				.
* 11	B3 MOSI				.  
* 12	B4 MISO				.
* 13 	B5 SCK & LED		.  

* 14	A0					.  Output to Relay
* 15  	A1					.  Manual Trigger Button
* 16  	A2					.  LoadCell SCK
* 17  	A3					.  LED Dat
* 18  	A4 SDA				.  LoadCell DOUT
* 19  	A5 SCK				.  SetTarget Button
*		A6 (A only)			.
*		A7 (A only)			.


 */
 // Settings likely to change/be tweaked

/*Version Notes
1.1		4/21/2020
		Removed unused flags byte
		Added default values for things read from EEPRROM
		Change ReadMyScale to bool - return false if scal not present
		switch to N/O switches 
*/


 // Most things below this line shouldn't be changed unless you know what you are doing.
 // but feel free to read, learn and experiment.

 //***************************************************************************

 // Pin Renames
	const byte LOADCELL_DOUT_PIN = A4;
	const byte LOADCELL_SCK_PIN = A2;
	const byte MAIN_OUTPUT = A0;
	const byte SET_PIN = A5;
	const byte MANUAL_PIN = A1;
	const byte LED_DAT = A3;

 // Other Constants

	//create a loadcell instance
	HX711 loadcell;

	//create a NeoPixel
	Adafruit_NeoPixel LED(1, LED_DAT, NEO_GRB + NEO_KHZ800);
		// Argument 1 = Number of pixels in NeoPixel strip
		// Argument 2 = Arduino pin number (most are valid)
		// Argument 3 = Pixel type flags, add together as needed:
		//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
		//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
		//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
		//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
		//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
	//Colors
	const uint32_t COLOR_OFF = 0;
	const uint32_t COLOR_RED = 0xFF0000;		// Red
	const uint32_t COLOR_ORANGE = 0xD52A00;		// Orange
	const uint32_t COLOR_OR_YELLOW = 0xAB5500;		// Orange/Yellow
	const uint32_t COLOR_YELLOW = 0xAB7F00;		// Yellow
	const uint32_t COLOR_YEL_GREEN = 0xABAB00;		// Yellow/Green
	const uint32_t COLOR_GREEN = 0x56D500;		// Green
	const uint32_t COLOR_GREEN_BLUE = 0x00FF00;		// Green/BLue
	const uint32_t COLOR_LT_BLUE = 0x00D52A;		// Light Blue
	const uint32_t COLOR_MED_BLUE = 0x00AB55;		// Medium Blue
	const uint32_t COLOR_BLUE = 0x0056AA;		// Blue
	const uint32_t COLOR_DARK_BLUE = 0x0000FF;		// Blue
	const uint32_t COLOR_LT_PURPLE = 0x5500AB;		// Light Purple
	const uint32_t COLOR_PURPLE = 0x7F0081;		// Purple
	const uint32_t COLOR_PURPLE_PINK = 0xAB0055;		// Purple Pink
	const uint32_t COLOR_PINK = 0xD5002B;		// Pink

	/*
	Color usage:
		Reading Scale:		Blue
		Reading to high:	Orange
		Reading to low:		Purple
		Reading match:		Green
		Output Active:		Yellow
	*/


	//Polarities
	const bool OUTPUT_ON = true;
	const bool OUTPUT_OFF = !OUTPUT_ON;

	// Loadcell Adjustment settings
	const long LOADCELL_OFFSET = 54800;
	const long LOADCELL_DIVIDER = 10;

	//eeprom addresses
	const byte eeSettleDelay = 1;			//1+2 = 2 bytes for delay time in mS
	const byte eeTolerance = 3;				//3+4 = 2 bytes for tolerance number
	const byte eeOutputTime = 5;			//5+6 = 2 bytes for outpu time in mS
	const byte eeTargetWeight = 7;			//7,8,9,10 = 4 bytes for long target weight



	/*	writing/updated EEPROM values
			bool update(int address, uint8_t value);
			bool updateByte(int address, uint8_t value);
			bool updateInt(int address, uint16_t value);
			bool updateLong(int address, uint32_t value);
			bool updateFloat(int address, float value);
			bool updateDouble(int address, double);

		Reading EEPRom Values
			uint8_t read(int address);
			bool readBit(int address, byte bit)
			uint8_t readByte(int address);
			uint16_t readInt(int address);
			uint32_t readLong(int address);
			float readFloat(int address);
			double readDouble(int address);

	*/

 // Function Prototypes
	
	# define SoftReset asm volatile ( "jmp 0");

	bool DebounceSW(byte SWx);
	bool ReadMyScale(void);
	void DisplaySerialMenu(void);
	void CheckScale(void);
	void CheckOutput(void);
	void TriggerOutput(void);
	void CheckButtons(void);
	void CheckSerial(void);
	void GetNewSetting(String Prompt, byte eeAddress);
	void FlushSerialIn(void);
	void ReadEE(void);
	void UpdateLED(uint32_t NewColor);

	   	  
//Variables
	unsigned long ScaleReading = 0;
	unsigned long TargetWeight = 0;
	unsigned int Tolerance = 0;

	unsigned int SettleDelay = 0;
	unsigned long SettleTarget = 0;

	unsigned int OutputTime = 0;
	unsigned long OutputTarget = 0;
	
	bool Settling = false;
	bool Triggered = false;
	bool OutputActive = false;
	bool MonitorScale = false;

void setup()
{
	LED.begin();

	LED.setBrightness(100);

	// serial setup
	Serial.begin(115200);
	Serial.println("Serial interface started");
	Serial.println("Scale Trigger Version 1.1");
	Serial.println("From ClueConotrol.com");
	Serial.println("feedback@cluecontrol.com");

	pinMode(MAIN_OUTPUT, OUTPUT);
	digitalWrite(MAIN_OUTPUT, OUTPUT_OFF);

	pinMode(SET_PIN, INPUT_PULLUP);
	pinMode(MANUAL_PIN, INPUT_PULLUP);
	   
	//Initialize LoadCell
	loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
	loadcell.set_scale(LOADCELL_DIVIDER);
	loadcell.set_offset(LOADCELL_OFFSET);

	ReadEE();
	DisplaySerialMenu();

	UpdateLED(COLOR_OFF);
}

void loop()
{
	if (!OutputActive) CheckScale();
	if (OutputActive) CheckOutput();
	CheckButtons();
	CheckSerial();
	UpdateLED(COLOR_OFF);
}

bool DebounceSW(byte SWx)
{
	//Read the switch 5 times, 10mS apart.  Return 
	//the value of the majority of the reads.
	//this is to prevent false or erattic triggering
	//caused by the internal mechanical bounce of a switch

	byte HighCounts = 0;

	for (int x = 0; x < 5; x++)
	{
		if (!digitalRead(SWx))    //invert the reading due to the use of pullups
		{
			HighCounts++;
		}
		delay(10);
	}
	return (HighCounts > 2);    //if there are more than 2 hights, return high
}

bool ReadMyScale(void)
{
	ScaleReading = 0;

	if (loadcell.is_ready()) 
	{
		ScaleReading = loadcell.get_units(10);
		return (true);
	}
	else 
	{
		Serial.println("Error - Scale not found!");	
		return (false);
	}

}

void DisplaySerialMenu(void) 
{
	Serial.println();
	Serial.println();
	Serial.print(" 1. Target Weight     ");
	Serial.println(TargetWeight);

	Serial.print(" 2. Tolerance         ");
	Serial.println(Tolerance);

	Serial.print(" 3. Settle Time (mS)  ");
	Serial.println(SettleDelay);

	Serial.print(" 4. Output Time (mS)  ");
	Serial.println(OutputTime);

	Serial.println(" 5. Trigger Output");
	Serial.println(" 6. Monitor Scale");
	Serial.println();
	Serial.println("Enter a number: ");
}

void CheckScale(void)
{
	
	if (!ReadMyScale()) return;

	if (MonitorScale)
	{
		Serial.print("Reading = ");
		Serial.print(ScaleReading);
		Serial.print("      Low Limit = ");
		Serial.print(TargetWeight + Tolerance);
		Serial.print("     High Limit = ");
		Serial.print(TargetWeight - Tolerance);

		if ((ScaleReading > TargetWeight + Tolerance) || (ScaleReading < TargetWeight - Tolerance))
			Serial.println("   ->OUT OF RANGE");
		else
			Serial.println("    ->IN RANGE");

	}

	
	if ((ScaleReading > TargetWeight + Tolerance) || (ScaleReading < TargetWeight - Tolerance))
	{
		if (ScaleReading > TargetWeight + Tolerance) 
			UpdateLED(COLOR_ORANGE);
		else
			UpdateLED(COLOR_PURPLE);
		
		Triggered = false;
		Settling = false;
		return;
	}
	
	
	if (Triggered) return;
	//the code below executes only if the weight is within the target range and the output has not already 
	//been triggered

	UpdateLED(COLOR_GREEN);
	if (Settling)
	{
		if (millis() > SettleTarget)
		{
			TriggerOutput();
		}
	}
	else
	{
		Settling = true;
		SettleTarget = millis() + SettleDelay;
	}
}

void TriggerOutput(void)
{
	UpdateLED(COLOR_YELLOW);
	digitalWrite(MAIN_OUTPUT, OUTPUT_ON);
	OutputTarget = millis() + OutputTime;
	Triggered = true;
	OutputActive = true;
	Serial.println("Output Activated");
	

}

void CheckOutput(void)
{
	if (millis() > OutputTarget)
	{
		digitalWrite(MAIN_OUTPUT, OUTPUT_OFF);
		OutputActive = false;
		Serial.println("Output DE-activated");
		UpdateLED(COLOR_OFF);
	}

}

void CheckButtons(void)
{

	if (DebounceSW(MANUAL_PIN))
	{
		Serial.println("Manual Trigger");
		TriggerOutput();
	}

	if (DebounceSW(SET_PIN))
	{
		ReadMyScale();
		if (ScaleReading == 0) return;
		TargetWeight = ScaleReading;
		EEPROM.updateLong(eeTargetWeight, TargetWeight);
		DisplaySerialMenu();		
	}
}

void CheckSerial(void) 
{	
	if (Serial.available() <= 0) return;
	
		int incomingByte = 0; // for incoming serial data

		// read the incoming byte:
		incomingByte = Serial.read();
		if (MonitorScale)
		{
			MonitorScale = false;
			DisplaySerialMenu();
			return;
		}
		FlushSerialIn();
		switch (incomingByte)
		{
		case '1':
			GetNewSetting("Target Weight", eeTargetWeight);
			break;
		
		case '2':
			GetNewSetting("Tolerance (0 - 65,000)", eeTolerance);
			break;
		
		case '3':
			GetNewSetting("Settle Time (0 - 65,000 mS)",eeSettleDelay);
			break;
		
		case '4':
			GetNewSetting("Output Time (0 - 65,000 mS)",eeOutputTime);
			break;
		
		case '5':
			TriggerOutput();
			break;
		
		case '6':
			Serial.println();
			Serial.println("Staring monitor - send any key to stop");
			MonitorScale = true;
			break;
		}


}

void GetNewSetting(String Prompt, byte eeAddress) 
{
	String inString = "";    // string to hold input
	unsigned long NewVal = 0;
	bool ValFound = false;
	bool Confirm = false;

	Serial.println();
	Serial.print("Enter a new ");
	Serial.print(Prompt);
	Serial.println(" and hit enter");

	while (!ValFound) 
	{
		if (Serial.available() > 0)
		{
			int inChar = Serial.read();
			if (isDigit(inChar))
			{
				// convert the incoming byte to a char and add it to the string:
				inString += (char)inChar;
			}
			// if you get a newline, print the string, then the string's value:
			if (inChar == '\n')
			{
				NewVal = inString.toInt();
				Serial.print("Value: ");
				Serial.println(NewVal);
				Serial.println(" Correct?  Press Y to confirm");
				ValFound = true;
				FlushSerialIn();
			}
		}
	}

	while (!Confirm)
	{
		if (Serial.available() > 0)
		{
			int inChar = Serial.read();
			Confirm = true;
			if ((inChar == 'Y') || (inChar == 'y'))
			{	
				if (eeAddress == eeTargetWeight)
				{
					EEPROM.updateLong(eeTargetWeight, NewVal);
				}
				else
				{
					EEPROM.updateInt(eeAddress, NewVal);
				}
			}
		}
	}
	ReadEE();
	FlushSerialIn();
	DisplaySerialMenu();
}

void FlushSerialIn(void)
{
	delay(100);
	while (Serial.available() > 0) {
		Serial.read();
	}
}

void ReadEE(void)
{
	SettleDelay = EEPROM.readInt(eeSettleDelay);
	Tolerance = EEPROM.readInt(eeTolerance);
	OutputTime = EEPROM.readInt(eeOutputTime);
	TargetWeight = EEPROM.readLong(eeTargetWeight);

	if (SettleDelay == 65535) SettleDelay = 500;
	if (Tolerance == 65535) Tolerance = 500;
	if (TargetWeight == 4294946325) TargetWeight = 5000000;
	if (OutputTime == 65535) OutputTime = 5000;

}

void UpdateLED(uint32_t NewColor)
{
	LED.setPixelColor(0, NewColor);
	LED.show();
}

