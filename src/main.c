#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "uart-modbus.h"
#include "bme280.h"
#include "I2C_controller.h"
#include "pid.h"
#include "oven_controller.h"
// return command from get_command function
int command, uart0_filestream, running, functioning_state, on_state, control_mode;
read_uart_return_t ans;
pthread_t thread_oven_process;
pthread_t thread_get_command;
pthread_t thread_interface;

// pthread_t thread_get_command;

void *get_command();
void *interface();
void *get_ambient_temperature();
void *parse_command();
void *oven_process();
void sigint_handler(int sig_num);
void finish_all();
void init();
void pid_init();
void parse_send_command(int send_command);
// void* get_command(void* uart_arg);

int main(int argc, const char *argv[])
{
    // treats ctrl+c
    init();
    signal(SIGINT, sigint_handler);
    pthread_create(&thread_get_command, NULL, get_command, NULL);
    pthread_join(thread_get_command, NULL);
    

    finish_all();

    return 0;
}

void *get_command()
{
    while (1)
    {
        uart0_filestream = read_commands(GET_COMMAND, uart0_filestream);
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        command = ans.command;
        parse_command();
        usleep(500 * 1000);
    }
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
        functioning_state = 1;
        pthread_create(&thread_oven_process, NULL, oven_process, NULL);
        pthread_join(thread_oven_process, NULL);
    }
    else if (command == STOP_OVEN)
    {
        uart0_filestream = write_commands(STOP_OVEN, uart0_filestream, 0, 0);
        functioning_state = 0;
    }
    else if (command == TOGGLE_CONTROL_MODE)
    {
        uart0_filestream = write_commands(TOGGLE_CONTROL_MODE, uart0_filestream, 0, !functioning_state);
    }
    else if (command == SEND_AMBIENT_TEMPERATURE)
    {
        get_ambient_temperature();
    }
    else if (command == 0)
    {
        // do nothing
        ;
    }
    else
    {
        printf("Comando não é válido - %d\n", command);
    }
    return NULL;
}

void *oven_process()
{
    printf("Começando processo .....  (Estado de funcionamento = %d)\n", functioning_state);
    // functioning_state = 1;
    // loop ate forno parar
    while (functioning_state == 1)
    {
        // solicitar temperatura interna
        uart0_filestream = read_commands(GET_INTERNAL_TEMPERATURE, uart0_filestream);
        // copy read_uart return to ans
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        float internal_temperature = ans.internal_temperature;
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
        double control_signal = pid_controle(internal_temperature);
        printf("Control signal: %lf\n", control_signal);
        int control_signal_int = (int)control_signal;
        // send control signal to uart
        uart0_filestream = write_commands(SEND_CONTROL_SIGNAL, uart0_filestream, 0, control_signal_int);
        // pwm
        if (control_signal_int < 0)
        {
            control_signal_int = control_signal_int * -1;
            control_signal_int = (control_signal_int < 40) ? 40 : (control_signal_int > 100) ? 100
                                                                                             : control_signal_int;
            fan_pwm(control_signal_int);
        }
        else
        {
            control_signal_int = (control_signal_int < 40) ? 40 : (control_signal_int > 100) ? 100
                                                                                             : control_signal_int;
            resistor_pwm(control_signal_int);
        }
        // sleep 1
        sleep(1);
    }
}

void sigint_handler(int sig)
{
    printf("SIGINT received. Exiting...\n");
    finish_all();
    exit(0);
}

void finish_all()
{
    running = 0;
    // desliga forno
    uart0_filestream = write_commands(TURN_OFF_OVEN, uart0_filestream, 0, 0);
    // close uart
    close(uart0_filestream);
    // send pwm to 0 on both channels
    resistor_pwm(0);
    fan_pwm(0);
}

void init()
{
    // init uart
    uart0_filestream = -1;
    uart0_filestream = open_uart(uart0_filestream);
    uart0_filestream = write_commands(TURN_OFF_OVEN, uart0_filestream, 0, 0);
    running = 0;
    command = 0;
    functioning_state = 0;
    // init pwm
    pwm_init();
    // init pid
    pid_init();
    // init signal handler
}

void pid_init()
{
    float Kp, Ki, Kd;
    // default values
    Kp = 30.0;
    Ki = 0.2;
    Kd = 400.0;
    printf("configurar costantes manualmente?\n");
    printf("1 - sim\n");
    printf("2 - nao\n");
    int choice;
    scanf("%d", &choice);
    fflush(stdin);
    if (choice == 1)
    {
        printf("Kp: ");
        scanf("%lf", &Kp);
        fflush(stdin);
        printf("Ki: ");
        scanf("%lf", &Ki);
        fflush(stdin);
        printf("Kd: ");
        scanf("%lf", &Kd);
        fflush(stdin);
    }
    pid_configura_constantes(Kp, Ki, Kd);
}

//debug
void *interface()
{   int send_command;
    printf("Oven controller interface\n");
    printf("1 - Turn on oven\n");
    printf("2 - Turn off oven\n");
    printf("3 - Start oven\n");
    printf("4 - Stop oven\n");
    printf("5 - Toggle control mode\n");
    printf("6 - Get ambient temperature\n");
    printf("7 - Get oven temperature\n");
    printf("8 - Get oven state\n");
    printf("9 - Get control mode\n");
    scanf("%d", &send_command);
    fflush(stdin);
    parse_send_command(send_command);
    return NULL;
}
//debug
void parse_send_command(int send_command)
{
    switch (send_command)
    {
    case 1:
        uart0_filestream = write_commands(TURN_ON_OVEN, uart0_filestream, 0, 0);
        break;
    case 2:
        uart0_filestream = write_commands(TURN_OFF_OVEN, uart0_filestream, 0, 0);
        break;
    case 3:
        uart0_filestream = write_commands(START_OVEN, uart0_filestream, 0, 0);
    case 4:
        uart0_filestream = write_commands(STOP_OVEN, uart0_filestream, 0, 0);
        break;
    case 5:
        uart0_filestream = write_commands(SEND_CONTROL_MODE, uart0_filestream, 0, 0);
        break;
    case 6:
        get_ambient_temperature();
        break;
    case 7:
        uart0_filestream = read_commands(GET_INTERNAL_TEMPERATURE, uart0_filestream);
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        printf("Internal temperature: %f\n", ans.internal_temperature);
        break;
    case 8:
        uart0_filestream = read_commands(GET_REFERENCE_TEMPERATURE, uart0_filestream);
        ans = read_uart(uart0_filestream);
        uart0_filestream = ans.uart0_filestream;
        printf("Reference temperature: %f\n", ans.reference_temperature);
        break;
    default:
        break;
    }
}