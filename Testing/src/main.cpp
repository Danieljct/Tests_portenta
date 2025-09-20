/*
  WiFi Web Server LED Control - Portenta H7 Client Mode
  
  Este código convierte el Portenta H7 en un cliente WiFi que se conecta a una red existente.
  Permite controlar los LEDs RGB a través de una interfaz web.
  
  Circuit:
  * Arduino Portenta H7
  * LEDs RGB integrados en la placa
*/

#include <WiFi.h>
#include "arduino_secrets.h" 

char ssid[] = SECRET_SSID;        // nombre de la red WiFi
char pass[] = SECRET_PASS;        // contraseña de la red WiFi

int status = WL_IDLE_STATUS;
WiFiServer server(80);
void printWifiStatus();
void setup() {
  Serial.begin(9600);      // inicializar comunicación serial
  
  // Configurar los LEDs del Portenta H7
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  
  // Inicialmente apagar todos los LEDs (lógica invertida en Portenta)
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  // Verificar el módulo WiFi
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  // Intentar conectarse a la red WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(3000);
  }
  // Una vez conectado, iniciar el servidor web
  server.begin();
  
  // Imprimir información de la conexión WiFi
  printWifiStatus();
}

void loop() {
  WiFiClient client = server.available();   // escuchar clientes entrantes

  if (client) {                             // si hay un cliente,
    Serial.println("new client");           // imprimir mensaje en el puerto serie
    String currentLine = "";                // crear cadena para guardar datos del cliente
    
    while (client.connected()) {            // bucle mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer del cliente,
        char c = client.read();             // leer un byte
        Serial.write(c);                    // imprimir en el monitor serie
        
        if (c == '\n') {                    // si el byte es un carácter de nueva línea
          // si la línea actual está vacía, tienes dos caracteres de nueva línea seguidos.
          // ese es el final de la solicitud HTTP del cliente, así que envía una respuesta:
          if (currentLine.length() == 0) {
            // Las cabeceras HTTP siempre comienzan con un código de respuesta (ej. HTTP/1.1 200 OK)
            // y un tipo de contenido para que el cliente sepa qué viene, luego una línea en blanco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // El contenido de la respuesta HTTP sigue a la cabecera:
            client.print("<html><head>");
            client.print("<style>");
            client.print("* { font-family: sans-serif;}");
            client.print("body { padding: 2em; font-size: 2em; text-align: center;}");            
            client.print("a { -webkit-appearance: button;-moz-appearance: button;appearance: button;text-decoration: none;color: initial; padding: 25px;} #red{color:red;} #green{color:green;} #blue{color:blue;}");
            client.print("</style></head>");
            client.print("<body><h1> LED CONTROLS </h1>");
            client.print("<h2><span id=\"red\">RED </span> LED </h2>");
            client.print("<a href=\"/Hr\">ON</a> <a href=\"/Lr\">OFF</a>");
            client.print("<h2> <span id=\"green\">GREEN</span> LED </h2>");
            client.print("<a href=\"/Hg\">ON</a> <a href=\"/Lg\">OFF</a>");
            client.print("<h2> <span id=\"blue\">BLUE</span> LED </h2>");
            client.print("<a href=\"/Hb\">ON</a> <a href=\"/Lb\">OFF</a>");
            client.print("</body></html>");

            // La respuesta HTTP termina con otra línea en blanco:
            client.println();
            // salir del bucle while:
            break;
          } else {    // si tienes una nueva línea, limpia currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // si tienes cualquier cosa menos un retorno de carro,
          currentLine += c;      // añádelo al final de currentLine
        }

        // Comprobar si la solicitud del cliente fue para controlar los LEDs:
        if (currentLine.endsWith("GET /Hr")) {
          digitalWrite(LEDR, LOW);               // Encender LED rojo (LOW = encendido en Portenta)
        }
        if (currentLine.endsWith("GET /Lr")) {
          digitalWrite(LEDR, HIGH);              // Apagar LED rojo (HIGH = apagado en Portenta)
        }
        if (currentLine.endsWith("GET /Hg")) {
          digitalWrite(LEDG, LOW);               // Encender LED verde
        }
        if (currentLine.endsWith("GET /Lg")) {
          digitalWrite(LEDG, HIGH);              // Apagar LED verde
        }
        if (currentLine.endsWith("GET /Hb")) {
          digitalWrite(LEDB, LOW);               // Encender LED azul
        }
        if (currentLine.endsWith("GET /Lb")) {
          digitalWrite(LEDB, HIGH);              // Apagar LED azul
        }
      }
    }
    // cerrar la conexión:
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWifiStatus() {
  // imprimir el SSID de la red:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // imprimir la dirección IP de tu placa:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // imprimir la intensidad de la señal:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  
  // imprimir dónde ir en un navegador:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}