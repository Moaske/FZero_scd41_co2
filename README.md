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

<img src="https://github.com/Moaske/FZero_scd41_co2/blob/main/docs/PXL_20260708_083506756.jpeg" width=800></img>
