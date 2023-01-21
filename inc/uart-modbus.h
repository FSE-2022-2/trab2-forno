#ifndef UART_MODBUS_H
#define UART_MODBUS_H

int open_uart(int uart0_filestream);
void close_uart(int uart0_filestream);
// int read_commands(int command, int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer);
int read_commands(int command, int uart0_filestream);
// int write_commands(int command, int uart0_filestream, unsigned char *tx_buffer, unsigned char *p_tx_buffer);
int write_commands(int command, int uart0_filestream);
int* read_uart(int uart0_filestream);

#endif