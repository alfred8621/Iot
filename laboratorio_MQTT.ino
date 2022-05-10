#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "ESP32_MailClient.h"
WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);
SMTPData datosSMTP;

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const char* ssid     = "UNISANGIL YOPAL";
const char* password = "";

char *server = "broker.emqx.io";
int port = 1883;

int ledpin= 26;
int fotopin=33;
bool puerta=false;

int var = 0;
int ledval = 0;
int fotoval = 0;
char datos[40];
float limite_temperatura = 30;
String resultS = "";

void wifiInit() {
    Serial.print("Conectándose a ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
        delay(500);  
    }
    Serial.println("");
    Serial.println("Conectado a WiFi");
    Serial.println("Dirección IP: ");
    Serial.println(WiFi.localIP());
  }

void callback(char* topic, byte* payload, unsigned int length) {
  String incoming ="";
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for(int i = 0; i < length; i++){
     incoming += (char)payload[i]; 
    }

    incoming.trim();

    if(String(topic)=="topic_raiz/temperatura"){
      limite_temperatura=incoming.toFloat();
      Serial.print("Limite actualizado");
      Serial.println(String(limite_temperatura));
      }

    if(String(topic)=="topic_raiz/puerta"){
      if(String(incoming)=="abrir"){
        
        Serial.println("Abriendo puerta");
        puerta=true;
        digitalWrite(32,HIGH);
        correo("La puerta ha sido abierta");
        }
        if(String(incoming)=="cerrar"){
        
        Serial.println("Cerrando puerta");
        puerta=false;
        digitalWrite(32,LOW);
        correo("La puerta ha sido cerrada");
        }

        
      limite_temperatura=incoming.toFloat();
      Serial.print("Limite actualizado");
      Serial.println(String(limite_temperatura));
      }
  char payload_string[length + 1];
  
  int resultI;

  memcpy(payload_string, payload, length);
  payload_string[length] = '\0';
  resultI = atoi(payload_string);
  
  var = resultI;

  resultS = "";
  
  for (int i=0;i<length;i++) {
    resultS= resultS + (char)payload[i];
  }
  Serial.println();
}



void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Intentando conectarse MQTT...");

    if (mqttClient.connect("arduinoClient")) {
      Serial.println("Conectado");

      mqttClient.subscribe("topic_raiz/temperatura");
      mqttClient.subscribe("topic_raiz/humedad");
      mqttClient.subscribe("topic_raiz/puerta");
      Serial.println("Suscrito correctamente");
      
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  dht.begin();
  pinMode(ledpin,OUTPUT);
  Serial.begin(115200);
  delay(10);
  wifiInit();
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);
  //correo();
}



void loop()
{
   if (!mqttClient.connected()) {
    reconnect();
  }
  temperatura();

  if(puerta){
      Serial.println("Puerta abierta");
    }
  else{
    Serial.println("Puerta cerrada");
    }
  mqttClient.loop();

  Serial.print("String: ");
  Serial.println(resultS);

  if(var == 0)
  {
  //digitalWrite(ledpin,LOW);
  }
  else if (var == 1)
  {
  //digitalWrite(ledpin,HIGH);
  }

  //fotoval = analogRead(fotopin);
  //Serial.print("temperatura: ");
  //Serial.println(fotoval);
  
  sprintf(datos, "Valor temperatura: %d ", fotoval);
  mqttClient.publish("salida",datos);
  delay(5000);
  
  
}

void correo(String mensaje){
//Configuración del servidor de correo electrónico SMTP, host, puerto, cuenta y contraseña
datosSMTP.setLogin("smtp.gmail.com", 465, "jltinjaca@gmail.com", "pjqralrzzjxtkdcw");
// Establecer el nombre del remitente y el correo electrónico
datosSMTP.setSender("ESP32S", "electivaiotunisangil@gmail.com");
// Establezca la prioridad o importancia del correo electrónico High, Normal, Low o 1 a 5 (1 es el más alto)
datosSMTP.setPriority("High");
// Establecer el asunto
datosSMTP.setSubject("Probando envio de correo con ESP32");
// Establece el mensaje de correo electrónico en formato de texto (sin formato)
//datosSMTP.setMessage("Hola soy el esp32s! y me estoy comunicando contigo", false);
datosSMTP.setMessage(mensaje, false);
// Agregar destinatarios, se puede agregar más de un destinatario
datosSMTP.addRecipient("alfredmpiga@unisangil.edu.co");
 //Comience a enviar correo electrónico.
if (!MailClient.sendMail(datosSMTP))
Serial.println("Error enviando el correo, " + MailClient.smtpErrorReason());
//Borrar todos los datos del objeto datosSMTP para liberar memoria
datosSMTP.empty();
delay(10000);
digitalWrite(23, LOW);
}
void temperatura(){
  Serial.println("Leyendo temperatura");
      delay(1000);
    float h = dht.readHumidity();
    delay(1000);
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;}

    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print("% Temperatura: ");
    Serial.print(t);
    Serial.println("°C ");

    if(t>limite_temperatura){
        digitalWrite(26, HIGH);
      }else{
        digitalWrite(26, LOW);
        }
  }
