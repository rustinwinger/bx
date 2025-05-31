#include "comms/DInputBackend.hpp"

#include "core/CommunicationBackend.hpp"
#include "core/state.hpp"

#include "modes/MeleeLimits.hpp"

#include <TUGamepad.hpp>

DInputBackend::DInputBackend(InputSource **input_sources, size_t input_source_count, bool nerfOn)
    : CommunicationBackend(input_sources, input_source_count) {
    _gamepad = new TUGamepad();
    _gamepad->begin();

    _nerfOn = nerfOn;

    while (!USBDevice.mounted()) {
        delay(1);
    }
}

DInputBackend::~DInputBackend() {
    _gamepad->resetInputs();
    delete _gamepad;
}

void DInputBackend::SendReport() {
    //ScanInputs(InputScanSpeed::SLOW);
    //ScanInputs(InputScanSpeed::MEDIUM);

    while (!_gamepad->ready()) {
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
                _gamepad->setButton(0, nerfedOutputs.b);
                _gamepad->setButton(1, nerfedOutputs.a);
                _gamepad->setButton(2, nerfedOutputs.y);
                _gamepad->setButton(3, nerfedOutputs.x);
                _gamepad->setButton(4, nerfedOutputs.buttonR);
                _gamepad->setButton(5, nerfedOutputs.triggerRDigital);
                _gamepad->setButton(6, nerfedOutputs.buttonL);
                _gamepad->setButton(7, nerfedOutputs.triggerLDigital);
                _gamepad->setButton(8, nerfedOutputs.select);
                _gamepad->setButton(9, nerfedOutputs.start);
                _gamepad->setButton(10, nerfedOutputs.rightStickClick);
                _gamepad->setButton(11, nerfedOutputs.leftStickClick);
                _gamepad->setButton(12, nerfedOutputs.home);

                // Analog outputs
                _gamepad->leftXAxis(nerfedOutputs.leftStickX);
                _gamepad->leftYAxis(255 - nerfedOutputs.leftStickY);
                _gamepad->rightXAxis(nerfedOutputs.rightStickX);
                _gamepad->rightYAxis(255 - nerfedOutputs.rightStickY);
                _gamepad->triggerLAnalog(nerfedOutputs.triggerLAnalog + 1);
                _gamepad->triggerRAnalog(nerfedOutputs.triggerRAnalog + 1);

                // D-pad Hat Switch
                _gamepad->hatSwitch(nerfedOutputs.dpadLeft, nerfedOutputs.dpadRight, nerfedOutputs.dpadDown, nerfedOutputs.dpadUp);
            } else {
                // Digital outputs
                _gamepad->setButton(0, _outputs.b);
                _gamepad->setButton(1, _outputs.a);
                _gamepad->setButton(2, _outputs.y);
                _gamepad->setButton(3, _outputs.x);
                _gamepad->setButton(4, _outputs.buttonR);
                _gamepad->setButton(5, _outputs.triggerRDigital);
                _gamepad->setButton(6, _outputs.buttonL);
                _gamepad->setButton(7, _outputs.triggerLDigital);
                _gamepad->setButton(8, _outputs.select);
                _gamepad->setButton(9, _outputs.start);
                _gamepad->setButton(10, _outputs.rightStickClick);
                _gamepad->setButton(11, _outputs.leftStickClick);
                _gamepad->setButton(12, _outputs.home);

                // Analog outputs
                _gamepad->leftXAxis(_outputs.leftStickX);
                _gamepad->leftYAxis(255 - _outputs.leftStickY);
                _gamepad->rightXAxis(_outputs.rightStickX);
                _gamepad->rightYAxis(255 - _outputs.rightStickY);
                _gamepad->triggerLAnalog(_outputs.triggerLAnalog + 1);
                _gamepad->triggerRAnalog(_outputs.triggerRAnalog + 1);

                // D-pad Hat Switch
                _gamepad->hatSwitch(_outputs.dpadLeft, _outputs.dpadRight, _outputs.dpadDown, _outputs.dpadUp);
            }
        }
        if(loopTime > minLoop+(minLoop >> 1)) {//if the loop time is 50% longer than expected
            detect = true;//stop scanning inputs briefly and re-measure timings
            loopCount = 0;
        }
    }

    _gamepad->sendState();
}
