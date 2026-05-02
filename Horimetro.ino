//                                                                     ↓  [ LIBS ]
#include <SPI.h>
#include <SD.h>
#include <Preferences.h>
#include "esp_timer.h"
#include <WiFi.h>
#include <ESP32Ping.h>

Preferences preferences;
//                                                                     ↑  [ LIBS ]
//                                                                     ↓  [ GLOBAIS ]

const char* ssid = "R2X - Recepcao";
const char* password = "R2x12024";

uint64_t espCLK = 0;
uint64_t segCLK = 0;

const int CS_PIN = 21;

int RAM = 0;
int VAULT = 0;
int NVS = 0;
int TOTAL = 0;
//                                                                     ↑  [ GLOBAIS ]
//                                                                     ↓  [ FUNÇÕES ]
//                                                                     ↓  [ scanWIFI ]
void scanWiFi() {
  Serial.println("scanWiFi:  Iniciando scan WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("Nenhuma rede encontrada");
  } else {
    Serial.print(n);
    Serial.println(" redes encontradas:");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (RSSI: ");
      Serial.print(WiFi.RSSI(i));
      Serial.print(") ");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Aberta" : "Segura");
      delay(10);
    }
  }
}
//                                                                     ↑  [ scanWIFI ]
//                                                                     ↓  [ initWIFI ]
void initWiFi() {
  Serial.println("Tentando conectar ao WiFi...");
  WiFi.begin(ssid, password);
  Serial.println(ssid);
  Serial.println(password);

  int tentativas = 0;
  const int maxTentativas = 20; //10s

  while (WiFi.status() != WL_CONNECTED && tentativas < maxTentativas) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("initWiFi:  WiFi conectado");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("initWiFi:  Falha ao conectar WiFi");
    delay(1000);
  }
}
//                                                                     ↑  [ initWIFI ]
//                                                                     ↓  [ initSD ]
bool initSD() {
  
  Serial.println("SDBEGGIN:  Iniciando conexão com o cartão SD");
  SPI.begin(18, 19, 23, CS_PIN);

      
  if (!SD.begin(CS_PIN, SPI, 1000000)) {
    
    Serial.println("SDBEGGIN-ERROR: Falha!  conexão não realizada");   
    digitalWrite(33, HIGH);
        
    return false;
    }
      
    File test = SD.open("/TEST.txt", FILE_WRITE);
    if (!test) {
      
      Serial.println("SDBEGGIN-ERROR: Falha!  conexão interrompida");
      digitalWrite(33, HIGH);
        
      return false;
    }
    
  test.println("Teste OK");
  test.flush();
  test.close();
  SD.remove("/TEST.txt");
  Serial.println("SDBEGGIN: Sucesso!  Conexão estabelecida");
  digitalWrite(33, LOW);
  return true;
}
//                                                                       ↑  [ initSD ]
//                                                                       ↓  [ parseSD ]
int parseSD() {
  
    File lercontador = SD.open("/CONT_TIMER.txt", FILE_READ);
    if(lercontador){
      
      String SDval = lercontador.readStringUntil('\n');
      lercontador.close();

      SDval.trim();
      Serial.print("ParseSD:  Valor atual do SD = ");
      Serial.println(SDval);
      digitalWrite(33, LOW);
      return SDval.toInt();
    
    }
  
  }



//                                                                       ↑  [ parseSD ]
//                                                                       ↑  [ FUNÇÕES ]
void setup() { //                                                        ↓  [ SETUP ]
  
pinMode(27, OUTPUT);
pinMode(26, OUTPUT);
pinMode(25, OUTPUT);
pinMode(33, OUTPUT);
pinMode(32, OUTPUT);
pinMode(14, OUTPUT);
  
Serial.begin(115200);
delay(1000);

espCLK = esp_timer_get_time();

initWiFi();

if (WiFi.status() == WL_CONNECTED) {
  
  Serial.println("initWiFi:  WiFi Conectado");
  Serial.println("ESP32Ping:  Realizando ping em 8.8.8.8");

  if (Ping.ping("8.8.8.8")) {
    
    Serial.println("ESP32Ping:  Ping bem-sucedido");
    Serial.print("ESP32Ping:  Tempo médio (ms): ");
    Serial.println(Ping.averageTime());
    digitalWrite(32, LOW);
    digitalWrite(14, HIGH);
  } else {
    Serial.println("ESP32Ping:  Falha no ping.");
  }
  
} else {
  Serial.println("initWiFi:  WiFi desconectado!");
  digitalWrite(32, HIGH);
  digitalWrite(14, LOW);
  
  }

}//                                                                      ↑  [ SETUP ]
void loop() {//                                                          ↓  [ LOOP ] 
//                                                                       ↓  [CONTADOR]
uint64_t espRAM = esp_timer_get_time();
digitalWrite(27, HIGH);


if (espRAM - espCLK >= 1000000) {
  espCLK += 1000000;
  RAM++;

  

  
  
  
  Serial.print("#RAM:  ");
  Serial.println(RAM);
  
}
     
if (RAM == 60) {

  RAM=0;
  VAULT++;
       
  digitalWrite(25, HIGH);
       
  Serial.print("#VAULT:  ");
  Serial.println(VAULT);
//                                                                       ↓  [ESCRITA] 
  
  bool initSD_bool = initSD();

  if (initSD_bool == true) { //SE SD OK
    
    preferences.begin("NVSBKP", false);
    NVS = preferences.getInt("NVS", 0);


    
    Serial.print("NVS: Alerta! valor recuperado da NVS = ");
    Serial.println(NVS);    

    

    int parseSD_int=parseSD();
    TOTAL = VAULT+NVS+parseSD_int;

    preferences.clear();
    preferences.end();
    VAULT = 0;
    NVS = 0;
    digitalWrite(26, LOW);
      
    File writecontador = SD.open("/CONT_TIMER.txt", FILE_WRITE);      
    if (writecontador) {

      

      
      writecontador.println(TOTAL);
      writecontador.close();
      
      
      
      Serial.print("CONT_TIMER:  Valor inserido no SD = ");
      Serial.println(TOTAL);
      Serial.print("initSD:  ");
      Serial.println(initSD_bool);



      
      } 
          
  } else { //SE SD NÃO OK
        
        Serial.print("initSD:  ");
        Serial.println(initSD_bool);
        
        preferences.begin("NVSBKP", false);
        
        NVS = preferences.getInt("NVS", 0);
        preferences.putInt("NVS", NVS+VAULT);
        preferences.end();

        Serial.print("NVS: Emergência! Valor atual do VAULT = ");
        Serial.println(VAULT);
        Serial.print("NVS: Emergência! Valor atual da NVS = ");
        Serial.println(NVS);
        VAULT = 0; 
        digitalWrite(26, HIGH);
        
        abort();
        }
//                                                                       ↑  [ESCRITA]    
}//                                                                      ↑  [CONTADOR] 
digitalWrite(25, LOW);
}//                                                                      ↑  [ LOOP ]