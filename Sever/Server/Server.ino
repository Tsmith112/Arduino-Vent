
#include <SPI.h>
#include <WiFiNINA.h>
#include <RTCZero.h>
#include <ArduinoJson.h>




#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11


//TODO
//Learn how to set via blue tooth in app set up 
char ssid[] = "nah";      //need new network name
char pass[] = "Bitches1";     //Need to find out how to connect to EAP-PEAP (MSCHAPv2) netwrok





int status = WL_IDLE_STATUS;
//int Steps = 0;
boolean Direction = true;
int led = LED_BUILTIN;
//Init wifi client library
WiFiSSLClient apiclient;
WiFiSSLClient client;
WiFiServer server(80);
int port = 443; 

//API things
char serverName[] = "developer-api.nest.com";
String result;
int fBracket = 0;
int bBracket = 0;

bool sendRequest = true;        //used to understand if the http request must be sent

StaticJsonDocument<200> jsonBuffer;
JsonObject root = jsonBuffer.to<JsonObject>();


void setup() {

  
  //Init serial and wait for port to open
  Serial.begin(9600);
  while (!Serial){

  }

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  

  String fv = WiFi.firmwareVersion();
    if (fv  < "1.0.0"){
      Serial.println("Please upgrade the firmware!");
    }
    
  connectToAP();
  printWifiStatus();
  server.begin();

}

void loop() {
  if(status != WiFi.status()){
    status = WiFi.status();
  

    if (status == WL_AP_CONNECTED){
      Serial.println("Device connected to AP");

    }
    else{
      Serial.println("Device discconected from AP");
    }
  }
  
  WiFiClient client = server.available(); //listening for clients 

  if (client) {
    Serial.println("new client");
    String currentLine = "";
    while(client.connected()) {
      if (client.available()){
        char c = client.read();
        Serial.write(c);
        if (c == '\n'){
          if(currentLine.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // The HTTP response follows the ehader
            client.print("Click <a href=\"/H\">here</a> turn the  Motor high<br>");
            client.print("Click <a href=\"/L\">here</a> turn the Motor low<br>");
            client.print("Click <a href=\"/G\">here</a> to try and call Nest<br>");
            //the HTTP reponse end with another blank line
            client.println();
            break;

          }
          else{
            currentLine = "";
          }
        }
        else if (c != '\r'){
          currentLine += c;
        }

        if(currentLine.endsWith("GET /H")){
          digitalWrite(led, HIGH);
          //more motor control responce
          Direction = 1;
          stepper(1000, 0);
          
            
 
          
        }

        if(currentLine.endsWith("GET /L")){
          digitalWrite(led, LOW);
          Direction = 0;
          stepper(1000, 7);
          

        }

        if(currentLine.endsWith("GET /G")) {
          getNest();
        }

      }
    }
    // close the connection
    client.stop();
    Serial.println("client disconnected");
    
  }








}

void printWifiStatus(){
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
  
    
}

void connectToAP(){
  if(WiFi.status() == WL_NO_SHIELD) {
      Serial.print("WiFi shield not present, stoping program in infit loop");
      while(true){
        
      }     
  }

  while(status != WL_CONNECTED) {
    Serial.print("attempting to connect to SSID: ");
    Serial.println(ssid);

     //this is for WPA/WPA2 network
     status = WiFi.begin(ssid, pass);

     delay(1000);
    
  }
  
}

/*void stepperDriver(){

    stepper(1, 0);
    delayMicroseconds(800);
   
}*/

//TODO
//Breakout into own file & Clean up Variable names
void stepper(int xw, int Steps) {
  int Step = Steps;
  for (int x = 0; x < xw; x++) {
      switch (Step) {
        case 0:
          digitalWrite(IN1, LOW);
          digitalWrite(IN2, LOW);
          digitalWrite(IN3, LOW);
          digitalWrite(IN4, HIGH);
          break;
        case 1:
          digitalWrite(IN1, LOW);
          digitalWrite(IN2, LOW);
          digitalWrite(IN3, HIGH);
          digitalWrite(IN4, HIGH);
          break;
      case 2:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break;
      case 3:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break;
      case 4:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
      case 5:
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
      case 6:
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
      case 7:
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        break;
      default:
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break;
     }
      
      Step = SetDirection(Step);
      delayMicroseconds(800);
    
  }
}

int SetDirection(int Step) {
  int Steps = Step;
  if (Direction == 1) {
    Steps++;
  }
  if (Direction == 0) {
    Steps--;
  }
  if (Steps > 7) {
    Steps = 0;
  }
  if (Steps < 0) {
    Steps = 7;
  }
  //Serial.println(Steps);
  return Steps;   
}


void getNest(){

  
  if(apiclient.connectSSL(serverName, port)){
    Serial.println("Connected to nest");
    //char call[] = "GET HTTP/1.1\n Host: developer-api.nest.com\n Content-Type: application/json\n Authorization: Bearer c.ixorKjTrTOtb6LQA6kZ4c97iCQ3YNHiddasXcg0MqrGKMxZjSmNzsoMzYZ4OPGK83c07Or6OUE0bWpHZW1I0gWVIa75nH4Xwf1Slmsr1US8GVPjcArS2lFhph6yLcfwvLEtVs6caDP5SdoCq\n";
    //apiclient.print(call);
    apiclient.println("GET HTTP/1.1");
    apiclient.println("Host: developer-api.nest.com");
    apiclient.println("Content-Type: application/json");
    apiclient.println("Authorization: Bearer c.ixorKjTrTOtb6LQA6kZ4c97iCQ3YNHiddasXcg0MqrGKMxZjSmNzsoMzYZ4OPGK83c07Or6OUE0bWpHZW1I0gWVIa75nH4Xwf1Slmsr1US8GVPjcArS2lFhph6yLcfwvLEtVs6caDP5SdoCq");
    //apiclient.println("cache-control: no-cache");
    //apiclient.println("Postman-Token: 01209b90-6020-4b7b-8092-cd425af8f86c");
    apiclient.println();
    Serial.println(apiclient.available());

    
  }
  else{
  Serial.println("Failed to connect");
  }

  Serial.println(apiclient.available());
  while(apiclient.connected() && !apiclient.available()) delay(1);

  while(apiclient.available()) {
    char c = apiclient.read();
    result += c;

 }
 
  Serial.print(result);
  apiclient.stop();
  fBracket = result.indexOf('{');
  bBracket = result.indexOf('}');
  result = result.substring(fBracket+1,bBracket);
  Serial.println("apicall");
  Serial.println(result);

}
