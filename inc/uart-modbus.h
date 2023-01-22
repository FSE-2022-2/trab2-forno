#ifndef UART_MODBUS_H
#define UART_MODBUS_H

//define protocol codes
#define ESP32_ADDRESS 0x01
#define CCODE 0x23
#define DCODE 0x16
#define MATRICULA 8159
#define GET_INTERN_TEMPERATURE 0xC1
#define GET_REFERENCE_TEMPERATURE 0xC2
#define GET_COMMAND 0xC3
#define SEND_CONTROL_SIGNAL 0xD1
#define SEND_REFERENCE_SIGNAL 0xD2
#define SEND_SYSTEM_ON_STATE 0xD3
#define SEND_CONTROL_MODE 0xD4
#define SEND_SYSTEM_FUNCTIONING_STATE 0xD5
#define SEND_AMBIENT_TEMPERATURE 0xD6

// define command codes
#define TURN_ON_OVEN 0xA1
#define TURN_OFF_OVEN 0xA2
#define START_OVEN 0xA3
#define STOP_OVEN 0xA4
#define TOGGLE_CONTROL_MODE 0xA5

// create struct to return read_uart function
struct read_uart_return
{
    int uart0_filestream;
    float inside_temperature, reference_temperature, value;
    int command;
};

typedef struct read_uart_return read_uart_return_t;

int open_uart(int uart0_filestream);
void close_uart(int uart0_filestream);
// int read_commands(int command, int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer);
int read_commands(int command, int uart0_filestream);
// int write_commands(int command, int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer);
int write_commands(int command, int uart0_filestream, float value_float, int value_int);
read_uart_return_t read_uart(int uart0_filestream);

#endif