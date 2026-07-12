# Flipper Zero SCD41 CO2 Monitor

CO2 monitor and data logger for Flipper Zero with (Pimoroni breakout) SCD41 sensor.

<img src="https://github.com/Moaske/FZero_scd41_co2/blob/main/docs/scd41_co2meter_flipperzero.jpg"></img>   <img src="https://github.com/Moaske/FZero_scd41_co2/blob/main/docs/wiring_screen.png"></img>

The original repo by https://github.com/matthinc/fz_scd30_co2, of which this is a fork, started as a port of https://github.com/thzinc/flipperzero-firmware to a recent version and evolved to a completetly new project. 

I then forked Matthinc's repo to adapt it to the SCD41 sensor. Currently version 1.2.0
### v1.2
- Added wiring screen at startup
### v1.1
- The CSV logging was simplified
- Temperature now shows one decimal accuracy.

## Wiring
- SCD41 VIN <--> 3.3V  <-->  RED
- SCD41 GND <--> GND  <-->  BLACK
- SCD41 SCL <--> PC0  <-->  BLUE
- SCD41 SDA <--> PC1  <-->  YELLOW

(Colours refer to Qwiic JST cable on the connector)

## Features

- Displays CO2, Temperature, Humidity
- Continous logging to a CSV file
- LED Color based on current CO2 level (500 = green ... 2500 = red)
- Bar graph (0 ... 3000ppm)
- Calibration

## Calibrate sensor

Place the sensor outside for at least 5 minutes - push and hold the "UP" button. The CO2 sensor will calibrate to 420 ppm.
Important  note on the SCD41 calibration:

    // FRC is very different from the SCD30's calibrate command:
    //  - the sensor must be in idle mode (call stop_periodic_measurement()
    //    first), it will NACK this while periodic measurement is running
    //  - the sensor should have been running continuously for >= 3 minutes
    //    before this is called, for the correction to be meaningful
    //  - it takes ~400ms to execute, much longer than a normal command

<img src="https://github.com/Moaske/FZero_scd41_co2/blob/main/docs/PXL_20260708_083506756.jpeg" width=800></img>

The SCD41 has a known temperature abberation that is caused by the pre-heating of the sensor itself. Depending on how you deploy it (in a very open or more closed enclosure), you may need to tweak this line of code in `flipperscd41.cpp` :

``// Tune this after calibrating against a reference thermometer -- see the
// procedure in the project README / commit message. Factory default is
// 4.0C; self-heating in your specific enclosure may need more or less.
// static constexpr float SCD41_TEMPERATURE_OFFSET_C = 4.0f;``

Where 4.0f is the default offset, which you can increment with 1 for every degree you want to lower the Temp reading.
Check with a reference thermometer for at least 5 to 10 minutes.

## Enclosure

Currently working on 3D Printed enclosure for the small Pimoroni sensor. Will also publish .STL files as soon as I have a satisfactory design.
So far this is the first prototype:

<img src="https://github.com/Moaske/FZero_scd41_co2/blob/main/docs/enclosure_prototyping.jpg"></img>
