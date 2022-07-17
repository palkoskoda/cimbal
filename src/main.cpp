#include <Arduino.h>
/*
    ArduinoPrintADC.ino
    Author: Seb Madgwick
    Sends up to all 6 analogue inputs values in ASCII as comma separated values
    over serial.  Each line is terminated with a carriage return character ('\r').
    The number of channels is sent by sending a character value of '1' to '6' to 
    the Arduino.
    
    Tested with "arduino-1.0.3" and "Arduino Uno".
 */

#include <stdlib.h> // div, div_t


#define POCET_STRUN 2
#define CRASH_TOLERANCE 10


typedef struct {
    int pin=0;
    float signal_avg=0;
    int edge_down_number;
    int edge_max_hold;
    bool flag1=false;
    bool flag2=false;
    bool falling_edge_detector=false;
    unsigned long time_flag=0;
    int note;
    int velocity2send;
} struny;
struny struna[POCET_STRUN];

void send_notes () {
    for (int i=0; i<POCET_STRUN; i++) {
        if(struna[i].velocity2send>0){
            Serial.write(0x91); //cmd
            Serial.write(struna[i].note); //pitch
            Serial.write(struna[i].velocity2send); //velocity
            Serial.write(0x81); //cmd
            Serial.write(struna[i].note); //pitch
            Serial.write(0); //velocity
            struna[i].velocity2send=0;
        }
    }
}
void setup() {
    struna[0].pin=A4;
    struna[0].note=50;
    struna[1].pin=A1;
    struna[1].note=54;

    Serial.begin(115200);
    for (int i=0; i<POCET_STRUN; i++) { 
        pinMode(struna[i].pin, INPUT);
        struna[i].signal_avg=analogRead(struna[i].pin);
    } //nacita prvu hodnotu, aby sa priemer prilis nelisil
}

void loop() {
    for (int j=0; j<POCET_STRUN; j++){

        int signal_read=analogRead(struna[j].pin);
        int signal=signal_read-struna[j].signal_avg;

        if (struna[j].edge_down_number<3) {
            if (struna[j].edge_max_hold<signal) struna[j].edge_max_hold = signal;
            else struna[j].edge_down_number++;
            goto koniec;
        } //skratka ked sleduje zostupnu hranu
        else if (struna[j].edge_down_number==3) {
            int vystup=(signal*1.5)+38;
            vystup = max(vystup, 30);
            vystup = min(vystup, 126);
            struna[j].edge_down_number++;
            struna[j].velocity2send=vystup;
        }

        unsigned long myTime = millis();        
        if (signal>2 && struna[j].flag1==0) {
            struna[j].flag1=true; 
            struna[j].flag2=true; 
            struna[j].time_flag=myTime;      
            struna[j].edge_down_number=0;
        }

        if (struna[j].flag2==false) {struna[j].signal_avg=(49*struna[j].signal_avg + signal_read) / 50;}
        struna[j].signal_avg=(999*struna[j].signal_avg + signal_read) / 1000;

        if ((struna[j].flag1==true) && (myTime - struna[j].time_flag)>30) {struna[j].flag1=0;}
        if ((struna[j].flag2==true) && (myTime - struna[j].time_flag)>300) {struna[j].flag2=0;}
        
    }
    send_notes();

    koniec:
    ;

}