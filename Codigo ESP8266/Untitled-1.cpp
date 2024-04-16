#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <HX711.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const char* ssid = "Proyectos";
const char* password = "Proyectos@2024";
const char* serverAddress = "192.168.0.104"; // Dirección IP de tu servidor local
const int serverPort = 80; // Puerto del servidor (generalmente 80 para HTTP)
const String endpoint = "/ApiSMC/guardar_datos.php"; // Ruta al script PHP en el servidor

const int DOUT = 13; // Pin D7 (DT) del HX711
const int CLK = 15;  // Pin D8 (SCK) del HX711
const int botonPin = 2; // Pin D4 para el botón
const int trigPin = 14; // Pin D5 (Trigger) del sensor HC-SR04 (cable morado)
const int echoPin = 12; // Pin D6 (Echo) del sensor HC-SR04 (cable naranja)
const int alturaFija = 210; // Altura fija del sensor ultrasonico en cm
//temperatura amarillo d2
//temperatura verde d1

HX711 balanza;

// Prototipos de funciones
void testearMLX();
float getDistance();
float obtenerTalla(float distance);
void enviarDatos(float peso, float talla, float temp_objeto);

void setup() {
  Serial.begin(9600);
  delay(100);

  // Iniciar el sensor MLX90614
  testearMLX();

  // Conexión a la red WiFi
  Serial.println();
  Serial.println("Conectando a la red WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conexión WiFi establecida");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Inicializar el sensor de la balanza
  balanza.begin(DOUT, CLK);
  pinMode(botonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT); // Configura el pin de Trigger como salida
  pinMode(echoPin, INPUT);  // Configura el pin de Echo como entrada

  Serial.println("Lectura del valor del ADC:");
  Serial.println(balanza.read());
  Serial.println("No ponga ningún objeto sobre la balanza");
  Serial.println("Destarando...");
  balanza.set_scale(22931);
  balanza.tare(20);
  Serial.println("Listo para pesar y medir distancia. Presione el botón para comenzar.");
}

void loop() {
  if (digitalRead(botonPin) == LOW) { // Comprueba si el botón está presionado
    // Medición de peso
    float sum = 0;
    int readings = 10; // Número de lecturas para promediar
    for (int i = 0; i < readings; i++) {
      sum += balanza.get_units(5);
      delay(10); // Breve pausa entre lecturas
    }
    float averageWeight = sum / readings;
    Serial.print("Peso promedio: ");
    Serial.print(averageWeight, 3);
    Serial.println(" kg");
    
    // Medición de distancia
    float distance = getDistance();
    Serial.print("Distancia: ");
    Serial.print(distance, 2);
    Serial.println(" cm");

    // Obtención de la talla
    float height = obtenerTalla(distance);
    Serial.print("Talla: ");
    Serial.print(height, 2);
    Serial.println(" cm");
    
    // Leer y mostrar las temperaturas
    Serial.print("Temperatura Ambiente= "); 
    Serial.print(mlx.readAmbientTempC()); Serial.println(" °C");
    Serial.print("Temperatura del Objeto= "); 
    float temp_objeto = mlx.readObjectTempC();
    Serial.print(temp_objeto); Serial.println(" °C"); 
    Serial.println();
    delay(100);

    // Envío de los datos a la base de datos
    if (WiFi.status() == WL_CONNECTED) {
      enviarDatos(averageWeight, height, temp_objeto);
    } else {
      Serial.println("Error: No hay conexión WiFi.");
    }

    // Espera a que se suelte el botón
    while(digitalRead(botonPin) == LOW) {
      delay(10);
    }
  }
}

void testearMLX() {
  if (!mlx.begin()) {
    Serial.println("Error: No se pudo encontrar el sensor MLX90614. Verifique las conexiones.");
    while (1);
  }
}

float getDistance() {
  // Limpia el pin Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Activa el pin Trig por 10 microsegundos
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Lee el tiempo que tarda en llegar el eco
  long duration = pulseIn(echoPin, HIGH);
  // Calcula la distancia
  float distance = duration * 0.034 / 2; // Velocidad del sonido en cm/s dividida por 2 (ida y vuelta)
  return distance;
}

float obtenerTalla(float distance) {
  float height = alturaFija - distance;
  return height;
}

void enviarDatos(float peso, float talla, float temp_objeto) {
  // Crea una instancia del cliente WiFi
  WiFiClient client;
  
  // Realiza la petición POST al servidor
  if (client.connect(serverAddress, serverPort)) {
    // Construye el cuerpo de la solicitud POST
    String postData = "peso=" + String(peso) + "&talla=" + String(talla) +
                     "&temp_objeto=" + String(temp_objeto);
    
    // Envía la solicitud POST al servidor
    client.println("POST " + endpoint + " HTTP/1.1");
    client.println("Host: " + String(serverAddress));
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);
    delay(10);
  } else {
    Serial.println("Error: No se pudo conectar al servidor.");
  }
  
  // Espera a que el servidor envíe una respuesta
  while(client.connected() && !client.available()) delay(1);
  
  // Lee la respuesta del servidor
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  // Cierra la conexión
  client.stop();
}