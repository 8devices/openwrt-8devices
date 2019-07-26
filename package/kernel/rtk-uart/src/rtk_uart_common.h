#define UART_USR	0x1f

unsigned int dwapb_serial_in(struct uart_port *p, int offset);
void dwapb_serial_out(struct uart_port *p, int offset, int value);
int dwapb_serial_irq(struct uart_port *p);
