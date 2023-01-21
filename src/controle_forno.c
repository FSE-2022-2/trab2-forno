#include<wiringPi.h>
#include<softPwm.h>
#include<stdio.h>

#include "controle_forno.h"

// create funtions to control pwm
// ventoinha is the pin 23 of the raspberry pi gpio
// resistor is the pin 24 of the raspberry pi gpio

void pwm_init(){
    wiringPiSetup();
    softPwmCreate(VENTOINHA_PIN, 0, 100);
    softPwmCreate(RESISTENCIA_PIN, 0, 100);
}

void pwm_ventoinha(int pwm){
    softPwmWrite(VENTOINHA_PIN, pwm);
}

void pwm_resistencia(int pwm){
    softPwmWrite(RESISTENCIA_PIN, pwm);
}