#include "bme280.h"

/************************************/
/* Private Function prototypes      */
/************************************/
static void readTrim(void);
static void readData(void);
static int32_t calibration_T(int32_t adc_T);
static uint32_t calibration_P(int32_t adc_P);
static uint32_t calibration_H(int32_t adc_H);

/************************************/
/* Private Variables                */
/************************************/
static uint32_t hum_raw,temp_raw,pres_raw;
static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t dig_H1, dig_H3;
static int16_t dig_H2, dig_H4, dig_H5;
static int8_t dig_H6;
static int32_t t_fine;

/************************************/
/* Global Functions                 */
/************************************/
void bme280_init(void) {
    uint8_t ctrl_hum_reg;
    uint8_t ctrl_meas_reg;
    uint8_t config_reg;

    ctrl_hum_reg = BME280_CTRL_HUM_VAL;
    ctrl_meas_reg = BME280_CTRL_MEAS_VAL;
    config_reg = BME280_CONFIG_VAL;

    i2c_init();
    i2c_writeReg(BME280_ADDRESS, BME280_REG_CTRL_HUM, &ctrl_hum_reg, 1, I2C_BME280_TIMEOUT);
    i2c_writeReg(BME280_ADDRESS, BME280_REG_CTRL_MEAS, &ctrl_meas_reg, 1, I2C_BME280_TIMEOUT);
    i2c_writeReg(BME280_ADDRESS, BME280_REG_CONFIG, &config_reg, 1, I2C_BME280_TIMEOUT);
    readTrim();

    return;
}

void bme280_exec(void) {
    readData();

    return;
}

double bme280_getTemp(void) {
    double temp_act;
    int32_t temp_cal;

    temp_cal = calibration_T(temp_raw);
    temp_act = (double)temp_cal / 100.0;

    return temp_act;
}

double bme280_getPress(void) {
    double press_act;
    uint32_t press_cal;

    press_cal = calibration_P(pres_raw);
    press_act = (double)press_cal / 100.0;

    return press_act;
}

double bme280_getHum(void) {
    double hum_act;
    uint32_t hum_cal;

    hum_cal = calibration_H(hum_raw);
    hum_act = (double)hum_cal / 1024.0;

    return hum_act;
}

/************************************/
/* Private Functions                */
/************************************/
static void readTrim(void) {
    uint8_t data[32];

    i2c_readReg(BME280_ADDRESS, BME280_REG_CALIB00, &data[0], 24, I2C_BME280_TIMEOUT);
    i2c_readReg(BME280_ADDRESS, BME280_REG_CALIB25, &data[25], 1, I2C_BME280_TIMEOUT);
    i2c_readReg(BME280_ADDRESS, BME280_REG_CALIB26, &data[25], 7, I2C_BME280_TIMEOUT);

    dig_T1 = (data[1] << 8) | data[0];
    dig_T2 = (data[3] << 8) | data[2];
    dig_T3 = (data[5] << 8) | data[4];
    dig_P1 = (data[7] << 8) | data[6];
    dig_P2 = (data[9] << 8) | data[8];
    dig_P3 = (data[11]<< 8) | data[10];
    dig_P4 = (data[13]<< 8) | data[12];
    dig_P5 = (data[15]<< 8) | data[14];
    dig_P6 = (data[17]<< 8) | data[16];
    dig_P7 = (data[19]<< 8) | data[18];
    dig_P8 = (data[21]<< 8) | data[20];
    dig_P9 = (data[23]<< 8) | data[22];
    dig_H1 = data[24];
    dig_H2 = (data[26]<< 8) | data[25];
    dig_H3 = data[27];
    dig_H4 = (data[28]<< 4) | (0x0F & data[29]);
    dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
    dig_H6 = data[31];

    return;
}

static void readData(void) {
    uint8_t data[8];

    i2c_readReg(BME280_ADDRESS, 0xF7, &data[0], 8, I2C_BME280_TIMEOUT);

    pres_raw = data[0];
    pres_raw = (pres_raw<<8) | data[1];
    pres_raw = (pres_raw<<4) | (data[2] >> 4);

    temp_raw = data[3];
    temp_raw = (temp_raw<<8) | data[4];
    temp_raw = (temp_raw<<4) | (data[5] >> 4);

    hum_raw  = data[6];
    hum_raw  = (hum_raw << 8) | data[7];

    return;
}

static int32_t calibration_T(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T; 
}

static uint32_t calibration_P(int32_t adc_P) {
    int32_t var1, var2;
    uint32_t P;

    var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
    var2 = (((var1>>2) * (var1>>2)) >> 11) * ((int32_t)dig_P6);
    var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
    var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
    var1 = (((dig_P3 * (((var1>>2)*(var1>>2)) >> 13)) >>3) + ((((int32_t)dig_P2) * var1)>>1))>>18;
    var1 = ((((32768+var1))*((int32_t)dig_P1))>>15);
    if (var1 == 0) {
        return 0;
    }
    P = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
    if( P < 0x80000000 ) {
       P = (P << 1) / ((uint32_t) var1);
    } else {
        P = (P / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)dig_P9) * ((int32_t)(((P>>3) * (P>>3))>>13)))>>12;
    var2 = (((int32_t)(P>>2)) * ((int32_t)dig_P8))>>13;
    P = (uint32_t)((int32_t)P + ((var1 + var2 + dig_P7) >> 4));

    return P;
}

static uint32_t calibration_H(int32_t adc_H) {
    int32_t v_x1;

    v_x1 = (t_fine - ((int32_t)76800));
    v_x1 = (((((adc_H << 14) -(((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1)) + 
              ((int32_t)16384)) >> 15) * (((((((v_x1 * ((int32_t)dig_H6)) >> 10) * 
              (((v_x1 * ((int32_t)dig_H3)) >> 11) + ((int32_t) 32768))) >> 10) + (( int32_t)2097152)) * 
              ((int32_t) dig_H2) + 8192) >> 14));
    v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v_x1 = (v_x1 < 0 ? 0 : v_x1);
    v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);

    return (uint32_t)(v_x1 >> 12);   
}
