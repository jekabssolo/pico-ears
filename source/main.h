#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
extern "C" {
    #include "pico/pdm_microphone.h"
}
#include "tusb.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

// App settings
const bool VIBRATE_ON_ALERT = true;
const bool USE_STDIO = true;
const bool USE_ALERT = true;

// Define used GPIO pins
const uint STATUS_LED = 25;
const uint ALERT_LED = 2;
const uint ALERT_VIBRO = 17;
const uint MIC_DAT = 18;
const uint MIC_CLK = 19;

// Microphone configuration
const struct pdm_microphone_config config = {
    // GPIO pin for the PDM DAT signal
    .gpio_data = MIC_DAT,

    // GPIO pin for the PDM CLK signal
    .gpio_clk = MIC_CLK,

    // PIO instance to use
    .pio = pio0,

    // PIO State Machine instance to use
    .pio_sm = 0,

    // sample rate in Hz
    .sample_rate = 16000,

    // number of samples to buffer
    .sample_buffer_size = 256,
};