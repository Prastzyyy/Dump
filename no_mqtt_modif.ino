#include <ArduinoOTA.h>
#include <Ticker.h>
#include <WiFi.h>         //esp32
//#include <ESP8266WiFi.h>
#include <HTTPClient.h>   //esp32
//#include <ESP8266HTTPClient.h>  
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "fuzzy_function.h"

// Sistem OTA
const char* ota_password = "123";
String OTA_msg = "cuy";

// Google Apps Script 
String Web_App_URL = "https://script.google.com/macros/s/AKfycbwVtR04NDLnKFMrjlx2EHCEvLHktAMAuHcxUE4rBzrnD1UwdEL0UoegfaXF4j328ZQHUQ/exec";
String status;
String warning;
int dataSpreadsheet;
void spreadsheet();

// MQTT
const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* mqtt_server = "ee.unsoed.ac.id";
//const char* mqtt_server = "broker.hivemq.com";

const char* mqtt_user = "123"; 
const char* mqtt_password = "123"; 

const char* SUBTOPIC_SETJAM = "greenhouse/input/setjam";
const char* SUBTOPIC_SETMENIT = "greenhouse/input/setmenit";
const char* SUBTOPIC_SETPH = "greenhouse/input/setph";
const char* SUBTOPIC_SETPPM = "greenhouse/input/setppm";
const char* SUBTOPIC_PENYIRAM = "greenhouse/input/penyiram";
const char* SUBTOPIC_MIXER = "greenhouse/input/mixer";
const char* SUBTOPIC_PHUP = "greenhouse/input/phup";
const char* SUBTOPIC_PHDOWN = "greenhouse/input/phdown";
const char* SUBTOPIC_NUTRISI = "greenhouse/input/nutrisi";
const char* SUBTOPIC_AMBILSAMPEL = "greenhouse/input/ambilsampel";
const char* SUBTOPIC_BUANGSAMPEL = "greenhouse/input/buangsampel";
const char* SUBTOPIC_BUKATANGKI = "greenhouse/input/bukatangki";
const char* SUBTOPIC_MAINTASK = "greenhouse/input/maintask";  
const char* SUBTOPIC_OTOMATISPH = "greenhouse/input/otomatisph";  
const char* SUBTOPIC_OTOMATISNUTRISI = "greenhouse/input/otomatisnutrisi"; 
const char* SUBTOPIC_GANTISAMPEL = "greenhouse/input/gantisampel";  
const char* SUBTOPIC_MODEOTOMATIS = "greenhouse/input/modeotomatis";
const char* SUBTOPIC_DATASPREADSHEET = "greenhouse/input/dataspreadsheet";
const char* SUBTOPIC_TEST = "greenhouse/input/test";

const char* PUBTOPIC_PH = "greenhouse/output/ph";
const char* PUBTOPIC_PPM = "greenhouse/output/ppm";
const char* PUBTOPIC_TINGGI = "greenhouse/output/tinggi";
const char* PUBTOPIC_SUHU = "greenhouse/output/suhu";
const char* PUBTOPIC_WAKTU = "greenhouse/output/waktu";
const char* PUBTOPIC_AKTUATOR1 = "greenhouse/output/aktuator1";
const char* PUBTOPIC_AKTUATOR2 = "greenhouse/output/aktuator2";
const char* PUBTOPIC_AKTUATOR3 = "greenhouse/output/aktuator3";
const char* PUBTOPIC_AKTUATOR4 = "greenhouse/output/aktuator4";
const char* PUBTOPIC_AKTUATOR5 = "greenhouse/output/aktuator5";
const char* PUBTOPIC_AKTUATOR6 = "greenhouse/output/aktuator6";
const char* PUBTOPIC_AKTUATOR7 = "greenhouse/output/aktuator7";
const char* PUBTOPIC_AKTUATOR8 = "greenhouse/output/aktuator8";
const char* PUBTOPIC_OTA = "greenhouse/output/OTA";
const char* PUBTOPIC_WARNING = "greenhouse/output/warning";
const char* PUBTOPIC_STATUS_SISTEM = "greenhouse/output/status";
const char* PUBTOPIC_ALARM = "greenhouse/output/alarm";

// Koneksi WiFi
//const char* ssid ="Buoyed";
//const char* password = "TranquilBegetsEnlightment";
const char* ssid ="KEDAIREKA";
const char* password = "Unsoed1963";
//const char* ssid ="Prastzy.net";
//const char* password = "123456781";

// Deklarasi Pin
#define ONE_WIRE_BUS 13
#define pin_pH 36 
#define pin_EC 39 
#define PIN_TRIG 5
#define PIN_ECHO 18
#define relay_pHup 12
#define relay_pHdn 14
#define relay_nutrisi 15
#define relay_mixer 4 
#define relay_ambilSampel 27  
#define relay_buangSampel 26
#define relay_penyiram 16
#define relay_tangki 0

// Deklarasi Object
Ticker ticker1, ticker2;
WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27,20,4);
RTC_DS1307 rtc;

// Adjust nilai variabel
int setWaktu [] = {8, 0}; //Waktu penyiraman
int delay_pompaSampel;
int delay_mixer;
int delay_penyiram;

char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String aktuator1 = "Padam", aktuator2 = "Padam", aktuator3 = "Padam", aktuator4 = "Padam";
String aktuator5 = "Padam", aktuator6 = "Padam", aktuator7 = "Padam", aktuator8 = "Padam";
float set_jarak = 100, set_pH = 7, set_PPM = 800;
String fuzzyph, fuzzyppm;
float voltagepH, voltageEC, pH, PPM, suhu, tinggi_cm, jarak_cm, nilai_tinggi;
int adcValuepH, adcValueEC;
long duration;
String alarm1 ;
bool kondisi1 = false;
String statusSistem = "Mode Manual";
bool saklar_tangki = false;
float kalibrasi_suhu = -0.3;
// Kalibrasi (berdasarkan rumus y = 0.0146x - 12.1996)
float m1 = 0.0146;  // Kemiringan
float b1 = -12.1996;  // Offset
float voltageph2, voltageph1, inputph2, inputph1, input_pH, input_PPM;
String hasilkalibrasiph; 
float m2 = 0.00585;
float b2 = 0.59305; 
float voltageec2, voltageec1, inputec2, inputec1;
String hasilkalibrasiec; 
int input_tinggi;
bool mixeron = false;
unsigned long previousMillis = 0;

// Deklarasi Prosedur
void mainTask();
void baca_pH();
void baca_PPM();
void baca_jarak();
void baca_suhu();
void kontrol_pH();
void kontrol_PPM();
void kontrol_tinggi();
void baca_RTC();
void outputFuzzy();
void setup_wifi();
void reconnect();
void callback(char *topic, byte *payload, unsigned int length);
void monitoring ();
void gantiSampel();
void mixer();
void penyiram();

void setup() {
  //analogReadResolution(12);
  //analogSetAttenuation(ADC_11db);
  pinMode(pin_pH, INPUT);
  pinMode(pin_EC, INPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(relay_pHup, OUTPUT);
  pinMode(relay_pHdn, OUTPUT);
  pinMode(relay_nutrisi, OUTPUT);
  pinMode(relay_mixer, OUTPUT);
  pinMode(relay_penyiram, OUTPUT);
  pinMode(relay_ambilSampel, OUTPUT);
  pinMode(relay_buangSampel, OUTPUT); 
  pinMode(relay_tangki, OUTPUT); 
  
  Serial.begin(115200);
  sensors.begin();
  
  // RTC
  if (! rtc.begin()) {
    Serial.println("RTC tidak ditemukan");
    while (1);
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC tidak berjalan, setting waktu...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set waktu sesuai waktu kompilasi
  }
  
  // LCD Setup   
  lcd.begin();  
  lcd.backlight();
  lcd.setCursor(3,1);
  lcd.print("START  PROGRAM");
  lcd.setCursor(3,2);
  lcd.print("==============");
  delay(1000);
  lcd.clear();

  setup_wifi();
  ArduinoOTA.setHostname("esp32 - Nutrisi Otomatis");
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin();  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  ticker1.attach(2, monitoring);
}

void loop() { 
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
  // Mode Manual/Otomatis
  if(kondisi1) { 
    /*
    DateTime now = rtc.now();
    if (now.hour() == setWaktu[0] && now.minute() == setWaktu[1] && now.second() == setWaktu[2] ){
      mainTask();
    }  
    */
  }
  yield();
}

void callback(char *topic, byte *payload, unsigned int length) {
  baca_pH();
  outputFuzzy();
  baca_jarak();
  Serial.print("Receive Topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';  
  Serial.println(msg);
  //===================================================================
  // kontrol Manual Penyiram
  if (!strcmp(topic, SUBTOPIC_PENYIRAM)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_penyiram, HIGH);
      Serial.println("Penyiram manual ON");
      aktuator5 = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_penyiram, LOW);
      Serial.println("Penyiram manual OFF");
      aktuator5 = "Padam";
    }
  }
  // kontrol Manual Mixer
  if (!strcmp(topic, SUBTOPIC_MIXER)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_mixer, HIGH);
      Serial.println("Mixer Manual ON");
      aktuator4 = "Menyala";
      mixeron = true;
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_mixer, LOW);
      Serial.println("Mixer Manual OFF");
      aktuator4 = "Padam";
    }
  }
  // kontrol Manual pH Up
  if (!strcmp(topic, SUBTOPIC_PHUP)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_pHup, HIGH);
      Serial.println("Pompa pH Up Manual ON");
      aktuator6 = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_pHup, LOW);
      Serial.println("Pompa pH Up Manual OFF");
      aktuator6 = "Padam";
    }
  }
  // kontrol Manual pH Down
  if (!strcmp(topic, SUBTOPIC_PHDOWN)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_pHdn, HIGH);
      Serial.println("Pompa pH Down Manual ON");
      aktuator7 = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_pHdn, LOW);
      Serial.println("Pompa pH Down Manual OFF");
      aktuator7 = "Padam";
    }
  }
  // kontrol Manual Nutrisi
  if (!strcmp(topic, SUBTOPIC_NUTRISI)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_nutrisi, HIGH);
      Serial.println("Pompa Nutrisi Manual ON");
      aktuator8 = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_nutrisi, LOW);
      Serial.println("Pompa Nutrisi Manual OFF");
      aktuator8 = "Padam";
    }
  }
  // kontrol Manual Ambil Sampel
  if (!strcmp(topic, SUBTOPIC_AMBILSAMPEL)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_ambilSampel, HIGH);
      Serial.println("Pompa Ambil Sampel Manual ON");
      aktuator2 = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_ambilSampel, LOW);
      Serial.println("Pompa Ambil Sampel Manual OFF");
      aktuator2 = "Padam";
    }
  }
  // kontrol Manual Buang Sampel
  if (!strcmp(topic, SUBTOPIC_BUANGSAMPEL)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_buangSampel, HIGH);
      Serial.println("Pompa Buang Sampel Manual ON");
      aktuator3 = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_buangSampel, LOW);
      Serial.println("Pompa Buang Sampel Manual OFF");
      aktuator3 = "Padam";
    }
  }
  // kontrol Manual buka tangki
  if (!strcmp(topic, SUBTOPIC_BUKATANGKI)) {
    if (!strncmp(msg, "on", length)) {
      digitalWrite(relay_tangki, HIGH);
      Serial.println("Isi tangki Manual ON");
      saklar_tangki = true;
      aktuator1 = "Menyala";
    } else if (!strncmp(msg, "off", length)) {
      digitalWrite(relay_tangki, LOW);
      Serial.println("Isi tangki Manual OFF");
      saklar_tangki = false;
      aktuator1 = "Padam";
    }
  }
  //===================================================================
  
  // kontrol Otomatisasi mainTask
  if (!strcmp(topic, SUBTOPIC_MAINTASK)) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengaktifkan kontrol otomatis keseluruhan");
      mainTask();
    } 
  }
  // kontrol Otomatisasi pH
  if (!strcmp(topic, SUBTOPIC_OTOMATISPH)) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengaktifkan kontrol otomatis pH");
      kontrol_pH();
    }
  }
  // kontrol Otomatisasi Nutrisi
  if (!strcmp(topic, SUBTOPIC_OTOMATISNUTRISI)) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengaktifkan kontrol otomatis Nutrisi");
      kontrol_PPM();
    } 
  }
  // kontrol Otomatisasi Ganti Sampel
  if (!strcmp(topic, SUBTOPIC_GANTISAMPEL)) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengaktifkan kontrol otomatis Ganti Sampel");
      gantiSampel();
    }
  }
  // Mode Otomatis
  if (!strcmp(topic, SUBTOPIC_MODEOTOMATIS)) {
    if (!strncmp(msg, "on", length)) {
      kondisi1 = true;
      statusSistem = "Otomatis";
      client.publish(PUBTOPIC_STATUS_SISTEM, statusSistem.c_str()); 
    } else if (!strncmp(msg, "off", length)) {
      kondisi1 = false;
      statusSistem = "Manual";
      client.publish(PUBTOPIC_STATUS_SISTEM, statusSistem.c_str()); 
    }
  }

  //=======================================================================
  // Set Jam
  if (!strcmp(topic, SUBTOPIC_SETJAM)) {
    setWaktu[0] = atoi(msg); 
    Serial.print("Set Jam menjadi : ");
    Serial.println(setWaktu[0]);
    alarm1 = String(setWaktu[0] + " : " + String(setWaktu[1]));
    client.publish(PUBTOPIC_ALARM, alarm1.c_str());
  }

  // Set Menit
  if (!strcmp(topic, SUBTOPIC_SETMENIT)) {
    setWaktu[1] = atoi(msg); 
    Serial.print("Set Menit menjadi : ");
    Serial.println(setWaktu[1]);
    alarm1 = String(setWaktu[0]) + " : " + String(setWaktu[1]);
    client.publish(PUBTOPIC_ALARM, alarm1.c_str());
  }
  
  // Set pH
  if (!strcmp(topic, SUBTOPIC_SETPH)) {
    set_pH = atof(msg); 
    Serial.print("Set pH menjadi : ");
    Serial.println(set_pH);
  }

  // Set PPM
  if (!strcmp(topic, SUBTOPIC_SETPPM)) {
    set_PPM = atof(msg); 
    Serial.print("Set PPM menjadi : ");
    Serial.println(set_PPM);
  }
  // Set delay pompa sampel
  if (!strcmp(topic, "greenhouse/input/delaypompa")) {
    delay_pompaSampel = atof(msg); 
    Serial.print("Set delay pompa sampel menjadi : ");
    Serial.println(delay_pompaSampel);
  }
  // Set delay mixer
  if (!strcmp(topic, "greenhouse/input/delaymixer")) {
    delay_mixer = atof(msg); 
    Serial.print("Set delay mixer menjadi : ");
    Serial.println(delay_mixer);
  }
  // Set delay pompa sampel
  if (!strcmp(topic, "greenhouse/input/delaypenyiram")) {
    delay_penyiram = atof(msg); 
    Serial.print("Set delay penyiram menjadi : ");
    Serial.println(delay_penyiram);
  }
  
  // Ambil data Spreadsheet
  if (!strcmp(topic, SUBTOPIC_DATASPREADSHEET)) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengambil data untuk spreadsheet");
      if(mixeron){
        status = "sesudah";
      }else{
        status = "sebelum";
      }
      spreadsheet();
      mixeron = false;
    }
  }
  // Input pH
  if (!strcmp(topic, "greenhouse/input/inputph")) {
    input_pH = atof(msg); 
    baca_pH();
    outputFuzzy();
    baca_jarak();
    if(input_pH < set_pH){
      fuzzyph = "Pompa pH Up akan menyala selama : " + String(lamaPompa_pH*(tinggi_cm/set_jarak)) + " detik";
    }else{
      fuzzyph = "Pompa pH Down akan menyala selama : " + String(lamaPompa_pH*(tinggi_cm/set_jarak)) + " detik";
    }
    Serial.println(fuzzyph);
    client.publish ("greenhouse/output/fuzzyph", fuzzyph.c_str());
    Serial.print("Input pH menjadi : ");
    Serial.println(input_pH);
    
  }
  // Input PPM
  if (!strcmp(topic, "greenhouse/input/inputppm")) {
    input_PPM = atof(msg); 
    baca_PPM();
    outputFuzzy();
    baca_jarak();
    fuzzyppm = "Pompa Nutrisi akan menyala selama : " + String(lamaPompa_PPM*(tinggi_cm/set_jarak)) + " detik";
    Serial.println(fuzzyppm);
    client.publish ("greenhouse/output/fuzzyppm", fuzzyppm.c_str());
    Serial.print("Input PPM menjadi : ");
    Serial.println(input_PPM);
  }
  // Input Tinggi
  if (!strcmp(topic, "greenhouse/input/inputtinggi")) {
    input_tinggi = atoi(msg); 
    baca_PPM();
    baca_pH();
    outputFuzzy();
    baca_jarak();
    if(input_pH < set_pH){
      fuzzyph = "Pompa pH Up akan menyala selama : " + String(lamaPompa_pH*(tinggi_cm/set_jarak)) + " detik";
    }else{
      fuzzyph = "Pompa pH Down akan menyala selama : " + String(lamaPompa_pH*(tinggi_cm/set_jarak)) + " detik";
    }
    fuzzyppm = "Pompa Nutrisi akan menyala selama : " + String(lamaPompa_PPM*(tinggi_cm/set_jarak)) + " detik";
    Serial.println(fuzzyph);
    client.publish ("greenhouse/output/fuzzyph", fuzzyph.c_str());
    Serial.println(fuzzyppm);
    client.publish ("greenhouse/output/fuzzyppm", fuzzyppm.c_str());
    Serial.print("Input Tinggi menjadi : ");
    Serial.println(input_tinggi);
  }
  //==========================================================
  //KALIBRASI SENSOR PH
  // Input nilai larutan pH ke 1
  if (!strcmp(topic, "greenhouse/input/inputph1")) {
    inputph1 = atof(msg); 
    Serial.print("Input nilai larutan pH ke 1  : ");
    Serial.println(inputph1);
  }
  // Input nilai larutan pH ke 2
  if (!strcmp(topic, "greenhouse/input/inputph2")) {
    inputph2 = atof(msg); 
    Serial.print("Input nilai larutan pH ke 2 : ");
    Serial.println(inputph2);
  }
  // voltage ph ke 1
  if (!strcmp(topic, "greenhouse/input/voltageph1")) {
    voltageph1 = atof(msg); 
    Serial.print("Input nilai voltage pH ke 1 : ");
    Serial.println(voltageph1);
  }
  // voltage ph ke 2
  if (!strcmp(topic, "greenhouse/input/voltageph2")) {
    voltageph2 = atof(msg); 
    Serial.print("Input nilai voltage pH ke 2 : ");
    Serial.println(voltageph2);
  }
  // saklar kalibrasi pH
  if (!strcmp(topic, "greenhouse/input/kalibrasiph")) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengkalibrasi sensor pH");
      m1 = (inputph2 - inputph1)/(voltageph2 - voltageph1);
      b1 = inputph1 - (voltageph1 * m1);
      char m1Str[10]; char b1Str[10];
      dtostrf(m1, 1, 5, m1Str);
      dtostrf(b1, 1, 5, b1Str);
      hasilkalibrasiph = "y = (" + String(m1Str)+ ")x + (" + String(b1Str) + ")";
      Serial.println(hasilkalibrasiph);
    }
  }
  //==============================================================
  // KALIBRASI SENSOR SUHU
  if (!strcmp(topic, "greenhouse/input/kalibrasisuhu")) {
    kalibrasi_suhu = atof(msg); 
    Serial.print("Set kalibrasi Suhu menjadi : ");
    Serial.println(kalibrasi_suhu);
    client.publish("greenhouse/output/kalibrasisuhu", String(kalibrasi_suhu).c_str());
  }
  //==========================================================
  //KALIBRASI SENSOR EC
  // Input nilai larutan EC ke 1
  if (!strcmp(topic, "greenhouse/input/inputec1")) {
    inputec1 = atof(msg); 
    Serial.print("Input nilai larutan EC ke 1  : ");
    Serial.println(inputec1);
  }
  // Input nilai larutan EC ke 2
  if (!strcmp(topic, "greenhouse/input/inputec2")) {
    inputec2 = atof(msg); 
    Serial.print("Input nilai larutan EC ke 2 : ");
    Serial.println(inputec2);
  }
  // voltage EC ke 1
  if (!strcmp(topic, "greenhouse/input/voltageec1")) {
    voltageec1 = atof(msg); 
    Serial.print("Input nilai voltage EC ke 1 : ");
    Serial.println(voltageec1);
  }
  // voltage EC ke 2
  if (!strcmp(topic, "greenhouse/input/voltageec2")) {
    voltageec2 = atof(msg); 
    Serial.print("Input nilai voltage EC ke 2 : ");
    Serial.println(voltageec2);
  }
  // saklar kalibrasi EC
  if (!strcmp(topic, "greenhouse/input/kalibrasiec")) {
    if (!strncmp(msg, "on", length)) {
      Serial.println("Mengkalibrasi sensor EC");
      m2 = (inputec2 - inputec1)/(voltageec2 - voltageec1);
      b2 = inputec1 - (voltageec1 * m2);
      char m2Str[10]; char b2Str[10];
      dtostrf(m2, 1, 5, m2Str);
      dtostrf(b2, 1, 5, b2Str);
      hasilkalibrasiec = "y = (" + String(m2Str)+ ")x + (" + String(b2Str) + ")";
      Serial.println(hasilkalibrasiec);
    }
  }
  //==============================================================
}

void reconnect() {
  while (!client.connected()) {
    yield();
    Serial.println("Attempting MQTT connection...");
    String clientId = "esp32-clientId-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
  //=================================================================
  baca_pH(); baca_PPM(); baca_jarak(); baca_suhu(); 
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("MQTT Connected");
  lcd.setCursor(0,1); 
  lcd.print("nilai pH: "); lcd.print(pH);
  lcd.setCursor(0,2); 
  lcd.print("nilai EC: "); lcd.print(PPM);
  lcd.setCursor(0,3); 
  lcd.print("Nilai Suhu: "); lcd.print(suhu); lcd.print(" C");
  delay(1000);
  lcd.clear();
  //=============================================================
      Serial.println("Connected");
      Serial.print("Status OTA : ");
      Serial.println(OTA_msg);
      client.publish(PUBTOPIC_OTA, OTA_msg.c_str());
      client.subscribe(SUBTOPIC_PENYIRAM);
      client.subscribe(SUBTOPIC_MIXER);
      client.subscribe(SUBTOPIC_PHUP);
      client.subscribe(SUBTOPIC_PHDOWN);
      client.subscribe(SUBTOPIC_NUTRISI);
      client.subscribe(SUBTOPIC_AMBILSAMPEL);
      client.subscribe(SUBTOPIC_BUANGSAMPEL);
      client.subscribe(SUBTOPIC_BUKATANGKI);
      client.subscribe(SUBTOPIC_SETJAM);
      client.subscribe(SUBTOPIC_SETMENIT);
      client.subscribe(SUBTOPIC_SETPH);
      client.subscribe(SUBTOPIC_SETPPM);
      client.subscribe(SUBTOPIC_MAINTASK);
      client.subscribe(SUBTOPIC_GANTISAMPEL);
      client.subscribe(SUBTOPIC_OTOMATISPH);
      client.subscribe(SUBTOPIC_OTOMATISNUTRISI);
      client.subscribe(SUBTOPIC_MODEOTOMATIS);
      client.subscribe(SUBTOPIC_DATASPREADSHEET);
      client.subscribe(SUBTOPIC_TEST);
      client.subscribe("greenhouse/input/delaypompa");
      client.subscribe("greenhouse/input/delaymixer");
      client.subscribe("greenhouse/input/delaypenyiram");
      client.subscribe("greenhouse/input/inputph1");
      client.subscribe("greenhouse/input/inputph2");
      client.subscribe("greenhouse/input/voltageph1");
      client.subscribe("greenhouse/input/voltageph2");
      client.subscribe("greenhouse/input/kalibrasiph");
      client.subscribe("greenhouse/input/kalibrasisuhu");
      client.subscribe("greenhouse/input/inputec1");
      client.subscribe("greenhouse/input/inputec2");
      client.subscribe("greenhouse/input/voltageec1");
      client.subscribe("greenhouse/input/voltageec2");
      client.subscribe("greenhouse/input/kalibrasiec");
      client.subscribe("greenhouse/input/inputph");
      client.subscribe("greenhouse/input/inputppm");
      client.subscribe("greenhouse/input/inputtinggi");
    } else {
      yield();
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      //=================================================================
  baca_pH(); baca_PPM(); baca_jarak(); baca_suhu(); 
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Connecting to MQTT");
  lcd.setCursor(0,1); 
  lcd.print("nilai pH: "); lcd.print(pH);
  lcd.setCursor(0,2); 
  lcd.print("nilai EC: "); lcd.print(PPM);
  lcd.setCursor(0,3); 
  lcd.print("Nilai Suhu: "); lcd.print(suhu); lcd.print(" C");
  //=============================================================
      delay(3000);
    }
    yield();
  }
}

// Prosedur Utama
void mainTask (){
  Serial.println("==================================");
  Serial.println("kondisi didalam mainTask ");
  Serial.println("==================================");
  status = "Sebelum";
  gantiSampel();
  kontrol_pH();
  kontrol_PPM(); 
  mixer();
  gantiSampel();
  penyiram(); 
}

void monitoring(){
  baca_pH(); 
  baca_PPM(); 
  baca_jarak(); 
  baca_suhu();  
  baca_RTC();
  DateTime now = rtc.now(); 
  
  // Print LCD
  lcd.setCursor(0,1); 
  lcd.print("nilai pH: "); lcd.print(pH);
  lcd.setCursor(0,2); 
  lcd.print("nilai EC: "); lcd.print(PPM);
  lcd.setCursor(0,3); 
  lcd.print("Nilai Suhu: "); lcd.print(suhu); lcd.print(" C");
  
   
  // Print MQTT
  client.publish(PUBTOPIC_PH, String(pH).c_str());
  client.publish(PUBTOPIC_PPM, String(PPM).c_str());
  client.publish(PUBTOPIC_SUHU, String(suhu).c_str());
  client.publish(PUBTOPIC_TINGGI, String(nilai_tinggi).c_str());
  client.publish("greenhouse/output/voltageph", String(voltagepH).c_str());
  client.publish("greenhouse/output/voltageec", String(voltageEC).c_str());
  client.publish(PUBTOPIC_AKTUATOR1, aktuator1.c_str());
  client.publish(PUBTOPIC_AKTUATOR2, aktuator2.c_str());
  client.publish(PUBTOPIC_AKTUATOR3, aktuator3.c_str());
  client.publish(PUBTOPIC_AKTUATOR4, aktuator4.c_str());
  client.publish(PUBTOPIC_AKTUATOR5, aktuator5.c_str());
  client.publish(PUBTOPIC_AKTUATOR6, aktuator6.c_str());
  client.publish(PUBTOPIC_AKTUATOR7, aktuator7.c_str());
  client.publish(PUBTOPIC_AKTUATOR8, aktuator8.c_str());
  client.publish ("greenhouse/output/kalibrasiph", hasilkalibrasiph.c_str());
  client.publish ("greenhouse/output/kalibrasiec", hasilkalibrasiec.c_str());
}

//Mengaktifkan Mixer  
void mixer(){
  Serial.println("==================================");
  Serial.println("Mengaktifjan relay_mixer ");
  Serial.println("==================================");
  aktuator4 = "Menyala";
  Serial.print("delay ");Serial.print(delay_mixer);Serial.println(" detik");
  unsigned long startMillis = millis();
  while (millis() - startMillis < delay_mixer*1000) {   
    digitalWrite (relay_mixer, HIGH);  
    yield();
  }
  Serial.println("Menonaktifkan relay_mixer");
  digitalWrite (relay_mixer, LOW); 
  aktuator4 = "Padam";  
  status = "Sesudah";
  mixeron = true;
}

//Mengaktifkan Penyiram 
void penyiram (){
  Serial.println("==================================");
  Serial.println("Mengaktifkan relay_penyiram");
  Serial.println("=================================="); 
  aktuator5 = "Menyala";
  Serial.print("delay ");Serial.print(delay_penyiram);Serial.println(" detik");
  unsigned long startMillis = millis();
  while (millis() - startMillis < delay_penyiram*1000) {
    digitalWrite (relay_penyiram, HIGH); 
    yield();
  }
  Serial.println("Menonaktifkan relay_penyiram");
  digitalWrite (relay_penyiram, LOW); 
  aktuator5 = "Padam";  
}

// Mengganti Sampel
void gantiSampel (){
  Serial.println("==================================");
  Serial.println("Mengaktifkan relay_buangSampel");
  Serial.println("==================================");
  aktuator3 = "Menyala";
  Serial.print("delay ");Serial.print(delay_pompaSampel);Serial.println(" detik"); 
  unsigned long startMillis1 = millis();
  while (millis() - startMillis1 < delay_pompaSampel*1000) {
    digitalWrite (relay_buangSampel, HIGH); 
    yield();
  }
  Serial.println("Menonaktifkan relay_buangSampel");
  digitalWrite (relay_buangSampel, LOW); 
  aktuator3 = "Padam";

  Serial.println("==================================");
  Serial.println("Mengaktifkan relay_ambilSampel");
  Serial.println("==================================");
  aktuator2 = "Menyala";
  Serial.print("delay ");Serial.print(delay_pompaSampel);Serial.println(" detik");
  unsigned long startMillis2 = millis();
  while (millis() - startMillis2 < delay_pompaSampel*1000) {
    digitalWrite (relay_ambilSampel, HIGH); 
    yield();
  }
  Serial.println("Menonaktifkan relay_ambilSampel");
  digitalWrite (relay_ambilSampel, LOW); 
  aktuator2 = "Padam";
  if(mixeron){
    status = "sesudah";
  }else{
    status = "sebelum";
  }
  spreadsheet();
  mixeron = false;
}

void baca_pH(){
  
  adcValuepH = analogRead(pin_pH);
  voltagepH = (adcValuepH / 4095.0) * 3300;
  pH = m1 * voltagepH + b1-1;
  pH = constrain(pH, 0.0, 14.0);
  
  // Angka random dari 5 sampai 9 dengan interval 0,1
  //int randomNumber = random(0, 41);  
 // pH = 5.0 + (randomNumber * 0.1);
  //voltagepH = random(1000, 1200);
  //==============================================
  if(kondisi1){
    Error_pH = abs(set_pH - pH);
  }else{
    Error_pH = abs(set_pH - input_pH);
  }
}

void kontrol_pH(){
  baca_pH();
  outputFuzzy();
  baca_jarak();
  if (pH < set_pH || input_pH < set_pH ){
    Serial.println("==================================");
    Serial.println("Mengaktifkan relay_pHup");
    Serial.println("==================================");
    aktuator6 = "Menyala";
    aktuator7 = "Padam";
    Serial.print("delay : ");
    Serial.print(lamaPompa_pH*(tinggi_cm/set_jarak)); Serial.println(" detik");
    unsigned long startMillis = millis();
    while (millis() - startMillis < lamaPompa_pH*1000*(tinggi_cm/set_jarak)) {
      digitalWrite(relay_pHup, HIGH); 
      digitalWrite(relay_pHdn, LOW);
      yield();
    }
    Serial.println("Menonaktifkan relay_pHup");
    digitalWrite(relay_pHup,LOW);
    aktuator6 = "Padam";
    digitalWrite(relay_pHdn, LOW);
    aktuator7 = "Padam";
  }
  else if (pH >= set_pH || input_pH >= set_pH ){
    Serial.println("==================================");
    Serial.println("Mengaktifkan relay_pHdn");
    Serial.println("==================================");
    aktuator6 = "Padam";
    aktuator7 = "Menyala";
    Serial.print("delay : ");
    Serial.print(lamaPompa_pH*(tinggi_cm/set_jarak)); Serial.println(" detik");
    unsigned long startMillis = millis();
    while (millis() - startMillis < lamaPompa_pH*1000*(tinggi_cm/set_jarak)) {
      digitalWrite(relay_pHup, LOW); 
      digitalWrite(relay_pHdn, HIGH);
      yield();
    }
    Serial.println("Menonaktifkan relay_pHdn");
    digitalWrite(relay_pHup, LOW);
    aktuator6 = "Padam";
    digitalWrite(relay_pHdn, LOW);
    aktuator7 = "Padam";
  }
  status = "sebelum";
}

void baca_PPM(){
  
  adcValueEC = analogRead(pin_EC);
  voltageEC = (adcValueEC / 4095.0) * 3300;
  PPM = ((m2 * voltageEC) + b2)*500;
  
  // Angka random dari 550 sampai 1051
  //voltageEC = random(1000, 1200);
  //PPM = random(550, 1051); 
  //==================================
  if(kondisi1){
    if (PPM <= set_PPM){ 
      Error_PPM = set_PPM - PPM;
    }else if (PPM > set_PPM){
      Error_PPM = 0;
    }
  }else{
    if (input_PPM <= set_PPM){
      Error_PPM = set_PPM - input_PPM;
    }else if (input_PPM > set_PPM){
      Error_PPM = 0;
    }
  }
}

void kontrol_PPM(){
  baca_PPM();
  outputFuzzy();
  baca_jarak();
  if (PPM < set_PPM || input_PPM < set_PPM ){
    Serial.println("==================================");
    Serial.println("Mengaktifkan relay_nutrisi");
    Serial.println("==================================");
    aktuator8 = "Menyala";
    Serial.print("delay : ");
    Serial.print(lamaPompa_PPM*(tinggi_cm/set_jarak)); Serial.println(" detik");
    unsigned long startMillis = millis();
    while (millis() - startMillis < lamaPompa_PPM*1000*(tinggi_cm/set_jarak)) {
      digitalWrite(relay_nutrisi, HIGH);
      yield();
    }
    Serial.println("Menonaktifkan relay_nutrisi");
    digitalWrite(relay_nutrisi, LOW);
    aktuator8 = "Padam";
  }
  else if (PPM >= set_PPM || input_PPM >= set_PPM  ){
    Serial.println("==================================");
    Serial.println("Menonaktifkan relay_nutrisi");
    Serial.println("==================================");
    digitalWrite(relay_nutrisi, LOW);
    aktuator8 = "Padam";
  }
  status = "Sebelum";
}

void baca_jarak(){
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  duration = pulseIn(PIN_ECHO, HIGH);
  jarak_cm = duration * 0.034 / 2;
  
  // Angka random dari 0 sampai 21
  //jarak_cm  = random(10, 31); 
  //==============================
 /* 
  */
  nilai_tinggi = set_jarak - jarak_cm;
  if(kondisi1){
    
    if(tinggi_cm > set_jarak){
      tinggi_cm = 1;
    }else{
      tinggi_cm = set_jarak - jarak_cm;
    }
    
    //tinggi_cm = 100;
  }else{
    tinggi_cm = input_tinggi;
  }
}

void baca_suhu(){
  
  sensors.requestTemperatures(); 
  suhu = sensors.getTempCByIndex(0);
  suhu += kalibrasi_suhu;
  
  // Angka random dari 25 sampai 35
  //suhu = random(25, 35); 
  //==============================
}

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    
    Serial.print(".");
    //=================================================================
  baca_pH(); baca_PPM(); baca_jarak(); baca_suhu(); 
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Connecting to WiFi");
  lcd.setCursor(0,1); 
  lcd.print("nilai pH: "); lcd.print(pH);
  lcd.setCursor(0,2); 
  lcd.print("nilai EC: "); lcd.print(PPM);
  lcd.setCursor(0,3); 
  lcd.print("Nilai Suhu: "); lcd.print(suhu); lcd.print(" C");
  //=============================================================
  delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void baca_RTC(){
  
  // Print waktu di MQTT
  DateTime now = rtc.now();
  String pesan = "MQTT < ";
  pesan += String(now.hour());
  pesan += ':';
  pesan += String(now.minute());
  pesan += " >";
  //Serial.println(pesan);  // Kirimkan pesan ke serial monitor
  client.publish(PUBTOPIC_WAKTU, pesan.c_str());  // Kirimkan pesan ke MQTT


  // Print waktu di LCD
  
  lcd.setCursor(0,0);
  lcd.print("IoT MQTT <");
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);
  lcd.print(">");
  
}

void outputFuzzy() {
  baca_pH();
  baca_PPM();
  Momen();
  lamaPompa_pH = deffuzzyfikasi_pH();
  lamaPompa_PPM = deffuzzyfikasi_PPM();
}

void spreadsheet(){
  if (WiFi.status() == WL_CONNECTED) {
    String Send_Data_URL = Web_App_URL + "?sts=write";
    Send_Data_URL += "&pH=" + String (pH);
    Send_Data_URL += "&PPM=" + String(PPM);
    Send_Data_URL += "&tinggi_cm=" + String(nilai_tinggi);
    Send_Data_URL += "&suhu=" + String(suhu);
    Send_Data_URL += "&status=" + status;
    Serial.println();
    Serial.println("-------------");
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Send_Data_URL);

    WiFiClientSecure client;
    client.setInsecure(); 
    HTTPClient http;
    http.begin(client, Send_Data_URL); 
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); 
    int httpCode = http.GET(); 
    http.end();
    
    String Read_Data_URL = Web_App_URL + "?sts=read";
    http.begin(client, Read_Data_URL); 
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    httpCode = http.GET();
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Data ke : " + payload);
      dataSpreadsheet = payload.toInt(); 
    }
    http.end();
    Serial.println("-------------");
  }
  // Sistem Peringatan data SpreadSheet
  if (dataSpreadsheet > 1000){
    warning = "Data sudah terlalu banyak";
  }else{
    warning = String (dataSpreadsheet);  
  }
  client.publish(PUBTOPIC_WARNING, warning.c_str());
}