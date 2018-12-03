;Interrupt Service Routine
		.orig x1200
		
		;Push Registers
		ADD R6, R6, #-2
        STW R0, R6, #0
        ADD R6, R6, #-2
        STW R1, R6, #0
        ADD R6, R6, #-2
        STW R2, R6, #0
        ADD R6, R6, #-2
        STW R3, R6, #0
		
		LEA R1, PTBR
		LDW R1, R1, #0
		;R1 = PTBR
		LEA R2, COUNT
		LDW R2, R2, #0
		;R2 = COUNT
		LEA R3, CLEAR
		LDW R3, R3, #0
		;R3 = CLEAR

A		LDW R0, R1, #0	;R0 = PTE
		AND R0, R0, R3	;Clear Reference
		STW R0, R1, #0	;Store Cleared PTE
		ADD R1, R1, #2	;Increment address
		ADD R2, R2, #-1	;Decrement counter
		BRp A
		
		;Pop Registers			
		LDW R3, R6, #0;
        ADD R6, R6, #2;
        LDW R2, R6, #0;
        ADD R6, R6, #2;
        LDW R1, R6, #0;
        ADD R6, R6, #2;
        LDW R0, R6, #0;
        ADD R6, R6, #2;		
		
		;Return from Interrupt
		RTI

CLEAR		.fill xFFFE
COUNT		.fill x0080
PTBR		.fill x1000
.end