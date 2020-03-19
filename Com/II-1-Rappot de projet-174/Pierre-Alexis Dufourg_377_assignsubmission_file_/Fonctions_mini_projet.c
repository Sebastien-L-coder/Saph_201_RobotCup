//Fonctions du mini projet

#include "Bibliotheque_mini_projet.h"



///////
//I2C//
///////

void MPU6050_write_reg(uint8_t reg_num, uint8_t valeur) {

	uint8_t I2CMasterBuffer[3]; // ad, #reg, valeur
	uint8_t I2CWriteLength = 2;
	I2CMasterBuffer[0] = MPU6050_ADDR;
	I2CMasterBuffer[1] = reg_num;
	I2CMasterBuffer[2] = valeur;
	I2CmasterWrite(I2CMasterBuffer, I2CWriteLength);
}

uint8_t MPU6050_read_reg(uint8_t reg_num) {
	uint8_t I2CMasterBuffer[2]; // ad, #reg
	uint8_t I2CSlaveBuffer[1];
	uint8_t I2CWriteLength = 1;
	uint8_t I2CReadLength = 1;
	I2CMasterBuffer[0] = MPU6050_ADDR;
	I2CMasterBuffer[1] = reg_num;
	I2CmasterWriteRead(I2CMasterBuffer, I2CSlaveBuffer, I2CWriteLength,
			I2CReadLength);
	// autre possibilité :
//	I2CmasterWrite(I2CMasterBuffer, I2CWriteLength );
//	I2CmasterRead( MCP23_I2C_AD, I2CSlaveBuffer, I2CReadLength );
	return I2CSlaveBuffer[0];
}

void MPU6050_read_reg2(uint8_t reg_num, uint8_t count, uint8_t * dest) {///////////////////////////////////////////////////////////////////////////
	uint8_t I2CMasterBuffer[2]; // ad, #reg
	//uint8_t I2CSlaveBuffer[15];
	uint8_t I2CWriteLength = 1;
	if (count>15)count=15;
	uint8_t I2CReadLength = count;
	I2CMasterBuffer[0] = MPU6050_ADDR;
	I2CMasterBuffer[1] = reg_num;
	I2CmasterWriteRead(I2CMasterBuffer, dest, I2CWriteLength,
			I2CReadLength);
	// autre possibilité :
//	I2CmasterWrite(I2CMasterBuffer, I2CWriteLength );
//	I2CmasterRead( MCP23_I2C_AD, I2CSlaveBuffer, I2CReadLength );
	//for(int ii = 0; ii < count; ii++) {
	     //dest[ii] = I2CSlaveBuffer[ii];
	//}
}



/////////////
//Fonctions//
/////////////

float aRes; // scale resolutions per LSB for the sensors
uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
uint16_t ii, packet_count, fifo_count;
int32_t gyro_bias[3] = { 0, 0, 0 };
int32_t accel_bias[3] = { 0, 0, 0 };

// Set initial input parameters
enum Ascale {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
};
enum Gscale {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
};

// Specify sensor full scale
int Gscale = GFS_250DPS;
int Ascale = AFS_2G;

void init_carte(void) {

	//Configuration de l'horloge à 15 MHz
	LPC_PWRD_API->set_fro_frequency(30000);

	// Peripheral reset to the GPIO0 and pin interrupt modules. '0' asserts, '1' deasserts reset.
	LPC_SYSCON->PRESETCTRL0 &= (GPIO0_RST_N & GPIOINT_RST_N);
	LPC_SYSCON->PRESETCTRL0 |= ~(GPIO0_RST_N & GPIOINT_RST_N);

	//Mise en fonctionnement des périphériques utilisés
	LPC_SYSCON->SYSAHBCLKCTRL0 |= (IOCON | GPIO0 | SWM | CTIMER0 | GPIO_INT
			| MRT | UART1);
}

void config_liaison_serie(void) {
	LPC_PWRD_API->set_fro_frequency(30000); //on règle la fréquence d'horloge à 30Hz, qui ser par la suite divisé par 2
	LPC_SYSCON->SYSAHBCLKCTRL0 |= IOCON | GPIO0 | SWM | UART0; //active les ports, dont UART0
	LPC_SYSCON->UART0CLKSEL = 0; //configuration de l'horloge de la liaison UART0
	LPC_GPIO_PORT->DIR0 |= (1<<17); //activation de LED2
	LPC_SWM->PINASSIGN0 &= 0xFFFF0004; // FF ne fait rien, FF ne fait rien, 00 connecte U0_RXD à PIO0_0, 04 connecte U0_TXD à PIO0_4
	LPC_SYSCON->PRESETCTRL0 &= (UART0_RST_N); // reset USART0
	LPC_SYSCON->PRESETCTRL0 |= ~(UART0_RST_N); // fin reset USART0
	LPC_USART0->BRG = 7; // Configuration registre USART0 BRG avec la formule BRGVAL à arrondir pour fonctionner à 115200 bits/s
    LPC_USART0->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_1; // Configure registre USART0 CFG avec donnée de 8 bits, pas de parité et  1 bit de stop
    LPC_USART0->OSR = 15; // Configuration registre USART0 CTL avec aucune modification (15 par défaut)
    LPC_USART0->CTL = 0; // Configuration registre USART0 CTL avec aucune modification
    LPC_USART0->STAT = 0xFFFF; // initialise le statue à 0
    LPC_USART0->CFG |= UART_EN; // Active USART0
    }

void config_liaison_bluetooth(void) {
	LPC_SYSCON->SYSAHBCLKCTRL0 |= IOCON | GPIO0 | SWM | UART1; //active les ports, dont UART0
	LPC_SYSCON->UART1CLKSEL = 0; //configuration de l'horloge de la liaison UART1
	LPC_SWM->PINASSIGN1 &= 0xFF1412FF; // FF ne fait rien, 14 (=20 en base 10) connecte UART_RX à PO20, 12 (=18 en base 10) connecte UART_TX à PO18
	LPC_SYSCON->PRESETCTRL0 &= (UART1_RST_N); // reset USART1
	LPC_SYSCON->PRESETCTRL0 |= ~(UART1_RST_N); // fin reset USART1
	LPC_USART1->BRG = 7; // Configuration registre USART0 BRG avec la formule BRGVAL à arrondir pour fonctionner à 115200 bits/s
	LPC_USART1->CFG = (1 << 2); // Configure registre USART1 CFG avec donnée de 8 bits, pas de parité et 1 bit de stop
	LPC_USART1->CTL = 0; // Configuration registre USART1 CTL avec aucune modification
	LPC_USART1->STAT = 0xFFFF; // initialise le statue à 0
	LPC_USART1->CFG |= UART_EN; // Active USART1
}

void reset_MPU6050(void) {
	MPU6050_write_reg(PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
	for (int i = 0; i < 10000; i++)
		;
}

void calibrate_MPU6050(float * dest1, float * dest2) {

	// reset device, reset all registers, clear gyro and accelerometer bias registers
	MPU6050_write_reg(PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
	for (int i = 0; i < 10000; i++);

	// get stable time source
	// Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
	MPU6050_write_reg(PWR_MGMT_1, 0x01);
	MPU6050_write_reg(PWR_MGMT_2, 0x00);
	for (int i = 0; i < 20000; i++);

	// Configure device for bias calculation
	MPU6050_write_reg(INT_ENABLE, 0x00);  // Disable all interrupts
	MPU6050_write_reg(FIFO_EN, 0x00);      // Disable FIFO
	MPU6050_write_reg(PWR_MGMT_1, 0x00); // Turn on internal clock source
	MPU6050_write_reg(I2C_MST_CTRL, 0x00); // Disable I2C master
	MPU6050_write_reg(USER_CTRL, 0x00); // Disable FIFO and I2C master modes
	MPU6050_write_reg(USER_CTRL, 0x0C);    // Reset FIFO and DMP
	for (int i = 0; i < 1500; i++);

	// Configure MPU6050 gyro and accelerometer for bias calculation
	MPU6050_write_reg(CONFIG, 0x01); // Set low-pass filter to 188 Hz
	MPU6050_write_reg(SMPLRT_DIV, 0x00); // Set sample rate to 1 kHz
	MPU6050_write_reg(GYRO_CONFIG, 0x00); // Set gyro full-scale to 250 degrees per second, maximum sensitivity
	MPU6050_write_reg(ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity

	uint16_t gyrosensitivity = 131;   // = 131 LSB/degrees/sec
	uint16_t accelsensitivity = 16384;  // = 16384 LSB/g

	// Configure FIFO to capture accelerometer and gyro data for bias calculation
	MPU6050_write_reg(USER_CTRL, 0x40);   // Enable FIFO
	MPU6050_write_reg(FIFO_EN, 0x78); // Enable gyro and accelerometer sensors for FIFO  (max size 1024 bytes in MPU-6050)
	for (int i = 0; i < 8000; i++);

	// At end of sample accumulation, turn off FIFO sensor read
	MPU6050_write_reg(FIFO_EN, 0x00); // Disable gyro and accelerometer sensors for FIFO
	MPU6050_read_reg2(FIFO_COUNTH, 2, &data[0]); // read FIFO sample count
	fifo_count = ((uint16_t) data[0] << 8) | data[1];
	packet_count = fifo_count / 12; // How many sets of full gyro and accelerometer data for averaging

	for (ii = 0; ii < packet_count; ii++) {
		int16_t accel_temp[3] = { 0, 0, 0 }, gyro_temp[3] = { 0, 0, 0 };
		MPU6050_read_reg2(FIFO_R_W, 12, &data[0]); // read data for averaging
		accel_temp[0] = (int16_t) (((int16_t) data[0] << 8) | data[1]); // Form signed 16-bit integer for each sample in FIFO
		accel_temp[1] = (int16_t) (((int16_t) data[2] << 8) | data[3]);
		accel_temp[2] = (int16_t) (((int16_t) data[4] << 8) | data[5]);
		gyro_temp[0] = (int16_t) (((int16_t) data[6] << 8) | data[7]);
		gyro_temp[1] = (int16_t) (((int16_t) data[8] << 8) | data[9]);
		gyro_temp[2] = (int16_t) (((int16_t) data[10] << 8) | data[11]);

		accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
		accel_bias[1] += (int32_t) accel_temp[1];
		accel_bias[2] += (int32_t) accel_temp[2];
		gyro_bias[0] += (int32_t) gyro_temp[0];
		gyro_bias[1] += (int32_t) gyro_temp[1];
		gyro_bias[2] += (int32_t) gyro_temp[2];

	}
	accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
	accel_bias[1] /= (int32_t) packet_count;
	accel_bias[2] /= (int32_t) packet_count;
	gyro_bias[0] /= (int32_t) packet_count;
	gyro_bias[1] /= (int32_t) packet_count;
	gyro_bias[2] /= (int32_t) packet_count;

	if (accel_bias[2] > 0L) {
		accel_bias[2] -= (int32_t) accelsensitivity;
	}  // Remove gravity from the z-axis accelerometer bias calculation
	else {
		accel_bias[2] += (int32_t) accelsensitivity;
	}

	// Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
	data[0] = (-gyro_bias[0] / 4 >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
	data[1] = (-gyro_bias[0] / 4) & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
	data[2] = (-gyro_bias[1] / 4 >> 8) & 0xFF;
	data[3] = (-gyro_bias[1] / 4) & 0xFF;
	data[4] = (-gyro_bias[2] / 4 >> 8) & 0xFF;
	data[5] = (-gyro_bias[2] / 4) & 0xFF;

	// Push gyro biases to hardware registers
	MPU6050_write_reg(XG_OFFS_USRH, data[0]);
	MPU6050_write_reg(XG_OFFS_USRL, data[1]);
	MPU6050_write_reg(YG_OFFS_USRH, data[2]);
	MPU6050_write_reg(YG_OFFS_USRL, data[3]);
	MPU6050_write_reg(ZG_OFFS_USRH, data[4]);
	MPU6050_write_reg(ZG_OFFS_USRL, data[5]);

	dest1[0] = (float) gyro_bias[0] / (float) gyrosensitivity; // construct gyro bias in deg/s for later manual subtraction
	dest1[1] = (float) gyro_bias[1] / (float) gyrosensitivity;
	dest1[2] = (float) gyro_bias[2] / (float) gyrosensitivity;

	// Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
	// factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
	// non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
	// compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
	// the accelerometer biases calculated above must be divided by 8.

	int32_t accel_bias_reg[3] = { 0, 0, 0 }; // A place to hold the factory accelerometer trim biases
	MPU6050_read_reg2(XA_OFFSET_H, 2, &data[0]); // Read factory accelerometer trim values
	accel_bias_reg[0] = (int16_t) ((int16_t) data[0] << 8) | data[1];
	MPU6050_read_reg2(YA_OFFSET_H, 2, &data[0]);
	accel_bias_reg[1] = (int16_t) ((int16_t) data[0] << 8) | data[1];
	MPU6050_read_reg2(ZA_OFFSET_H, 2, &data[0]);
	accel_bias_reg[2] = (int16_t) ((int16_t) data[0] << 8) | data[1];

	uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
	uint8_t mask_bit[3] = { 0, 0, 0 }; // Define array to hold mask bit for each accelerometer bias axis

	for (ii = 0; ii < 3; ii++) {
		if (accel_bias_reg[ii] & mask)
			mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
	}

	// Construct total accelerometer bias, including calculated average accelerometer bias from above
	accel_bias_reg[0] -= (accel_bias[0] / 8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
	accel_bias_reg[1] -= (accel_bias[1] / 8);
	accel_bias_reg[2] -= (accel_bias[2] / 8);

	data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
	data[1] = (accel_bias_reg[0]) & 0xFF;
	data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
	data[3] = (accel_bias_reg[1]) & 0xFF;
	data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
	data[5] = (accel_bias_reg[2]) & 0xFF;
	data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

	// Push accelerometer biases to hardware registers
	//  writeByte(MPU6050_ADDRESS, XA_OFFSET_H, data[0]);
	//  writeByte(MPU6050_ADDRESS, XA_OFFSET_L_TC, data[1]);
	//  writeByte(MPU6050_ADDRESS, YA_OFFSET_H, data[2]);
	//  writeByte(MPU6050_ADDRESS, YA_OFFSET_L_TC, data[3]);
	//  writeByte(MPU6050_ADDRESS, ZA_OFFSET_H, data[4]);
	//  writeByte(MPU6050_ADDRESS, ZA_OFFSET_L_TC, data[5]);

	// Output scaled accelerometer biases for manual subtraction in the main program
	dest2[0] = (float) accel_bias[0] / (float) accelsensitivity;
	dest2[1] = (float) accel_bias[1] / (float) accelsensitivity;
	dest2[2] = (float) accel_bias[2] / (float) accelsensitivity;
}

void init_MPU6050(void) {

	// wake up device
	MPU6050_write_reg(PWR_MGMT_1, 0x00); // Clear sleep mode bit (6), enable all sensors
	for (int i = 0; i < 10000; i++)
		; // Delay 100 ms for PLL to get established on x-axis gyro; should check for PLL ready interrupt

	// get stable time source
	MPU6050_write_reg(PWR_MGMT_1, 0x01); // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001

	// Configure Gyro and Accelerometer
	// Disable FSYNC and set accelerometer and gyro bandwidth to 44 and 42 Hz, respectively;
	// DLPF_CFG = bits 2:0 = 010; this sets the sample rate at 1 kHz for both
	// Maximum delay is 4.9 ms which is just over a 200 Hz maximum rate
	MPU6050_write_reg(CONFIG, 0x03);

	// Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
	MPU6050_write_reg(SMPLRT_DIV, 0x04); // Use a 200 Hz rate; the same rate set in CONFIG above

	// Set gyroscope full scale range
	// Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
	uint8_t c = MPU6050_read_reg( GYRO_CONFIG);
	MPU6050_write_reg(GYRO_CONFIG, c & ~0xE0); // Clear self-test bits [7:5]
	MPU6050_write_reg(GYRO_CONFIG, c & ~0x18); // Clear AFS bits [4:3]
	MPU6050_write_reg(GYRO_CONFIG, c | Gscale << 3); // Set full scale range for the gyro

	// Set accelerometer configuration
	c = MPU6050_read_reg(ACCEL_CONFIG);
	MPU6050_write_reg( ACCEL_CONFIG, c & ~0xE0); // Clear self-test bits [7:5]
	MPU6050_write_reg( ACCEL_CONFIG, c & ~0x18); // Clear AFS bits [4:3]
	MPU6050_write_reg( ACCEL_CONFIG, c | Ascale << 3); // Set full scale range for the accelerometer

	// Configure Interrupts and Bypass Enable
	// Set interrupt pin active high, push-pull, and clear on read of INT_STATUS, enable I2C_BYPASS_EN so additional chips
	// can join the I2C bus and all can be controlled by the Arduino as master
	MPU6050_write_reg(INT_PIN_CFG, 0x22);
	MPU6050_write_reg(INT_ENABLE, 0x01); // Enable data ready (bit 0) interrupt
}

void readAccelData(int16_t * destination)
{
  uint8_t rawData[6];  // x/y/z accel register data stored here
  MPU6050_read_reg2(ACCEL_XOUT_H, 6, &rawData[0]);  // Read the six raw data registers into data array
  destination[0] = (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ;  // Turn the MSB and LSB into a signed 16-bit value
  destination[1] = (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;
  destination[2] = (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ;
}

void getAres() {
  switch (Ascale)
  {
    // Possible accelerometer scales (and their register bit settings) are:
    // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
    // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case AFS_2G:
          aRes = 2.0/32768.0;
          break;
    case AFS_4G:
          aRes = 4.0/32768.0;
          break;
    case AFS_8G:
          aRes = 8.0/32768.0;
          break;
    case AFS_16G:
          aRes = 16.0/32768.0;
          break;
  }
}

