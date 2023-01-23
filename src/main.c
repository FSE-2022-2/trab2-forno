#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "uart-modbus.h"
#include "bme280.h"
#include "linux_userspace.h"
#include "pid.h"
#include "controle_forno.h"
// return command from get_command function
int command, uart0_filestream, running, functioning_state, on_state, control_mode;
read_uart_return_t ans;

void *get_command();
void *interface();
void *get_ambient_temperature();
void *parse_command();
void *oven_process();
// void* get_command(void* uart_arg);

int main(int argc, const char *argv[])
{

    uart0_filestream = -1;
    uart0_filestream = open_uart(uart0_filestream);
    running = 1;
    command = 0;
    // send command 0xA1 to uart
    uart0_filestream = write_commands(TURN_OFF_OVEN, uart0_filestream, 0, 0);
    // read command from uart in a thread
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, get_command, NULL);
    pthread_join(thread_id, NULL);

    // check if command change
    printf("command: %d\n", command);
    // if command change, send command to uart

    // send command to uart
    // uart0_filestream = write_commands(command, uart0_filestream, 0, 0); //change

    // get ambient temperature
    //  get_ambient_temperature();
    uart0_filestream = write_commands(TURN_ON_OVEN, uart0_filestream, 0, 0);
    uart0_filestream = write_commands(START_OVEN, uart0_filestream, 0, 0);
    oven_process();

    close(uart0_filestream);

    return 0;
}

void *get_command()
{
    // unsigned char tx_buffer[256];
    // unsigned char *p_tx_buffer = &tx_buffer[0];
    int count_test = 0;

    while (running)
    {
        // if input is cntl+c, stop running
        if (count_test == 10)
        {
            running = 0;
        }

        // 3 for 0xC3
        // uart0_filestream = read_commands(3, *(int*)uart_arg, tx_buffer, p_tx_buffer);
        uart0_filestream = read_commands(GET_COMMAND, uart0_filestream);
        // copy read_uart return to ans

        ans = read_uart(uart0_filestream);
        // uart0_filestream = ans[0];
        // command = ans[1];
        uart0_filestream = ans.uart0_filestream;
        command = ans.command;

        parse_command();
        // sleep 500ms
        usleep(500 * 1000);
        count_test++;
    }
    return NULL;
}

void *interface()
{
    // interface with user
    return NULL;
}

void *get_ambient_temperature()
{
    // read ambient temperature
    float ambient_temperature = init_bme280();
    printf("Ambient temperature: %f\n", ambient_temperature);
    // send ambient temperature to uart
    uart0_filestream = write_commands(SEND_AMBIENT_TEMPERATURE, uart0_filestream, ambient_temperature, 0);
    return NULL;
}

void *parse_command()
{
    // parse command
    if (command == TURN_ON_OVEN)
    {
        uart0_filestream = write_commands(TURN_ON_OVEN, uart0_filestream, 0, 0);
        // // read turn on oven command
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        // // convert ans.value to int
        on_state = (int)ans.value;
    }
    else if (command == TURN_OFF_OVEN)
    {
        uart0_filestream = write_commands(TURN_OFF_OVEN, uart0_filestream, 0, 0);
        // // read turn on oven command
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        // // convert ans.value to int
        on_state = (int)ans.value;
        on_state = 0;
    }
    else if (command == START_OVEN)
    {
       oven_process();
    }
    else if (command == STOP_OVEN)
    {
        uart0_filestream = write_commands(STOP_OVEN, uart0_filestream, 0, 0);
        // read stop oven command
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        // convert ans.value to int
        functioning_state = (int)ans.value;
    }
    else if (command == TOGGLE_CONTROL_MODE)
    {
        uart0_filestream = write_commands(TOGGLE_CONTROL_MODE, uart0_filestream, 0, !functioning_state);
    }
    else if (command == SEND_AMBIENT_TEMPERATURE)
    {
        // uart0_filestream = write_commands(SEND_AMBIENT_TEMPERATURE, uart0_filestream, get_ambient_temperature, 0);
        get_ambient_temperature();
    }
    else
    {
        printf("Command not found - %d\n", command);
    }
    return NULL;
}

void* oven_process()
{
    uart0_filestream = write_commands(START_OVEN, uart0_filestream, 0, 0);
    // read start oven command
    ans = read_uart(uart0_filestream);
    uart0_filestream = ans.uart0_filestream;
    // convert ans.value to int
    functioning_state = (int)ans.value;
    printf("functioning_state: %d\n", functioning_state);
    functioning_state = 1;
    pwm_init();

    // loop ate forno parar
    while (functioning_state == 1)
    {
        // solicitar temperatura interna
        uart0_filestream = read_commands(GET_INTERNAL_TEMPERATURE, uart0_filestream);
        // copy read_uart return to ans
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        float internal_temperature = ans.inside_temperature;
        printf("Internal temperature: %f\n", internal_temperature);
        // solicitar temperatura referência
        uart0_filestream = read_commands(GET_REFERENCE_TEMPERATURE, uart0_filestream);
        // copy read_uart return to ans
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        float reference_temperature = ans.reference_temperature;
        printf("Reference temperature: %f\n", reference_temperature);
        // solicitar temperatura ambiente
        get_ambient_temperature();
        // send control mode to uart
        uart0_filestream = write_commands(SEND_CONTROL_MODE, uart0_filestream, 0, functioning_state);
        // calcular pid
        pid_atualiza_referencia(reference_temperature);
        // calcula sinal de controle
        float control_signal = pid_controle(internal_temperature);
        // convert float to int
        int control_signal_int = (int)control_signal;
        // send control signal to uart
        uart0_filestream = write_commands(SEND_CONTROL_SIGNAL, uart0_filestream, 0, control_signal_int);
        
    }
}