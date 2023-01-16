#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include "uart-modbus.h"

int main(int argc, const char * argv[]) {

    int command, uart0_filestream = -1;
    
    uart0_filestream = open_uart(uart0_filestream);
    unsigned char tx_buffer[10];
    unsigned char *p_tx_buffer = &tx_buffer[0];
    printf("Digite o comando: \n");
    fscanf(stdin, "%d", &command);
    uart0_filestream = write_uart(command, uart0_filestream, tx_buffer, p_tx_buffer);
    
    sleep(1);
    
    uart0_filestream = read_uart(uart0_filestream);

    close(uart0_filestream);

    return 0;
}

