#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include "pid.h"
#include "crc16.h"

#define clean_stdin() do { int c; while ((c = getchar()) != '\n' && c != EOF); } while (0)

unsigned char* pega_crc(unsigned char *p_tx_buffer, unsigned char *tx_buffer)
{       
        int size;
        unsigned short crc;
        // get size of current filled buffer
        size = p_tx_buffer - tx_buffer;
        crc = calcula_CRC(tx_buffer, size);
        // memcpy 2 bytes de crc
        memcpy(p_tx_buffer, &crc, 2);
        p_tx_buffer += 2;
        return p_tx_buffer;
}

int check_crc(unsigned char *rx_buffer, int rx_length)
{
    // a implementar
    return 1;
}

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

int write_to_uart(int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer) {
    int size;
    // get size of current filled buffer
    size = p_tx_buffer - tx_buffer;
    // write to uart
    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, tx_buffer, size);      //Filestream, bytes to write, number of bytes to write
        if (count < 0)
        {
            printf("UART TX error\n");
        }
        else
        {
            printf("Escrito na Uart\n");
        }
    }
    return uart0_filestream;
}

int read_commands(int command, int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer) {
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
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        // check crc
        if (check_crc(tx_buffer, p_tx_buffer - tx_buffer))
        {
            printf("CRC OK\n");
        }
        else
        {
            printf("CRC NOK\n");
        }
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
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
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
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        break;
    default:
        printf("Comando inválido\n");
        break;
    }

    // call write_to_uart
    uart0_filestream = write_to_uart(uart0_filestream, tx_buffer, p_tx_buffer);

    return uart0_filestream;
}

// alterar pra n usar scanf so escolha de opção e colocar os codigos como define
int write_commands(int command, int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer) {
    // int size;
    // unsigned short crc;
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
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        // check crc
        if (check_crc(tx_buffer, p_tx_buffer - tx_buffer))
        {
            printf("CRC OK\n");
        }
        else
        {
            printf("CRC NOK\n");
        }
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
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
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
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        break;
    case 4:
        printf("case 4\n");
        int sinal_controle;    // ativa resistor ou ventoinha (-100 a 100) envia gpio dps aciona ou inverso
        *p_tx_buffer++ = 0x01; // Endereço da ESP32
        *p_tx_buffer++ = 0x16; // Código
        *p_tx_buffer++ = 0xD1; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9; 

        // ta errando usar como atribuição sinal_controle = 50 funfa
        printf("Digite o sinal de controle: \n");
        // clean stdin
        clean_stdin();
        fflush(stdin);
        // get sinal de controle
        sinal_controle = fscanf(stdin, "%d", &sinal_controle);
        memcpy(p_tx_buffer, &sinal_controle, 4);
        p_tx_buffer += 4;
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        break;
    case 5:
        printf("case 5\n");
        float sinal_de_referencia;
        *p_tx_buffer++ = 0x01; // Endereço da ESP32
        *p_tx_buffer++ = 0x16; // Código
        *p_tx_buffer++ = 0xD2; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9; 

        printf("Digite o sinal de referência: \n");
        // clean stdin
        clean_stdin();
        sinal_de_referencia = fscanf(stdin, "%f", &sinal_de_referencia);
        memcpy(p_tx_buffer, &sinal_de_referencia, 4);
        p_tx_buffer += 4;
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        break;
    case 6:
        printf("case 6\n");
        char sinal;
        *p_tx_buffer++ = 0x01; // Endereço da ESP32
        *p_tx_buffer++ = 0x16; // Código
        *p_tx_buffer++ = 0xD3; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9; 
        // pela dashboard - checar se comando recebido por 0xC3 é 0xA1(161) ou 0xA2(162)
    
        // clean stdin
        sinal = 1;
        memcpy(p_tx_buffer, &sinal, 1);
        p_tx_buffer ++;
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        break;
    default:
        break;
    }

    // write to uart
    printf("Buffers de memória criados!\n");
    // if (uart0_filestream != -1)
    // {
    //     printf("Escrevendo caracteres na UART ...");
    //     int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
    //     if (count < 0)
    //     {
    //         printf("UART TX error\n");
    //     }
    //     else
    //     {
    //         printf("escrito.\n");
    //     }
    // }

    // call write_to_uart
    uart0_filestream = write_to_uart(uart0_filestream, tx_buffer, p_tx_buffer);

    return uart0_filestream;
}

int* read_uart(int uart0_filestream) {
    int ans[2];
    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {
        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);  
        ans[0] = uart0_filestream; 
        //Filestream, buffer to store in, number of bytes to read (max)
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
            float *pf;
            int *p;
            // switch case para verificar o sub-código no byte 3
            switch (rx_buffer[2])
            {
                case 0xC1:  // float
                    //Bytes received
                    rx_buffer[rx_length] = '\0';
                    // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                    pf = (float *) &rx_buffer[3];
                    printf("Temperatura interna recebida: %.2f\n", *pf);
                    ans[1] = *pf;
                    break;
                case 0xC2:
                    //Bytes received
                    rx_buffer[rx_length] = '\0';
                    // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                    pf = (float *) &rx_buffer[3];
                    printf("Temperatura de referencia recebida: %.2f\n", *pf);
                    ans[1] = *pf;
                    break;
                case 0xC3: // int
                    //Bytes received
                    rx_buffer[rx_length] = '\0';
                    // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                    p = (int *) &rx_buffer[3];
                    // change printf for each case
                    printf("Comando recebido: %d\n", *p);
                    ans[1] = *p;
                    break;
                // 0xD1, 0xD2 nao tem retorno
                default:
                    //Bytes received
                    rx_buffer[rx_length] = '\0';
                    // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                    pf = (float *) &rx_buffer[3];
                    printf("Valor recebido: %.2f\n", *pf);
                    ans[1] = *pf;
                    break;
            }
        }
    }
    // pass ans to main, avoid warning of passing address of local variable
    
    int* ans_ptr = ans;
    return ans_ptr;
}

void close_uart(int uart0_filestream) {
    close(uart0_filestream);
}

