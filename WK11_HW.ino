#include <ArduinoJson.h>
#include <WiFi.h>

const char* ssid = "D-Link_82525589";  // insert your SSID
const char* password = "1234qwer";  // insert your password
const char* ifttt_host = "maker.ifttt.com"; //IFTTT server網址
const char* event = "line_notify";  //IFTTT事件名稱
const char* apiKey = "eS359d7Pnx5pLXzuf7uVn";  //IFTTT Applet key
const char* openweather_host = "api.openweathermap.org";
const char* resource = "/data/2.5/weather?id=1668341&appid=9ac37d88ad7dd79fde8755afd44ca6b8";
char jsonRead[600]; //讀取response後儲存JSON資料的變數，必須是全域變數

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {

  //以char*格式儲存回傳的json
  char* json = GetWeatherData();

  Serial.println(json);

  //將字串轉換為JSON，儲存在doc中
  StaticJsonDocument<600> doc;
  deserializeJson(doc, json);

  const char* location = doc["name"];
  const String weather = doc["weather"][0]["description"];
  double temp = doc["main"]["temp"];
  double humidity = doc["main"]["humidity"];

  Serial.print("*** ");
  Serial.print(location);
  Serial.println(" ***");
  Serial.print("Type: ");
  Serial.println(weather);
  Serial.print("Temp: ");
  Serial.print(temp - 273);
  Serial.println("C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.println("--------------------");

  String weatherfix = "";
  for (int i = 0; i < weather.length(); i++) {
    if (weather[i] != ' ')weatherfix += weather[i];
    else if (weather[i] == ' ')weatherfix += "%20"; //將空白轉換成"%20"，為URL中的空白字符
  }

  SendLine(String(weatherfix), String(temp - 273), String(humidity));

  delay(3000); // the OWM free plan API does NOT allow more then 60 calls per minute
}

char* GetWeatherData() {

  WiFiClient client;  //建立Client物件
  const int httpPort = 80;  //預設通訊阜80
  String JsonString = "";  //此範例不會用到

  //Client連結Server
  if (client.connect(openweather_host, httpPort)) {

    //Client傳送
    client.println(String("POST ") + resource + " HTTP/1.1");
    client.println(String("Host: ") + openweather_host);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(JsonString.length());
    client.println();
    client.println(JsonString);

    //等待5秒，每0.1秒偵測一次是否有接收到response資料
    int timeout = 0;
    while (!client.available() && (timeout++ <= 50)) {
      delay(100);
    }

    //如果無回應
    if (!client.available()) {
      Serial.println("No response...");
    }

    //Checking for the end of HTTP stream
    while (!client.find("\r\n\r\n")) {
      // wait for finishing header stream reading ...
    }

    //讀取資料並儲存在jsonRead中
    client.readBytes(jsonRead, 600);

    //停止Client
    client.stop();

    //回傳
    return jsonRead;
  }
}

void SendLine(String value1, String value2, String value3) {

  WiFiClient client;  //建立Client物件
  const int httpPort = 80;  //預設通訊阜80
  String JsonString = "";  //此範例不會用到

  //Client連結Server
  if (client.connect(ifttt_host, httpPort)) {

    //Webhook API
    String url = "/trigger/" + String(event) + "/with/key/" + String(apiKey);
    //Query String
    url += "?value1=" + value1 + "&value2=" + value2 + "&value3=" + value3;

    //Client傳送
    client.println(String("POST ") + url + " HTTP/1.1");
    client.println(String("Host: ") + ifttt_host);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(JsonString.length());
    client.println();
    client.println(JsonString);

    //等待5秒，每0.1秒偵測一次是否有接收到response資料
    int timeout = 0;
    while (!client.available() && (timeout++ <= 50)) {
      delay(100);
    }

    //如果無回應
    if (!client.available()) {
      Serial.println("No response...");
    }
    //用while迴圈一字一字讀取Response
    while (client.available()) {
      Serial.write(client.read());
    }

    //停止Client
    client.stop();
  }
}
