#pragma once

#include <furi_hal_i2c.h>
#include <furi.h>
#include <furi_hal.h>
#include <string>
#include <vector>

struct SCD41Data {
    float co2_ppm;
    float temperature;
    float humidity;

    std::string ts;

    bool result_valid;
};

class FlipperSCD41 {
public:
    FlipperSCD41() = default;
    virtual ~FlipperSCD41() = default;

    bool send_command(std::vector<uint8_t> cmd);
    bool send_command(std::vector<uint8_t> cmd, std::vector<uint8_t> data);
    bool send_command_and_read(
        std::vector<uint8_t> cmd,
        uint8_t* result,
        int len,
        uint32_t exec_time_ms);

    bool start_periodic_measurement();
    bool stop_periodic_measurement();
    bool data_ready();
    bool perform_forced_recalibration(uint16_t target_ppm);
    bool set_automatic_self_calibration(bool enable);

    SCD41Data read_measurements();
};

class FlipperSCD41WorkerThread {
public:
    explicit FlipperSCD41WorkerThread(int interval);
    virtual ~FlipperSCD41WorkerThread();

    void stop();
    void start();

    void calibrate_to(uint16_t ppm);

    SCD41Data get_data();
    bool has_data();

    int interval;
    uint16_t next_calibration = 0;
    bool running = false;
    bool data_available = false;
    FuriThread* thread;
    FlipperSCD41 scd41;
    SCD41Data last_data;
};
