
#include <zephyr/drivers/i2c.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(imu, LOG_LEVEL_ERR);

float imu_x, imu_y, imu_z;
bool imu_recognized_upside_down;

static const struct device* dev_i2c = DEVICE_DT_GET(DT_NODELABEL(arduino_i2c));

const int addr_lis3dh = 0x19;

uint8_t read_lis3dh(uint8_t addr)
{
	uint8_t buf;
	if (i2c_write_read(dev_i2c, addr_lis3dh, &addr, 1, &buf, 1))
	{
		LOG_ERR("lis3dh read failed");
		return 0xff;
	}

	LOG_DBG("lis3dh read %02x -> %02x", addr, buf);
	return buf;
}

void write_lis3dh(uint8_t addr, uint8_t value)
{
	uint8_t buf[] = {addr, value};
	if (i2c_write(dev_i2c, buf, 2, addr_lis3dh))
	{
		LOG_ERR("lis3dh write failed");
	}
}


void lis3dh_calc_value(uint16_t raw_value, float *final_value, bool isAccel) {
    // Convert with respect to the value being temperature or acceleration reading 
    float scaling;
    float senstivity = 0.004f; // g per unit

    if (isAccel == true) {
        scaling = 64 / senstivity;
    } else {
        scaling = 64;
    }

    // raw_value is signed
    *final_value = (float) ((int16_t) raw_value) / scaling;
}

void lis3dh_read_data(uint8_t reg, float *final_value, bool IsAccel) {
    // Read two bytes of data and store in a 16 bit data structure
    uint8_t lsb = read_lis3dh(reg);
    reg |= 0x01;
    uint8_t msb = read_lis3dh(reg);

    uint16_t raw_accel = (msb << 8) | lsb;

    LOG_INF("lis3dh_read_data(%d) -> %d", reg, raw_accel);

    lis3dh_calc_value(raw_accel, final_value, IsAccel);
}

bool ok = true;

#define IMU_REG_CTRL_REG_1 0x20
#define IMU_REG_CTRL_REG_4 0x23
#define IMU_REG_CTRL_REG_5 0x24
#define IMU_REG_TEMP_CFG_REG 0x1f

bool did_post_imu = false;

void imu_init()
{
	uint8_t whoami = read_lis3dh(0x0F);
	LOG_INF("lis3dh whoami -> %02x (expect 33)", whoami);
	if (whoami != 0x33) 
	{
		LOG_ERR("lis3dh whoami=%02x ???", whoami);
		ok = false;
		return;
	}

	write_lis3dh(IMU_REG_CTRL_REG_5,   0b10000000); // "reboot memory contents"

	k_msleep(10);

	write_lis3dh(IMU_REG_CTRL_REG_1,   0b10010111);
    write_lis3dh(IMU_REG_CTRL_REG_4,   0b10000000);
    write_lis3dh(IMU_REG_TEMP_CFG_REG, 0b10000000); // 0b10000000 enable ADC; 0b11000000 enable ADC + wire ADC3 to temp

    k_msleep(10);

    imu_update();

    float sum_accel = sqrt(imu_x*imu_x + imu_y*imu_y + imu_z*imu_z);

    did_post_imu = (sum_accel > 0.8) && (sum_accel < 1.2);
}

void imu_update()
{
	if (!ok) return;

	lis3dh_read_data(0x28, &imu_x, true);
    lis3dh_read_data(0x2A, &imu_y, true);
    lis3dh_read_data(0x2C, &imu_z, true);

    imu_recognized_upside_down = (imu_y > 0.8) && (fabs(imu_x) < 0.2) && (fabs(imu_z) < 0.2);
}
