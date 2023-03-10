#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include "pid.h"
#include "crc16.h"
#include "uart-modbus.h"

unsigned char *pega_crc(unsigned char *p_tx_buffer, unsigned char *tx_buffer)
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

int check_crc(unsigned char *p_tx_buffer, unsigned char *tx_buffer)
{
    int size;
    unsigned short crc;
    // get size of current filled buffer
    size = p_tx_buffer - tx_buffer - 2;
    crc = calcula_CRC(tx_buffer, size);
    if (crc == p_tx_buffer[-2] + p_tx_buffer[-1] * 256)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int open_uart(int uart0_filestream)
{
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY); // Open in non blocking read/write mode
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
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD; //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    return uart0_filestream;
}

int write_to_uart(int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer)
{
    int size;
    // get size of current filled buffer
    size = p_tx_buffer - tx_buffer;
    // write to uart
    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, tx_buffer, size); // Filestream, bytes to write, number of bytes to write
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


int read_commands(int command, int uart0_filestream)
{
    unsigned char tx_buffer[256];
    unsigned char *p_tx_buffer = &tx_buffer[0];
    int check = 0, ttl = 0;
    do
    {
        switch (command)
        {
        case GET_INTERNAL_TEMPERATURE:
            printf("Solicitando Temperatura Interna\n");
            *p_tx_buffer++ = ESP32_ADDRESS;            // Endereço da ESP32
            *p_tx_buffer++ = CCODE;                    // Código
            *p_tx_buffer++ = GET_INTERNAL_TEMPERATURE; // Sub-código + Matrícula (8159)
            *p_tx_buffer++ = 8;
            *p_tx_buffer++ = 1;
            *p_tx_buffer++ = 5;
            *p_tx_buffer++ = 9;
            // pega crc
            p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
            check = check_crc(p_tx_buffer, tx_buffer);
            if (check)
            {
                printf("CRC OK\n");
            }
            else
            {
                printf("CRC NOK\n");
                // repeat switch
            }
            break;
        case GET_REFERENCE_TEMPERATURE:
            printf("Solicitando Temperatura de Referencia\n");
            *p_tx_buffer++ = ESP32_ADDRESS;             // Endereço da ESP32
            *p_tx_buffer++ = CCODE;                     // Código
            *p_tx_buffer++ = GET_REFERENCE_TEMPERATURE; // Sub-código + Matrícula (8159)
            *p_tx_buffer++ = 8;
            *p_tx_buffer++ = 1;
            *p_tx_buffer++ = 5;
            *p_tx_buffer++ = 9;
            // pega crc
            p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
            // check crc
            check = check_crc(p_tx_buffer, tx_buffer);
            if (check)
            {
                printf("CRC OK\n");
            }
            else
            {
                printf("CRC NOK\n");
                // repeat switch
            }
            break;
        case GET_COMMAND:
            printf("Lendo comando ...\n");
            *p_tx_buffer++ = ESP32_ADDRESS; // Endereço da ESP32
            *p_tx_buffer++ = CCODE;         // Código
            *p_tx_buffer++ = GET_COMMAND;   // Sub-código + Matrícula (8159)
            *p_tx_buffer++ = 8;
            *p_tx_buffer++ = 1;
            *p_tx_buffer++ = 5;
            *p_tx_buffer++ = 9;
            // pega crc
            p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
            check = check_crc(p_tx_buffer, tx_buffer);
            if (check)
            {
                printf("CRC OK\n");
            }
            else
            {
                printf("CRC NOK\n");
                // repeat switch
            }
            break;
        default:
            printf("Comando inválido\n");
            return uart0_filestream;
            break;
        }
        ttl++;
        if (ttl == 3)
        {
            printf("TTL excedido\n");
            return uart0_filestream;
        }

    } while (check != 1);

    // call write_to_uart
    uart0_filestream = write_to_uart(uart0_filestream, tx_buffer, p_tx_buffer);

    return uart0_filestream;
}

int write_commands(int command, int uart0_filestream, float value_float, int value_int)
{
    unsigned char tx_buffer[256];
    unsigned char *p_tx_buffer = &tx_buffer[0];
    int check = 0, ttl = 0;
    switch (command)
    {
    case SEND_CONTROL_SIGNAL:
        printf("Envia sinal de controle Int (4 bytes)\n");
        int control_signal;                   // ativa resistor ou ventoinha (-100 a 100) envia gpio dps aciona ou inverso
        *p_tx_buffer++ = ESP32_ADDRESS;       // Endereço da ESP32
        *p_tx_buffer++ = DCODE;               // Código
        *p_tx_buffer++ = SEND_CONTROL_SIGNAL; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9;

        control_signal = value_int;
        memcpy(p_tx_buffer, &control_signal, 4);
        p_tx_buffer += 4;
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        // check crc
        check = check_crc(p_tx_buffer, tx_buffer);
        if (check)
        {
            printf("CRC OK\n");
        }
        else
        {
            printf("CRC NOK\n");
            // repeat switch
        }
        break;
    case SEND_REFERENCE_SIGNAL:
        printf("Envia sinal de Referência Float (4 bytes)\n");
        float sinal_de_referencia;
        *p_tx_buffer++ = ESP32_ADDRESS;         // Endereço da ESP32
        *p_tx_buffer++ = DCODE;                 // Código
        *p_tx_buffer++ = SEND_REFERENCE_SIGNAL; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9;

        sinal_de_referencia = value_float;
        memcpy(p_tx_buffer, &sinal_de_referencia, 4);
        p_tx_buffer += 4;
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        break;
    case SEND_AMBIENT_TEMPERATURE:
        printf("Envia Temperatura Ambiente (Float))\n");
        float temperatura_ambiente;
        *p_tx_buffer++ = ESP32_ADDRESS;            // Endereço da ESP32
        *p_tx_buffer++ = DCODE;                    // Código
        *p_tx_buffer++ = SEND_AMBIENT_TEMPERATURE; // Sub-código + Matrícula (8159)
        *p_tx_buffer++ = 8;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 5;
        *p_tx_buffer++ = 9;
        // pela dashboard - checar se comando recebido por 0xC3 é 0xA1(161) ou 0xA2(162)

        printf("Enviando temperatura ambiente \n");
        temperatura_ambiente = value_float;
        memcpy(p_tx_buffer, &temperatura_ambiente, 4);
        p_tx_buffer += 4;
        // pega crc
        p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
        // check crc
        check = check_crc(p_tx_buffer, tx_buffer);
        if (check)
        {
            printf("CRC OK\n");
        }
        else
        {
            printf("CRC NOK\n");
            // repeat switch
        }
        break;
    default:
        if (command == TURN_ON_OVEN || command == TURN_OFF_OVEN)
        {

            *p_tx_buffer++ = ESP32_ADDRESS;        // Endereço da ESP32
            *p_tx_buffer++ = DCODE;                // Código
            *p_tx_buffer++ = SEND_SYSTEM_ON_STATE; // Sub-código + Matrícula (8159)
            *p_tx_buffer++ = 8;
            *p_tx_buffer++ = 1;
            *p_tx_buffer++ = 5;
            *p_tx_buffer++ = 9;

            if (command == TURN_ON_OVEN)
            {
                printf("Ligando forno ...\n");
                *p_tx_buffer++ = 1;
            }
            else
            {
                printf("Desligando forno ...\n");
                *p_tx_buffer++ = 0;
            }
            // pega crc
            p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
            // check crc
            check = check_crc(p_tx_buffer, tx_buffer);
            if (check == 1)
            {
                printf("CRC OK\n");
            }
            else
            {
                printf("CRC NOK\n");
            }
        }
        else if (command == START_OVEN || command == STOP_OVEN)
        {

            printf("Envia Estado de Funcionamento (Funcionando = 1 / Parado = 0)\n");
            *p_tx_buffer++ = ESP32_ADDRESS;                 // Endereço da ESP32
            *p_tx_buffer++ = DCODE;                         // Código
            *p_tx_buffer++ = SEND_SYSTEM_FUNCTIONING_STATE; // Sub-código + Matrícula (8159)
            *p_tx_buffer++ = 8;
            *p_tx_buffer++ = 1;
            *p_tx_buffer++ = 5;
            *p_tx_buffer++ = 9;
            // // pela dashboard - checar se comando recebido por 0xC3 é 0xA3(163) ou 0xA4(164)
            if (command == START_OVEN)
            {
                printf("Iniciando funcionamento do forno ...\n");
                *p_tx_buffer++ = 1;
            }
            else
            {
                printf("Parando forno ...\n");
                *p_tx_buffer++ = 0;
            }
            // pega crc
            p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
            // check crc
            check = check_crc(p_tx_buffer, tx_buffer);
            if (check)
            {
                printf("CRC OK\n");
            }
            else
            {
                printf("CRC NOK\n");
                // repeat switch
            }
        }
        else if (command == TOGGLE_CONTROL_MODE)
        {
            int sinal;

            *p_tx_buffer++ = ESP32_ADDRESS;     // Endereço da ESP32
            *p_tx_buffer++ = DCODE;             // Código
            *p_tx_buffer++ = SEND_CONTROL_MODE; // Sub-código + Matrícula (8159)
            *p_tx_buffer++ = 8;
            *p_tx_buffer++ = 1;
            *p_tx_buffer++ = 5;
            *p_tx_buffer++ = 9;
            // // pela dashboard - checar se comando recebido por 0xC3 é 0xA5(165)
            sinal = value_int;
            // inverte sinal anterior
            if (sinal == 0)
            {
                printf("Modo de Controle da Temperatura de referência (Dashboard = 0 / Curva/Terminal = 1)\n");
                *p_tx_buffer++ = 1;
            }
            else
            {
                printf("Modo de Controle da Temperatura de referência (Dashboard = 0 / Curva/Terminal = 1)\n");
                // sinal = 0;
                *p_tx_buffer++ = 0;
            }
            // pega crc
            p_tx_buffer = pega_crc(p_tx_buffer, tx_buffer);
            // check crc
            check = check_crc(p_tx_buffer, tx_buffer);
            if (check)
            {
                printf("CRC OK\n");
            }
            else
            {
                printf("CRC NOK\n");
                // repeat switch
            }
        }
        else
        {
            printf("Comando inválido!\n");
            return uart0_filestream;
        }
        break;
    }

    printf("Buffers de memória criados!\n");

    // write to uart
    uart0_filestream = write_to_uart(uart0_filestream, tx_buffer, p_tx_buffer);

    return uart0_filestream;
}

// read uart struct return
read_uart_return_t read_uart(int uart0_filestream)
{
    read_uart_return_t ans;
    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {
        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void *)rx_buffer, 255);
        ans.uart0_filestream = uart0_filestream;
        // Filestream, buffer to store in, number of bytes to read (max)
        if (rx_length < 0)
        {
            printf("Erro na leitura.\n"); // An error occured (will occur if there are no bytes)
        }
        else if (rx_length == 0)
        {
            printf("Nenhum dado disponível.\n"); // No data waiting
        }
        else
        {
            float *pf;
            int *p;
            // switch case para verificar o sub-código no byte 3
            switch (rx_buffer[2])
            {
            case GET_INTERNAL_TEMPERATURE: // float
                // Bytes received
                rx_buffer[rx_length] = '\0';
                // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                pf = (float *)&rx_buffer[3];
                printf("Temperatura interna recebida: %.2f\n", *pf);
                // ans[1] = *pf;
                ans.internal_temperature = *pf;
                break;
            case GET_REFERENCE_TEMPERATURE:
                // Bytes received
                rx_buffer[rx_length] = '\0';
                // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                pf = (float *)&rx_buffer[3];
                printf("Temperatura de referencia recebida: %.2f\n", *pf);
                // ans[1] = *pf;
                ans.reference_temperature = *pf;
                break;
            case GET_COMMAND: // int
                // Bytes received
                rx_buffer[rx_length] = '\0';
                // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                p = (int *)&rx_buffer[3];
                // change printf for each case
                printf("Comando recebido: %d\n", *p);
                // ans[1] = *p;
                ans.command = *p;
                break;
            // 0xD1, 0xD2 nao tem retorno
            default:
                // Bytes received
                rx_buffer[rx_length] = '\0';
                // parse to get the end float 0x00 0x23 0xC1 + float (4 bytes)
                pf = (float *)&rx_buffer[3];
                printf("Valor recebido: %.2f\n", *pf);
                ans.value = *pf;
                break;
            }
        }
    }

    return ans;
}

void close_uart(int uart0_filestream)
{
    close(uart0_filestream);
}
