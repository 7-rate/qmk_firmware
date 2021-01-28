#include <stdint.h>
#include "i2c_master.h"

/************************************/
/* Global Functions                 */
/************************************/
void bme280_init(void);
void bme280_exec(void);
double bme280_getTemp(void);
double bme280_getPress(void);
double bme280_getHum(void);

/************************************/
/* Global Definitions               */
/************************************/
#define BME280_ADDRESS (0x76<<1)

#define BME280_REG_CALIB00   (0x88)
#define BME280_REG_CALIB25   (0xA1)
#define BME280_REG_CALIB26   (0xE1)
#define BME280_REG_CTRL_HUM  (0xF2)
#define BME280_REG_CTRL_MEAS (0xF4)
#define BME280_REG_CONFIG    (0xF5)
#define BME280_REG_PRESS_MSB (0xf7)

#define I2C_BME280_TIMEOUT (20)

/******************************/
/* BME280 configurator values */
/******************************/
/* [2:0]         Humidity oversampling */
/* 000           Skipped               */
/* 001           oversampling x1       */
/* 010           oversampling x2       */
/* 011           oversampling x4       */
/* 100           oversampling x8       */
/* 101,others    oversampling x16      */
#define BME280_CTRL_HUM_VAL (0x01)


/* [7:5]         Pressure oversampling    */
/* 000           Skipped                  */
/* 001           oversampling x1          */
/* 010           oversampling x2          */
/* 011           oversampling x4          */
/* 100           oversampling x8          */
/* 101,others    oversampling x16         */

/* [4:2]         Temperature oversampling */
/* 000           Skipped                  */
/* 001           oversampling x1          */
/* 010           oversampling x2          */
/* 011           oversampling x4          */
/* 100           oversampling x8          */
/* 101,others    oversampling x16         */

/* [1:0]         Mode                     */
/* 00            Sleep mode               */
/* 11            Normal mode              */
#define BME280_CTRL_MEAS_VAL (0x27)

/* [7:5]        t_standby[ms]       */
/* 000          0.5                 */
/* 001          62.5                */
/* 010          125                 */
/* 011          250                 */
/* 100          500                 */
/* 101          1000                */
/* 110          10                  */
/* 111          20                  */

/* [4:2]        Filter corefficient */
/* 000          Filter off          */
/* 001          2                   */
/* 010          4                   */
/* 011          8                   */
/* 100,others   16                  */

/* [0]          SPI interface       */
/* 0            4-wire              */
/* 1            3-wire              */
#define BME280_CONFIG_VAL (0xA0)


