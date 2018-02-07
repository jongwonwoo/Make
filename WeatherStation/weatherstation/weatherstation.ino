#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MAX7219_Dot_Matrix.h> //https://github.com/SensorsIot/MAX7219-4-digit-display-Library-for-ESP8266-

const byte chips = 4;
MAX7219_Dot_Matrix display (chips, 2);  // Chips / LOAD
unsigned long lastMoved = 0;
unsigned long MOVE_INTERVAL = 50;  // mS
int  messageOffset;
char message [] = "09:00 clear sky -99.99 / 18:00 clear sky -99.99";

const char* WIFI_SSID = "your wifi ssid";
const char* WIFI_PASS = "your wifi password";

const int UPDATE_INTERVAL_SECS = 60 * 10;
long lastDownloadUpdate = 0;

String APIKEY = "your api key";
String CityID = "your city";
int TimeZone = 9; //Seoul GMT +9
WiFiClient client;
char servername[]="api.openweathermap.org";


void setup() {
// put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("");

  display.begin ();
  
  connectWifi();
}

void loop() {
// put your main code here, to run repeatedly:

// Check if we should update weather information
  updateWeatherDataIfNeeded();

  if (millis () - lastMoved >= MOVE_INTERVAL) {
    updateDisplay ();
    lastMoved = millis ();
  }
}

// Weather Data
void updateWeatherDataIfNeeded() {
  if (lastDownloadUpdate == 0 || (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS)) {
    updateWeatherData();
    lastDownloadUpdate = millis();
  }
}

void updateWeatherData() {
  getWeatherData();
}

void getWeatherData() {
  String result = "";
  
  if (client.connect(servername, 80)) {
    String cnt = "5";
    client.println("GET /data/2.5/forecast?id="+CityID+"&units=metric&cnt="+cnt+"&APPID="+APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } 
  else {
    Serial.println("connection failed");
    Serial.println();
  }

  while(client.connected() && !client.available()) {
    delay(1); //waits for data
  }

  Serial.println("Waiting for data");

  while (client.connected() || client.available()) {
    char c = client.read();
    result = result+c;
  }

  client.stop();

  String current = currentWeather(result);
  String later = laterWeather(result);
  if (current.length() == 0 || later.length() == 0) {
    updateWeatherData();
    return;
  }
  
  String weatherMessage = current + " / " + later;
  showWeather(weatherMessage);
}

String currentWeather(String source) {
  String jsonString = firstWeatherData(source);
  return makeWeatherMessage(jsonString);
}

String laterWeather(String source) {
  String jsonString = lastWeatherData(source);
  return makeWeatherMessage(jsonString);
}

String firstWeatherData(String sourceData) {
  String oneJsonObject = "";
  
  int startIndex = sourceData.indexOf("{\"dt");
  if (startIndex >= 0) {
    int endIndex = sourceData.indexOf("},{", startIndex);
    if (endIndex >= 0) {
      endIndex += 1;
      oneJsonObject = sourceData.substring(startIndex, endIndex);
    }
  }

  return oneJsonObject;
}

String lastWeatherData(String sourceData) {
  String oneJsonObject = "";
  
  int startIndex = sourceData.lastIndexOf("{\"dt");
  if (startIndex >= 0) {
    int endIndex = sourceData.indexOf("}],\"city", startIndex);
    if (endIndex >= 0) {
      endIndex += 1;
      oneJsonObject = sourceData.substring(startIndex, endIndex);
    }
  }

  return oneJsonObject;
}

String makeWeatherMessage(String jsonString) {
  int length = jsonString.length() + 1;
  char jsonArray [length];
  jsonString.toCharArray(jsonArray,sizeof(jsonArray));

  DynamicJsonBuffer json_buf;
  JsonObject &root = json_buf.parseObject(jsonArray);
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return "";
  }

  String timeS = root["dt_txt"];
  timeS = convertGMTTimeToLocal(timeS);
  String temperature = root["main"]["temp"];
  String description = root["weather"][0]["description"];
  String idString = root["weather"][0]["id"];
  int weatherId = idString.toInt();

  String weatherMessage = timeS + " " + description + " " + temperature;

  return weatherMessage;
}

String convertGMTTimeToLocal(String timeS) {
  int length = timeS.length();
  timeS = timeS.substring(length-8,length-6);
  int time = timeS.toInt();
  time = time+TimeZone;
  if (time > 24) {
    time -= 24;
  }
  timeS = String(time)+":00";
  return timeS;
}

// Display
void showWeather(String weatherMessage) {
  setMessage(weatherMessage);
}

void setMessage(String newMessage) {
  int length = newMessage.length() + 1;
  char tempChar[length];
  newMessage.toCharArray(tempChar, length);
  strcpy(message, tempChar);
}

void updateDisplay () {
  display.sendSmooth (message, messageOffset);
  
  // next time show one pixel onwards
  if (messageOffset++ >= (int) (strlen (message) * 8)) {
    messageOffset = - chips * 8;
  }
}

// WiFi
void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already Connected");
    return;
  }
  
  Serial.println("Connecting to WiFi");
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected");
}
