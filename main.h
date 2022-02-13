#ifndef _main_H
#define _main_H
int main(int argc, char **argv);
void loop();
void readfileintoram(char *filename, char **buffer);
void interrupt_setup();
void interrupt_handler();
#endif

