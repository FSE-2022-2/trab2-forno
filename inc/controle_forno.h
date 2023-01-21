#ifndef PWD_H_
#define PWD_H_

#define VENTOINHA_PIN 5
#define RESISTENCIA_PIN 4

void pwm_init(void);
void pwm_ventoinha(int);
void pwm_resistencia(int);

#endif /* PWD_H_ */
