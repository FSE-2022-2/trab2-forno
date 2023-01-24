#ifndef PWD_H_
#define PWD_H_

#define FAN_PIN 5
#define RESISTOR_PIN 4

void pwm_init(void);
void fan_pwm(int);
void resistor_pwm(int);

#endif /* PWD_H_ */
