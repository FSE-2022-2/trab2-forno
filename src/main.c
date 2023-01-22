#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "uart-modbus.h"
#include "bme280.h"
#include "linux_userspace.h"
// return command from get_command function
int command, uart0_filestream, running;
read_uart_return_t ans;

void* get_command();
void* interface();
void* get_ambient_temperature();
// void* get_command(void* uart_arg);

int main(int argc, const char * argv[]) {

    
    uart0_filestream = -1;
    uart0_filestream = open_uart(uart0_filestream);
    running = 1;
    // send command 0xA1 to uart
    uart0_filestream = write_commands(TURN_ON_OVEN, uart0_filestream, 0, 0);
    // read command from uart in a thread
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, get_command, NULL);
    pthread_join(thread_id, NULL);

    // check if command change
    printf("command: %d\n", command);
    // if command change, send command to uart

    // send command to uart
    uart0_filestream = write_commands(command, uart0_filestream, 0, 0); //change
    
    //get ambient temperature
    get_ambient_temperature();

    close(uart0_filestream);

    return 0;
}

void* get_command(){
    // unsigned char tx_buffer[256];
    // unsigned char *p_tx_buffer = &tx_buffer[0];
    int count_test = 0;

    while(running){
        if (count_test == 6)
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
        // sleep 500ms
        usleep(500*1000);
        count_test++;
    }
    return NULL;
}

void* interface(){
    // interface with user
    return NULL;
}

void* get_ambient_temperature(){
    // read ambient temperature
    float ambient_temperature = init_bme280();
    printf("Ambient temperature: %f\n", ambient_temperature);
    // send ambient temperature to uart
    uart0_filestream = write_commands(SEND_AMBIENT_TEMPERATURE, uart0_filestream, ambient_temperature, 0);
    return NULL;
}


