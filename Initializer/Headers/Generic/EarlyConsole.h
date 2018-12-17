/**
 * @file EarlyConsole.h
 *
 * Presents a neat, non-complex interface for text-output on the early-boot
 * screen. The early-console interface allows developers to debug the
 * initializer when the INITOR_DEBUG macro is declared.
 *
 * @version 1.00
 * @since Silcos 3.05
 * @author Shukant Pal
 */
#ifndef INITIALIZER_GENERIC_EARLYCONSOLE_H_
#define INITIALIZER_GENERIC_EARLYCONSOLE_H_

void InitConsole(unsigned char *consoleBuffer);
void Write(const char *textOutput);
void WriteInteger(unsigned long num);
void WriteLine(const char *textOutput);

#endif/* Generic/EarlyConsole.h */
