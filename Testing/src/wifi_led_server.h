#pragma once
#include <WiFi.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void printWifiStatus();
void sendHttpResponse(WiFiClient client);

inline void wifiLedServerSetup() {
  Serial.begin(9600);

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  if (strlen(pass) < 8) {
    Serial.println("Password must be at least 8 characters");
    while (true);
  }

  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  status = WiFi.beginAP(ssid, pass);

  while (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    delay(5000);
    status = WiFi.beginAP(ssid, pass);
  }

  server.begin();
  printWifiStatus();
}

inline void wifiLedServerLoop() {
  if (status != WiFi.status()) {
    status = WiFi.status();
    if (status == WL_AP_CONNECTED) {
      Serial.println("Device connected to AP");
    } else {
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    String currentLine = "";
    bool requestComplete = false;
    unsigned long requestStartTime = millis();
    const unsigned long requestTimeout = 5000;

    while (client.connected() && !requestComplete) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n') {
          if (currentLine.length() == 0) {
            Serial.println("End of HTTP request detected");
            requestComplete = true;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /Hr")) {
          digitalWrite(LEDR, LOW);
          Serial.println("Red LED ON");
        }
        if (currentLine.endsWith("GET /Lr")) {
          digitalWrite(LEDR, HIGH);
          Serial.println("Red LED OFF");
        }
        if (currentLine.endsWith("GET /Hg")) {
          digitalWrite(LEDG, LOW);
          Serial.println("Green LED ON");
        }
        if (currentLine.endsWith("GET /Lg")) {
          digitalWrite(LEDG, HIGH);
          Serial.println("Green LED OFF");
        }
        if (currentLine.endsWith("GET /Hb")) {
          digitalWrite(LEDB, LOW);
          Serial.println("Blue LED ON");
        }
        if (currentLine.endsWith("GET /Lb")) {
          digitalWrite(LEDB, HIGH);
          Serial.println("Blue LED OFF");
        }
      }

      if (millis() - requestStartTime > requestTimeout) {
        Serial.println("Request timeout");
        requestComplete = true;
      }
    }

    if (requestComplete) {
      Serial.println("Sending HTTP response");
      sendHttpResponse(client);
      delay(200);
      client.flush();
    }

    delay(100);
    client.stop();
    Serial.println("client disconnected");
  }
}

inline void sendHttpResponse(WiFiClient client) {
  // ...existing code from sendHttpResponse...
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println("Cache-Control: no-cache, no-store, must-revalidate");
  client.println("Pragma: no-cache");
  client.println("Expires: 0");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<title>Portenta LED Control</title>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; text-align: center; margin: 20px; background-color: #f5f5f5; }");
  client.println("h1 { color: #333; margin-bottom: 30px; }");
  client.println("h2 { margin-top: 30px; color: #555; }");
  client.println("a { display: inline-block; margin: 10px; padding: 15px 25px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 5px; font-weight: bold; box-shadow: 0 2px 5px rgba(0,0,0,0.2); }");
  client.println("a:hover { background-color: #45a049; transform: translateY(-2px); box-shadow: 0 4px 8px rgba(0,0,0,0.2); }");
  client.println("a:active { transform: translateY(1px); box-shadow: 0 1px 3px rgba(0,0,0,0.2); }");
  client.println(".red { color: #e74c3c; }");
  client.println(".green { color: #2ecc71; }");
  client.println(".blue { color: #3498db; }");
  client.println(".container { max-width: 600px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }");
  client.println(".led-control { margin-bottom: 20px; padding: 15px; border-radius: 8px; background-color: #f9f9f9; }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class=\"container\">");
  client.println("<h1>Portenta H7 LED Control</h1>");
  client.println("<div class=\"led-control\">");
  client.println("<h2><span class=\"red\">RED</span> LED</h2>");
  client.println("<a href=\"/Hr\">ON</a>");
  client.println("<a href=\"/Lr\">OFF</a>");
  client.println("</div>");
  client.println("<div class=\"led-control\">");
  client.println("<h2><span class=\"green\">GREEN</span> LED</h2>");
  client.println("<a href=\"/Hg\">ON</a>");
  client.println("<a href=\"/Lg\">OFF</a>");
  client.println("</div>");
  client.println("<div class=\"led-control\">");
  client.println("<h2><span class=\"blue\">BLUE</span> LED</h2>");
  client.println("<a href=\"/Hb\">ON</a>");
  client.println("<a href=\"/Lb\">OFF</a>");
  client.println("</div>");
  client.println("</div>");
  client.println("</body>");
  client.println("</html>");
  Serial.println("HTTP response sent successfully");
}

inline void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
