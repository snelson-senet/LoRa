/* This sketch allows an Arduino to send data on a LoRa network 
 * Developed by Mike & Anna
 * Feb 2016
 * 
 * This project is for a MultiTech mDot attached to an Arduino Uno via an Xbee Shield.
 * The Arduino communicates with the laptop via the standard serial ports, D0 and D1.
 * The Arduino communicates with the mDot via the software-defined serial ports, D10 and D11. 
 *  
*/


//create software-defined serial port on pins 10 & 11
//from: https://www.arduino.cc/en/Tutorial/SoftwareSerialExample
#include <SoftwareSerial.h>
SoftwareSerial mDotSerial(10, 11); // RX, TX

int blinkrate = 1000; // time between AT commands
int wait_time_to_join_network = 5000; // set delay to join the network
int mDotResetPin = A0;  // super important! Make sure A0 is connected to mDot pin 5!
String incomingString; // string coming from Serial
String mDotString; // string coming from mDot
bool   onNetwork = false;

#define MDOT_BAUD_RATE  9600

#define APP_EUI "00:25:0C:00:00:01:00:01"

// Select device by uncommenting one of the following:
// #define MINT
// #define WALNUT
// #define GRAPE

#ifdef MINT   // mint on Anna's orange account 
#define APP_KEY "CB:2C:63:CB:C8:AC:F2:31:FB:E8:BF:55:35:27:89:CC"
#elif WALNUT  // walnut on Mike's orange account  
#define APP_KEY "7E:74:B1:46:8D:5E:5E:0D:03:35:70:82:AC:E4:5B:04"
#elif GRAPE    // grape on Mike's orange account  
#define APP_KEY "9B:77:EE:F7:F7:A8:51:91:8C:00:A1:40:62:C6:52:A5"
#else
#warning APP_KEY for this device needs to be defined
#define APP_KEY ""
#endif

#define APP_KEY_SIZE 47
#define APP_EUI_SIZE 23

  
String sendToMDot(String& command)
{
  // String command = _command;
  String output;
  
  // Dump command to Arduino Serial
  Serial.print("\nSend to mDot: ");
  Serial.println(command);

  // Write command to mDot
  mDotSerial.println(command);
    
  // Read mDot command result
  output = mDotSerial.readString();

  // Remove leading and trailing whitespace
  output.trim();
    
  // Dump output to Arduino serial
  Serial.print("Received from mDot: ");
  Serial.println(output);

  // Return mDot output
  return output;
}

bool sendMDotCommand(String& command)
{
  int    ok;
  String output;

  // Send command to mDot
  output = sendToMDot(command);

  // Command was successful if mDot returned "OK"
 return (output.indexOf("OK") != -1);
}

bool sendJoinCommand()
{
  bool   ok      = false;
  bool   joined  = false;
  String command = "AT+JOIN";
  String output;
   
  for(int i = 0; (i < 100) && (joined == false); i++){  
 
    // Dump command to Arduino Serial
    Serial.println("JOINING");
    mDotSerial.println(command);
    
    for(int j = 0; (j < 3) && (joined == false); j++){
      // Read mDot command result
      output = mDotSerial.readString();
      
      // Remove leading and trailing whitespace
      output.trim();
      
      // Dump output to Arduino serial
      Serial.print("\nJOIN received from mDot: ");
      Serial.println(output);

      if (output.indexOf("OK") != -1){
        Serial.println("\nJOINED NETWORK!!");
        joined = true;
      }
      else{
        delay(1000);
      }
    }
  }
  return joined;
}


void setup() {
  
  delay(2000);    // allow for the power rails to power up so that the Arduino and mDot are running properly
  pinMode(10, INPUT);
  pinMode(11, OUTPUT);
  
  // reset the mDot with the reset pin (Arduino pin A0 to mDot pin 5)
  pinMode(mDotResetPin, OUTPUT);
  digitalWrite(mDotResetPin, HIGH);
  delay(500); 
  digitalWrite(mDotResetPin, LOW);
  delay(500); //send the reset signal for 500ms
  digitalWrite(mDotResetPin, HIGH);
  //have 2sec delay to let mDot completely reset
  delay(2000);  
  
  //configure the USB to the computer
  Serial.begin(9600); 
  Serial.println("configured serial to home computer");
  //configure USB to the xbee/mDot
  mDotSerial.begin(MDOT_BAUD_RATE); //initialize the serial port. Set baud rate to 115200, same as mDot firmware
  Serial.println("configured serial to mDot");
  //clear any existing outputs
  mDotSerial.flush();
  Serial.flush();
  //clear out whatever might have been in the receive buffer
  while (mDotSerial.available() != 0) mDotSerial.read();

  // Connect to network.
  while (get_on_the_network() == false){
    delay(1000);
  }
} // end of setup()


bool get_on_the_network() {
  
  #define MDOT_CONFIG_COMMANDS 5
  String mDotCfgCmd[MDOT_CONFIG_COMMANDS]= { "AT+PN=1",         // Senet is a public network
                                             "AT+NI=0,"APP_EUI, // The Application Id this device is binding to 
                                             "AT+NK=0,"APP_KEY, // The Application key for this specific device.
                                             "AT+TXP=20",       // Transmit power
                                             "AT+ADR 1"         // Turn on Adaptive Rate Control
                                            }; 

   String appKey = APP_KEY;
   if(appKey.length() != APP_KEY_SIZE){
      while(true){
        Serial.println("APP_KEY is invalid!!\n");
        delay(5000);
      }
   }

   String appEui = APP_EUI;
   if(appEui.length() != APP_EUI_SIZE){
    while(true){
        Serial.println("APP_EUI is invalid!!\n");
        delay(5000);
      }
   }

  // Set mDot LoRa configuration
  for (int i = 0; i < MDOT_CONFIG_COMMANDS; i++){
    while (sendMDotCommand(mDotCfgCmd[i]) == false){
      delay(1000);
    }
  }

  // Display mDot configuration for debug
  String statusCmd = "AT&V";
  sendToMDot(statusCmd);
 
  // Join network
  return (sendJoinCommand());
}

void sendSensorData(String data)
{
  String command = "AT+SEND " + data;
  sendMDotCommand(command);
}
    
void loop() {
  String measurement = "0123456789";
  Serial.println("Transmit sensor data");
  sendSensorData(measurement);
  delay(30000);
  
} // end of loop()



