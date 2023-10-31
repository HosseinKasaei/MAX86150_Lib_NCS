// MAX86150 Library for NCS v2.x.x by Hossein Kasaei - 2023
// To use this library in nRF Connect SDK you have to inculde these lines in the devicetree overlay file.
// Select i2c driver based on your need i2c0 or i2c1 or...
// &i2c0{
//     max86150: max86150@5e{
//         compatible = "i2c-device";
//         reg = <0x5e>;
//         label = "MAX86150";
//     };
// };

#ifndef MAX86150_H
#define MAX86150_H

#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define MAX86150_I2C_ADDR 0x5E

// Register addresses
#define MAX86150_REG_INTR_STATUS_1          0x00
#define MAX86150_REG_INTR_STATUS_2          0x01
#define MAX86150_REG_INTR_ENABLE_1          0x02
#define MAX86150_REG_INTR_ENABLE_2          0x03
#define MAX86150_REG_FIFO_WRITE_PTR         0x04
#define MAX86150_REG_FIFO_OVERFLOW_CTR      0x05
#define MAX86150_REG_FIFO_READ_PTR          0x06
#define MAX86150_REG_FIFO_DATA              0x07
#define MAX86150_REG_FIFO_CONFIG            0x08
#define MAX86150_REG_FIFO_DATA_CONTROL1     0x09
#define MAX86150_REG_FIFO_DATA_CONTROL2     0x0A
#define MAX86150_REG_SYSTEM_CONTROL         0x0D
#define MAX86150_REG_PPG_CONFIG1            0x0E
#define MAX86150_REG_PPG_CONFIG2            0x0F
#define MAX86150_REG_PROX_INTR_THRESHOLD    0x10
#define MAX86150_REG_LED1_PA                0x11 // LED1 -> IR
#define MAX86150_REG_LED2_PA                0x12 // LED2 -> RED
#define MAX86150_REG_LED_RANGE              0x14
#define MAX86150_REG_LED_PILOT_PA           0x15
#define MAX86150_REG_ECG_CONFIG1            0x3C
#define MAX86150_REG_ECG_CONFIG3            0x3E
#define MAX86150_REG_ID                     0xFF

#define MAX86150_PART_ID                    0x1E

// Mask Defines
#define MAX86150_SYS_CTRL_RESET_MASK        0x01 
#define MAX86150_FIFO_ROLLS_ON_FULL_MASK    (1 << 4)
#define MAX86150_FIFO_ENABLE_MASK           0x04
#define MAX86150_IR_TIME_SLOT_MASK          0x01
#define MAX86150_RED_TIME_SLOT_MASK         0x02
#define MAX86150_PILOT_LED1_TIME_SLOT_MASK  0x05
#define MAX86150_PILOT_LED2_TIME_SLOT_MASK  0x06
#define MAX86150_ECG_TIME_SLOT_MASK         0x09
#define MAX86150_SHUTDOWN_MASK              0x02
#define MAX86150_WAKEUP_MASK                0x04
#define MAX86150_PPG_FIFO_DATA_MASK         ((1 << 19) - 1)

enum max86150_slot {
    MAX86150_SLOT_DISABLED = 0,
    MAX86150_SLOT_IR_LED1 = MAX86150_IR_TIME_SLOT_MASK,
    MAX86150_SLOT_RED_LED2 = MAX86150_RED_TIME_SLOT_MASK,
    MAX86150_SLOT_PILOT_LED1 = MAX86150_PILOT_LED1_TIME_SLOT_MASK,
    MAX86150_SLOT_PILOT_LED2 = MAX86150_PILOT_LED2_TIME_SLOT_MASK,
    MAX86150_SLOT_ECG = MAX86150_ECG_TIME_SLOT_MASK
};

enum max86150_adc_range {
    PPG_ADC_RANGE_4096 = 0, // LSB = 7.8125 pA, Full Scale = 4096 nA
    PPG_ADC_RANGE_8192, // LSB = 15.625 pA, Full Scale = 8192 nA
    PPG_ADC_RANGE_16384, // LSB = 31.25 pA, Full Scale = 16384 nA
    PPG_ADC_RANGE_32768  // LSB = 62.5 pA, Full Scale = 32768 nA
};

// Datasheet page 29 : If a sample rate is set that can not be supported by the selected pulse width
// and LED mode then the highest available sample rate will be automatically set. The user can read back 
// this register to confirm the sample rate.
enum max86150_ppg_sample_rate {
    PPG_SAMPLE_RATE_10_N_1 = 0, // 10 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_20_N_1, // 20 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_50_N_1, // 50 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_84_N_1, // 84 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_100_N_1, // 100 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_200_N_1, // 200 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_400_N_1, // 400 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_800_N_1, // 800 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_1000_N_1, // 1000 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_1600_N_1, // 1600 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_3200_N_1, // 800 Samples per second, N = 1 (Pulses per sample)
    PPG_SAMPLE_RATE_10_N_2, // 10 Samples per second, N = 2 (Pulses per sample)
    PPG_SAMPLE_RATE_20_N_2, // 20 Samples per second, N = 2 (Pulses per sample)
    PPG_SAMPLE_RATE_50_N_2, // 50 Samples per second, N = 2 (Pulses per sample)
    PPG_SAMPLE_RATE_84_N_2, // 84 Samples per second, N = 2 (Pulses per sample)
    PPG_SAMPLE_RATE_100_N_2, // 100 Samples per second, N = 2 (Pulses per sample)
};


enum max86150_ppg_pulse_width {
    PPG_PULSE_WIDTH_50 = 0, // Pulse width = 50us, Integration Time = 50us
    PPG_PULSE_WIDTH_100, // Pulse width = 100us, Integration Time = 100us
    PPG_PULSE_WIDTH_200, // Pulse width = 200us, Integration Time = 200us
    PPG_PULSE_WIDTH_400, // Pulse width = 400us, Integration Time = 400us
};

enum max86150_ppg_sample_avg {
    PPG_SAMPLE_AVG_1 = 0, // Average 1 sample (no average)
    PPG_SAMPLE_AVG_2, // Average 2 samples
    PPG_SAMPLE_AVG_4, // Average 4 samples
    PPG_SAMPLE_AVG_8, // Average 8 samples
    PPG_SAMPLE_AVG_16, // Average 16 samples
    PPG_SAMPLE_AVG_32, // Average 32 samples
};

enum max86150_led_range {
    PPG_LED_CURRENT_50 = 0, // LED Current range = 50 mA
    PPG_LED_CURRENT_100, // LED Current range = 100 mA
};

enum max86150_ecg_sample_rate {
    ECG_SAMPLE_RATE_1600 = 0, // Sample Rate = 1600 Hz, Filter BW (70%) = 420 Hz
    ECG_SAMPLE_RATE_800, // Sample Rate = 800 Hz, Filter BW (70%) = 210 Hz
    ECG_SAMPLE_RATE_400, // Sample Rate = 400 Hz, Filter BW (70%) = 105 Hz
    ECG_SAMPLE_RATE_200, // Sample Rate = 200 Hz, Filter BW (70%) = 52 Hz
    ECG_SAMPLE_RATE_3200, // Sample Rate = 3200 Hz, Filter BW (70%) = 840 Hz
    ECG_SAMPLE_RATE_1600_2,
    ECG_SAMPLE_RATE_800_2,
    ECG_SAMPLE_RATE_400_2,
};

enum max86150_ecg_pga_gain {
    ECG_PGA_GAIN_1 = 0, // PGA Gain = 1 V/V
    ECG_PGA_GAIN_2, // PGA Gain = 2 V/V
    ECG_PGA_GAIN_4, // PGA Gain = 4 V/V
    ECG_PGA_GAIN_8, // PGA Gain = 8 V/V
};

enum max86150_ecg_ia_gain {
    ECG_IA_GAIN_5 = 0, // IA Gain = 5 V/V
    ECG_IA_GAIN_9_5, // IA Gain = 9.5 V/V
    ECG_IA_GAIN_20, // IA Gain = 20 V/V
    ECG_IA_GAIN_50, // IA Gain = 50 V/V
};

struct max86150_config {
    uint8_t IR_LED_PA;
    uint8_t RED_LED_PA;
    uint8_t PILOT_LED_PA;
    enum max86150_slot slot[4];
    enum max86150_adc_range adc_range;
    enum max86150_ppg_sample_rate ppg_sample_rate;
    enum max86150_ppg_pulse_width ppg_pulse_width;
    enum max86150_ppg_sample_avg ppg_sample_avg;
    enum max86150_led_range IR_current;
    enum max86150_led_range Red_current;
    enum max86150_ecg_sample_rate ECG_sample_rate;
    enum max86150_ecg_pga_gain ECG_PGA_gain;
    enum max86150_ecg_ia_gain ECG_IA_gain;
};

struct max86150_data {
    uint32_t channel_data[4];
    uint8_t num_channels;
};

int max86150_init(const struct i2c_dt_spec *dev_i2c, struct max86150_config *config, struct max86150_data *data);
int max86150_shutdown(const struct i2c_dt_spec *dev_i2c);
int max86150_setPA_IR(const struct i2c_dt_spec *dev_i2c, uint8_t PA_value);
int max86150_setPA_Red(const struct i2c_dt_spec *dev_i2c, uint8_t PA_value);
int max86150_wakeup(const struct i2c_dt_spec *dev_i2c);
int max86150_fetch_data(const struct i2c_dt_spec *dev_i2c, struct max86150_config *config, struct max86150_data *data);

#endif /* MAX86150_H */

