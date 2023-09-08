#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define SoundSensor 33 //INA:33 INB:15
int SoundValue; //音のアナログ値

const char* ssid = "AP01-01"; //無線ルーターのssidを入力
const char* password = "1qaz2wsx"; //無線ルーターのパスワードを入力

//IFTTT用
const String endpoint = "https://maker.ifttt.com/trigger/";//定型url
const String eventName = "noise_detection_app";//IFTTTのEvent Name
const String conn = "/with/key/";//定型url
const String Id = "dx8-tTZ6ajlQ_U-rQzhWoY4ueWfCTIC91FVrI6qSgn5";//自分のIFTTTのYour Key
String value ="?value1=";//値 value1=xxxx value2=xxxxx value3=xxxxx

//glide用
const char* server = "script.google.com";
const String roomNum = "101";

// センサーの感度に応じて調整
const float referenceValue = 1023.0; // センサーの最大値
const float referenceVoltage = 3.3;  // 使用しているArduinoの電圧

void setup () {
  Serial.begin (115200);
  pinMode (SoundSensor, INPUT);
}
void loop () {
  SoundValue=analogRead(SoundSensor);

  // アナログ値をデシベルに変換
  float voltage = (SoundValue / referenceValue) * referenceVoltage;
  float dB = 20 * log10(voltage); // デシベルの計算

    //騒音発生
    if(SoundValue >= 2000){

        //wifiに接続
        connectWiFi();

        if ((WiFi.status() == WL_CONNECTED)) {
            //IFTTT経由でグループラインに通知
            sendLine(dB);

            //glide用Googleスプレッドシートに通知
            sendGss(dB, roomNum);

        
        } else {
            Serial.println("UnConnected to the WiFi network");
        }

        //WiFiを切断
        WiFi.mode(WIFI_OFF);
        Serial.println("wifiを切断しました");
        delay(15000);   //30秒おきに更新

        
    }

  Serial.println(SoundValue);
  Serial.println(dB);
  delay(100);
}

//WiFiに接続
void connectWiFi(){

  Serial.println("wifiに接続します。");

  WiFi.begin(ssid, password);

  Serial.print("WiFiに接続中");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("接続しました。");
}

//IFTTT用送信
void sendLine(float dB) {
    HTTPClient http;

    value = value + String(dB);
 
    http.begin(endpoint + eventName + conn + Id + value); //URLを指定
    int httpCode = http.GET();  //GETリクエストを送信
        
    if (httpCode == 200) { //返答がある場合
        Serial.println("200.OK");
    }else {
        Serial.println("Error on HTTP request");
    }
    http.end(); //Free the resources

    value = "?value1=";
}

////glide用Googleスプレッドシート送信
void sendGss(float dB, String roomNum){

  WiFiClientSecure sslclient;

  String url = "https://script.google.com/macros/s/AKfycbwKtqBq5VsVEwFhPl6-FyZ4Ceao0M6W4AQp5o39UwlOCgWKz-kX57n0KmzcXppDnGss/exec";  //googlescript web appのurlを入力

  //urlの末尾に測定値を加筆
  url += "?";
  url += "&1_cell=";
  url += dB;
  url += "&2_cell=";
  url += roomNum;

  // サーバーにアクセス
  Serial.println("サーバーに接続中...");
  sslclient.setInsecure();//skip verification
  
  //データの送信
  if (!sslclient.connect(server, 443)) {
    Serial.println("接続に失敗しました");
    Serial.println("");//改行
    return;
  }

  Serial.println("サーバーに接続しました");

  sslclient.println("GET " + url);
  delay(1000);
  sslclient.stop();

  Serial.println("データ送信完了");
  Serial.println("");//改行

}
