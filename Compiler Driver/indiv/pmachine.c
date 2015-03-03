/*
 * Drew Nagy-Kato
 * COP 3402 - Systems Software
 * Assignment 1 - P-Machine (Virtual Machine)
 * 6/5/12
**/

/** NOTES
 *	- To terminate 'while' loop:
		- at end of each loop, do a condition check
			- if false, break loop
			- if still true, do nothing
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_HEIGHT 2000
#define CODE_SIZE 500
#define MAX_LEXI_LEVELS 3

struct instruction
{
	int op;				// OPcode
	int l;				// L
	int m;				// M
};

enum OPR_M {OPR_RTN = 0, OPR_NEG, OPR_ADD, OPR_SUB, OPR_MUL, OPR_DIV, OPR_ODD, OPR_MOD, OPR_EQL, OPR_NEQ, OPR_LSS, OPR_LEQ, OPR_GTR, OPR_GEQ};
enum OPCODE {LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, WRT, RED};


//Function Prototypes
int base(int l, int base,int* stack);
int read();


int main(void)
{
    struct instruction IR;
    struct instruction MEM[CODE_SIZE];
    int stack[MAX_STACK_HEIGHT];
    int SP; 
    int BP; 
    int PC;
    
    char OPstring[4];
    int OPnum;
    int L;
    int M;
    int i;
    int line;
    int AR[MAX_STACK_HEIGHT];
    int level_count;
    
    BP = 1;
    PC = 0;
    SP = 0;
    
    &IR == malloc(sizeof(struct instruction));
    MEM == calloc(CODE_SIZE, sizeof(struct instruction));
	
    FILE* fin;
    FILE* fout;
    
    fin = fopen("T2_vminput.txt", "r");
    fout = fopen("T2_stackout.txt", "w");
    
    // Fetch Cycle - Start
    fprintf(fout, "LINE   OP      L     M\n");
    printf("LINE   OP      L     M\n");
	PC = 0;
    while(!feof(fin))
    {
        //Copy contents of input file into MEM
        fscanf(fin, "%d  ", &OPnum);
        fscanf(fin, "%d  ", &L);
        fscanf(fin, "%d  ", &M);
        
		MEM[PC].op = OPnum;
		MEM[PC].l = L;
		MEM[PC].m = M;
		
		if(OPnum == LIT)          // LIT
			strcpy(OPstring, "LIT");
        if(OPnum == OPR)          // OPR
			strcpy(OPstring, "OPR");
        if(OPnum == LOD)          // LOD
			strcpy(OPstring, "LOD");
        if(OPnum == STO)          // STO
			strcpy(OPstring, "STO");
        if(OPnum == CAL)          // CAL
			strcpy(OPstring, "CAL");
        if(OPnum == INC)          // INC
			strcpy(OPstring, "INC");
        if(OPnum == JMP)          // JMP
			strcpy(OPstring, "JMP");
        if(OPnum == JPC)          // JPC
			strcpy(OPstring, "JPC");
        if(OPnum == WRT)          // SIO  WRITE
			strcpy(OPstring, "SOI");
        if(OPnum == RED)         // SIO  READ
			strcpy(OPstring, "SIO");
        
        fprintf(fout, "%3d    %s     %d    %2d\n", PC, OPstring, L, M);
        printf("%3d    %s     %d    %2d\n", PC, OPstring, L, M);
        PC++;
    }
    fclose(fin);
    // Fetch Cycle - End
    
    stack[0] = 0;       // static link      SL
    stack[1] = 0;       // dynamic link     DL
    stack[2] = 0;       // return address   RA
    SP = 0; 
    BP = 1; 
    PC = 0;
    line = 0;
    int decoder = 0;
    
    fprintf(fout, "\n\nInitial Values                Post-Instruction Values\n"
       " PC    OP      L     M        PC   BP     SP    Stack\n");
    printf("\n\nInitial Values              Post-Instruction Values\n"
       " PC     OP     L     M      PC   BP    SP    Stack\n");

	do{
		// Fetch cycle - start
		IR = MEM[PC];
		PC++;
		decoder = IR.op;
		//printf("%d \n", decoder);
		// Fetch cycle - end

		switch(decoder){
			case LIT:          // LIT
		    	strcpy(OPstring, "LIT");
		    	break;
        	case OPR:          // OPR
	        	strcpy(OPstring, "OPR");
	        	break;
	        case LOD:          // LOD
	            strcpy(OPstring, "LOD");
	            break;
	        case STO:          // STO
			    strcpy(OPstring, "STO");
			    break;
	        case CAL:          // CAL
	       		strcpy(OPstring, "CAL");
	       		break;
	        case INC:          // INC
	        	strcpy(OPstring, "INC");
	        	break;
	        case JMP:          // JMP
	      		strcpy(OPstring, "JMP");
	      		break;
	        case JPC:          // JPC
		    	strcpy(OPstring, "JPC");
		    	break;
	        case WRT:          // SIO
		    	strcpy(OPstring, "SIO");
		    	break;
	        case RED:         // SIO
	            strcpy(OPstring, "SIO");
	            break;
			default:
				strcpy(OPstring, "ERR");
				break;
		}

        //Output of initial values
        fprintf(fout, "%3d    %3s     %d     %-2d    ", PC-1, OPstring, IR.l, IR.m);
        printf("%3d    %3s     %d     %-2d   ", PC-1, OPstring, IR.l, IR.m);

		int mod = IR.m;

		// Execute cycle - start
		switch(decoder){
			case LIT:     // LIT 0, M
				SP += 1;
            	stack[SP] = IR.m;
            	break;
			case OPR:     // OPR 0, #
				switch(mod){
					case OPR_RTN:     // RTN 0, 0
						SP = BP - 1;
						PC = stack[SP + 3];
						BP = stack[SP + 2];
						AR[SP] = 0;
		            	break;
					case OPR_NEG:     // NEG 0, 1
						stack[SP] = -(stack[SP]);
		            	break;
					case OPR_ADD:     // ADD 0, 2
						SP -= 1;
		            	stack[SP] += stack[SP + 1];
		            	break;
					case OPR_SUB:     // SUB 0, 3
						SP -= 1;
		            	stack[SP] -= stack[SP + 1];
		            	break;
					case OPR_MUL:     // MUL 0, 4
						SP -= 1;
		            	stack[SP] *= stack[SP + 1];
		            	break;
					case OPR_DIV:     // DIV 0, 5
						SP -= 1;
		            	stack[SP] = stack[SP] / stack[SP + 1];
		            	break;
					case OPR_ODD:     // ODD 0, 6
						stack[SP] = stack[SP] % 2;
		            	break;
					case OPR_MOD:     // MOD 0, 7
						SP -= 1;
		            	stack[SP] = stack[SP] % stack[SP + 1];
		            	break;
					case OPR_EQL:     // EQL 0, 8
						SP -= 1;
		            	stack[SP] = (stack[SP] == stack[SP + 1]) ? 1 : 0;
		            	break;
					case OPR_NEQ:     // NEQ 0, 9
						SP -= 1;
		            	stack[SP] = (stack[SP] != stack[SP + 1]) ? 1 : 0;
		            	break;
					case OPR_LSS:    // LSS 0, 10
						SP -= 1;
		            	stack[SP] = (stack[SP] < stack[SP + 1]) ? 1 : 0;
		            	break;
					case OPR_LEQ:    // LEQ 0, 11
						SP -= 1;
		            	stack[SP] = (stack[SP] <= stack[SP + 1]) ? 1 : 0;
		            	break;
					case OPR_GTR:    // GTR 0, 12
						SP -= 1;
		            	stack[SP] = (stack[SP] > stack[SP + 1]) ? 1 : 0;
		            	break;
					case OPR_GEQ:    // GEQ 0, 13
						SP -= 1;
		            	stack[SP] = (stack[SP] >= stack[SP + 1]) ? 1 : 0;
						break;
					default:
						printf("Your input file contains an improper opr mod code.\n"
		                    "Please be sure to check it for errors and try again.\n");
						return 0;
				}
            	break;
			case LOD:     // LOD L, M
				SP += 1;
            	stack[SP] = stack[base(IR.l, BP, stack) + IR.m];
            	break;
			case STO:     // STO L, M
				stack[base(IR.l, BP, stack) + IR.m] = stack[SP];
				SP -= 1;
            	break;
			case CAL:     // CAL L, M
				stack[SP + 1] = base(IR.l, BP, stack);       	// static link		(SL)
	            stack[SP + 2] = BP;                             // dynamic link		(DL)
		        stack[SP + 3] = PC;	 		                    // return address	(RA)
	            BP = SP + 1;
	            PC = IR.m;
	            AR[SP] = 1;
            	break;
			case INC:     // INC 0, M
				for(i = SP+1; i <= SP + IR.m; i++)
					stack[i] = 0;
				SP += IR.m;
            	break;
			case JMP:     // JMP 0, M
				PC = IR.m;
            	break;
			case JPC:     // JPC 0, M
				if(stack[SP] == 0){
					PC = IR.m;
					SP -= 1;
				}
            	break;
			case WRT:     // SIO 0, 1     WRITE
				printf("\nTop stack element: %d\n			  ", stack[SP]);
				//printf("			  ");
				SP -= 1;
            	break;
			case RED:   // SIO 0, 2     READ
				SP += 1;
				stack[SP] = read();
				printf("			  ");
				break;
			default:
				printf("Improper Opcode!!\n");
				return 0;
				break;
		}
		
		//Output of stack after instruction processing
        fprintf(fout, "   %-3d    %-d     %-2d   ", PC, BP, SP);
        printf("  %-3d    %-d    %-2d   ", PC, BP, SP);

		level_count = 1;
		
		for(i = 1; i <= SP; i++){
            // Insert "|"
            if(AR[i-1]){
                 fprintf(fout, "| ");
                 printf("| ");
                 level_count++;
            }

			fprintf(fout, "%-2d ", stack[i]);
            printf("%-2d ", stack[i]);
            
        }
        
        if(level_count > MAX_LEXI_LEVELS){
			printf("Lexigraphical Level Overflow!\n");
			break;
        }
        
		fprintf(fout, "\n");
        printf("\n");
	}while(BP > 0);



    fclose(fout);
    
    //printf("\n\nCongratulations! The input file has been successfully processed. Please refer\n"
    //"to the file 'stackout.txt', located in the same directory as this program.\n\n");
    
    return 0;
}

int base(int l,int base,int* stack){		// l stand for L in the instruction format  
	int b1;									// find base L levels down
	b1 = base;
	while(l > 0){
		b1 = stack[b1];
		l--;
	}
	return b1;
}

int read(){
	int input;
	
	printf("\nPlease enter a value to add to the stack:\n");
	scanf("%d", &input);
	return input;
}
