#include <WiFi.h>
#include <HTTPClient.h>

#define LIMIT_SWITCH_PIN 18
#define SSR_PIN 25
const char* ssid = "vivo V29";
const char* password = "0967081763";

const char* serverName = "http://192.168.76.240/log_status.php"; // แก้ URL ให้ตรง

void setup() {
    Serial.begin(115200);
    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(SSR_PIN, OUTPUT);
    digitalWrite(SSR_PIN, HIGH);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
}

unsigned long onStartTime = 0;
unsigned long totalOnTime = 0; // หน่วย: milliseconds
bool lastSSRState = true;

#define POWER_WATT 80          // กำลังวัตต์ของโหลด
#define ELECTRICITY_RATE 4    // ค่าไฟต่อหน่วย (บาท/หน่วย)

void loop() {
    int switchState = digitalRead(LIMIT_SWITCH_PIN);
    String switchText = (switchState == LOW) ? "PRESSED" : "RELEASED";
    String ssrText = (switchState == LOW) ? "OFF" : "ON";  // เปลี่ยน ON/ OFF ให้ตรงกับความต้องการ

    bool currentSSRState = (switchState == HIGH);  // ปล่อยสวิตช์ → เปิด

    // จับเวลาเปิด
    if (currentSSRState && !lastSSRState) {
    onStartTime = millis(); // เริ่มจับเวลา
}

    // จับเวลาปิด
    if (!currentSSRState && lastSSRState) {
    totalOnTime += millis() - onStartTime;
}

// ⚡ ควบคุม SSR 16DA (Active LOW)
    digitalWrite(SSR_PIN, currentSSRState ? LOW : HIGH);  // LOW = เปิด

    lastSSRState = currentSSRState;


    // คำนวณ kWh และค่าไฟ
    float hoursOn = totalOnTime / 1000.0 / 3600.0; // ms → hr
    float energyKWh = (POWER_WATT * hoursOn) / 1000.0;
    float cost = energyKWh * ELECTRICITY_RATE;

    Serial.printf("📦 ส่งข้อมูล: Switch: %s | SSR: %s\n", switchText.c_str(), ssrText.c_str());
    Serial.printf("⚡ เวลาทำงาน: %.2f ชั่วโมง | พลังงาน: %.4f kWh | ค่าไฟ: %.2f บาท\n", hoursOn, energyKWh, cost);

    sendToServer(switchText, ssrText, cost);
    sendToDiscord("🔔 Switch: " + switchText + " | SSR: " + ssrText + " | 💰 Cost: " + String(cost, 2) + " บาท");

    delay(2000); 
}



void sendToServer(String switchState, String ssrState, float cost) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(serverName) + "?switch=" + switchState + "&ssr=" + ssrState + "&cost=" + String(cost, 2);

        Serial.println("🌐 กำลังส่งไปที่: " + url);
        http.begin(url);

        int httpCode = http.GET();
        Serial.print("📥 HTTP Response Code: ");
        Serial.println(httpCode);

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("🖥️ Server ตอบกลับ: " + payload);
        } else {
            Serial.println("❌ ไม่สามารถเชื่อมต่อ server หรือไม่ได้รับ response");
        }

        http.end();
    } else {
        Serial.println("📡 ยังไม่ได้เชื่อม WiFi");
    }
}

const char* discordWebhookUrl = "https://discordapp.com/api/webhooks/1364862209326579752/hr2OxVxEEL5qUZLmVURZUb9qqPqpYfqrOaYy1GGJwb7SDj14fXn67BLnTrcwSu1KLHgv";  // ใส่ URL ของคุณตรงนี้

void sendToDiscord(String message) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(discordWebhookUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"content\":\"" + message + "\"}";

        int httpCode = http.POST(jsonPayload);
        if (httpCode > 0) {
            Serial.println("✅ แจ้งเตือน Discord สำเร็จ");
        } else {
            Serial.println("❌ ส่ง Discord ล้มเหลว");
        }
        http.end();
    } else {
        Serial.println("📡 ยังไม่ได้เชื่อม WiFi");
    }
}




