;///
;/// @file IRQCallbacks.asm
;/// @module HAL (kernel.silcos.hal)
;///
;/// This file implements the IRQ-callback "labels" that are mapped into the
;/// IDT directly for device-interrupts whose range is in 32 to 191 inclusive
;/// allowing the generic device-IRQ handler to do the right thing based
;/// on the interrupt no.
;///
;/// This file is only used when __attribute__((interrupt)) is not supported
;/// by the compiler-chain. GCC supports this (ToDo: add support to fallback
;/// here if __attribute__((interrupt)) not supported, currently this file
;/// isn't used.
;/// -------------------------------------------------------------------
;/// This program is free software: you can redistribute it and/or modify
;/// it under the terms of the GNU General Public License as published by
;/// the Free Software Foundation, either version 3 of the License, or
;/// (at your option) any later version.
;///
;/// This program is distributed in the hope that it will be useful,
;/// but WITHOUT ANY WARRANTY; without even the implied warranty of
;/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;/// GNU General Public License for more details.
;///
;/// You should have received a copy of the GNU General Public License
;/// along with this program.  If not, see <http://www.gnu.org/licenses/>
;///
;/// Copyright (C) 2017 - Shukant Pal
;///

;
; ToDo: We have to shift the architecture to some other configuration
; file for the build. Right now, just working for IA32 so it doesn't matter
; much.
;
EXTERN HandleDeviceIRQ ;//! @see Processor.cpp

%define ARCH_32
%define IA32

;///
;/// Macro to declare a IRQ-callback using its vector number in the IDT. It
;/// is clearly used to create all the irq-handlers in this file. It also
;/// creates a global symbol __irqCallback%n which is used in the C++ source
;/// file.
;///
;/// @version 1.0
;/// @since Silcos 3.02
;/// @author Shukant Pal
;///
%macro decl_IRQ 1
global __irqCallback%1
__irqCallback%1:
	PUSHAD
	PUSH DWORD %1
	CALL HandleDeviceIRQ
	ADD ESP, 4
	POPAD
	IRETD
%endmacro

ALIGN 4096

decl_IRQ 32
decl_IRQ 33
decl_IRQ 34
decl_IRQ 35
decl_IRQ 36
decl_IRQ 37
decl_IRQ 38
decl_IRQ 39
decl_IRQ 40
decl_IRQ 41
decl_IRQ 42
decl_IRQ 43
decl_IRQ 44
decl_IRQ 45
decl_IRQ 46
decl_IRQ 47
decl_IRQ 48
decl_IRQ 49
decl_IRQ 50
decl_IRQ 51
decl_IRQ 52
decl_IRQ 53
decl_IRQ 54
decl_IRQ 55
decl_IRQ 56
decl_IRQ 57
decl_IRQ 58
decl_IRQ 59
decl_IRQ 60
decl_IRQ 61
decl_IRQ 62
decl_IRQ 63
decl_IRQ 64
decl_IRQ 65
decl_IRQ 66
decl_IRQ 67
decl_IRQ 68
decl_IRQ 69
decl_IRQ 70
decl_IRQ 71
decl_IRQ 72
decl_IRQ 73
decl_IRQ 74
decl_IRQ 75
decl_IRQ 76
decl_IRQ 77
decl_IRQ 78
decl_IRQ 79
decl_IRQ 80
decl_IRQ 81
decl_IRQ 82
decl_IRQ 83
decl_IRQ 84
decl_IRQ 85
decl_IRQ 86
decl_IRQ 87
decl_IRQ 88
decl_IRQ 89
decl_IRQ 90
decl_IRQ 91
decl_IRQ 92
decl_IRQ 93
decl_IRQ 94
decl_IRQ 95
decl_IRQ 96
decl_IRQ 97
decl_IRQ 98
decl_IRQ 99
decl_IRQ 100
decl_IRQ 101
decl_IRQ 102
decl_IRQ 103
decl_IRQ 104
decl_IRQ 105
decl_IRQ 106
decl_IRQ 107
decl_IRQ 108
decl_IRQ 109
decl_IRQ 110
decl_IRQ 111
decl_IRQ 112
decl_IRQ 113
decl_IRQ 114
decl_IRQ 115
decl_IRQ 116
decl_IRQ 117
decl_IRQ 118
decl_IRQ 119
decl_IRQ 120
decl_IRQ 121
decl_IRQ 122
decl_IRQ 123
decl_IRQ 124
decl_IRQ 125
decl_IRQ 126
decl_IRQ 127
decl_IRQ 128
decl_IRQ 129
decl_IRQ 130
decl_IRQ 131
decl_IRQ 132
decl_IRQ 133
decl_IRQ 134
decl_IRQ 135
decl_IRQ 136
decl_IRQ 137
decl_IRQ 138
decl_IRQ 139
decl_IRQ 140
decl_IRQ 141
decl_IRQ 142
decl_IRQ 143
decl_IRQ 144
decl_IRQ 145
decl_IRQ 146
decl_IRQ 147
decl_IRQ 148
decl_IRQ 149
decl_IRQ 150
decl_IRQ 151
decl_IRQ 152
decl_IRQ 153
decl_IRQ 154
decl_IRQ 155
decl_IRQ 156
decl_IRQ 157
decl_IRQ 158
decl_IRQ 159
decl_IRQ 160
decl_IRQ 161
decl_IRQ 162
decl_IRQ 163
decl_IRQ 164
decl_IRQ 165
decl_IRQ 166
decl_IRQ 167
decl_IRQ 168
decl_IRQ 169
decl_IRQ 170
decl_IRQ 171
decl_IRQ 172
decl_IRQ 173
decl_IRQ 174
decl_IRQ 175
decl_IRQ 176
decl_IRQ 177
decl_IRQ 178
decl_IRQ 179
decl_IRQ 180
decl_IRQ 181
decl_IRQ 182
decl_IRQ 183
decl_IRQ 184
decl_IRQ 185
decl_IRQ 186
decl_IRQ 187
decl_IRQ 188
decl_IRQ 189
decl_IRQ 190
decl_IRQ 191
