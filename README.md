# Flipper Zero SCD41 CO2 Monitor

CO2 monitor and data logger for Flipper Zero with SCD41 sensor.

<img src="https://github.com/Moaske/FZero_scd41_co2/blob/main/docs/scd41_co2meter_flipperzero.jpg"></img>

The original repo by https://github.com/matthinc/fz_scd30_co2, of which this is a fork, started as a port of https://github.com/thzinc/flipperzero-firmware to a recent version and evolved to a completetly new project. 

I then took Matthinc's repo to adapt it to the SCD41 sensor

## Wiring
- SCD41 VIN <--> 3.3V
- SCD41 GND <--> GND
- SCD41 SCL <--> PC0
- SCD41 SDA <--> PC1

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

## Enclosure

Currently working on 3D Printed enclosure for the small Pimoroni sensor. Will also publish .STL files as soon as I have a satisfactory design.
So far this is the first prototype:

<img src="https://github.com/Moaske/FZero_scd41_co2/blob/main/docs/enclosure_prototyping.jpg"></img>
