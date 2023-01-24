#include<wiringPi.h>
#include<softPwm.h>
#include<stdio.h>

#include "oven_controller.h"

// create funtions to control pwm
// ventoinha is the pin 23 of the raspberry pi gpio
// resistor is the pin 24 of the raspberry pi gpio

void pwm_init(){
    wiringPiSetup();
    softPwmCreate(FAN_PIN, 0, 100);
    softPwmCreate(RESISTOR_PIN, 0, 100);
}

void fan_pwm(int pwm){
    softPwmWrite(FAN_PIN, pwm);
}

void resistor_pwm(int pwm){
    softPwmWrite(RESISTOR_PIN, pwm);
}