//Main du mini projet

#include "Bibliotheque_mini_projet.h"

int main(void) {

	// Bias corrections for gyro and accelerometer
	float gyroBias[3] = { 0, 0, 0 };
	float accelBias[3] = { 0, 0, 0 };

	// scale resolutions per LSB for the sensors
	float aRes = 1;

	// Stores the 16-bit signed accelerometer sensor output
	int16_t accelCount[3];

	// Stores the real accel value
	float ax = 0.0, ay = 0.0;

	// Chaine de caractère transmise
	char text[50];

	// Indice de caractère dans la chaine de caractère
	int i = 0;

	// Accélération suivant l'axe x et y
	int axx, ayy = 0;

	//Configuration des composants
	init_carte(); // Initialisation de la carte
	init_lcd(); // Initialisation de l'écran pour initialiser l'I2C
	//config_liaison_serie(); //Configuration de la liaison série
	config_liaison_bluetooth(); //Configuration de la liaison bluetooth
	reset_MPU6050(); // Reset registers to default in preparation for device calibration
	calibrate_MPU6050(gyroBias, accelBias); // Calibrate gyro and accelerometers, load biases in bias registers
	init_MPU6050(); // Initialize device for active mode read of acclerometer, gyroscope, and temperature

	while (1) {

		if (MPU6050_read_reg(INT_STATUS) & 0x01) { // check if data ready interrupt
			readAccelData(accelCount);  // Read the x/y/z adc values
			getAres(); // Determine accelerometer scales

			// Now we'll calculate the accleration value into actual g's
			ax = (float) accelCount[0] * aRes - accelBias[0]; // get actual g value, this depends on scale being set
			ay = (float) accelCount[1] * aRes - accelBias[1];

			if ((ax <= 3000) && (ax >= -3000)) // Permet de ne pas tenir compte des légères inclinaisons
				axx = 0;
			else if(ax<-3000) // Si on incline vers l'avant
				axx = 2;
			else // Si on incline vers l'arrière
				axx = 1;


			if ((ay <= 3000) && (ay >= -3000)) // Permet de ne pas tenir compte des légères inclinaisons
				ayy = 0;
			 else if(ay<-3000) // Si on incline vers la gauche
				ayy = 2;
			 else // Si on incline vers la droite
				 ayy=1;

			sprintf(text, "%d%d\r\n", axx, ayy);

			i = 0; // Mise à 0 de l'indice du caractère

			do
			{
				if ((LPC_USART1->STAT & TXRDY) != 0) //si l'on peut envoyer des données depuis la carte
				{
					LPC_USART1->TXDAT = text[i]; //envoyer le caractère d'indice i de chaine
					i++; // incrémenter l'indice
				}
			} while (text[i] != '\0');
		}
	}
}

