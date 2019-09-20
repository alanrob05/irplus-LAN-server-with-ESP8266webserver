#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define IRLED 4

#ifndef STASSID
#define STASSID "YOUR SSID"
#define STAPSK  "YOUR PASSWORD"
#endif

ESP8266WebServer server(80);

const char *ssid = STASSID;
const char *password = STAPSK;

void pulse (int duration, int halfPeriodSign) {
  unsigned long cutoff_time=duration+micros();
  while (micros()<cutoff_time) {
    digitalWrite(IRLED, HIGH);
    delayMicroseconds(halfPeriodSign);
    digitalWrite(IRLED,LOW);
    delayMicroseconds(halfPeriodSign);
  }
}

void no_pulse (int duration) {
  delayMicroseconds(duration);
}


void handleRoot () {
  server.send(200);
  if (server.argName(0)=="carrier" && server.argName(1)=="code") {
    int carrier=server.arg(0).toInt();
    String code = server.arg(1);
    String delays[200];
    Serial.print("Number of args : ");
    Serial.println(server.args());
    Serial.print("Arg(0) = ");
    Serial.println(carrier);
    Serial.print("Arg(1) = ");
    Serial.println(code);
    
    
    int on_off_times[200] ={0};
    // Création du tableau d'entiers comprenant les délais on et off
    int a_delimiter=0;
    int b_delimiter=-1; // Démarre à -1 pour que le premier parse démarre à la case 0
    int count=0;
    for (int i=0 ; i<code.length() ; i++ ) {
      if (code[i]==' ') {
        a_delimiter=b_delimiter;
        b_delimiter=i;
        on_off_times[count]=code.substring(a_delimiter+1,b_delimiter).toInt();
        count++;
      }
    }

    int period = (int)1000000/carrier;
    int halfPeriod = (int)period/2;

    for (int i=0; on_off_times[i]!=0 ; i++) {
      if (i%2==0) {
        pulse(on_off_times[i],halfPeriod);
      }
      else {
        no_pulse(on_off_times[i]);
      }
    }
    
  }

}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  MDNS.update();
}
