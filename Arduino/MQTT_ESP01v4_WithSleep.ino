
/*****************************************
 * 
 * 
 * This is the hackup to work on esp01
 * This script currently worked on the 1 Sept 2017. 
 * ESP01
 * tx -- nothing
 * CHPD -- +3.3v
 * RST -- nothing
 * VCC -- +3.3v
 * GND -- Ground
 * GPIO2 -- Base pin of NPN (NPN running 5v, through1K resistor and IR LED.
 * GPIO0 -- nothing
 * RX -- nothing
 ****************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


//WIFI Constants
const char* ssid = "xxxxxxx";            //  SSID of LAN/WLAN
const char* password = "xxxxx";        //  password

//MQTT constants
const char* mqttServer = "192.168.0.xxx"; //TODO: replace with server name... 
const int mqttPort = 1883;

//MQTT topic to subscribe to
String mqttTopicThis = "zoe_ac"; //update this one for the individual unit
String mqttTopicAll= "all";

//JSON object attributes
const String jsonMode = "mode";
const String jsonTemp = "temp";
const String jsonFan = "fan";
const String jsonOn = "on";
const String jsonProfile = "profile";

//Define and then instantiate the wifi / MQTT Client
WiFiClient client;
PubSubClient mqttClient(client);

//Global IR Declarations 
int halfPeriodicTime;
int IRpin;
int khz;

// HVAC MITSUBISHI Heavy Industries IR Mark & Space timings
#define HVAC_MITSUBISHI_HI_HDR_MARK    3368 
#define HVAC_MITSUBISHI_HI_HDR_SPACE   1684 
#define HVAC_MITSUBISHI_HI_BIT_MARK    421 
#define HVAC_MITSUBISHI_HI_ONE_SPACE   1263 
#define HVAC_MISTUBISHI_HI_ZERO_SPACE  421 
#define HVAC_MITSUBISHI_HI_RPT_MARK    421 
#define HVAC_MITSUBISHI_HI_RPT_SPACE   17100 // Above original iremote limit

long previousMillis = 0;
long previousMillis2 = 0;
long interval = 10000;

byte data[19]; 

// Memory pool for JSON object tree.
//
// Inside the brackets, 200 is the size of the pool in bytes,
// If the JSON object is more complex, you need to increase that value.
// See https://bblanchon.github.io/ArduinoJson/assistant/
//StaticJsonBuffer<200> jsonBuffer;


//method to establish wifi connection
void setup_wifi() {

  delay(10);

  //Start the wifi
  WiFi.begin(ssid, password);

  //wait until it is up
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

}

//Callback function for reading incoming message
void callback(char* topic, byte* payload, unsigned int length){

  DynamicJsonBuffer  jsonBuffer(200);


  String sTopic(topic);
 
  //Payload to string
  String sMessy = (String)(char*)payload;
  String sMessage2 = sMessy.substring(0, length);

  //debug messages
  Serial.println("|");
  Serial.println(sMessage2);
  Serial.println("|");
  //Serial.print(payload);

  //Check to see if the message should update the settings
  if(sTopic == mqttTopicThis ||sTopic == mqttTopicAll){
    PublishString("debug","here");
    //dumpString();

    //debug
    //PublishString("debug",sMessage2);
    //char *cstring = (char *) payload; 

    PublishString("debug","here2");
    
    //parse the json message
    JsonObject& root = jsonBuffer.parseObject(sMessage2);


    // handle message arrived
    //char inData[length];

  

 //for(int i =0; i<length; i++){
   //inData[(i)] = (char)payload[i];
 //}

 
//JsonObject& root = jsonBuffer.parseObject(inData); 

PublishString("debug","here3");

    //Debug statement
    PublishString("debug",root["on"]);
    PublishString("debug",root[jsonMode]);
    PublishString("debug",root[jsonProfile]);
    PublishString("debug",root[jsonTemp]);
    PublishString("debug",root[jsonFan]);
    
    //Update all settings
    setSwitchAndMode(root);
    PublishString("debug","here4");
    setTemp(root);
    PublishString("debug","here5");
    setProfileAndFan(root); //may have to add a profile "manual" to allow fan setting to work.
    PublishString("debug","here6");
    //send the current settings out over IR
    sendHvacMitsubishiHI();
    PublishString("debug","here7");

    //Debug - dump out the HEX codes of what was sent.
    dumpString();
    
  }  

  sTopic = "";
    
}

//Debug method - to dump out the current HEX codes
void dumpString(){
  
  String debugString;
  for (int i = 0; i < 19; i++) {
    debugString += String(data[i], HEX);
  }
  PublishString("debugverbose", debugString);
}

//Method to take JSON input and set HEX codes for On/off and mode based on the JSON input.
void setSwitchAndMode(JsonObject& root){
PublishString("debug","here3a");
  String sOn = root[jsonOn];
  PublishString("debug","here3b");
  // Byte 6 & 7 - On / Off and mode
  if (sOn == "false") {
 PublishString("debug","here3b");
    //Switch Off in Auto mode
    data[5] = (byte) 0xFF; 
    data[6] = (byte) 0x00;
    
  } else {
    PublishString("debug","here3c");
    //Switch On in the correct mode based on the json input
    setMode(root);
    PublishString("debug","here3d");
  }
  
}

//Method to set ON mode based on JSON input
void setMode(JsonObject& root){
PublishString("debug","here3ca");
  String sMessage = root[jsonMode];
PublishString("debug","here3cb");
  if (sMessage == "Auto"){data[5] = (byte) 0xF7; data[6] = (byte) 0x08; }
  else if (sMessage == "Cold"){ data[5] = (byte) 0xF6; data[6] = (byte) 0x09; }
  else if (sMessage == "Dry"){  data[5] = (byte) 0xF5; data[6] = (byte) 0x0A; }
  else if (sMessage == "Fan"){  data[5] = (byte) 0xF4; data[6] = (byte) 0x0B; }
  else if (sMessage == "Hot"){  data[5] = (byte) 0xF3; data[6] = (byte) 0x0C; }
  else {//no match
  };
  PublishString("debug","here3cd");

}

//Method to set profile and fan HEX code values based on JSON input
//NOTE profile other than NORMAL overrides the Fan setting
void setProfileAndFan(JsonObject& root){

  String sMessage = root[jsonProfile];
  
  // Profile to overrite - Byte 10 & 11
  if(sMessage == "NORMAL") { 
    //Set the fan manually
    setFan(root);
  } else if(sMessage == "ECO") {data[9] = (byte) 0xF9; data[10] = (byte) 0x06;}
  else if(sMessage == "POWER"){data[9] = (byte) 0xF3; data[10] = (byte) 0x0C;}
  else{
    //skip and don't over ride fan settings
    }
  
}

//Method to set temp HEX codes based on JSON input
void setTemp(JsonObject& root){
  // Byte 8 & 9 - Temperature
  // Check Min Max For Hot Mode
  int HVAC_Temp = root[jsonTemp];
  int iTemp;
  if (HVAC_Temp > 30) { iTemp = 30;}
  else if (HVAC_Temp < 18) { iTemp = 18; } 
  else { iTemp = HVAC_Temp; };
  
  data[7] = (byte) (255 - (iTemp - 17));
  data[8] = (byte) iTemp - 17;
 
}

//Method to set Fan HEX codes based on JSON input
//NOTE 0 = Auto - not off.
void setFan(JsonObject& root){
    
  // Byte 10 & 11 - FAN
  switch (root[jsonFan].as<int>())
  {
    case 0:    data[9] = (byte) 0xFF; data[10] = (byte) 0x00; break; //Set to Auto
    case 1:    data[9] = (byte) 0xFE; data[10] = (byte) 0x01; break; //set to fan speed 1
    case 2:    data[9] = (byte) 0xFD; data[10] = (byte) 0x02; break; //set to fan speed 2
    case 3:    data[9] = (byte) 0xFC; data[10] = (byte) 0x03; break; //set to fan speed 3
    case 4:    data[9] = (byte) 0xFB; data[10] = (byte) 0x04; break; //set to fan speed 4
    default: break;
  }

}



/*******************************************************
 * Function to connect the MQTT client using factory-ish pattern
 */
void reconnect(){

  //loop until reconnected
  while (!mqttClient.connected()){
    
    //attempt to connect
    if (mqttClient.connect("zoeClient")){
 
      //Subscribe to the settings
      mqttClient.subscribe("zoe_ac");
      mqttClient.subscribe("all");

      PublishString("help", "subscribed");
      
    } else {

      PublishString("help", "busted");
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" , try again in 5 seconds...");
      //Signal & wait... 
      //delay(5000);
    }
  }
}

//Setup method
void setup() {

  //Set IR PIN and PIN mode
  IRpin=2; // 4 for NodeMCU board - 2 for esp01  
  khz=38;
  halfPeriodicTime = 500/khz;
  pinMode(IRpin, OUTPUT);

  //Valid AC IR Frame HEX CODES -> On, Auto 23 degrees, and auto fan.
  //              52    AE    C3    1A    E5    F3    0C    F3    0C    FF    00    FF    00    FF    00    FF    00    7F    80

  data[0] = 0x52;
  data[1] = 0xAE;
  data[2] = 0xC3;
  data[3] = 0x1A;
  data[4] = 0xE5;
  data[5] = 0xF7;
  data[6] = 0x08;
  data[7] = 0xF9;
  data[8] = 0x06;
  data[9] = 0xFF;
  data[10] = 0x00;
  data[11] = 0xFF;
  data[12] = 0x00;
  data[13] = 0xFF;
  data[14] = 0x00;
  data[15] = 0xFF;
  data[16] = 0x00;
  data[17] = 0x7F;
  data[18] = 0x80;
 
  Serial.begin(115200);                        //  start serial for debug
  delay(10);
  
  //Establish WIFI
  setup_wifi();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

}

//Main loop
void loop() {
 
  if (!mqttClient.connected()){
    reconnect();  
    
  }
  mqttClient.loop(); 

  // Non-blocking delay to slow down the temp refreshes to avoud flooding the MQTT client. 
  // To be removed once the ESP sleep is in. 
  //unsigned long currentMillis = millis();

  //if (currentMillis - previousMillis > interval){
  
    //previousMillis = currentMillis;
    //if (mqttClient.connected()){
      //Send dummy data to MQTT to signal it is alive
      //PublishString("debug","MQTT still connected");
    //}
  //}

  //TODO add ESP Deep sleep
  //if (currentMillis - previousMillis2 > 50000){
  
    //previousMillis2 = currentMillis;
    //Read values and publish to MQTT queues
    
    // Put the ESP NODEMCU to sleep to save power
    //Serial.println("bedtime");
    //PublishString("sleep","Bedtime");
    //ESP.deepSleep(60 * 1000000, WAKE_RF_DEFAULT);// the 1000000 changes to seconds >> deepSleep for 1 minute
    //PublishString("sleep","going to sleep now");
    //delay(500);
    //The ESP will go to sleep now and will send a timed signal via D0 (GPIO16) to RST
    //The ESP will then reset back into wake up mode

  //}
  
}

//Method to send MQTT messages
void PublishString(String sTopic, String sPayload){
  char cTopic[50];
  char cMessage[50];
  
  //Convert message to char array
  sTopic.toCharArray(cTopic, sTopic.length() + 1);
  sPayload.toCharArray(cMessage, sPayload.length() + 1);
  mqttClient.publish(cTopic,cMessage);
  
}

/*****************************************************************************************
** Function to Send IR commands to a Mitsubishi Heavy Industries Split system AC, 
** Remote model: RLA502A700B. Note IRAnalysIR reports that it is using Sanyo152AC.
*****************************************************************************************/
void sendHvacMitsubishiHI()
{

#define  HVAC_MITSUBISHI_DEBUG;  // Un comment to access DEBUG information through Serial Interface

byte mask = 1; //our bitmask
byte i;

#ifdef HVAC_MITSUBISHI_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 19; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < 19; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  
  // Header for the Packet
  mark(HVAC_MITSUBISHI_HI_HDR_MARK);
  space(HVAC_MITSUBISHI_HI_HDR_SPACE);
  
  //loop through each hex value in the array.   
  for (i = 0; i < 19; i++) {
    
    // Send all Bits from Byte Data
    mask = 00000001;        
    //iterate through the byte by bit. 
    for(int k=0; k<8; k++){

      //Compare the byte against the target bit using the mask. 
      if (data[i] & mask) { // Bit ONE
        //Serial.print("1");
        mark(HVAC_MITSUBISHI_HI_BIT_MARK);
        space(HVAC_MITSUBISHI_HI_ONE_SPACE);
      }
      else { // Bit ZERO
        //Serial.print("0");
        mark(HVAC_MITSUBISHI_HI_BIT_MARK);
        space(HVAC_MISTUBISHI_HI_ZERO_SPACE);
      }
      
      //Shift the mask one place to the right, adding a zero to the front of the number.
      mask <<= 1;
    }
  }
    
  // End of Packet and retransmission of the Packet
  mark(HVAC_MITSUBISHI_HI_RPT_MARK);
  space(HVAC_MITSUBISHI_HI_RPT_SPACE);
  space(0); // Just to be sure

}

/****************************************************************************
/* enableIROut : Set global Variable for Frequency IR Emission
/***************************************************************************/ 
void enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  halfPeriodicTime = 500/khz; // T = 1/f but we need T/2 in microsecond and f is in kHz
}

/****************************************************************************
/* mark ( int time) 
/***************************************************************************/ 
void mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  long beginning = micros();
  while(micros() - beginning < time){
    digitalWrite(IRpin, HIGH);
    delayMicroseconds(halfPeriodicTime);
    digitalWrite(IRpin, LOW);
    delayMicroseconds(halfPeriodicTime); //38 kHz -> T = 26.31 microsec (periodic time), half of it is 13
  }
}

/****************************************************************************
/* space ( int time) 
/***************************************************************************/ 
/* Leave pin off for time (given in microseconds) */
void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  digitalWrite(IRpin, LOW);
  if (time > 0) delayMicroseconds(time);
}



