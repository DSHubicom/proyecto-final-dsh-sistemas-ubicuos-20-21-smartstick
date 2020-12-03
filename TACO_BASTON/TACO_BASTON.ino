#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>

// Set these to your desired credentials.
const char *ssid = "yourAP";
const char *password = "yourPassword";

const int BOTON=25;
int val;

WebServer server(80);

void setup_routing() {   
  //Set the route to access to the button value  
  server.on("/button", getButton);   
       
  //Start server    
  server.begin();    
}

void getButton() {
  //Get the button value and answer to the GET request in text format
  Serial.println("Get button value");
  val=digitalRead(BOTON);
  Serial.println(val);
  server.send(200, "text/html", (String)val);
}

void setup() {
  pinMode(25, INPUT_PULLUP); 
  pinMode(BOTON,INPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  //Configuring access point
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  //Print IP address where the server is
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  //Setup the server routing
  setup_routing();

  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
