#include <Arduino.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
LiquidCrystal_I2C lcd(0x27,16,2);

#define LINE_TOKEN "?"

WiFiClientSecure client;
HTTPClient http;

IRAM_ATTR void handleInterrupt();

const int command_wait = 0;
const int product_count = 1;
const int product_count_pass = 2;
const int product_count_complete = 3;

bool isCheck= false;  

int flag = false;
int ob = 0;
int count = 0;
String command;
int state;
unsigned long previousMillis = 0; // เวลาเริ่มต้น
const long interval = 10000; 
unsigned long currentMillis =0;

String name;
int id,total;
int totalstart;

// MQTT Broker settings
const char *mqtt_broker = "broker.emqx.io";   // EMQX broker endpoint
const char *mqtt_topics[] = {"mqtt_in"}; // Array of MQTT topics
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
String mqttMessage;

void setupWiFiManager();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);

JsonDocument doc_pub, doc_sub;
char jsonBuffer[512];

void sendLineNotification(String message)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    client.setInsecure(); // สำหรับการเชื่อมต่อ HTTPS แบบไม่ตรวจสอบใบรับรอง
    http.begin(client, "https://notify-api.line.me/api/notify");
 
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Bearer " + String(LINE_TOKEN));
 
    String payload = "message=" + message;
    int httpResponseCode = http.POST(payload);
 
    if (httpResponseCode > 0)
    {
      Serial.print("LINE Notify Response Code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error sending LINE Notify: ");
      Serial.println(httpResponseCode);
    }
 
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}
void sendLineNotificationValue(String varName, int count)
{
  String strValue = String(count); // แปลงจาก int เป็น String
  {
    sendLineNotification(varName + " = " + strValue);
  }
}

void sendLineNotificationTotal(String varName, int count)
{
  String strValue = String(count); // แปลงจาก int เป็น String
  {
    sendLineNotification("ชื่อสินค้า " + varName + " จำนวนที่นับได้ล่าสุด " + strValue + " ชิ้น");
  }
}

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    setupWiFiManager();
    sendLineNotification("เครื่องนับถูกเปิดการใช้งาน");
    pinMode(D3,INPUT);
    pinMode(D8, INPUT);
    pinMode(D5,OUTPUT);
    pinMode(D6,OUTPUT);
    pinMode(D7, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(D8), handleInterrupt, RISING);
    state = command_wait;
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTTBroker();
}

void loop() {
    if(state == command_wait){
        // Serial.println(state);
        // Serial.println(isCheck);
        // delay(1000);
        digitalWrite(D6, 0);
        digitalWrite(D5, 1);
        mqtt_client.loop();
        if(isCheck == true){
            deserializeJson(doc_sub, mqttMessage);
            int doc_sub_id = doc_sub["id"];
            const char* doc_sub_name = doc_sub["product_name"];
            int doc_sub_quantity = doc_sub["quantity"];

            id = doc_sub_id;
            name = doc_sub_name;
            total = doc_sub_quantity;
            totalstart = total;
            if(doc_sub_name != NULL && doc_sub_quantity >0){
                previousMillis = millis(); // เริ่มนับเวลาเมื่อเปลี่ยนสถานะเป็น product_count
                tone(D7, 1000);
                digitalWrite(D6, 1);
                digitalWrite(D5, 0); 
                sendLineNotification("-----------------------------");
                sendLineNotification("เริ่มนับ " + name);
                sendLineNotificationValue("จำนวนสินค้าเริ่มต้นนับ ", total);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("product : ");
                lcd.print(name);
                lcd.setCursor(0,1);
                lcd.print("amount : ");
                lcd.print(total);
                noTone(D7);
                state = product_count;
            }
        }
    }else if(state == product_count){
        // delay(1000);
        // Serial.println(state);
        // Serial.println(isCheck);
    mqtt_client.loop();
    ob = digitalRead(D3);
    currentMillis = millis();

    if (flag == true){
         flag = false;
         delay(50);
        if(digitalRead(D8==1)){
            sendLineNotification("มีการกดปุ่มหยุดการนับ");
            doc_pub["command"] = "นับเสร็จเเล้ว";
        serializeJson(doc_pub, jsonBuffer);
        mqtt_client.publish("mqtt_count", jsonBuffer);
            state = product_count_complete;
      }
    }
    
    // ถ้า object ถูกตรวจจับ
    if(ob == 0){
      total = total + 1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("product : ");
      lcd.print(name);
      lcd.setCursor(0,1);
      lcd.print("amount : ");
      lcd.print(total);
      doc_pub["id"] = id;
        doc_pub["product_name"] = name;
        doc_pub["quantity"] = total;
        doc_pub["command"] = "กำลังนับ";
        serializeJson(doc_pub, jsonBuffer);
        mqtt_client.publish("mqtt_count", jsonBuffer);
    //   Serial.print(command);
    //   Serial.print(" :");
    //   Serial.print(count);
    //   Serial.print(" ชิ้น\n");
      state = product_count_pass;
    }
    
    if(currentMillis - previousMillis >= interval && ob == 1) {
      previousMillis = currentMillis; // รีเซ็ตเวลาใหม่เมื่อครบ 10 วินาที
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Time out!");
      sendLineNotification("ไม่มีการนับสินค้าในเวลาที่กำหนด");
      doc_pub["command"] = "นับเสร็จเเล้ว";
        serializeJson(doc_pub, jsonBuffer);
        mqtt_client.publish("mqtt_count", jsonBuffer);
            state = product_count_complete;
      delay(200);
      state = product_count_complete;
    }
  } else if(state == product_count_pass){
    ob = digitalRead(D3);
    tone(D7, 1000);
    
    // ถ้าสินค้าผ่านไปแล้ว
    if(ob == 1){
      noTone(D7);
      sendLineNotificationValue(name, total);
      previousMillis = millis(); // รีเซ็ตเวลาใหม่เมื่อกลับมา product_count
      state = product_count; // กลับไปที่สถานะ product_count
    }
  } else if(state == product_count_complete){
    // Serial.println(state);
    // Serial.println(isCheck);
    // delay(1000);
    mqtt_client.loop();
    sendLineNotification("นับสินค้าเสร็จเรียบร้อย");
    sendLineNotificationTotal(name, total);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("command finish");
    delay(500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("waiting command");
    sendLineNotification("-----------------------------");
    sendLineNotification("สั่งนับสินค้าถัดไปได้เลย");
        doc_pub["id"] = id;
        doc_pub["product_name"] = name;
        doc_pub["quantity"] = total;
        doc_pub["command"] = "นับเสร็จเเล้ว";
        serializeJson(doc_pub, jsonBuffer);
        mqtt_client.publish("mqtt_out", jsonBuffer);
    count = 0;
    isCheck= false;
    state = command_wait;
  }
}

void setupWiFiManager() {
    WiFiManager wm;
    bool res = wm.autoConnect("esp8266_fifa"); // anonymous ap

    if (!res) {
        lcd.setCursor(0,0);
        lcd.print("No WiFi");
    } else {
        sendLineNotification("อุปกรณ์นี้เชื่อมต่อ WiFi แล้ว");
        lcd.setCursor(0,0);
        lcd.print("waiting command");
    }
}

void connectToMQTTBroker() {
    while (!mqtt_client.connected()) {
        String client_id = "esp8266-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        
        if (mqtt_client.connect(client_id.c_str())) {
            Serial.println("Connected to MQTT broker");

            // Subscribe to each topic in the array
            for (const char* topic : mqtt_topics) {
                mqtt_client.subscribe(topic);
                Serial.printf("Subscribed to topic: %s\n", topic);
            }

            // Publish message upon successful connection
            //mqtt_client.publish(mqtt_topics[2], "Hi EMQX I'm ESP8266 ^^");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    isCheck = true ;
    mqttMessage = ""; // รีเซ็ต mqttMessage เป็นสตริงว่าง
    for (unsigned int i = 0; i < length; i++) {
        mqttMessage += (char)payload[i];  // แปลง *byte เป็น string
    }
    Serial.println(mqttMessage);
}

IRAM_ATTR void handleInterrupt() {
  flag = true;
}
