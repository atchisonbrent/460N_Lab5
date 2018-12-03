/*
	Name 1: Brent Atchison
	UTEID 1: bma862
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
	IRD,
	COND1, COND0,
	J5, J4, J3, J2, J1, J0,
	LD_MAR,
	LD_MDR,
	LD_IR,
	LD_BEN,
	LD_REG,
	LD_CC,
	LD_PC,
	GATE_PC,
	GATE_MDR,
	GATE_ALU,
	GATE_MARMUX,
	GATE_SHF,
	PCMUX1, PCMUX0,
	DRMUX,
	SR1MUX,
	ADDR1MUX,
	ADDR2MUX1, ADDR2MUX0,
	MARMUX,
	ALUK1, ALUK0,
	MIO_EN,
	R_W,
	DATA_SIZE,
	LSHF1,
	/* MODIFY: you have to add all your new control signals */
	INT_EN,
	UNALIGNED_EN,
	PROT_EN,
	FAULT_EN,
	UNKNOWN,
	Save_JBITS,
	VAT_GO,
	LD_VA,
	GateVA,
	GatePTBR,
	VAT_RETURN,
	PTE_Modify,
	LD_PSR,
	GatePSR,
	LD_STP,
	SPMUX1,
	SPMUX0,
	GateSP,
	LD_TEMP,
	Restore_SP,
	Save_SP,
	PRI_SET,
	LD_NewSP,
	GateVector,
	MODIFY,
	CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x) { return(x[IRD]); }
int GetCOND(int *x) { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x) {
	return((x[J5] << 5) + (x[J4] << 4) +
		(x[J3] << 3) + (x[J2] << 2) +
		(x[J1] << 1) + x[J0]);
}
int GetLD_MAR(int *x) { return(x[LD_MAR]); }
int GetLD_MDR(int *x) { return(x[LD_MDR]); }
int GetLD_IR(int *x) { return(x[LD_IR]); }
int GetLD_BEN(int *x) { return(x[LD_BEN]); }
int GetLD_REG(int *x) { return(x[LD_REG]); }
int GetLD_CC(int *x) { return(x[LD_CC]); }
int GetLD_PC(int *x) { return(x[LD_PC]); }
int GetGATE_PC(int *x) { return(x[GATE_PC]); }
int GetGATE_MDR(int *x) { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x) { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x) { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x) { return(x[GATE_SHF]); }
int GetPCMUX(int *x) { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x) { return(x[DRMUX]); }
int GetSR1MUX(int *x) { return(x[SR1MUX]); }
int GetADDR1MUX(int *x) { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x) { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x) { return(x[MARMUX]); }
int GetALUK(int *x) { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x) { return(x[MIO_EN]); }
int GetR_W(int *x) { return(x[R_W]); }
int GetDATA_SIZE(int *x) { return(x[DATA_SIZE]); }
int GetLSHF1(int *x) { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */
/* LAB 4 */
int GetLD_NEWSTP(int *x) { return(x[LD_NewSP]); }
int GetLD_PSR(int *x) { return(x[LD_PSR]); }
int GetLD_STP(int *x) { return(x[LD_STP]); }
int GetLD_TEMP(int *x) { return(x[LD_TEMP]); }
int GetGATE_PSR(int *x) { return(x[GatePSR]); }
int GetGATE_STP(int *x) { return(x[GateSP]); }
int GetGATE_VECTOR(int *x) { return(x[GateVector]); }
int GetSPMUX0(int *x) { return(x[SPMUX0]); }
int GetSPMUX1(int *x) { return(x[SPMUX1]); }
int GetREST_STP(int *x) { return(x[Restore_SP]); }
int GetSAVE_STP(int *x) { return(x[Save_SP]); }
int GetPRI_SET(int *x) { return(x[PRI_SET]); }
int GetMODIFY(int *x) { return(x[MODIFY]); }
/* LAB 5 */
int GetLD_VA(int *x) { return(x[LD_VA]); }
int GetGATE_PTBR(int *x) { return(x[GatePTBR]); }
int GetGATE_VA(int *x) { return(x[GateVA]); }
int GetINT_EN(int *x) { return(x[INT_EN]); }
int GetPROT_EN(int *x) { return(x[PROT_EN]); }
int GetFAULT_EN(int *x) { return(x[FAULT_EN]); }
int GetSAVE_JBITS(int *x) { return(x[Save_JBITS]); }
int GetVAT_GO(int *x) { return(x[VAT_GO]); }
int GetVAT_RETURN(int *x) { return(x[VAT_RETURN]); }
int GetPTE_MOD(int *x) { return(x[PTE_Modify]); }
int GetUNALIGNED_EN(int *x) { return(x[UNALIGNED_EN]); }
int GetUNKNOWN(int *x) { return(x[UNKNOWN]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */ 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct {

	int PC,		/* program counter */
		MDR,	/* memory data register */
		MAR,	/* memory address register */
		IR,		/* instruction register */
		N,		/* n condition bit */
		Z,		/* z condition bit */
		P,		/* p condition bit */
		BEN;        /* ben register */

	int READY;	/* ready bit */
	  /* The ready bit is also latched as you dont want the memory system to assert it
		 at a bad point in the cycle*/

	int REGS[LC_3b_REGS]; /* register file. */

	int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

	int STATE_NUMBER; /* Current State Number - Provided for debugging */

	/* For lab 4 */
	int INTV; /* Interrupt vector register */
	int EXCV; /* Exception vector register */
	int SSP; /* Initial value of system stack pointer */
	/* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */
	int VBASE;
	int PSR;
	int PSR_OLD;
	int INT;
	int EXC;
	int STP;
	int STP_TEMP;
	int POP_TEMP;

	/* For lab 5 */
	int PTBR; /* This is initialized when we load the page table */
	int VA;   /* Temporary VA register */
	/* MODIFY: you should add here any other registers you need to implement virtual memory */
	int JBITS;
	int MOD;

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
	printf("----------------LC-3bSIM Help-------------------------\n");
	printf("go               -  run program to completion       \n");
	printf("run n            -  execute program for n cycles    \n");
	printf("mdump low high   -  dump memory from low to high    \n");
	printf("rdump            -  dump the register & bus values  \n");
	printf("?                -  display this help menu          \n");
	printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

	eval_micro_sequencer();
	cycle_memory();
	eval_bus_drivers();
	drive_bus();
	latch_datapath_values();

	CURRENT_LATCHES = NEXT_LATCHES;

	CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
	int i;

	if (RUN_BIT == FALSE) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating for %d cycles...\n\n", num_cycles);
	for (i = 0; i < num_cycles; i++) {
		if (CURRENT_LATCHES.PC == 0x0000) {
			RUN_BIT = FALSE;
			printf("Simulator halted\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
	if (RUN_BIT == FALSE) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating...\n\n");
	while (CURRENT_LATCHES.PC != 0x0000)
		cycle();
	RUN_BIT = FALSE;
	printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
	int address; /* this is a byte address */

	printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
	printf("-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
	printf("\n");

	/* dump the memory contents into the dumpsim file */
	fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
	fprintf(dumpsim_file, "-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
	fprintf(dumpsim_file, "\n");
	fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
	int k;

	printf("\nCurrent register/bus values :\n");
	printf("-------------------------------------\n");
	printf("Cycle Count  : %d\n", CYCLE_COUNT);
	printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
	printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
	printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
	printf("BUS          : 0x%0.4x\n", BUS);
	printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
	printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
	printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
	printf("Registers:\n");
	for (k = 0; k < LC_3b_REGS; k++)
		printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
	printf("\n");

	/* dump the state information into the dumpsim file */
	fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
	fprintf(dumpsim_file, "-------------------------------------\n");
	fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
	fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
	fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
	fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
	fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
	fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
	fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
	fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
	fprintf(dumpsim_file, "Registers:\n");
	for (k = 0; k < LC_3b_REGS; k++)
		fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
	fprintf(dumpsim_file, "\n");
	fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
	char buffer[20];
	int start, stop, cycles;

	printf("LC-3b-SIM> ");

	scanf("%s", buffer);
	printf("\n");

	switch (buffer[0]) {
	case 'G':
	case 'g':
		go();
		break;

	case 'M':
	case 'm':
		scanf("%i %i", &start, &stop);
		mdump(dumpsim_file, start, stop);
		break;

	case '?':
		help();
		break;
	case 'Q':
	case 'q':
		printf("Bye.\n");
		exit(0);

	case 'R':
	case 'r':
		if (buffer[1] == 'd' || buffer[1] == 'D')
			rdump(dumpsim_file);
		else {
			scanf("%d", &cycles);
			run(cycles);
		}
		break;

	default:
		printf("Invalid Command\n");
		break;
	}
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
	FILE *ucode;
	int i, j, index;
	char line[200];

	printf("Loading Control Store from file: %s\n", ucode_filename);

	/* Open the micro-code file. */
	if ((ucode = fopen(ucode_filename, "r")) == NULL) {
		printf("Error: Can't open micro-code file %s\n", ucode_filename);
		exit(-1);
	}

	/* Read a line for each row in the control store. */
	for (i = 0; i < CONTROL_STORE_ROWS; i++) {
		if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
			printf("Error: Too few lines (%d) in micro-code file: %s\n",
				i, ucode_filename);
			exit(-1);
		}

		/* Put in bits one at a time. */
		index = 0;

		for (j = 0; j < CONTROL_STORE_BITS; j++) {
			/* Needs to find enough bits in line. */
			if (line[index] == '\0') {
				printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
					ucode_filename, i);
				exit(-1);
			}
			if (line[index] != '0' && line[index] != '1') {
				printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
					ucode_filename, i, j);
				exit(-1);
			}

			/* Set the bit in the Control Store. */
			CONTROL_STORE[i][j] = (line[index] == '0') ? 0 : 1;
			index++;
		}

		/* Warn about extra bits in line. */
		if (line[index] != '\0')
			printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
				ucode_filename, i);
	}
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
	int i;

	for (i = 0; i < WORDS_IN_MEM; i++) {
		MEMORY[i][0] = 0;
		MEMORY[i][1] = 0;
	}
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename, int is_virtual_base) {
	FILE * prog;
	int ii, word, program_base, pte, virtual_pc;

	/* Open program file. */
	prog = fopen(program_filename, "r");
	if (prog == NULL) {
		printf("Error: Can't open program file %s\n", program_filename);
		exit(-1);
	}

	/* Read in the program. */
	if (fscanf(prog, "%x\n", &word) != EOF)
		program_base = word >> 1;
	else {
		printf("Error: Program file is empty\n");
		exit(-1);
	}

	if (is_virtual_base) {
		if (CURRENT_LATCHES.PTBR == 0) {
			printf("Error: Page table base not loaded %s\n", program_filename);
			exit(-1);
		}

		/* convert virtual_base to physical_base */
		virtual_pc = program_base << 1;
		pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) |
			MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

		printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
		if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
			program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
			printf("physical base of program: %x\n\n", program_base);
			program_base = program_base >> 1;
		}
		else {
			printf("attempting to load a program into an invalid (non-resident) page\n\n");
			exit(-1);
		}
	}
	else {
		/* is page table */
		CURRENT_LATCHES.PTBR = program_base << 1;
	}

	ii = 0;
	while (fscanf(prog, "%x\n", &word) != EOF) {
		/* Make sure it fits. */
		if (program_base + ii >= WORDS_IN_MEM) {
			printf("Error: Program file %s is too long to fit in memory. %x\n",
				program_filename, ii);
			exit(-1);
		}

		/* Write the word to memory array. */
		MEMORY[program_base + ii][0] = word & 0x00FF;
		MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
		ii++;
	}

	if (CURRENT_LATCHES.PC == 0 && is_virtual_base)
		CURRENT_LATCHES.PC = virtual_pc;

	printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) {
	int i;
	init_control_store(argv[1]);

	init_memory();
	load_program(argv[2], 0);
	for (i = 0; i < num_prog_files; i++) {
		load_program(argv[i + 3], 1);
	}
	CURRENT_LATCHES.Z = 1;
	CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
	memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
	CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

/* MODIFY: you can add more initialization code HERE */
	CURRENT_LATCHES.VBASE = 0x0200;
	CURRENT_LATCHES.PSR = 0x8000;
	CURRENT_LATCHES.PTBR = 0x1000;

	NEXT_LATCHES = CURRENT_LATCHES;

	RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
	FILE * dumpsim_file;

	/* Error Checking */
	if (argc < 4) {
		printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
			argv[0]);
		exit(1);
	}

	printf("LC-3b Simulator\n\n");

	initialize(argv, argc - 3);

	if ((dumpsim_file = fopen("dumpsim", "w")) == NULL) {
		printf("Error: Can't open dumpsim file\n");
		exit(-1);
	}

	while (1)
		get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
   with a "MODIFY:" comment.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
   /***************************************************************/


void eval_micro_sequencer() {

	/*
	 * Evaluate the address of the next state according to the
	 * micro sequencer logic. Latch the next microinstruction.
	 */

	int curr_inst = CURRENT_LATCHES.MICROINSTRUCTION;
	
	/* Set Interrupt Flag */
	if (CYCLE_COUNT == 300) {
		NEXT_LATCHES.INT = 1;
		CURRENT_LATCHES.INT = 1;
	}

	/* Set Protect Flag */
	int prot_flag = 0;
	if ((CURRENT_LATCHES.IR & 0xF000) != 0xF000) {
		int PSR = CURRENT_LATCHES.PSR & 0x8000;
		int PROT = CURRENT_LATCHES.MDR & 0x0008;
		if ((PSR != 0) && (PROT == 0)) { prot_flag = 1; }
	}

	/* Set Page Fault Flag */
	int pfault_flag = 0;
	if ((CURRENT_LATCHES.MAR & 0x0001) != 0) { pfault_flag = 1; }

	/* Set Unaligned Flag */
	int u_flag = 0;
	if (CURRENT_LATCHES.MAR & 0x0001) { u_flag = 1; }

	/* Interrupt */
	if (CURRENT_LATCHES.INT && GetINT_EN(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = 50;
		int* next_inst = CONTROL_STORE[50];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = 50;
		NEXT_LATCHES.INT = 0;
		CURRENT_LATCHES.INT = 0;
		NEXT_LATCHES.INTV = 0x0001;
	}

	/* Unaligned Access */
	else if (u_flag && GetUNALIGNED_EN(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = 50;
		int* next_inst = CONTROL_STORE[50];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = 50;
		NEXT_LATCHES.EXCV = 0x0003;
	}

	/* Protection */
	else if (prot_flag && GetPROT_EN(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = 50;
		int* next_inst = CONTROL_STORE[50];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = 50;
		NEXT_LATCHES.EXCV = 0x0004;
	}

	/* Page Fault */
	else if (pfault_flag && GetFAULT_EN(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = 50;
		int* next_inst = CONTROL_STORE[50];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = 50;
		NEXT_LATCHES.EXCV = 0x0002;
	}

	/* Unknown Opcode */
	else if (GetUNKNOWN(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = 50;
		int* next_inst = CONTROL_STORE[50];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = 50;
		NEXT_LATCHES.EXCV = 0x0005;
	}

	else if (GetVAT_GO(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = 42;
		int* next_inst = CONTROL_STORE[42];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = 42;
	}

	else if (GetVAT_RETURN(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = CURRENT_LATCHES.JBITS;
		int* next_inst = CONTROL_STORE[CURRENT_LATCHES.JBITS];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = CURRENT_LATCHES.JBITS;
	}

	else if (GetIRD(curr_inst)) {
		NEXT_LATCHES.STATE_NUMBER = (CURRENT_LATCHES.IR >> 12) & 0x000F;
		int* next_inst = CONTROL_STORE[(CURRENT_LATCHES.IR >> 12) & 0x000F];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = (CURRENT_LATCHES.IR >> 12) & 0x000F;
	}

	else {
		CURRENT_LATCHES.STATE_NUMBER = GetJ(curr_inst);
		int cc = GetCOND(curr_inst);
		int br = (CURRENT_LATCHES.BEN && cc == 2) << 2;
		int rdy = (CURRENT_LATCHES.READY && cc == 1) << 1;
		int adm = (CURRENT_LATCHES.IR & 0x0800) && cc == 3;
		int next_state = Low16bits(GetJ(curr_inst) | br | rdy | adm);
		int* next_inst = CONTROL_STORE[GetJ(curr_inst)];
		for (int i = 0; i < CONTROL_STORE_BITS; i++) { NEXT_LATCHES.MICROINSTRUCTION[i] = next_inst[i]; }
		NEXT_LATCHES.STATE_NUMBER = next_state;
	}

	if (GetSAVE_JBITS(curr_inst)) { NEXT_LATCHES.JBITS = GetJ(curr_inst); }
}


/**************************************************************
 * This function emulates memory and the WE logic.
 * Keep track of which cycle of MEMEN we are dealing with.
 * If fourth, we need to latch Ready bit at the end of
 * cycle to prepare microsequencer for the fifth cycle.
 **************************************************************/
int memory, mem_cycles, w_e[2]; // Memory Variables
void cycle_memory() {

	/* Get Current Microinstruction */
	int* curr_inst = CURRENT_LATCHES.MICROINSTRUCTION;

	/* Get Memory Enable */
	if (GetMIO_EN(curr_inst)) {

		int mar = CURRENT_LATCHES.MAR & 0x0001;
		int d_size = GetDATA_SIZE(curr_inst);
		int write = GetR_W(curr_inst);
		w_e[0] = (!mar) && write;
		w_e[1] = write && (mar ^ d_size);

		/* Update Cycle Count */
		if (mem_cycles < 4) { mem_cycles++; }
		if (mem_cycles == 4) {
			NEXT_LATCHES.READY = 1;
			mem_cycles++;
		}

		/* Memory Ready */
		if (CURRENT_LATCHES.READY) {
			if (write) {    /* Write */
				if (w_e[0]) { MEMORY[CURRENT_LATCHES.MAR >> 1][0] = Low16bits(CURRENT_LATCHES.MDR & 0x00FF); }
				if (w_e[1]) { MEMORY[CURRENT_LATCHES.MAR >> 1][1] = Low16bits((CURRENT_LATCHES.MDR & 0xFF00) >> 8); }
			}
			else {        /* Read */
				memory = Low16bits(MEMORY[CURRENT_LATCHES.MAR / 2][0] +
					MEMORY[CURRENT_LATCHES.MAR / 2][1] * 256);
			}
			NEXT_LATCHES.READY = 0;
			mem_cycles = 0;
		}

	}
	else {    /* Reset */
		mem_cycles = 0;
		w_e[0] = 0;
		w_e[1] = 0;
	}
}


/**************************************************************
 * Datapath Routine
 * Emulate operations before driving the bus.
 * Evaluate the input of tristate drivers
 *         Gate_MARMUX,
 *         Gate_PC,
 *         Gate_ALU,
 *         Gate_SHF,
 *         Gate_MDR.
 **************************************************************/
int pc_out, marmux_out, mdr_out, alu_out, shf_out, addr_sum;    // Bus variables
int stp_out, vector_out, psrold_out, ptb_out, va_out;			// New
void eval_bus_drivers() {

	/* Get Current Microinstruction */
	int* curr_inst = CURRENT_LATCHES.MICROINSTRUCTION;

	/* PC */
	pc_out = CURRENT_LATCHES.PC;

	/* MUX 1 */
	int mux_1;
	if (GetADDR1MUX(curr_inst)) {
		int sr = Low16bits((CURRENT_LATCHES.IR & 0x01C0) >> 6);
		mux_1 = Low16bits(CURRENT_LATCHES.REGS[sr]);
	}
	else { mux_1 = Low16bits(CURRENT_LATCHES.PC); }

	/* MUX 2 */
	int mux_2;
	if (GetADDR2MUX(curr_inst) == 1) {
		int ir_ext = CURRENT_LATCHES.IR & 0x003F;
		if (ir_ext & 0x0020) { ir_ext = Low16bits(ir_ext | 0xFFC0); }
		if (GetLSHF1(curr_inst)) { mux_2 = Low16bits(ir_ext * 2); }
		else { mux_2 = Low16bits(ir_ext); }
	}
	else if (GetADDR2MUX(curr_inst) == 2) {
		int ir_ext = CURRENT_LATCHES.IR & 0x01FF;
		if (ir_ext & 0x0100) { ir_ext = Low16bits(ir_ext | 0xFE00); }
		if (GetLSHF1(curr_inst)) { mux_2 = Low16bits(ir_ext * 2); }
		else { mux_2 = Low16bits(ir_ext); }
	}
	else if (GetADDR2MUX(curr_inst) == 3) {
		int ir_ext = CURRENT_LATCHES.IR & 0x07FF;
		if (ir_ext & 0x0400) { ir_ext = Low16bits(ir_ext | 0xF800); }
		if (GetLSHF1(curr_inst)) { mux_2 = Low16bits(ir_ext * 2); }
		else { mux_2 = Low16bits(ir_ext); }
	}
	else { mux_2 = 0; }

	addr_sum = mux_2 + mux_1;   // Sum muxes for address

	/* MARMUX */
	if (GetMARMUX(curr_inst) == 0) { marmux_out = Low16bits((CURRENT_LATCHES.IR & 0x00FF) << 1); }
	else { marmux_out = addr_sum; }

	/* SR2 MUX */
	int sr2_mux;
	int ir_5 = CURRENT_LATCHES.IR & 0x0020;     // Check for offset
	if (ir_5 == 0) {    /* SR1 */
		int sr2 = CURRENT_LATCHES.IR & 0x0007;  // Mask SR2
		sr2_mux = CURRENT_LATCHES.REGS[sr2];    // Set mux
	}
	else {            /* Offset */
		int imm5 = CURRENT_LATCHES.IR & 0x001F;                     // Mask Offset
		if (imm5 & 0x0010) { sr2_mux = Low16bits(imm5 | 0xFFE0); }  // Negative
		else { sr2_mux = Low16bits(imm5); }                         // Postiive
	}

	/* MDR */
	if (GetDATA_SIZE(curr_inst)) { mdr_out = Low16bits(CURRENT_LATCHES.MDR); }
	else {
		if (CURRENT_LATCHES.MAR & 0x0001) { // Odd address
			if (CURRENT_LATCHES.MDR & 0x8000) { mdr_out = Low16bits((CURRENT_LATCHES.MDR >> 8) | 0xFF00); }
			else { mdr_out = Low16bits(CURRENT_LATCHES.MDR >> 8) & 0x00FF; }
		}
		else {                              // Normal
			if (CURRENT_LATCHES.MDR & 0x0080) { mdr_out = Low16bits(CURRENT_LATCHES.MDR | 0xFF00) & 0x00FF; }
			else { mdr_out = Low16bits(CURRENT_LATCHES.MDR) & 0x00FF; }
		}
	}

	/* ALU */
	int sr1 = Low16bits((CURRENT_LATCHES.IR & 0x01C0) >> 6);
	int a = CURRENT_LATCHES.REGS[sr1];
	int b = sr2_mux;
	if (GetALUK(curr_inst) == 0) { alu_out = Low16bits(a + b); }        // ADD
	else if (GetALUK(curr_inst) == 1) { alu_out = Low16bits(a & b); }   // AND
	else if (GetALUK(curr_inst) == 2) { alu_out = Low16bits(a ^ b); }   // XOR
	else {                                                              // SR1
		if (!(GetSR1MUX(curr_inst))) { sr1 = Low16bits(CURRENT_LATCHES.IR >> 9) & 0x0007; }
		alu_out = Low16bits(CURRENT_LATCHES.REGS[sr1]);
	}

	/* SHF */
	int shift = CURRENT_LATCHES.IR & 0x000F;
	int sr = Low16bits((CURRENT_LATCHES.IR & 0x01C0) >> 6);
	int val = CURRENT_LATCHES.REGS[sr];
	if (CURRENT_LATCHES.IR & 0x0010) {                                          // >>
		if (CURRENT_LATCHES.IR & 0x0020) { shf_out = Low16bits(val >> shift); } // Logical
		else {                                                                  // Arithmetic
			if (val & 0x8000) {                                                 // Negative
				for (int i = 0; i < shift; i++) { val = Low16bits((val >> 1) | 0x8000); }
				shf_out = Low16bits(val);
			}
			else { shf_out = Low16bits(val >> shift); }                       // Positive
		}
	}
	else { shf_out = Low16bits(val << shift); }                               // <<

	/* STP */
	if (GetSPMUX0(curr_inst) == 0 && GetSPMUX1(curr_inst) == 0) { stp_out = CURRENT_LATCHES.STP; }
	else if (GetSPMUX0(curr_inst) == 0 && GetSPMUX1(curr_inst) == 1) { stp_out = CURRENT_LATCHES.STP - 2; }
	else if (GetSPMUX0(curr_inst) == 1 && GetSPMUX1(curr_inst) == 0) { stp_out = CURRENT_LATCHES.STP + 2; }
	else { stp_out = 0x3000; }

	/* PSR */
	psrold_out = Low16bits(CURRENT_LATCHES.PSR_OLD);

	/* PTBR */
	ptb_out = Low16bits(CURRENT_LATCHES.PTBR + (2 * ((CURRENT_LATCHES.VA & 0xFE00) >> 9)));

	/* VA */
	va_out = Low16bits((CURRENT_LATCHES.MDR & 0x3E00) + (CURRENT_LATCHES.VA & 0x01FF));

	/* Vector */
	vector_out = Low16bits(CURRENT_LATCHES.VBASE + 2 * CURRENT_LATCHES.INTV + 2 * CURRENT_LATCHES.EXCV);
}


/**************************************************************
 * Datapath routine for driving the bus from
 * one of the 5 possible tristate drivers.
 **************************************************************/
void drive_bus() {

	/* Get Current Microinstruction */
	int* curr_inst = CURRENT_LATCHES.MICROINSTRUCTION;

	if (GetGATE_PC(curr_inst)) { BUS = pc_out; }                // PC
	else if (GetGATE_MARMUX(curr_inst)) { BUS = marmux_out; }   // MARMUX
	else if (GetGATE_MDR(curr_inst)) { BUS = mdr_out; }         // MDR
	else if (GetGATE_ALU(curr_inst)) { BUS = alu_out; }         // ALU
	else if (GetGATE_SHF(curr_inst)) { BUS = shf_out; }         // SHF
	else if (GetGATE_STP(curr_inst)) { BUS = stp_out; }			// STP
	else if (GetGATE_PSR(curr_inst)) { BUS = psrold_out; }		// PSR
	else if (GetGATE_PTBR(curr_inst)) { BUS = ptb_out; }		// PTBR
	else if (GetGATE_VA(curr_inst)) { BUS = va_out; }			// VA
	else if (GetGATE_VECTOR(curr_inst)) { BUS = vector_out; }	// VECTOR
	else { BUS = BUS; }                                         // Same

}


/**************************************************************
 * Datapath routine for computing all functions that need to
 * latch values in the data path at the end of this cycle.
 * Some values require sourcing the bus;
 * Therefore, this routine has to come after drive_bus.
 **************************************************************/
void latch_datapath_values() {

	/* Get Current Microinstruction */
	int* curr_inst = CURRENT_LATCHES.MICROINSTRUCTION;

	/* PC */
	if (GetLD_PC(curr_inst)) {
		if (GetPCMUX(curr_inst) == 1) { NEXT_LATCHES.PC = Low16bits(BUS); }             // From bus
		else if (GetPCMUX(curr_inst) == 2) { NEXT_LATCHES.PC = Low16bits(addr_sum); }   // From offset
		else if (GetPCMUX(curr_inst) == 3) { NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC - 2); }
		else {																			// Increment
			if (GetINT_EN(curr_inst) && CURRENT_LATCHES.INT == 1) { NEXT_LATCHES.PC = CURRENT_LATCHES.PC; }
			else { NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2); }
		}
	} else { NEXT_LATCHES.PC = CURRENT_LATCHES.PC; }									// Stay

	/* MAR */
	if (GetLD_MAR(curr_inst)) { NEXT_LATCHES.MAR = Low16bits(BUS); }

	/* MDR */
	if (GetLD_MDR(curr_inst)) {
		if (GetMIO_EN(curr_inst)) { NEXT_LATCHES.MDR = Low16bits(memory); } // Read
		else {                                                              // Write
			if (GetDATA_SIZE(curr_inst) == 0) {
				int d = BUS & 0x00FF;
				if (d & 0x0080) { d |= 0xFF00; }
				NEXT_LATCHES.MDR = Low16bits(d);
			}
			else { NEXT_LATCHES.MDR = Low16bits(BUS); }
		}
	}

	/* IR */
	if (GetLD_IR(curr_inst)) { NEXT_LATCHES.IR = Low16bits(BUS); }
	else { NEXT_LATCHES.IR = CURRENT_LATCHES.IR; }

	/* BEN */
	if (GetLD_BEN(curr_inst)) {
		int checkP = (CURRENT_LATCHES.IR >> 9) & CURRENT_LATCHES.P;
		int checkN = (CURRENT_LATCHES.IR >> 11) & CURRENT_LATCHES.N;
		int checkZ = (CURRENT_LATCHES.IR >> 10) & CURRENT_LATCHES.Z;
		NEXT_LATCHES.BEN = (checkN | checkZ | checkP);
	}

	/* REG */
	if (GetLD_REG(curr_inst)) {
		int dr = Low16bits((CURRENT_LATCHES.IR & 0x0E00) >> 9);
		if (GetDRMUX(curr_inst)) { dr = 7; }
		NEXT_LATCHES.REGS[dr] = Low16bits(BUS);
	}

	/* Copy Registers */
	for (int i = 0; i < LC_3b_REGS; i++) { NEXT_LATCHES.REGS[i] = CURRENT_LATCHES.REGS[i]; }

	/* CC */
	if (GetLD_CC(curr_inst)) {
		if ((signed short) BUS > 0) {
			NEXT_LATCHES.P = 1;
			NEXT_LATCHES.Z = 0;
			NEXT_LATCHES.N = 0;
		}
		else if ((signed short) BUS < 0) {
			NEXT_LATCHES.P = 0;
			NEXT_LATCHES.Z = 0;
			NEXT_LATCHES.N = 1;
		}
		else {
			NEXT_LATCHES.P = 0;
			NEXT_LATCHES.Z = 1;
			NEXT_LATCHES.N = 0;
		}
	}

	/* PSR */
	if (GetLD_PSR(curr_inst)) {
		if (GetLD_NEWSTP(curr_inst)) {
			int n = CURRENT_LATCHES.N << 2;
			int z = CURRENT_LATCHES.Z << 1;
			int p = CURRENT_LATCHES.P;
			CURRENT_LATCHES.PSR |= n | z | p;
			NEXT_LATCHES.PSR_OLD = CURRENT_LATCHES.PSR;
			NEXT_LATCHES.PSR = Low16bits((CURRENT_LATCHES.PSR & 0x7FFF));
		}
		else {
			NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR_OLD;
			NEXT_LATCHES.N = (CURRENT_LATCHES.PSR_OLD & 0x0004) >> 2;
			NEXT_LATCHES.Z = (CURRENT_LATCHES.PSR_OLD & 0x0002) >> 1;
			NEXT_LATCHES.P = (CURRENT_LATCHES.PSR_OLD & 0x0001);
		}
	}

	/* STP */
	if (GetLD_SP(curr_inst)) {
		NEXT_LATCHES.STP = Low16bits(stp_out);
		NEXT_LATCHES.REGS[6] = Low16bits(stp_out);
	}

	/* NEW STP */
	if (GetLD_NEWSTP(curr_inst)) {
		NEXT_LATCHES.STP = CURRENT_LATCHES.SSP;
		NEXT_LATCHES.STP_TEMP = CURRENT_LATCHES.REGS[6];
	}

	/* PTE */
	if (GetPTE_MOD(curr_inst) == 1) {
		NEXT_LATCHES.MDR = (CURRENT_LATCHES.MDR | 0x0001);
		if (CURRENT_LATCHES.MOD) { NEXT_LATCHES.MDR = NEXT_LATCHES.MDR | 0x0002; }
		NEXT_LATCHES.MOD = 0;
	}

	/* PTE MOD */
	if (GetMODIFY(curr_inst) == 1) {
		NEXT_LATCHES.MOD = 1;
		CURRENT_LATCHES.MOD = 1;
	}

	/* TEMP */
	if (GetLD_TEMP(curr_inst)) { NEXT_LATCHES.POP_TEMP = Low16bits(BUS); }

	/* VA */
	if (GetLD_VA(curr_inst)) { NEXT_LATCHES.VA = CURRENT_LATCHES.MAR; }

	/* MAR <- VA */
	if (GetGATE_VA(curr_inst)) { NEXT_LATCHES.MAR = va_out; }
}