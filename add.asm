.orig x3000

AND R0, R0, #0		;Clear R0
AND R5, R5, #0		;Clear R5
ADD R5, R5, #20		;R5 = 20
LEA R2, START		;R2 = xC000
LDW R2, R2, #0		;R2 = M[xC000]

A 	LDB R1, R2, #0	;R1 = (1B) M[R2]
	ADD R2, R2, #1	;R2 = R2 + 1
	ADD R0, R0, R1	;R0 += R1
	ADD R5, R5, #-1	;R5 -= 1
	BRp A			;Loop

LEA R3, SAVE		;R3 = xC014
LDW R3, R3, #0		;R3 = M[xC014]
LEA R2, MASK		;R2 = x00FF
LDW R2, R2, #0		;R2 = M[R2]
AND R0, R0, R2		;R0 = R0 && x00FF
STW R0, R3, #0		;R0 -> xC014
JMP R0

HALT

START .fill xC000
SAVE  .fill xC014
MASK  .fill x00FF
.end