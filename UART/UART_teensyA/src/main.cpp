// TEENSY A - USB CONNECTED
#include <Arduino.h>
#include <TeensyTimerTool.h>
#include <vector>
#include <cmath>
using namespace std;
using namespace TeensyTimerTool;

int ping_counter = 0;
vector<int> recorded_times(10000);
PeriodicTimer t1(TCK_RTC);

void exchange_ping();

void analyze_results() {
    int total_time = 0;
    for (int t : recorded_times) {
        total_time += t;
    }
    float mean = static_cast<float>(total_time/10000.f);

    float sigma = 0;
    for (int t : recorded_times) {
        sigma += ((t-mean)*(t-mean));
    }

    float standard_dev = sqrt(sigma/9999.f);

    Serial.println("\n==== 10k PING SEND RESULTS ====");
    Serial.printf("Total Time: %.6fsec\n", static_cast<float>(total_time/1000000.0));
    Serial.printf("Mean Time/Exchange: %f\n", mean);
    Serial.printf("Standard Deviation: %f\n", standard_dev);

    delay(1000);
    ping_counter = 0;
    t1.begin(exchange_ping, 100);
}

void exchange_ping() {
    int initial_time = micros();
    int data = 123456789;

    //SEND DATA...
    Serial3.write((char*)&data, 4);
    Serial3.flush();

    //WAIT FOR RESPONSE...
    while (Serial3.available() < 4) {
        ;
    }

    //READ RESPONSE...
    int received;
    Serial3.readBytes((char*)&received, 4);

    float elapsed_time = micros() - initial_time;
    recorded_times.at(ping_counter) = elapsed_time;
    ping_counter++;
    if (ping_counter >= 10000) {
        t1.stop();
        analyze_results();
    }
}

void setup() {
    Serial.begin(115200);
    Serial3.begin(115200);

    while (!Serial);
    attachErrFunc(ErrorHandler(Serial));
    t1.begin(exchange_ping, 100);
}

void loop() {
    ;
}