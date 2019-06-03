#include <ESP8266WiFi.h>
#include<ArduinoJson.h>
#include<ESP8266HTTPClient.h>
#include <EEPROM.h>
 float getVPP();                              // current sensor function declaration

 const char* ssid     = "kyu chahiyia";           //wifi ssid
 const char* password = "8010119110";         //wifi password
 
 const int sensorIn = A0;
 int mVperAmp = 185;                          // use 100 for 20A Module and 66 for 30A Module
 double Voltage = 0;
 double VRMS = 0;
 int AmpsRMS=0;
 
 int EEaddress = 0;                           //initial address of eeprom
 int zero=0;                                  // value for clearing rom
 StaticJsonBuffer<300> JSONBuffer;                                  // creating object for jason formatr
 JsonObject& JSONencoder = JSONBuffer.createObject();
 JsonArray& current = JSONencoder.createNestedArray("current");
 char JSONmessageBuffer[300];

void setup() {
  Serial.begin(115200); 
  EEPROM.begin(4096);
  delay(100);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

/* while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());*/ 
}

void loop()
 {
    Voltage = getVPP();
    VRMS = (Voltage/2.0) *0.707;
    AmpsRMS = ((VRMS * 1000)/mVperAmp);

 const unsigned long fiveMinutes = 5 * 60 * 1000UL;
 static unsigned long lastSampleTime = 0 - fiveMinutes; 
 unsigned long now = millis();

 if (( WiFi.status()!=WL_CONNECTED)&&(AmpsRMS!=0))    //writing to eeprom when wifi is not connected
  {

    if (now- lastSampleTime>=fiveMinutes)               //wriring after every five min
    { 
      lastSampleTime+=fiveMinutes;
      EEPROM.write(EEaddress,AmpsRMS);
      EEPROM.commit();
      Serial.println("writing in eeprom");
      EEaddress+=2;
      } 
   
     if (EEaddress ==4096)                              // if EEProm is full then write back at initial address 
     {
      EEaddress = 0;
      EEPROM.commit();
     }
     }
   else if( WiFi.status()==WL_CONNECTED) 
   {
      if(EEPROM.read(EEaddress)>0)                         // checking if their is something to read in eeprom
     {
     StaticJsonBuffer<300> JSONBuffer;
      for(int i=0;EEPROM.read(EEaddress)!=0;i+=2,EEaddress+=2)           
      {
     current.add(EEPROM.read(i));                              // storing all the data in jason array
     EEPROM.write(i,zero);                                        // clearing data from eeprom
     EEPROM.commit();
      }
       JSONencoder["id"]="1001";
       JSONencoder["secret"]="1234";
       JSONencoder["voltage"]="220";  
       JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
       Serial.print("Requesting POST: ");
       HTTPClient http; 
       http.begin("https://houwrr73lj.execute-api.ap-southeast-1.amazonaws.com/test/savedata","C3:2F:BA:92:4C:3E:56:72:31:B2:EA:93:D4:80:2C:C2:FF:D1:6B:1C"); 
       http.addHeader("Content-Type", "application/json"); 
       int httpCode = http.POST(JSONmessageBuffer);   
       String payload = http.getString();              
       Serial.println(httpCode);  
       Serial.println(payload); 
       http.end();
   }
   else{
   if (now - lastSampleTime >= fiveMinutes)
   {
     lastSampleTime += fiveMinutes;

     JSONencoder["id"]="1001";
     JSONencoder["secret"]="1234";
     JSONencoder["current"]=AmpsRMS;
     JSONencoder["voltage"]="220";
     JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
 
     Serial.print("Requesting POST: ");
   
     HTTPClient http; 
     http.begin("https://houwrr73lj.execute-api.ap-southeast-1.amazonaws.com/test/savedata","C3:2F:BA:92:4C:3E:56:72:31:B2:EA:93:D4:80:2C:C2:FF:D1:6B:1C"); 
     http.addHeader("Content-Type", "application/json"); 
     int httpCode = http.POST(JSONmessageBuffer);   
     String payload = http.getString();              
   
     Serial.println(httpCode);  
     Serial.println(payload); 
     http.end();
     }
    }
   } 
  }
 
  float getVPP()
  {
    float result;
    int readValue;             //value read from the sensor
    int maxValue = 0;          // store max value here
    int minValue = 1024;          // store min value here
  
    uint32_t start_time = millis();
    while((millis()-start_time) < 1000) //sample for 1 Sec
    {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
          // Subtract min from max
      result = ((maxValue - minValue) * 5.0)/1024.0;
      
       return result;
    }
 
