#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include "uart-modbus.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
// return command from get_command function
int command, uart0_filestream, running;

void get_command(int uart0_filestream);

int main(int argc, const char * argv[]) {

    
    uart0_filestream = -1;
    uart0_filestream = open_uart(uart0_filestream);
    // unsigned char tx_buffer[256];
    // unsigned char *p_tx_buffer = &tx_buffer[0];

    // // create two threads in parallel one to read_command(500 ms) function and other to read_uart function(each 500ms)
    // pthread_t thread1;
    // int  iret1;
    // /* Create independent threads each of which will execute function */
    // iret1 = pthread_create( &thread1, NULL, get_command, (void*) uart0_filestream);
    // running = 1;
    // if(iret1)
    // {
    //     fprintf(stderr,"Error - pthread_create() return code: %d",iret1);
    //     exit(EXIT_FAILURE);
    // }
    get_command(uart0_filestream);

    // // comando para mensagem 
    // printf("1 - Solicita Temperatura Interna\n");
    // printf("2 - Solicita Temperatura de Referencia\n");
    // printf("3 - Le comandos do usuario\n");
    // printf("4 - Envia sinal de controle Int (4 bytes)\n");
    // printf("5 - Envia sinal de Referência Float (4 bytes)\n");
    // printf("6 - Envia Estado do Sistema (Ligado = 1 / Desligado = 0)\n");
    // printf("7 - Modo de Controle da Temperatura de referência (Dashboard = 0 / Curva/Terminal = 1) (1 byte)\n");
    // printf("8 - Envia Estado de Funcionamento (Funcionando = 1 / Parado = 0)\n");
    // printf("9 - Envia Temperatura Ambiente (Float))\n");
    // printf("Digite o comando: \n");
    // fscanf(stdin, "%d", &command);
    // fflush(stdin);
    // // creat thread to read uart each 500ms and other process to read user input


    // uart0_filestream = write_uart(command, uart0_filestream, tx_buffer, p_tx_buffer);
    
    // sleep(1);
    
    // // implementar comando na leitura
    // uart0_filestream = read_uart(uart0_filestream);


    // close thread
    // if (running == 0)
    // {
        // pthread_exit(NULL);
        // iret1 = pthread_join( thread1, NULL);
    // }
    
    //close uart
    close(uart0_filestream);

    return 0;
}

void get_command(int uart0_filestream){
    unsigned char tx_buffer[256];
    unsigned char *p_tx_buffer = &tx_buffer[0];
    int count_test = 0, *ans;

    while(1){
        if (count_test == 6)
        {
            running = 0; 
            exit(0);
        }

        // 3 for 0xC3
        uart0_filestream = read_commands(3, uart0_filestream, tx_buffer, p_tx_buffer);
        // copy read_uart return to ans

        ans = read_uart(uart0_filestream);
        uart0_filestream = ans[0];
        command = ans[1];
        // sleep 500ms
        sleep(2);
        count_test++;
    }
}
