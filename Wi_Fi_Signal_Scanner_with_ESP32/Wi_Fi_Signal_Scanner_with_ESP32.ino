/********************************************************************************************************
    
    Wi-Fi Signal Scanner with ESP32! (Live Network Scan)
	
    YouTube Channel: https://www.youtube.com/@TeknoTrek
    
    For project details:   https://www.instructables.com/member/TeknoTrek/

*********************************************************************************************************/
//# Gerekli kütüphaneleri içe aktar (Import required libraries)
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2
#define BUTTON_PIN 0  // Butonun bağlı olduğu GPIO pini (Değiştirilebilir) (GPIO pin where the button is connected (Can be changed))
#define BUTTON_2_PIN 15 // SSID listeleme için buton (Button for listing SSIDs)

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
uint16_t colors[] = {ILI9341_GREEN, ILI9341_YELLOW, ILI9341_RED, ILI9341_BLUE, ILI9341_CYAN, ILI9341_MAGENTA, ILI9341_ORANGE, ILI9341_WHITE};

bool scanRequested = true;  // ESP32 başladığında ilk tarama yapılacak (Initial scan will be performed when ESP32 starts)

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    pinMode(BUTTON_PIN, INPUT_PULLUP); // Buton pinini giriş olarak ayarla (Set button pin as input)
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);

    if (scanRequested) {
        scanWiFi();
        scanRequested = false;  // İlk tarama tamamlandı, butona basılana kadar bekleyecek (Initial scan completed, will wait until button is pressed)
    }
}

void loop() {

    if (digitalRead(BUTTON_PIN) == LOW) {  // Butona basıldığında (When button is pressed)
        delay(200);  // Buton debounce'u önlemek için kısa gecikme (Short delay to prevent button bounce)
        while (digitalRead(BUTTON_PIN) == LOW);  // Buton bırakılana kadar bekle (Wait until button is released)
        scanWiFi();
    }

        if (digitalRead(BUTTON_2_PIN) == LOW) { // Buton 2'ye basılınca SSID listesi (When button 2 is pressed, list SSIDs)
        delay(200);
        while (digitalRead(BUTTON_2_PIN) == LOW);
        listWiFi();
    }

}

void scanWiFi() {
    uint16_t titleColor = colors[random(8)];
    tft.setTextSize(2);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(titleColor);

    // Metin genişliğini ve yüksekliğini hesapla (Calculate text width and height)
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds("TEKNOTREK WiFi Scanning...", 0, 0, &x1, &y1, &w, &h);

    // Metni ortala (Center text)
    int x = (tft.width() - w) / 2;
    int y = (tft.height() - h) / 2;

    tft.setCursor(x, y);
    tft.print("TEKNOTREK WiFi Scanning...");

    delay(2000); // Taramadan önce 2 saniye bekle (Wait 2 seconds before scanning)

    int networkCount = WiFi.scanNetworks();

    tft.fillScreen(ILI9341_BLACK);
    titleColor = colors[random(8)];
    tft.setTextColor(titleColor);

    tft.getTextBounds("TEKNOTREK WiFi List", 0, 0, &x1, &y1, &w, &h);
    x = (tft.width() - w) / 2;
    y = 8; // Başlığı üstte tut (Keep title at the top)
    tft.setCursor(x, y);
    tft.print("TEKNOTREK WiFi List");

    // WiFi ağlarının sayısını sağ üst köşede göster (Display the number of WiFi networks in the top right corner)
    tft.setCursor(tft.width() - 60, 37);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("Net: ");
    tft.print(networkCount);

    if (networkCount == 0) {
        tft.setCursor(10, 40);
        tft.print("No networks found.");
    } else {
        int barWidth = (tft.width() - 50) / networkCount - 10;
        int barSpacing = 10;
        int startX = 40;
        int bottomY = tft.height() - 30;
        int gridSpacing = 20;

        tft.drawFastVLine(30, 30, tft.height() - 60, ILI9341_DARKGREY);

        for (int y = 30; y <= bottomY; y += gridSpacing) {
            tft.drawFastHLine(30, y, tft.width() - 30, ILI9341_DARKGREY);
            int rssiValue = map(y, 30, bottomY, -30, -100);
            tft.setTextSize(1);
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(5, y - 5);
            tft.print(rssiValue);
        }

        tft.setTextSize(1);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(5, 215);
        tft.print("SSID>"); 

        tft.setTextSize(1);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(5, 225);
        tft.print("  CH>"); 

        for (int i = 0; i < networkCount; i++) {
            int signalStrength = WiFi.RSSI(i);
            int barHeight = map(signalStrength, -100, -30, 10, bottomY - 30);
            uint16_t color = colors[i % 8];

            int barX = startX + i * (barWidth + barSpacing);

            tft.fillRect(barX, bottomY - barHeight, barWidth, barHeight, color);

            tft.setTextSize(1);
            tft.setTextColor(color);
            tft.setCursor(barX + (barWidth / 4), bottomY - barHeight - 15);
            tft.print(signalStrength);

            tft.setCursor(barX, bottomY + 5);
            String ssid = WiFi.SSID(i);
            if (ssid.length() > 3) {
                ssid = ssid.substring(0, 3);
            }
            tft.print(ssid);

            if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) {
                tft.setCursor(barX + (barWidth / 2), bottomY - barHeight - 25);
                tft.print("*");
            }

            tft.setCursor(barX, bottomY + 15);
            tft.print(" ");
            tft.print(WiFi.channel(i));
        }
    }
}

void listWiFi() {

      uint16_t titleColor = colors[random(8)];
    tft.setTextSize(2);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(titleColor);

    // Metin genişliğini ve yüksekliğini hesapla (Calculate text width and height)
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds("TEKNOTREK WiFi Scanning...", 0, 0, &x1, &y1, &w, &h);

    // Metni ortala (Center text)
    int x = (tft.width() - w) / 2;
    int y = (tft.height() - h) / 2;

    tft.setCursor(x, y);
    tft.print("TEKNOTREK WiFi Scanning...");

    delay(2000); // Taramadan önce 2 saniye bekle (Wait 2 seconds before scanning)

    int networkCount = WiFi.scanNetworks();

    if (networkCount == 0) {
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 40);
        tft.setTextColor(ILI9341_RED);
        tft.print("No networks found.");
        return;
    }

    String ssidList[networkCount];
    int rssiList[networkCount];
    int channelList[networkCount];
    bool encryptionList[networkCount];

    // Ağları listeye al (Add networks to the list)
    for (int i = 0; i < networkCount; i++) {
        ssidList[i] = WiFi.SSID(i);
        rssiList[i] = WiFi.RSSI(i);
        channelList[i] = WiFi.channel(i);
        encryptionList[i] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }

    // RSSI'ye göre sıralama (Büyükten küçüğe) (Sort by RSSI (From highest to lowest))
    for (int i = 0; i < networkCount - 1; i++) {
        for (int j = i + 1; j < networkCount; j++) {
            if (rssiList[j] > rssiList[i]) {
                std::swap(ssidList[i], ssidList[j]);
                std::swap(rssiList[i], rssiList[j]);
                std::swap(channelList[i], channelList[j]);
                std::swap(encryptionList[i], encryptionList[j]);
            }
        }
    }

    // Arka planı temizle ve başlığı ekrana yaz (Clear the background and display the title)
    //tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    
    tft.fillScreen(ILI9341_BLACK);
    titleColor = colors[random(8)];
    tft.setTextColor(titleColor);

    tft.getTextBounds("TEKNOTREK WiFi List", 0, 0, &x1, &y1, &w, &h);
    x = (tft.width() - w) / 2;
    y = 8; // Başlığı üstte tut (Keep title at the top)
    tft.setCursor(x, y);
    tft.print("TEKNOTREK WiFi List");

    tft.setTextSize(1);

    // Renkler dizisi (Farklı satırlar için) (Array of colors (For different lines))
    uint16_t colors[] = {ILI9341_GREEN, ILI9341_YELLOW, ILI9341_RED, ILI9341_BLUE, ILI9341_CYAN, ILI9341_MAGENTA, ILI9341_ORANGE, ILI9341_WHITE};
    int colorCount = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < networkCount && i < 26; i++) {
        tft.setCursor(10, 36 + (i * 12));
        tft.setTextColor(colors[i % colorCount]);  // Her satır için farklı renk kullan (Use different colors for each line)
        
        tft.print(ssidList[i]);   // SSID
        tft.print(" ");
        tft.print(rssiList[i]);   // Sinyal Gücü (Signal Strength)
        tft.print("dBm Ch:");
        tft.print(channelList[i]); // Kanal Numarası (Channel Number)
        tft.print(" | ");
        tft.print(encryptionList[i] ? "Pwd" : "NotPwd"); // Şifreli mi? (Encrypted or not?)
    }
}