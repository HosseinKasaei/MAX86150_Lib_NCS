#include <zephyr/logging/log.h>
#include "max86150.h"

LOG_MODULE_REGISTER(max86150, LOG_LEVEL_DBG);

int max86150_init(const struct i2c_dt_spec *dev_i2c, struct max86150_config *config, 
            struct max86150_data *data) {

    uint8_t part_ID;
    uint8_t mode_cfg;
    int i;
    
    // Check if I2C device is ready
    if (!device_is_ready(dev_i2c->bus)) {
		LOG_ERR("I2C Bus %s is not ready\n", dev_i2c->bus->name);
		return -1;
	}   
    
    // Read device part ID
    if (i2c_reg_read_byte_dt(dev_i2c, MAX86150_REG_ID, &part_ID)) {
        LOG_ERR("Could not get Part ID\n");
        return -1;
    }
    // Check if part ID is correct
    if (part_ID != MAX86150_PART_ID) {
        LOG_ERR("Got part ID 0x%02x, expected 0x%02x", part_ID, MAX86150_PART_ID);
        return -1;
    }

    // Reset MAX86150 before configuration
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_SYSTEM_CONTROL, MAX86150_SYS_CTRL_RESET_MASK)) {
        LOG_ERR("MAX86150 Reset failed");
        return -1;
    }
    // Check if reseting is done
	do {
		if (i2c_reg_read_byte_dt(dev_i2c, MAX86150_REG_SYSTEM_CONTROL, &mode_cfg)) {
            LOG_ERR("Reset state read failed");
            return -1;
        }
	} while (mode_cfg & MAX86150_SYS_CTRL_RESET_MASK);

	// FIFO Config
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_FIFO_CONFIG, MAX86150_FIFO_ROLLS_ON_FULL_MASK)){
        LOG_ERR("FIFO config failed");
        return -1;
    }

    // FIFO Time Slots
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_FIFO_DATA_CONTROL1, 
                (config->slot[1] << 4) | (config->slot[0]))) {
        LOG_ERR("FIFO Data Control1 write failed");
        return -1;
    }
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_FIFO_DATA_CONTROL2, 
                (config->slot[3] << 4) | (config->slot[2]))) {
        LOG_ERR("FIFO Data Control2 write failed");
        return -1;        
    }

	// PPG Configs
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_PPG_CONFIG1,
                 (config->adc_range << 6) | (config->ppg_sample_rate << 2) | (config->ppg_pulse_width))) {
        LOG_ERR("PPG config1 reg write failed");
        return -1;
    }	
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_PPG_CONFIG2, config->ppg_sample_avg)) {
        LOG_ERR("PPG config2 reg write failed");
        return -1;
    }

	// LEDs PA set
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_LED1_PA, config->IR_LED_PA)) {
        LOG_ERR("LED1 PA reg write failed");
        return -1;
    }
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_LED2_PA, config->RED_LED_PA)) {
        LOG_ERR("LED2 PA reg write failed");
        return -1;
    }
    
	// LED Current Ranges
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_LED_RANGE,
                 (config->Red_current << 2) | (config->IR_current))) {
        LOG_ERR("led range reg write failed");
        return -1;
    }
	
	// LED Pilot PA
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_LED_PILOT_PA, config->PILOT_LED_PA)) {
        LOG_ERR("LED Pilot PA reg write failed");
        return -1;
    }

    // ECG Configuration
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_ECG_CONFIG1, config->ECG_sample_rate)){
        LOG_ERR("ECG config 1 reg write failed");
        return -1;
    }
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_ECG_CONFIG3,
                 (config->ECG_PGA_gain << 2) | (config->ECG_IA_gain))){
        LOG_ERR("ECG config 3 reg write failed");
        return -1;
    }

	// Start FIFO
	if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_SYSTEM_CONTROL, MAX86150_FIFO_ENABLE_MASK)) {
        LOG_ERR("system control reg write for fifo start failed");
        return -1;
    }

    // Counting number of channels and store it
    data->num_channels = 0;
    for (i = 0; i < 4; i++) {
        if (config->slot[i] != 0) data->num_channels++;
    }

    return 0;
}

int max86150_shutdown(const struct i2c_dt_spec *dev_i2c) {
    
    if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_SYSTEM_CONTROL, MAX86150_SHUTDOWN_MASK)) {
        LOG_ERR("MAX86150 shutdown failed");
        return -1;
    }

    return 0;
}

int max86150_setPA_IR(const struct i2c_dt_spec *dev_i2c, uint8_t PA_value) {

    if  (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_LED1_PA, PA_value)) {
        LOG_ERR("Red LED PA set Failed");
        return -1;
    }
    return 0;
}

int max86150_setPA_Red(const struct i2c_dt_spec *dev_i2c, uint8_t PA_value) {

    if  (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_LED2_PA, PA_value)) {
        LOG_ERR("Red LED PA set Failed");
        return -1;
    }
    return 0;
}

int max86150_wakeup(const struct i2c_dt_spec *dev_i2c) {
    
    if (i2c_reg_write_byte_dt(dev_i2c, MAX86150_REG_SYSTEM_CONTROL, MAX86150_WAKEUP_MASK)) {
        LOG_ERR("MAX86150 wakeup failed");
        return -1;
    }

    return 0;
}

int max86150_fetch_data(const struct i2c_dt_spec *dev_i2c, struct max86150_config *config, 
            struct max86150_data *data) {
    
    int num_bytes = 3 * data->num_channels;            
    uint8_t buffer[num_bytes];
    uint32_t fifo_data;
    int fifo_chan;
    int i;

    
    if (i2c_burst_read_dt(dev_i2c, MAX86150_REG_FIFO_DATA, buffer, num_bytes)) {
        LOG_ERR("Data register burst read failed");
        return -1;
    }

    fifo_chan = 0;
    for (i = 0; i < num_bytes; i += 3) {
        fifo_data = (buffer[i] << 16) | (buffer[i+1] << 8) | (buffer[i+2]);
        // if (config->slot[i/3] != 9) fifo_data &= MAX86150_PPG_FIFO_DATA_MASK; // if data is ecg don't mask it

        data->channel_data[fifo_chan++] = fifo_data;
    }

    return 0;
}