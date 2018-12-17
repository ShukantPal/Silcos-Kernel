; Copyright (C) 2017 - Sukant Pal

SECTION .text

	global ReadPort
	ReadPort:
		MOV EDX, [ESP + 4]
		IN AL, DX
		RET

	global WritePort
	WritePort:
		MOV EDX, [ESP + 4]
		MOV AL, [ESP + 8]
		OUT DX, AL
		RET
