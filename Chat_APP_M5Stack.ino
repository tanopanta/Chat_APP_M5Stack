#include <M5ez.h>
#include <vector>

#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.10";

std::vector<String> msgList;
bool inchat = false;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  String msg = String((char*) payload);
  Serial.println(msg);
  msgList.push_back(msg);
  redraw();  
}

void setup() {
  ez.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  ezMenu mainmenu("Welcome to M5ez");
  mainmenu.addItem("chat", chat_menu);
  mainmenu.addItem("settings", ez.settings.menu);
  mainmenu.run();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = getMacAddr();
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      client.subscribe("chat");
    }
    delay(1000); 
  }
}


void redraw() {
  if(!inchat) {
    return;
  }
  ez.canvas.clear();
  ez.header.show("chat");
  ez.canvas.font(&FreeSans9pt7b);
  ez.canvas.lmargin(10);

  // show 8 messages
  int total = 0;
  for(int i = msgList.size() - 1 ; i >= 0; i--) {
    if(total >= 8) {
      break;
    }
    ez.canvas.println(msgList[i]);
    total++;
  }
}

void chat_menu() {
  inchat = true;
  ez.buttons.show("up # Back # input # # down #");
  redraw();
  
  while(true) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    
    String btnpressed = ez.buttons.poll();
    if(btnpressed == "Back") {
      break;
    }else if(btnpressed == "input") {
      ez.buttons.clear(true);
      ez.canvas.clear();
      
      String msg = ez.textInput();
      msg.trim();
      
      if(msg.length() > 0) {
        //msgList.push_back(msg);
        client.publish("chat", msg.c_str());
      }
      redraw();
      ez.buttons.clear(true);
      ez.buttons.show("up # Back # input # # down #");
    }else if (btnpressed != "") {

    }
  }
  inchat = false;
}

String getMacAddr() {
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[16] = {0};
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}
