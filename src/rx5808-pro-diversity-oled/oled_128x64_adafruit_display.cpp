#include "settings.h"

#ifdef OLED_128x64_ADAFRUIT_SCREENS
#include "screens.h" // function headers
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <SPI.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if !defined SSD1306_128_64
#error("Screen incorrect, please fix Adafruit_SSD1306.h!");
#endif


screens::screens() {
    last_channel = -1;
    last_rssi = 0;
}

void screens::begin() {
    // Set the address of your OLED Display.
    // 128x64 ONLY!!
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D or 0x3C (for the 128x64)
    // init done
    display.clearDisplay();   // clears the screen and buffer
    display.display();
}

void screens::reset() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
}

void screens::flip() {
    display.setRotation(3);
}

void screens::drawTitleBox(const char *title) {
    display.drawRect(0, 0, display.width(), display.height(),WHITE);
    display.fillRect(0, 0, display.width(), 11,WHITE);

    display.setTextSize(1);
    display.setTextColor(BLACK);
    // center text
    display.setCursor(((display.width() - (strlen(title)*6)) / 2),2);
    display.print(title);
    display.setTextColor(WHITE);
}


void screens::mainMenu(uint8_t menu_id){
    drawMainMenu(menu_id);
    display.display();
}

void screens::drawMainMenu(uint8_t menu_id) {
    reset(); // start from fresh screen.
    drawTitleBox("MODE SELECTION");

    display.fillRect(0, 10*menu_id+12, display.width(), 10,WHITE);

    display.setTextColor(menu_id == 0 ? BLACK : WHITE);
    display.setCursor(5,10*0+13);
    display.print("AUTO SEARCH");
    display.setTextColor(menu_id == 1 ? BLACK : WHITE);
    display.setCursor(5,10*1+13);
    display.print("BAND SCANNER");
    display.setTextColor(menu_id == 2 ? BLACK : WHITE);
    display.setCursor(5,10*2+13);
    display.print("MANUAL MODE");

#ifdef USE_DIVERSITY
    display.setTextColor(menu_id == 3 ? BLACK : WHITE);
    display.setCursor(5,10*3+13);
    display.print("DIVERSITY");
#endif
    display.setTextColor(menu_id == 4 ? BLACK : WHITE);
    display.setCursor(5,10*4+13);
    display.print("SAVE SETUP");

}


void screens::seekMode(uint8_t state) {
    last_channel = -1;
    drawSeekMode(state);
    display.display();
}

void screens::drawSeekMode(uint8_t state) {
    reset(); // start from fresh screen.
    if (state == STATE_MANUAL)
    {
        drawTitleBox("MANUAL MODE");
    }
    else if(state == STATE_SEEK)
    {
        drawTitleBox("AUTO SEEK MODE");
    }
    display.setTextColor(WHITE);
    display.drawLine(0, 20, display.width(), 20, WHITE);
    display.drawLine(0, 32, display.width(), 32, WHITE);
    display.setCursor(5,12);
    display.drawLine(97,11,97,20,WHITE);
    display.print("BAND:");
    for(uint16_t i=0;i<8;i++) {
        display.setCursor(15*i+8,23);
        display.print((char) (i+'1'));
    }
    display.drawLine(0, 36, display.width(), 36, WHITE);
    display.drawLine(0, display.height()-11, display.width(), display.height()-11, WHITE);
    display.setCursor(2,display.height()-9);
    display.print("5645");
    display.setCursor(55,display.height()-9);
    display.print("5800");
    display.setCursor(display.width()-25,display.height()-9);
    display.print("5945");
}
void screens::updateSeekMode(uint8_t state, uint8_t channel, uint8_t rssi, uint16_t channelFrequency, bool locked) {
    drawUpdateSeekMode(state, channel, rssi, channelFrequency, locked);
    display.display();
}

void screens::drawUpdateSeekMode(uint8_t state, uint8_t channel, uint8_t rssi, uint16_t channelFrequency, bool locked) {
    // display refresh handler
    if(channel != last_channel) // only updated on changes
    {
        display.setTextColor(WHITE,BLACK);
        display.setCursor(36,12);
        // show current used channel of bank
        if(channel > 31)
        {
            display.print("C/Race   ");
        }
        else if(channel > 23)
        {
            display.print("F/Airwave");
        }
        else if (channel > 15)
        {
            display.print("E        ");
        }
        else if (channel > 7)
        {
            display.print("B        ");
        }
        else
        {
            display.print("A        ");
        }

        uint8_t active_channel = channel%CHANNEL_BAND_SIZE; // get channel inside band
        for(int i=0;i<8;i++) {
            display.fillRect(15*i+4,21,14,11,i==active_channel? WHITE:BLACK);
            display.setTextColor(i==active_channel? BLACK:WHITE);
            display.setCursor(15*i+8,23);
            display.print((char) (i+'1'));
        }

        // show frequence
        display.setCursor(101,12);
        display.setTextColor(WHITE,BLACK);
        display.print(channelFrequency);
    }
    // show signal strength
    uint8_t rssi_scaled=map(rssi, 1, 100, 1, display.width()-3);

    display.fillRect(1+rssi_scaled, 33, display.width()-3-rssi_scaled, 3, BLACK);
    display.fillRect(1, 33, rssi_scaled, 3, WHITE);

    rssi_scaled=map(rssi, 1, 100, 1, 14);
    display.fillRect((channel*3)+4,display.height()-12-14,3,14-rssi_scaled,BLACK);
    display.fillRect((channel*3)+4,(display.height()-12-rssi_scaled),3,rssi_scaled,WHITE);

    // handling for seek mode after screen and RSSI has been fully processed
    if(state == STATE_SEEK) //
    { // SEEK MODE
        if(locked) // search if not found
        {
            display.setTextColor(BLACK,WHITE);
            display.setCursor(((display.width()-14*6)/2),2);
            display.print("AUTO MODE LOCK");
        }
        else
        {
            display.setTextColor(BLACK,WHITE);
            display.setCursor(((display.width()-14*6)/2),2);
            display.print("AUTO SEEK MODE");
        }
    }

    last_channel = channel;
}


void screens::bandScanMode(uint8_t state) {
    drawBandScanMode(state);
    display.display();
}

void screens::drawBandScanMode(uint8_t state) {
    reset(); // start from fresh screen.
    if(state==STATE_SCAN)
    {
        drawTitleBox("BAND SCANNER");
        display.setCursor(5,12);
        display.print("BEST:");
    }
    else
    {
        drawTitleBox("RSSI SETUP");
        display.setCursor(5,12);
        display.print("Min:     Max:");
    }
    display.drawLine(0, 20, display.width(), 20, WHITE);

    display.drawLine(0, display.height()-11, display.width(), display.height()-11, WHITE);
    display.setCursor(2,display.height()-9);
    display.print("5645");
    display.setCursor(55,display.height()-9);
    display.print("5800");
    display.setCursor(display.width()-25,display.height()-9);
    display.print("5945");
}

void screens::updateBandScanMode(uint8_t state, uint8_t channel, uint8_t rssi, uint8_t channelName, uint16_t channelFrequency, uint16_t rssi_setup_min_a, uint16_t rssi_setup_max_a) {
    drawUpdateBandScanMode(state, channel, rssi, channelName, channelFrequency, rssi_setup_min_a, rssi_setup_max_a);
    display.display();
}
void screens::drawUpdateBandScanMode(uint8_t state, uint8_t channel, uint8_t rssi, uint8_t channelName, uint16_t channelFrequency, uint16_t rssi_setup_min_a, uint16_t rssi_setup_max_a) {
    rssi=map(rssi, 1, 100, 1, 30);
    uint16_t hight = (display.height()-12-rssi);
    display.fillRect((channel*3)+4,display.height()-12-30,3,30-rssi,BLACK);
    display.fillRect((channel*3)+4,hight,3,rssi,WHITE);

    if(channelName < 255) {
        display.setTextColor(WHITE,BLACK);
        display.setCursor(36,12);
        display.print(channelName, HEX);
        display.setCursor(52,12);
        display.print(channelFrequency);
    }
}

void screens::screenSaver(uint8_t channelName, uint16_t channelFrequency) {
    screenSaver(-1, channelName, channelFrequency);
}
void screens::screenSaver(uint8_t diversity_mode, uint8_t channelName, uint16_t channelFrequency) {
    drawScreenSaver(diversity_mode, channelName, channelFrequency);
    display.display();
}
void screens::updateScreenSaver(uint8_t rssi) {
    updateScreenSaver(-1, rssi, -1, -1);
}
void screens::updateScreenSaver(char active_receiver, uint8_t rssi, uint8_t rssiA, uint8_t rssiB) {
    drawUpdateScreenSaver(active_receiver, rssi, rssiA, rssiB);
    display.display();
}

void screens::drawScreenSaver(uint8_t diversity_mode, uint8_t channelName, uint16_t channelFrequency) {
    reset();
    display.setTextSize(6);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(channelName, HEX);
    display.setTextSize(1);
    display.setCursor(70,0);
    display.print(CALL_SIGN);
    display.setTextSize(2);
    display.setCursor(70,28);
    display.setTextColor(WHITE);
    display.print(channelFrequency);
    display.setTextSize(1);
#ifdef USE_DIVERSITY
    display.setCursor(70,18);
    switch(diversity_mode) {
        case useReceiverAuto:
            display.print("AUTO");
            break;
        case useReceiverA:
            display.print("ANTENNA A");
            break;
        case useReceiverB:
            display.print("ANTENNA B");
            break;
    }
    display.setTextColor(BLACK,WHITE);
    display.fillRect(0, display.height()-19, 7, 9, WHITE);
    display.setCursor(1,display.height()-18);
    display.print("A");
    display.setTextColor(BLACK,WHITE);
    display.fillRect(0, display.height()-9, 7, 9, WHITE);
    display.setCursor(1,display.height()-8);
    display.print("B");
#endif
}

void screens::drawUpdateScreenSaver(char active_receiver, uint8_t rssi, uint8_t rssiA, uint8_t rssiB) {
#ifdef USE_DIVERSITY
    // read rssi A
    #define RSSI_BAR_SIZE 119
    uint8_t rssi_scaled=map(rssiA, 1, 100, 3, RSSI_BAR_SIZE);
    display.fillRect(7 + rssi_scaled, display.height()-19, (RSSI_BAR_SIZE-rssi_scaled), 9, BLACK);
    if(active_receiver == useReceiverA)
    {
        display.fillRect(7, display.height()-19, rssi_scaled, 9, WHITE);
    }
    else
    {
        display.fillRect(7, display.height()-19, (RSSI_BAR_SIZE), 9, BLACK);
        display.drawRect(7, display.height()-19, rssi_scaled, 9, WHITE);
    }

    // read rssi B
    rssi_scaled=map(rssiB, 1, 100, 3, RSSI_BAR_SIZE);
    display.fillRect(7 + rssi_scaled, display.height()-9, (RSSI_BAR_SIZE-rssi_scaled), 9, BLACK);
    if(active_receiver == useReceiverB)
    {
        display.fillRect(7, display.height()-9, rssi_scaled, 9, WHITE);
    }
    else
    {
        display.fillRect(7, display.height()-9, (RSSI_BAR_SIZE), 9, BLACK);
        display.drawRect(7, display.height()-9, rssi_scaled, 9, WHITE);
    }
#else
    display.setTextColor(BLACK);
    display.fillRect(0, display.height()-19, 25, 19, WHITE);
    display.setCursor(1,display.height()-13);
    display.print("RSSI");
    #define RSSI_BAR_SIZE 101
    uint8_t rssi_scaled=map(rssi, 1, 100, 1, RSSI_BAR_SIZE);
    display.fillRect(25 + rssi_scaled, display.height()-19, (RSSI_BAR_SIZE-rssi_scaled), 19, BLACK);
    display.fillRect(25, display.height()-19, rssi_scaled, 19, WHITE);
#endif
    if(rssi < 20)
    {
        display.setTextColor((millis()%250 < 125) ? WHITE : BLACK, BLACK);
        display.setCursor(50,display.height()-13);
        display.print("LOW SIGNAL");
    }
#ifdef USE_DIVERSITY
    else {
        display.drawLine(50,display.height()-10,110,display.height()-10,BLACK);
    }
#endif
}


#endif
