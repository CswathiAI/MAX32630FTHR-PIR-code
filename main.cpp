#include "mbed.h"
#include <cstdio>
#include <cstring>

using namespace std::chrono;

// PIR motion sensor
DigitalIn pir(P5_4);

// RGB LEDs are active LOW
DigitalOut redLED(P2_4);
DigitalOut greenLED(P2_5);

// Serial terminal
BufferedSerial pc(USBTX, USBRX, 115200);

constexpr auto MOTION_HOLD_TIME = 20s;

void showRed()
{
    redLED = 0;
    greenLED = 1;
}

void showGreen()
{
    redLED = 1;
    greenLED = 0;
}

void printMessage(const char *message)
{
    pc.write(message, strlen(message));
}

int main()
{
    pc.set_format(8, BufferedSerial::None, 1);

    showGreen();

    bool redActive = false;
    int previousRemaining = -1;

    auto lastMotionTime = Kernel::Clock::now();

    printMessage("\r\nPIR Motion Detection Started\r\n");
    printMessage("Status: GREEN - Waiting for motion\r\n");

    while (true)
    {
        auto now = Kernel::Clock::now();

        // Any HIGH signal restarts the 20-second timer
        if (pir.read() == 1)
        {
            lastMotionTime = now;

            if (!redActive)
            {
                redActive = true;
                showRed();
                previousRemaining = -1;

                printMessage("\r\nMotion detected! RED LED ON\r\n");
            }
        }

        if (redActive)
        {
            auto elapsed = now - lastMotionTime;

            int remaining = 20 -
                duration_cast<seconds>(elapsed).count();

            if (remaining < 0)
                remaining = 0;

            // Print only when the displayed second changes
            if (remaining != previousRemaining)
            {
                char message[80];

                int len = snprintf(
                    message, sizeof(message),
                    "Red LED ON | Time remaining: %02d seconds\r\n",
                    remaining);

                pc.write(message, len);
                previousRemaining = remaining;
            }

            // Return to green after 20 seconds without motion
            if (elapsed >= MOTION_HOLD_TIME)
            {
                redActive = false;
                showGreen();

                printMessage(
                    "Countdown complete. GREEN LED ON\r\n"
                    "Waiting for motion...\r\n");

                previousRemaining = -1;
            }
        }

        ThisThread::sleep_for(50ms);
    }
}
