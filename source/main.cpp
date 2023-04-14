#include "main.h"

bool alert_is_on = false;
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {};

// microphone variables
int16_t sample_buffer[256];
volatile int samples_read = 0;
bool do_inference = false;

void on_pdm_samples_ready()
{
    // callback from library when all the samples in the library
    // internal sample buffer are ready for reading 
    samples_read = pdm_microphone_read(sample_buffer, 256);
}

void alert() {
    for (uint i = 0; i < 5; i++) {
        gpio_put(ALERT_LED, 1);
        if (VIBRATE_ON_ALERT) {
            gpio_put(ALERT_VIBRO, 1);
        }
        sleep_ms(250);
        gpio_put(ALERT_LED, 0);
        sleep_ms(250);
        gpio_put(ALERT_LED, 1);
        sleep_ms(250);
        gpio_put(ALERT_LED, 0);
        gpio_put(ALERT_VIBRO, 0);
        sleep_ms(250);
    }
}

void init_gpio() {
    // Initialize GPIO pins
    gpio_init(STATUS_LED);
    gpio_init(ALERT_LED);
    gpio_init(ALERT_VIBRO);
    // Setup GPIO pins
    gpio_set_dir(STATUS_LED, GPIO_OUT);
    gpio_set_dir(ALERT_LED, GPIO_OUT);
    gpio_set_dir(ALERT_VIBRO, GPIO_OUT);
}

void init_microphone() {
    // Initialize the PDM microphone
    if (pdm_microphone_init(&config) < 0) {
        printf("PDM microphone initialization failed!\n");
        while (1) { tight_loop_contents(); }
    }

    // Set callback that is called when all the samples in the library
    // internal sample buffer are ready for reading
    pdm_microphone_set_samples_ready_handler(on_pdm_samples_ready);
    
     // Start capturing data from the PDM microphone
    if (pdm_microphone_start() < 0) {
        printf("PDM microphone start failed!\n");
        while (1) { tight_loop_contents(); }
    }
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    // copy the features into the output buffer
    memcpy(out_ptr, &features[offset], length * sizeof(float));
    return 0;
}

void core1_main() {
    ei_impulse_result_t result = {nullptr};
    // add feature processing
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;
    ei_sleep(1000);
    while (true) {
        gpio_put(STATUS_LED, !gpio_get(STATUS_LED));
        // Check if the features array is the correct size
        if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            ei_printf("The size of your 'features' array is not correct. Expected %d items, but had %u\n",
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
            break;
        }

        // invoke the impulse
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);

        if (res != 0) {
            ei_printf("run_classifier returned: %d\n", res);
            break;
        }

        if (USE_STDIO) {
            ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);

            // print the predictions
            ei_printf("[");
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                ei_printf("%.5f", result.classification[ix].value);
                #if EI_CLASSIFIER_HAS_ANOMALY == 1
                    ei_printf(", ");
                #else
                    if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
                        ei_printf(", ");
                    }
                #endif
            }
            #if EI_CLASSIFIER_HAS_ANOMALY == 1
                printf("%.3f", result.anomaly);
            #endif
                printf("]\n");
        }

        if (result.classification[0].value < result.classification[1].value && result.classification[1].value > 0.9) {
            ei_printf("%s %.5f\n", result.classification[1].label, result.classification[1].value);
            alert();
            continue;
        }
        ei_printf("%s %.5f\n", result.classification[0].label, result.classification[0].value);
    }
}

int main() {
    init_gpio();
    stdio_usb_init();
    init_microphone();

    size_t feature_ix = 0;

    // Start core 1 for inferencing
    multicore_launch_core1(core1_main);

    while(true) {
        feature_ix = 0;
        while (feature_ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            // Wait for new samples
            while (samples_read == 0) { tight_loop_contents(); }

            // Store and clear the samples read from the callback
            int sample_count = samples_read;
            samples_read = 0;
            
            // Loop through any new collected samples
            for (int i = 0; i < sample_count; i++) {
                // Store the sample in the features array
                if (feature_ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
                    features[feature_ix++] = (float)sample_buffer[i];
                }
            }
        }
    }

    return 0;
}
