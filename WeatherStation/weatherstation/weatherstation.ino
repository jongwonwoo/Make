#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MAX7219_Dot_Matrix.h> //https://github.com/SensorsIot/MAX7219-4-digit-display-Library-for-ESP8266-

const byte chips = 4;
MAX7219_Dot_Matrix display (chips, 2);  // Chips / LOAD
unsigned long lastMoved = 0;
unsigned long MOVE_INTERVAL = 50;  // mS
int  messageOffset;
char message [100];

const char* WIFI_SSID = "your wifi ssid";
const char* WIFI_PASS = "your wifi password";

const int UPDATE_INTERVAL_SECS = 60 * 10;
long lastUpdateWeatherData = 0;

const int MAX_WAIT_SECS = 15;

const int MAX_RETRY_COUNT = 3;
int retryCount = 0;

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
  if (lastUpdateWeatherData == 0 || (millis() - lastUpdateWeatherData > 1000 * UPDATE_INTERVAL_SECS)) {
    Serial.println("updateWeatherDataIfNeeded");
    lastUpdateWeatherData = millis();
    retryCount = 0;
    
    updateWeatherData();
  }
}

void retryUpdateWeatherData() {
  if (retryCount < MAX_RETRY_COUNT) {
    retryCount++;
    updateWeatherData();
  }
}

void updateWeatherData() {
  Serial.println("update data");
  String weatherData = getWeatherData();
  if (weatherData.length() == 0) {
    Serial.println("retry");
    retryUpdateWeatherData();
    return;
  }
  
  String current = currentWeather(weatherData);
  String later = laterWeather(weatherData);
  if (current.length() == 0 || later.length() == 0) {
    Serial.println("retry 2");
    retryUpdateWeatherData();
    return;
  }
  
  String weatherMessage = current + " / " + later;
  Serial.println(weatherMessage);
  showWeather(weatherMessage);
}

String getWeatherData() {
  Serial.println("Getting Weather Data");
  if (client.connect(servername, 80)) {
    String cnt = "5";
    client.println("GET /data/2.5/forecast?id="+CityID+"&units=metric&cnt="+cnt+"&APPID="+APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
    Serial.println();
    client.stop();
    return "";
  }
  
  Serial.println("Reading data");
  long waitResponse = millis();
  String result = "";
  while (client.connected()) {
    if (millis() - waitResponse < 1000 * MAX_WAIT_SECS) {
      if (client.available()) {
        result = client.readStringUntil('\r');
        Serial.println(result); 
      }  
    } else {
      Serial.println("timeout of reading data");
      client.stop();
      return "";
    }
  }

  client.stop();
  return result;
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

  Serial.println(timeS);
  Serial.println(temperature);
  Serial.println(description);
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
    Serial.println("Waiting for Connection to WiFi");
  }
  Serial.println("Connected");
}
