#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#pragma region pinDefinition
#define trigPin 13
#define echoPin 12
#define trigPin2 11
#define echoPin2 10
#define trigPin3 9
#define echoPin3 8
#define btnPin 2
#define ledGoldPin 3
#define ledSilverPin 4
#define ledBronzePin 5
#define buzzerPin 7
#pragma endregion pinDefinition

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#pragma region internetDefinition
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74, 125, 232, 128); // numeric IP for Google (no DNS)
char testServer[] = "www.google.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Variables to measure the internet speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true; // set to false for better speed measurement

#pragma endregion internetDefinition

#pragma region internalVariables
// extern volatile unsigned long timer0_millis;
int points;
int buttonState;
bool started;
unsigned long previousMillis = 0; // last time update
long intervalGold = 60000;        //60000;
long intervalSilver = 80000;      // 80000;
long intervalBronze = 100000;      // 100000;
int currentRank = 0;              // 0 = Gold, 1 = Silver, 2 = Bronze
#pragma endregion internalVariables

void setup()
{
//nastaveni serial connection
#pragma region serialSetup
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#pragma endregion serialSetup

//nastaveni pinu
#pragma region pinSettings
  pinMode(btnPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
  pinMode(ledGoldPin, OUTPUT);
  pinMode(ledSilverPin, OUTPUT);
  pinMode(ledBronzePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH);
#pragma endregion pinSettings

//nastaveni internetu
#pragma region internetCheck
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true)
      {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  }
  else
  {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(testServer);
  Serial.println("...");

  // if you get a connection, report back via serial:
  if (client.connect(testServer, 80))
  {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  }
  else
  {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  beginMicros = micros();

#pragma endregion internetCheck

//TODO: tady je potreba nacist informace pro jak dlouho je ktery interval z api
#pragma region serverComm

#pragma endregion serverComm

//uvodni nastaveni displeje
#pragma region displaySetup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  displayText("------");
#pragma endregion displaySetup
}

void loop()
{

#pragma region rankTimingSetup
  unsigned long currentMillis = millis();
  if (started == false)
  {
    previousMillis = currentMillis;
  }

  long interval = currentMillis - previousMillis;
  // Serial.print(String(currentRank));
  // Serial.print(" ");
  // Serial.println(String(interval));

  if (currentRank == 0)
  {
    if (interval > intervalGold)
    {
      currentRank = 1;
      beep(200);
      digitalWrite(ledGoldPin, LOW);
      digitalWrite(ledSilverPin, HIGH);
    }
  }

  if (currentRank == 1)
  {
    if (interval > intervalSilver)
    {
      currentRank = 2;
      beep(200);
      digitalWrite(ledSilverPin, LOW);
      digitalWrite(ledBronzePin, HIGH);
    }
  }

  if (currentRank == 2)
  {
    if (interval > intervalBronze)
    {
      currentRank = 2;
      digitalWrite(ledBronzePin, LOW);
      beep(200);
      beep(200);
      beep(200);
      displayText(String("LOSS"));
      started = false;
    }
  }
#pragma endregion rankTimingSetup

#pragma region start
  buttonState = digitalRead(btnPin);
  if (buttonState == HIGH && started == false)
  {
    // startGame:
    beep(50);
    Serial.println("Started");
    started = true;
    digitalWrite(ledGoldPin, HIGH);
    points = 0;
    previousMillis = currentMillis;
    currentRank = 0;
  }
#pragma endregion start

#pragma region score
  if (started == true)
  {
    long duration, distance;
    //prvni senzor
    digitalWrite(trigPin, LOW); // Added this line
    delayMicroseconds(2);       // Added this line
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;
    if (distance < 5)
    { // This is where the LED On/Off happens
      Serial.print("Scored  ");
      Serial.println(distance);
      points++;
      displayText(String(points));
      delay(1000);
    }

    //druhy senzor
    // digitalWrite(trigPin2, LOW); // Added this line
    // delayMicroseconds(2);        // Added this line
    // digitalWrite(trigPin2, HIGH);
    // delayMicroseconds(10); // Added this line
    // digitalWrite(trigPin2, LOW);
    // duration = pulseIn(echoPin2, HIGH);
    // distance = (duration / 2) / 29.1;
    // if (distance < 5)
    // { // This is where the LED On/Off happens
    //   Serial.print("Scored 2  ");
    //   Serial.println(distance);
    //   points++;
    //   displayText(String(points));
    //   delay(1000);
    // }

    //treti senzor
    digitalWrite(trigPin3, LOW); // Added this line
    delayMicroseconds(2);        // Added this line
    digitalWrite(trigPin3, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin3, LOW);
    duration = pulseIn(echoPin3, HIGH);
    distance = (duration / 2) / 29.1;
    if (distance < 5)
    { // This is where the LED On/Off happens
      Serial.print("Scored 3   ");
      Serial.println(distance);
      points++;
      displayText(String(points));
      delay(1000);
    }
    displayText(String(points));
  }
#pragma endregion score

  delay(50);
}

#pragma region utilityFunctions
//buzzer sound
void beep(unsigned char delayms)
{
  digitalWrite(buzzerPin, LOW);
  delay(delayms);
  digitalWrite(buzzerPin, HIGH);
  delay(delayms);
}

//display text
void displayText(String text)
{
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(text);
  display.display();
}
#pragma endregion utilityFunctions
