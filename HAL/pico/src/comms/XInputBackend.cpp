#include "comms/XInputBackend.hpp"

#include "core/CommunicationBackend.hpp"
#include "core/state.hpp"

#include "modes/MeleeLimits.hpp"

#include <Adafruit_USBD_XInput.hpp>

XInputBackend::XInputBackend(InputSource **input_sources, size_t input_source_count, bool nerfOn)
    : CommunicationBackend(input_sources, input_source_count) {
    Serial.end();
    _xinput = new Adafruit_USBD_XInput();
    _xinput->begin();
    Serial.begin(115200);

    _nerfOn = nerfOn;

    TinyUSBDevice.setID(0x0738, 0x4726);

    while (!_xinput->ready()) {
        delay(1);
    }
}

XInputBackend::~XInputBackend() {
    delete _xinput;
}

void XInputBackend::SendReport() {
    //ScanInputs(InputScanSpeed::SLOW);
    //ScanInputs(InputScanSpeed::MEDIUM);

    while (!_xinput->ready()) {
        tight_loop_contents();
    }

    static uint32_t minLoop = 16384;
    static uint loopCount = 0;
    static bool detect = true;//false means run
    static uint sampleCount = 1;
    static uint32_t sampleSpacing = 0;
    static uint32_t oldSampleTime = 0;
    static uint32_t newSampleTime = 0;
    static uint32_t loopTime = 0;
    //const uint fastestLoop = 950; //fastest possible loop for AVR
    const uint fastestLoop = 450; //fastest possible loop for rp2040

    oldSampleTime = newSampleTime;
    //YOU CANNOT USE MICROS ON AVR
    //You need to manually use the low level timers
    newSampleTime = micros();
    loopTime = newSampleTime - oldSampleTime;

    if(detect) {
        //run loop time detection procedure
#ifdef TIMINGDEBUG
        gpio_put(1, loopCount%2 == 0);
#endif
        loopCount++;
        if(loopCount > 5 && loopTime > 300) {//screen out implausibly fast samples; the limit is 2500 Hz (400 us)
            minLoop = min(minLoop, loopTime);
        }
        if(loopCount >= 100) {
            detect = false;
            sampleCount = 1;
            /* we generally run at 1khz so don't increase the sample rate
            while(1000*sampleCount <= minLoop) {//we want [sampleCount] ms-spaced samples within the smallest possible loop
                sampleCount++;
            }
            */
            sampleSpacing = minLoop / sampleCount;
            if(sampleSpacing < fastestLoop && sampleCount > 1) {
                sampleCount--;
                sampleSpacing = minLoop / sampleCount;
            }
        }
    } else {
        //no delay stuff here
        for (uint i = 0; i < sampleCount; i++) {
            ScanInputs(InputScanSpeed::FAST);

            // Run gamemode logic.
            UpdateOutputs();

            //if(_nerfOn) {
            if(_gamemode->isMelee()) {
                //APPLY NERFS HERE
                OutputState nerfedOutputs;
                limitOutputs(sampleSpacing/4, _nerfOn ? AB_A : AB_B, _inputs, _outputs, nerfedOutputs);

                // Digital outputs
                _report.a = nerfedOutputs.a;
                _report.b = nerfedOutputs.b;
                _report.x = nerfedOutputs.x;
                _report.y = nerfedOutputs.y;
                _report.lb = nerfedOutputs.buttonL;
                _report.rb = nerfedOutputs.buttonR;
                _report.lt = nerfedOutputs.triggerLDigital ? 255 : nerfedOutputs.triggerLAnalog;
                _report.rt = nerfedOutputs.triggerRDigital ? 255 : nerfedOutputs.triggerRAnalog;
                _report.start = nerfedOutputs.start;
                _report.back = nerfedOutputs.select;
                _report.home = nerfedOutputs.home;
                _report.dpad_up = nerfedOutputs.dpadUp;
                _report.dpad_down = nerfedOutputs.dpadDown;
                _report.dpad_left = nerfedOutputs.dpadLeft;
                _report.dpad_right = nerfedOutputs.dpadRight;
                _report.ls = nerfedOutputs.leftStickClick;
                _report.rs = nerfedOutputs.rightStickClick;

                // Analog outputs
                _report.lx = (nerfedOutputs.leftStickX - 128) * 65535 / 255 + 128;
                _report.ly = (nerfedOutputs.leftStickY - 128) * 65535 / 255 + 128;
                _report.rx = (nerfedOutputs.rightStickX - 128) * 65535 / 255 + 128;
                _report.ry = (nerfedOutputs.rightStickY - 128) * 65535 / 255 + 128;
            } else {
                // Digital outputs
                _report.a = _outputs.a;
                _report.b = _outputs.b;
                _report.x = _outputs.x;
                _report.y = _outputs.y;
                _report.lb = _outputs.buttonL;
                _report.rb = _outputs.buttonR;
                _report.lt = _outputs.triggerLDigital ? 255 : _outputs.triggerLAnalog;
                _report.rt = _outputs.triggerRDigital ? 255 : _outputs.triggerRAnalog;
                _report.start = _outputs.start;
                _report.back = _outputs.select;
                _report.home = _outputs.home;
                _report.dpad_up = _outputs.dpadUp;
                _report.dpad_down = _outputs.dpadDown;
                _report.dpad_left = _outputs.dpadLeft;
                _report.dpad_right = _outputs.dpadRight;
                _report.ls = _outputs.leftStickClick;
                _report.rs = _outputs.rightStickClick;

                // Analog outputs
                _report.lx = (_outputs.leftStickX - 128) * 65535 / 255 + 128;
                _report.ly = (_outputs.leftStickY - 128) * 65535 / 255 + 128;
                _report.rx = (_outputs.rightStickX - 128) * 65535 / 255 + 128;
                _report.ry = (_outputs.rightStickY - 128) * 65535 / 255 + 128;
            }
        }
        if(loopTime > minLoop+(minLoop >> 1)) {//if the loop time is 50% longer than expected
            detect = true;//stop scanning inputs briefly and re-measure timings
            loopCount = 0;
        }
    }

    _xinput->sendReport(&_report);
}
