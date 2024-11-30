#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Configuración de red
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Dirección MAC del módulo Ethernet
IPAddress ip(192, 168, 0, 177);                      // IP fija para el Arduino (cámbiala según tu red)
IPAddress mqtt_server(192, 168, 0, 143);             // IP del broker MQTT (cambia según tu configuración)

// Configuración del broker MQTT
const int mqtt_port = 1883;
const char* mqtt_user = "admin";                     // Usuario MQTT
const char* mqtt_password = "admin";                 // Contraseña MQTT

// Pines controlados por botones (agrega más según sea necesario)
const int ledPin = 2;       // LED principal
const int pin3 = 3;         // Pin 3
const int pin4 = 4;         // Pin 4
const int pin5 = 5;         // Pin 5
const int pin6 = 6;         // Pin 6
int pinBuzzer = 12;         // Buzzer

// Tópicos MQTT para cada botón
const char* led_topic = "arduino/led/set";
const char* state_led_topic = "arduino/led/state";
const char* topic3 = "arduino/pin3";
const char* topic4 = "arduino/pin4";
const char* topic5 = "arduino/pin5";
const char* topic6 = "arduino/pin6";
const char* buzzer_topic = "arduino/buzzer";

// Cliente Ethernet y MQTT
EthernetClient ethClient;
PubSubClient client(ethClient);

// Función para configurar los pines como salida
void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  pinMode(pin5, OUTPUT);
  pinMode(pin6, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);

  Serial.begin(9600);

  // Iniciar Ethernet
  Ethernet.begin(mac, ip);
  Serial.print("Dirección IP: ");
  Serial.println(Ethernet.localIP());

  // Configurar el cliente MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Conectar al broker MQTT
  reconnect();
}

// Función para manejar la reconexión al broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("ArduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("Conectado!");
      // Suscribir los tópicos de cada botón
      client.subscribe(led_topic);
      client.subscribe(topic3);
      client.subscribe(topic4);
      client.subscribe(topic5);
      client.subscribe(topic6);
      client.subscribe(buzzer_topic);
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

// Función para manejar mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  // Acciones basadas en el tópico recibido
  if (String(topic) == led_topic) {
    if (message == "ON") {
      digitalWrite(ledPin, HIGH);
      client.publish(state_led_topic, "ON");
    } else if (message == "OFF") {
      digitalWrite(ledPin, LOW);
      client.publish(state_led_topic, "OFF");
    }
  } else if (String(topic) == topic3) {
    digitalWrite(pin3, message == "ON" ? HIGH : LOW);
  } else if (String(topic) == topic4) {
    digitalWrite(pin4, message == "ON" ? HIGH : LOW);
  } else if (String(topic) == topic5) {
    digitalWrite(pin5, message == "ON" ? HIGH : LOW);
  } else if (String(topic) == topic6) {
    digitalWrite(pin6, message == "ON" ? HIGH : LOW);
  } else if (String(topic) == buzzer_topic) {
    if (message == "ON") {
      tone(pinBuzzer, 440, 1000); // Reproduce un tono de 1 segundo
    } else {
      noTone(pinBuzzer); // Apaga el buzzer
    }
  }
}

// Función principal del programa
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

