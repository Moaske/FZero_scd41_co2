#include "flipperscd41.h"

#define UINT16_MSBS(val) static_cast<uint8_t>((val >> 8) & 0xFF)
#define UINT16_LSBS(val) static_cast<uint8_t>(val & 0xFF)

// SCD41/SCD40 share the same fixed I2C address (unlike SCD30's 0x61).
#define SCD41_ADDR 0x62

// Sensirion's CRC8 (poly 0x31, init 0xFF) is identical between SCD30 and
// SCD4x -- verified against Sensirion's embedded-i2c-scd4x sensirion_i2c.h.
static uint8_t crc(const std::vector<uint8_t>& data) {
    uint8_t crc = 0xFF;
    for(uint32_t x = 0; x < data.size(); x++) {
        crc ^= data[x];
        for(uint32_t i = 0; i < 8; i++) {
            if((crc & 0x80) != 0) {
                crc = (uint8_t)((crc << 1) ^ 0x31);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Every 16-bit word the SCD41 sends back is followed by its own CRC byte, so
// a 3-word response is 9 raw bytes: [MSB,LSB,CRC, MSB,LSB,CRC, MSB,LSB,CRC].
// word_index selects which of those 3-byte groups to decode.
static uint16_t buffer_to_word(uint8_t* buffer, int word_index) {
    int offset = word_index * 3;
    return (static_cast<uint16_t>(buffer[offset]) << 8) | buffer[offset + 1];
}

bool FlipperSCD41::send_command(std::vector<uint8_t> cmd) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    bool result =
        furi_hal_i2c_tx(&furi_hal_i2c_handle_external, (SCD41_ADDR << 1), cmd.data(), cmd.size(), 50);
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
    return result;
}

bool FlipperSCD41::send_command(std::vector<uint8_t> cmd, std::vector<uint8_t> data) {
    data.push_back(crc(data));
    cmd.insert(cmd.end(), data.begin(), data.end());
    return send_command(cmd);
}

// NOTE: this deliberately does NOT use furi_hal_i2c_trx(). On real firmware
// that single-acquisition tx+rx helper is unreliable for sensors that need a
// pause between the command and the response (see flipperzero-firmware issue
// #1670) -- and the SCD41 always needs one (its command execution time,
// e.g. ~1ms for read_measurement, up to 400ms for calibration). So we do two
// separate acquire/tx/release and acquire/rx/release cycles with a delay
// between them.
bool FlipperSCD41::send_command_and_read(
    std::vector<uint8_t> cmd,
    uint8_t* result,
    int len,
    uint32_t exec_time_ms) {
    if(!send_command(cmd)) {
        return false;
    }

    furi_delay_ms(exec_time_ms);

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    bool ok = furi_hal_i2c_rx(&furi_hal_i2c_handle_external, (SCD41_ADDR << 1), result, len, 50);
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
    return ok;
}

bool FlipperSCD41::start_periodic_measurement() {
    // High power mode: one measurement every 5s. There is no equivalent of
    // SCD30's set_interval() -- SCD41's interval is fixed by which start
    // command you use (this one, or 0x21ac for low-power/30s mode).
    return send_command({0x21, 0xb1});
}

bool FlipperSCD41::stop_periodic_measurement() {
    bool ok = send_command({0x3f, 0x86});
    // Datasheet: sensor only responds to other commands 500ms after this.
    furi_delay_ms(500);
    return ok;
}

bool FlipperSCD41::data_ready() {
    uint8_t buffer[3];
    if(!send_command_and_read({0xe4, 0xb8}, buffer, 3, 1)) {
        return false;
    }
    uint16_t status = buffer_to_word(buffer, 0);
    // Data is NOT ready only when the low 11 bits are all zero.
    return (status & 0x07FF) != 0;
}

bool FlipperSCD41::perform_forced_recalibration(uint16_t target_ppm) {
    // FRC is very different from the SCD30's calibrate command:
    //  - the sensor must be in idle mode (call stop_periodic_measurement()
    //    first), it will NACK this while periodic measurement is running
    //  - the sensor should have been running continuously for >= 3 minutes
    //    before this is called, for the correction to be meaningful
    //  - it takes ~400ms to execute, much longer than a normal command
    std::vector<uint8_t> result_bytes(3);
    bool ok = send_command_and_read(
        {0x36,
         0x2f,
         UINT16_MSBS(target_ppm),
         UINT16_LSBS(target_ppm),
         crc({UINT16_MSBS(target_ppm), UINT16_LSBS(target_ppm)})},
        result_bytes.data(),
        3,
        400);
    return ok;
}

bool FlipperSCD41::set_automatic_self_calibration(bool enable) {
    uint16_t val = enable ? 1 : 0;
    return send_command({0x24, 0x16}, {UINT16_MSBS(val), UINT16_LSBS(val)});
}

SCD41Data FlipperSCD41::read_measurements() {
    SCD41Data result;
    uint8_t buffer[9];

    result.result_valid = send_command_and_read({0xec, 0x05}, buffer, 9, 1);

    if(result.result_valid) {
        uint16_t co2_raw = buffer_to_word(buffer, 0);
        uint16_t temp_raw = buffer_to_word(buffer, 1);
        uint16_t rh_raw = buffer_to_word(buffer, 2);

        // Conversion formulas from the SCD4x datasheet -- these raw uint16
        // values are NOT IEEE-754 floats like the SCD30 sends, so this part
        // can't just reuse the old buffer_to_float() logic.
        result.co2_ppm = static_cast<float>(co2_raw);
        result.temperature = -45.0f + 175.0f * (static_cast<float>(temp_raw) / 65536.0f);
        result.humidity = 100.0f * (static_cast<float>(rh_raw) / 65536.0f);

        DateTime datetime;
        furi_hal_rtc_get_datetime(&datetime);

        auto pad2 = [](int n) { return (n < 10 ? "0" : "") + std::to_string(n); };
        result.ts = std::to_string(datetime.year) + "-" + pad2(datetime.month) + "-" + pad2(datetime.day) +
                    " " + pad2(datetime.hour) + ":" + pad2(datetime.minute) + ":" + pad2(datetime.second);
    }

    return result;
}

static int32_t run(void* context) {
    FlipperSCD41WorkerThread* worker_context =
        reinterpret_cast<FlipperSCD41WorkerThread*>(context);

    worker_context->scd41.start_periodic_measurement();
    // First measurement isn't ready until ~5s after start.
    furi_delay_ms(5000);

    while(worker_context->running) {
        if(worker_context->scd41.data_ready()) {
            SCD41Data data = worker_context->scd41.read_measurements();

            if(data.result_valid && data.co2_ppm > 0) {
                worker_context->last_data = data;
                worker_context->data_available = true;
            }
        }

        if(worker_context->next_calibration != 0) {
            // Unlike the SCD30, we can't calibrate while measuring: stop,
            // calibrate, then resume and wait out the warm-up again.
            worker_context->scd41.stop_periodic_measurement();
            worker_context->scd41.perform_forced_recalibration(worker_context->next_calibration);
            worker_context->next_calibration = 0;
            worker_context->scd41.start_periodic_measurement();
            furi_delay_ms(5000);
        }

        furi_delay_ms(worker_context->interval);
    }

    worker_context->scd41.stop_periodic_measurement();

    return 0;
}

FlipperSCD41WorkerThread::FlipperSCD41WorkerThread(int interval)
    : interval(interval) {
    thread = furi_thread_alloc();
    furi_thread_set_name(thread, "SensorWorker");
    furi_thread_set_stack_size(thread, 2048);
    furi_thread_set_context(thread, this);
    furi_thread_set_callback(thread, run);
}

FlipperSCD41WorkerThread::~FlipperSCD41WorkerThread() {
    furi_thread_free(thread);
}

void FlipperSCD41WorkerThread::stop() {
    running = false;
    furi_thread_join(thread);
}

void FlipperSCD41WorkerThread::start() {
    running = true;
    furi_thread_start(thread);
}

void FlipperSCD41WorkerThread::calibrate_to(uint16_t ppm) {
    next_calibration = ppm;
}

bool FlipperSCD41WorkerThread::has_data() {
    return data_available;
}

SCD41Data FlipperSCD41WorkerThread::get_data() {
    return last_data;
}
