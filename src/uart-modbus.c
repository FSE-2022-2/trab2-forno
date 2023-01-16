#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include "pid.h"
#include "crc16.h"

int open_uart(int uart0_filestream) {
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode
    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else
    {
        printf("UART inicializada!\n");
    }    
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    return uart0_filestream;
}

int write_uart(int command, int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer) {
    switch (command)
    {
    case 1:
        printf("case 1\n");
        *p_tx_buffer++ = 0x01; // Endereço da ESP32
        *p_tx_buffer++ = 0x23; // Código
        *p_tx_buffer++ = 0xC1; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9; 
        unsigned short crc = calcula_CRC(tx_buffer, 7);
        // memcpy 2 bytes de crc
        memcpy(p_tx_buffer, &crc, 2);
        p_tx_buffer += 2;
        break;
    case 2:
        printf("case 2\n");
        *p_tx_buffer++ = 0x01; // Endereço da ESP32
        *p_tx_buffer++ = 0x23; // Código
        *p_tx_buffer++ = 0xC2; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9; 
        crc = calcula_CRC(tx_buffer, 7);
        // memcpy 2 bytes de crc
        memcpy(p_tx_buffer, &crc, 2);
        p_tx_buffer += 2;
        break;
    case 3:
        printf("case 3\n");
        *p_tx_buffer++ = 0x01; // Endereço da ESP32
        *p_tx_buffer++ = 0x23; // Código
        *p_tx_buffer++ = 0xC3; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9; 
        crc = calcula_CRC(tx_buffer, 7);
        // memcpy 2 bytes de crc
        memcpy(p_tx_buffer, &crc, 2);
        p_tx_buffer += 2;
        break;
    case 4:
        *p_tx_buffer++ = 0x01; // Endereço da ESP32
        *p_tx_buffer++ = 0x16; // Código
        *p_tx_buffer++ = 0xD1; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9; 
        // //sinal de controle de 4 bytes
        // unsigned char sinal_controle[4];
        // *p_tx_buffer++ = sinal_controle[0];
        // *p_tx_buffer++ = sinal_controle[1];
        // *p_tx_buffer++ = sinal_controle[2];
        // *p_tx_buffer++ = sinal_controle[3];

        crc = calcula_CRC(tx_buffer, 7);
        // memcpy 2 bytes de crc
        memcpy(p_tx_buffer, &crc, 2);
        p_tx_buffer += 2;
        break;
    default:
        break;
    }


    printf("Buffers de memória criados!\n");
    if (uart0_filestream != -1)
    {
        printf("Escrevendo caracteres na UART ...");
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
        if (count < 0)
        {
            printf("UART TX error\n");
        }
        else
        {
            printf("escrito.\n");
        }
    }

    return uart0_filestream;
}

int read_uart(int uart0_filestream) {
    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {
        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);      //Filestream, buffer to store in, number of bytes to read (max)
        if (rx_length < 0)
        {
            printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
        }
        else if (rx_length == 0)
        {
            printf("Nenhum dado disponível.\n"); //No data waiting
        }
        else
        {
            //dummy for temp
            //Bytes received
            rx_buffer[rx_length] = '\0';
            // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
            float *p = (float *) &rx_buffer[3];
            printf("Valor recebido: %.2f\n", *p);
        }
    }

    return uart0_filestream;
}

void close_uart(int uart0_filestream) {
    close(uart0_filestream);
}

