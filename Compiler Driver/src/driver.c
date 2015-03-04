/**
 * Drew Nagy-Kato
 * COP 3402 - System Software
 * Assignment 4 - Compiler Driver
 * 8/27/12
**/

/** Program Process:
 *  - Program prints "In" to stdout
 *  - Program reads in external arguments
 *	- Program calls each subprogram (scanner, parser, VM) in turn
 *  - Each subprogram returns a success/fail flag upon completion
		- scanner
			- fails if input file cannot be found
		- parser fail
			- returns error number, warning printed out to screen
			- program terminates
		- VM
			- no possible ABEND
 *	- If additional arguments present, print out corresponding outputs
		- Arguments:
			- 'input_filename' (name of the input file to be read into the scanner)
			- '-l' (print the list of lexemes/token (scanner output) to the screen)
			- '-a' (print the generated assembly code (parser/codegen output) to the screen)
			- '-v' (print virtual machine execution trace (VM output) to the screen)
		- Outputs for each subprogram stored in permanant text files
 *  - Final output stored in text file "output.txt"
 *	- Program prints "Out" to stdout
**/

/** TODOs / Bugs
 *  - Resolve file I/O issues
		- consider converting to temp files to eliminate extra text files
		- consider removing intermediate file writes and reads
			- transfer between structs, or combine reduntant structs
    - Error in separateTokens(char string[])
		- not ignoring garbage code
			- error doesn't exist in scanner.c, though code is identical??
		(resolved by printing each string scanned to a temp file)??
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_TABLE_SIZE 500
#define CODE_SIZE 500
#define MAX_LEXI_LEVELS 3
#define MAX_CHARS 11
#define MAX_DIGS 5

// Scanner structs
struct lextable {
	char lexeme[13];			// lexeme string
	int number;             	// numbersym number
	int token_type;				// numerical designation of lexeme
};
// Parser structs
struct symtable {
	int kind;					// const = 1, var = 2, proc = 3
	char name[MAX_CHARS];		// name up to 11 chars
	int val;					// number (ASCII value)
	int level;					// L level
	int addr;                   // M address
};
struct codegen {
	int op;                     // opcode
	int l;                      // lexicographical level
	int m;                      // modifier
};
struct tokenlist {
	int token_num;              // Numerical token
	char symname[13];           // Symbol name
	char ident[13];             // Identifier string
	int number;                 // Number
};
// VM structs
struct instruction {
	int op;				// OPcode
	int l;				// L
	int m;				// M
};

// Driver global variables
int lex_flag = 0;
int assem_flag = 0;
int vmout_flag = 0;
int Derror_flag = 0;
int Perror_flag = 0;
int Serror_flag = 1;
char infile[20];
//Scanner global variables
int lexindex = 0;           // token index
int comm_flag = 0;          // flag for scanning comments
struct lextable** lexeme;
// Parser global variables
int cx = 0;
int errorNum = 0;
int tokindex = 0;
int symdex = 0;
int tok_max = 0;
struct tokenlist** token;
struct symtable** symbol;
struct codegen** code;
// VM global variables (none)

// Enums!!
enum SYMLIST {nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym, oddsym, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym, intsym, procsym, outsym, insym, elsesym, writesym, readsym, lcommentsym, rcommentsym};
enum OPR_M {OPR_RTN = 0, OPR_NEG, OPR_ADD, OPR_SUB, OPR_MUL, OPR_DIV, OPR_ODD, OPR_MOD, OPR_EQL, OPR_NEQ, OPR_LSS, OPR_LEQ, OPR_GTR, OPR_GEQ};
enum OPCODE {LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, WRT, RED};


// Driver function prototypes
void execute();
int Scanner(char scanin[]);
int Parser();
void VM();
// Scanner function prototypes
void insertLexeme(char string[]);
void separateTokens(char string[]);
int examineToken(char string[]);
int check_specsym(char* t);
int check_reserved(char* t);
int parseNumber(char string[]);
void printList(FILE* Scfout);
void printTable(FILE* Scfout);
// Parser function prototypes
void program();
void block(int L, int M);
void statement(int L, int M);
void condition();
void expression();
void term();
void factor();
int validateIdent(char* ident);
int getLevel(char* ident);
void emit(int op, int l, int m);
void insertSymbol(char* name, int kind, int L, int M, int val);
int checkRelOp();
void printTokens(FILE* Pfout);
void printSymbols(FILE* Pfout);
void printCode(FILE* fout);
void seedError();
void getSymname(int i);
void error(FILE* Pfout);
// VM function prototypes
int base(int l, int base,int* stack);
int read();

int main(int argc, char *argv[]){

	int i;

	printf("In\n");

	// argv[0] is just the name of the program
	if(argc > 2){
		if(!(strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "-v") == 0)){
			strcpy(infile, argv[1]);
			
			for(i = 2; i < argc; i++){
				if(strcmp(argv[i], "-l") == 0){
					if(lex_flag == 1){
						printf("You cannot use the same argument more than once!\n");
						Derror_flag = 1;
						break;
					}else{
						lex_flag = 1;
						//printf("lexemes will be printed\n");
						continue;
					}
				}else if(strcmp(argv[i], "-a") == 0){
					if(assem_flag == 1){
						printf("You cannot use the same argument more than once!\n");
						Derror_flag = 1;
						break;
					}else{
						assem_flag = 1;
						//printf("assembly code will be printed\n");
						continue;
					}
				}else if(strcmp(argv[i], "-v") == 0){
					if(vmout_flag == 1){
						printf("You cannot use the same argument more than once!\n");
						Derror_flag = 1;
						break;
					}else{
						vmout_flag = 1;
						//printf("vm output will be printed\n");
						continue;
					}
				}else{
					printf("Unknown argument detected!\n"
						"\nAcceptable addtional arguments: -l -a -v\n");
					Derror_flag = 0;
				}
			}
		}else{
			printf("The format for calling this program should be:\n"
			"\n./PL0driver someinputfilename addtl_args\n"
			"\nAcceptable addtional arguments: -l -a -v\n"
			"Please try again.\n");
			Derror_flag = 1;
		}
	}
	// Only arguments are the program name and the input file
	else if(argc == 2){
		if(!(strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "-v") == 0)){
			strcpy(infile, argv[1]);
		}else{
			printf("The format for calling this program should be:\n"
			"\n./PL0driver someinputfilename addtl_args\n"
			"\nAcceptable addtional arguments: -l -a -v\n"
			"Please try again.\n");
			Derror_flag = 1;
		}
	}
	// Improperly formatted program call
	else{
		printf("The format for calling this program should be:\n"
			"\n./PL0driver someinputfilename addtl_args\n"
			"\nAcceptable addtional arguments: -l -a -v\n"
			"Please try again.\n");
		Derror_flag = 1;
	}
	//printf("Input file: %s \n", infile);
	
	// If no argument errors, proceed running the rest of the driver
	if(!Derror_flag)
		execute();


	printf("\nOut\n");
	
	return 0;
}


void execute(){
	Serror_flag = Scanner(infile);
	
	// Scanner only fails if the input file cannot be found
	if(!Serror_flag){
		Perror_flag = Parser();

		//If the parser runs without issue, continue on to run the VM
		if(!Perror_flag)
			VM();
	}
}

// Scanner main
int Scanner(char scanin[]){
	FILE* Scfin;
    FILE* Scfout;
    FILE* Tout;
    int i;

    Scfin = fopen(scanin, "r");
    Scfout = fopen("output.txt", "w");
    Tout = tmpfile();

    if(Scfin == NULL){
        perror("Error opening input file\n");
        return 1;
    }
    else {
        char string[MAX_CHARS];

		lexeme = calloc(CODE_SIZE, sizeof(struct lextable*));

        //fprintf(Scfout, "Source Program:\n");
        if(ferror(Scfout)){
            printf("Error writing to file\n");
            return Serror_flag;
        }
        //printf("Source Program:\n");

		while(!feof(Scfin)){
			fscanf(Scfin, "%s ", string);
			//fprintf(Scfout, "%s ", string);
			//printf("%s ", string);
			fprintf(Tout, "%s ", string);
			separateTokens(string);
		}
		//fprintf(Scfout,"\n");
		//printf("\n");

		//printTable(Scfout);
		printList(Scfout);

		for(i = 0; i < CODE_SIZE; i++){
        	free(lexeme[i]);
		}
    	free(lexeme);

    }// eof else (!error)

	fclose(Scfin);
	fclose(Scfout);
	fclose(Tout);
	return 0;
}

// Parser main
int Parser(){
	FILE* Pfin;
	FILE* Pfout;
	int i;
	int len;

	Pfin = fopen("parserin.txt", "r");
	Pfout = fopen("output.txt", "a");

    if(Pfin == NULL){
		printf("Error reading file\n");
		system("PAUSE");
        return 0;
    }
    else{
		char symname[13];
        char strinput[MAX_CHARS];
        int dinput;
        int numinput;
        int num_elements = 0;

        // Initialize each element of the struct
        code = calloc(CODE_SIZE, sizeof(struct codegen*));
        token = calloc(CODE_SIZE, sizeof(struct tokenlist*));
        symbol = calloc(CODE_SIZE, sizeof(struct symtable*));
		for(i = 0; i < CODE_SIZE; i++){
            code[i] = malloc(sizeof(struct codegen));
            symbol[i] = malloc(sizeof(struct symtable));
		}

        //fprintf(Pfout, "A print out of the token file:\n");
        if(ferror(Pfout)){
            printf("Error writing to file\n");
        	return 0;
        }
        //printf("A print out of the token file:\n");

		// Read in token file, reprint
		while(!feof(Pfin)){
			token[tokindex] = malloc(sizeof(struct tokenlist));
			fscanf(Pfin,"%d ", &dinput);
			//fprintf(Pfout,"%d ", dinput);
			//printf("INPUT: %d ", dinput);
			token[tokindex]->token_num = dinput;
			if(dinput == identsym){
				fscanf(Pfin,"%s ", strinput);
				//fprintf(Pfout,"%s ", strinput);
				//printf("%s ", strinput);
				if(strlen(strinput) <= MAX_CHARS)
					strcpy(token[tokindex]->ident, strinput);
				else{
					//printf("\n\nSTRING: %s \n", strinput);
					errorNum = 26;
					break;
				}
			}else if(dinput == numbersym){
				fscanf(Pfin,"%d ", &numinput);
				//fprintf(Pfout,"%d ", numinput);
				//printf("%d ", numinput);
				if(numinput <= 99999)
					strcpy(token[tokindex]->ident, strinput);
				else{
					//printf("\n\nNUMBER: %d \n", numinput);
					errorNum = 25;
					break;
				}
				token[tokindex]->number = numinput;
			}
			tokindex++;
		}
		//printf("\n");
		//fprintf(Pfout, "\n");

		for(i = 0; i < tokindex; i++){
        	getSymname(i);
		}
		//printTokens(Pfout);

		if(errorNum == 0){
			// Run the parser
			program();
		}

		error(Pfout);
		//printf("%d\n", errorNum);
		if(errorNum == 0){
			// Print symbol table and generated code
			//printSymbols(Pfout);
			printCode(Pfout);
		}
		else{
			//printSymbols(Pfout);
			//seedError();
		}

		for(i = 0; i < CODE_SIZE; i++){
        	free(token[i]);
        	free(code[i]);
        	free(symbol[i]);
		}

    	free(token);
	    free(symbol);
	    free(code);
    } // eof else no error

    fclose(Pfin);
	fclose(Pfout);
	return errorNum;
}

// VM main
void VM(){
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

    FILE* Vfin;
    FILE* Vfout;

    Vfin = fopen("vminput.txt", "r");
    Vfout = fopen("output.txt", "a");

    // Fetch Cycle - Start
    //fprintf(Vfout, "\nLINE   OP      L     M\n");
    //printf("LINE   OP      L     M\n");

	PC = 0;
    while(!feof(Vfin))
    {
        //Copy contents of input file into MEM
        fscanf(Vfin, "%d  ", &OPnum);
        fscanf(Vfin, "%d  ", &L);
        fscanf(Vfin, "%d  ", &M);

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
			strcpy(OPstring, "SIO");
        if(OPnum == RED)          // SIO  READ
			strcpy(OPstring, "SIO");

        //fprintf(Vfout, "%3d    %s     %d     %-d\n", PC, OPstring, L, M);
		//printf("%3d    %s     %d    %2d\n", PC, OPstring, L, M);
        PC++;
    }
    fclose(Vfin);
    // Fetch Cycle - End

    stack[0] = 0;       // static link      SL
    stack[1] = 0;       // dynamic link     DL
    stack[2] = 0;       // return address   RA
    SP = 0;
    BP = 1;
    PC = 0;
    line = 0;
    int decoder = 0;

    fprintf(Vfout, "\n\nInitial Values                Post-Instruction Values\n"
       " PC    OP      L     M        PC   BP   SP    Stack\n");
    if(vmout_flag){
		printf("\n\nInitial Values              Post-Instruction Values\n"
       " PC     OP     L     M      PC   BP   SP    Stack\n");
	}

	do{
		// Fetch cycle - start
		IR = MEM[PC];
		PC++;
		decoder = IR.op;
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
        fprintf(Vfout, "%3d    %3s     %d     %-5d  ", PC-1, OPstring, IR.l, IR.m);
        if(vmout_flag)
			printf("%3d    %3s     %d     %-5d  ", PC-1, OPstring, IR.l, IR.m);

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
						printf("Improper OPR Mod code!\n");
						return;
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
			case WRT:     // SIO 0, 1	WRITE
				printf("\nTop stack element: %d\n			    ", stack[SP]);
				SP -= 1;
            	break;
			case RED:   // SIO 0, 2		READ
				SP += 1;
				printf("\n");
				stack[SP] = read();
				printf("			    ");
				break;
			default:
				printf("Improper Opcode!\n");
				return;
				break;
		}

		//Output of stack after instruction processing
        fprintf(Vfout, "  %-3d  %-d    %-2d    ", PC, BP, SP);
        if(vmout_flag)
			printf("%-3d  %-d    %-2d    ", PC, BP, SP);

		level_count = 1;

		for(i = 1; i <= SP; i++){
            // Insert "|"
            if(AR[i-1]){
                 fprintf(Vfout, "| ");
                 if(vmout_flag)
				 	printf("| ");
                 level_count++;
            }

			fprintf(Vfout, "%-2d ", stack[i]);
            if(vmout_flag)
				printf("%-2d ", stack[i]);

        }

        if(level_count > MAX_LEXI_LEVELS){
			printf("Lexicographical Level Overflow!\n");
			break;
        }

		fprintf(Vfout, "\n");
        if(vmout_flag)
			printf("\n");
	}while(BP > 0);



    fclose(Vfout);

    //printf("\n\nCongratulations! The input file has been successfully processed. Please refer\n"
    //"to the file 'stackout.txt', located in the same directory as this program.\n\n");
    
    return;
}

//////// Scanner Funtions ////////
// Insert string into the list
void insertLexeme(char string[]){
	int type;
	int number;
	type = examineToken(string);
    //printf("insert string: %s %d\n", string, type);
	lexeme[lexindex] = malloc(sizeof(struct lextable));

	if(comm_flag || type == rcommentsym){
		// Lexeme commented out, do nothing
	}else if(type == numbersym){
  		//printf("pre-number insertion\n");
		number = parseNumber(string);
		lexeme[lexindex]->number = number;
		lexeme[lexindex]->token_type = type;
		//printf("inserted: %d %d\n", lexeme[lexindex]->number, lexeme[lexindex]->token_type);
		lexindex++;
	}else if(type > 0){
		//printf("pre-other insertion\n");
		strcpy(lexeme[lexindex]->lexeme, string);
		//printf("string copied\n");
		lexeme[lexindex]->token_type = type;
		//printf("inserted: %s %d\n", lexeme[lexindex]->lexeme, lexeme[lexindex]->token_type);
		lexindex++;
	}else{
		// Unknown Error!!
		printf("Unknown Error!!\n");
	}
}

// Recursive method to separate each token based on punctuation
//(separation by spaces handled by scanning input)
void separateTokens(char string[]){
	char puncts[] = ".,;:=+-/*><()";
	//printf("%s \n", string);
	int s = strlen(string);
	char string1[s+1];
	char string2[s+1];
	char data[s+1];

	strcpy(data, string);
	int j;

	// Find the first character that is a punctuation marker
	j = strcspn(data, puncts);


	// If the string is empty, we do nothing
	if(j == 0 && s == 0){
		return;
	}
	// If the string has no punctuation, we just insert it
	else if(j == s){
		//printf("inserting: %s  j:%d s:%d\n", data, j, s);
		insertLexeme(data);
		return;
	}

	int left = s - j;
	// Break the string apart. The identifier before the punctuation stays in the current node
	// Everything in the original string starting with the first punctuation
	if(j == 0 && ispunct(data[0])){      // first char is punctuation
		//check for multi punct symbol
		if((data[0] == '>' && data[1] == '=') || ((data[0] == '<') && (data[1] == '>' || data[1] == '=')) || (data[0] == ':' && data[1] == '=')|| (data[0] == '/' && data[1] == '*') || (data[0] == '*' && data[1] == '/')){
			strncpy(string1, data, j+2);		// Copy the original token to a temp string
			memmove(data+0, data+j+2, left);	// Move the rest of the token to a new string
			strncpy(string2, data, left);
			string1[j+2] = '\0';           		// nullify everything after the token we want in the original

		}
		else{   // single punct symbol
			strncpy(string1, data, j+1);		// Copy the original token to a temp string
			memmove(data+0, data+j+1, left);	// Move the rest of the token to a new string
			strncpy(string2, data, left);
			string1[strlen(string1)-2] = '\0';	// nullify everything after the token we want in the original
		}
	}else{
		strncpy(string1, data, j);         	// Copy the original token to a temp string
		memmove(data+0, data+j, left);		// Move the rest of the token to a new string
		strncpy(string2, data, left);
		string1[j] = '\0';           		// Nullify everything after the token we want in the original
	}
	//if(strcmp(string1, " ") != 0)
		//printf("%s ", string1);
	//if(strcmp(string2, " ") != 0)
		//printf("%s\n", string2);
	//printf(" s:%d j:%d\n", s, j);
	insertLexeme(string1);
 	separateTokens(string2);
}

// Loop through the lexeme list, categorizing each entry
int examineToken(char string[]){
	int punct_num;
	int reserved;
	int spec;
	char puncts[] = ".,;:=+-/*><()";
	//printf("examining string: %s \n", string);

	spec = check_specsym(string);
	if(spec == lcommentsym){
		comm_flag = 1;
		return lcommentsym;
	}
	else if(spec == rcommentsym){
		comm_flag = 0;
		return rcommentsym;
	}

	if(!comm_flag){
		if(spec > 0){
			return spec;
		}
		else{
			reserved = check_reserved(string);

			if(reserved > 0)
				return reserved;
			else if(isalpha(string[0])){
				if(strlen(string) <= MAX_CHARS)
					return identsym;
				else
					return 0;
			}else if(isdigit(string[0])){
				if(strlen(string) <= MAX_DIGS)
					return numbersym;
				else
					return 0;
			}else
				return 0;
		}
	}else
		return 0;
}

// Checks for reserved special symbols. Returns the corresponding internal token value
int check_specsym(char* t){
	enum SYMLIST sym;

	if(t[0] == '+')
        return plussym;     	// plussym
    else if(t[0] == '-')
        return minussym;   		// minussym
    else if(t[0] == '*'){

		if(t[1] == '/')
			return rcommentsym;	// rcommentsym
		else
        	return multsym;		// multsym
	}
    else if(t[0] == '/'){
		if(t[1] == '*')
			return lcommentsym; // lcommentsym
		else
        	return slashsym;	// slashsym
	}else if(t[0] == '=')
        return eqlsym;       	// eqlsym
	else if(t[0] == '<'){
		if(t[1] == '>')
    		return neqsym;  	// neqsym
		if(t[1] == '=')
    		return leqsym;  	// leqsym
		else
    		return lessym;  	// lessym
	}else if(t[0] == '>'){
		if(t[1] =='=')
        	return geqsym;  	// geqsym
	    else
	        return gtrsym;  	// gtrsym
	}else if(t[0] == '(')
        return lparentsym;  	// lparentsym
    else if(t[0] == ')')
        return rparentsym;  	// rparentsym
    else if(t[0] == ',')
        return commasym;    	// commasym
    else if(t[0] == '.')
        return periodsym;		// periodsym
    else if(t[0] == ';')
        return semicolonsym;	// semicolonsym
	else if(t[0] == ':' && t[1] == '=')
		return becomessym;		// becomessym
    else
        return 0;
}

// Checks for reserved words. Returns the corresponding internal token value
int check_reserved(char* t){

	if(strcmp(t , "null") == 0)
        return nulsym;			// nulsym
	else if(strcmp(t , "odd") == 0)
        return oddsym;			// oddsym
	else if(strcmp(t , "begin") == 0)
        return	beginsym;		// beginsym
    else if(strcmp(t , "end") == 0)
        return endsym;    		// endsym
    else if(strcmp(t , "if") == 0)
        return ifsym;     		// ifsym
    else if(strcmp(t , "then") == 0)
        return thensym;     	// thensym
	else if(strcmp(t , "while") == 0)
        return whilesym;    	// whilesym
	else if(strcmp(t , "do") == 0)
        return dosym;       	// dosym
	else if(strcmp(t , "call") == 0)
        return callsym;     	// callsym
    else if(strcmp(t , "const") == 0)
        return constsym;    	// constsym
	else if(strcmp(t , "int") == 0)
        return intsym;      	// intsym
    else if(strcmp(t , "procedure") == 0)
        return procsym;     	// procsym
    else if(strcmp(t , "out") == 0)
        return outsym;			// outsym
    else if(strcmp(t , "in") == 0)
        return insym;  			// insym
    else if(strcmp(t , "else") == 0)
        return elsesym;     	// elsesym
	else if(strcmp(t , "write") == 0)
        return writesym;     	// writesym
	else if(strcmp(t , "read") == 0)
        return readsym;     	// readsym
    else
        return 0;
}

// Takes in a string of digits and converts it to a corresponding int
int parseNumber(char string[]){
	/**
	*   Take in a string, get length. For each position in string, get digit
	*   and add multiplied by correct base 10 to total
	*/
	int i;
	int digit;
	double number = 0;
	int len;
	len = strlen(string);

	for(i = len-1; i >= 0; i--){
		digit = string[i] - '0';
		//printf("string[i]: %d \n", string[i]);
		//printf("digit: %d \n", digit);
		number += digit*pow(10, (len-1)-i);
	}
	//printf("number: %lf", number);
	return (int)number;
}

// Prints the list of Lexemes
void printList(FILE* fout){
	int i;
	FILE* fparin;
	fparin = fopen("parserin.txt", "w");

	fprintf(fout, "Lexeme List:\n");
	if(lex_flag)
		printf("\nLexeme List:\n");

	for(i = 0; i < lexindex; i++){
		if(lexeme[i]->token_type == numbersym){
			if(lex_flag)
				printf("%d %d ", lexeme[i]->token_type, lexeme[i]->number);
			fprintf(fout, "%d %d ", lexeme[i]->token_type, lexeme[i]->number);
			fprintf(fparin, "%d %d ", lexeme[i]->token_type, lexeme[i]->number);
		}else if(lexeme[i]->token_type == identsym){
			if(lex_flag)
				printf("%d %s ", lexeme[i]->token_type, lexeme[i]->lexeme);
			fprintf(fout, "%d %s ", lexeme[i]->token_type, lexeme[i]->lexeme);
			fprintf(fparin, "%d %s ", lexeme[i]->token_type, lexeme[i]->lexeme);
		}else{
			if(lex_flag)
				printf("%d ", lexeme[i]->token_type);
			fprintf(fout, "%d ", lexeme[i]->token_type);
			fprintf(fparin, "%d ", lexeme[i]->token_type);
		}
	}
	if(lex_flag)
		printf("\n");
	//fprintf(fout, "\n");

	fclose(fparin);
}

// Prints the Lexeme Table
void printTable(FILE* fout){
	int i;

	fprintf(fout, "\n\nLexeme Table:\n");
	fprintf(fout, "lexeme       token type\n");
	printf("\n\nLexeme Table:\n");
	printf("lexeme       token type\n");

	for(i = 0; i < lexindex; i++){
		if(lexeme[i]->token_type == numbersym){
			printf("%-11d %-d\n", lexeme[i]->number, lexeme[i]->token_type);
			fprintf(fout, "%-11d %-d\n", lexeme[i]->number, lexeme[i]->token_type);
		}else{
			printf("%-11s %-d\n", lexeme[i]->lexeme, lexeme[i]->token_type);
			fprintf(fout, "%-11s %-d\n", lexeme[i]->lexeme, lexeme[i]->token_type);
		}
	}

	//fprintf(fout, "\n");
	//printf("\n");
}
//////// eof Scanner Functions ////////

//////// Parser Functions ////////
// block "."
void program(){
	int L = 0;  // first L level is zero
	int M = 3; 	// reserve space for the SL, DL, RA
	tok_max = tokindex;
	tokindex = 0;
	emit(INC, L, M);  // Increment - INC 0, M

	//printf("%s \n", token[tokindex]->symname);

	block(L, M);
	//printf("prgm post-block: %s \n", token[tokindex]->symname);
	if(errorNum == 0){
		tokindex++;
		//printf("prgm period: %s \n", token[tokindex]->symname);
		if(token[tokindex]->token_num != periodsym){
			errorNum = 9;
			return;
		}
		else
			emit(OPR, 0, OPR_RTN);    // Return - OPR 0, 0
	}
}

// block ::= const-declaration var-declaration procedure-declaration statement
void block(int L, int M){
	char name[MAX_CHARS];
	int kind = 0;
	int value;
	if(errorNum == 0){
		// constdeclaration ::= ["const" ident "=" number {"," ident "=" number} ";"].
		if(token[tokindex]->token_num == constsym){  // add constant
			kind = 1;

			do{
				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != identsym){     // needs INC after first call
					errorNum = 4;
					return;
				}else{
					strcpy(name, token[tokindex]->ident);
				}

				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != eqlsym){
					errorNum = 3;
					return;
				}

				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != numbersym){
					errorNum = 2;
					return;
				}else if(token[tokindex]->number >= 100000){
					errorNum = 25;
					return;
				}else
					value = token[tokindex]->number;

				insertSymbol(name, kind, L, M, value);
				emit(INC, L, 1);    	// Increment 1 space for each constant added
				emit(LIT, L, value);
				emit(STO, L, M);
				M++;
	   			tokindex++;
	   			//printf("%s \n", token[tokindex]->symname);
			} while(token[tokindex]->token_num == commasym);

			if(token[tokindex]->token_num != semicolonsym){
				errorNum = 10;
				return;
			}
			tokindex++;
			//printf("%s \n", token[tokindex]->symname);
		}
		// var-declaration ::= [ "int "ident {"," ident} �;"]
		if(token[tokindex]->token_num == intsym){   // add variable
			kind = 2;
			do{
				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != identsym){
					errorNum = 4;
					return;
				}
				else{
					strcpy(name, token[tokindex]->ident);
				}

				insertSymbol(name, kind, L, M, 0);
				emit(INC, L, 1);    // INC 1 space for each variable added
				M++;
				tokindex++;
				//printf("%s \n", token[tokindex]->symname);
			}while(token[tokindex]->token_num == commasym);

			if(token[tokindex]->token_num != semicolonsym){
				errorNum = 5;
				return;
			}
			tokindex++;
			//printf("%s \n", token[tokindex]->symname);
		}
		/*
		// procedure-declaration ::= { "procedure" ident ";" block ";" }
		while(token[tokindex]->token_num == procsym){  // add procedure
			kind = 3;

			tokindex++;
			//printf("%s \n", token[tokindex]->symname);
			if(token[tokindex]->token_num != identsym){
					errorNum = 4;
					return;
			}
			else{
				strcpy(name, token[tokindex]->ident);
			}
			if(token[tokindex]->token_num != semicolonsym){
				errorNum = 10;
				return;
			}

			insertSymbol(name, kind, L, M);

			M++;
			token[tokindex]->token_num
			block(L++, M);

			if(token[tokindex]->token_num != semicolonsym)
				errorNum = 10;
			L--;
			tokindex++;

			if(cx > CODE_SIZE)
				errorNum = 25;
			else{
				emit(2, 0, 0);   // Return - OPR 0,0
			}
			tokindex++;
			printf("%s \n", token[tokindex]->symname);
		}*/
		//printf("pre-statement: %s \n", token[tokindex]->symname);
		statement(L, M);   // final begin <statement> end
	}
}

//statement ::= [ ident ":=" expression || "begin" statement { ";" statement } "end" ||
// "if" condition "then" statement ["else" statement] || "while" condition "do" statement ||
// "read" ident || "write" expression || null ]
void statement(int L, int M){
	char name[MAX_CHARS];

	if(errorNum == 0){
		//printf("stmnt NEW_STATEMENT: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == identsym){ // ident ":=" expression
			//printf("stmnt ident: %s \n", token[tokindex]->symname);
			int sto_addr = validateIdent(token[tokindex]->ident);
			int sto_level = getLevel(token[tokindex]->ident);

			if(sto_addr > 0){

				tokindex++;
		  		//printf("stmnt post-ident: %s \n", token[tokindex]->symname);
				if(token[tokindex]->token_num != becomessym){   // :=
					errorNum = 13;
					return;
				}
				tokindex++;
				//printf("stmnt ident pre-epxression: %s \n", token[tokindex]->symname);
				expression();
				//printf("stmnt ident post-epxression: %s \n", token[tokindex]->symname);

				if(errorNum == 0){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else{
						emit(STO, sto_level, sto_addr);    // Store - STO L, M
					}
				}
			}
			else{
				errorNum = 11;
				return;
			}
		}
		/*
		else if(token[tokindex]->token_num == callsym){ // "call" ident
			current = current->next;
			if(token[tokindex]->token_num != identsym){
				errorNum = 4;
				return;
			}
			tokindex++;

			if(cx > CODE_SIZE)
				errorNum = 25;
			else{
				emit(CAL, L, M);  // Call - CAL L,M
			}
		}*/
		// "begin" statement {";" statement} "end"
		else if(token[tokindex]->token_num == beginsym){
	 		//int cx1;
			tokindex++;
			//printf("stmnt post-begin: %s %d\n", token[tokindex]->symname, tokindex);
			statement(L, M);
			//printf("stmnt bgn post-1st stmnt: %s %d\n", token[tokindex]->symname, tokindex);
			//tokindex++;
			while(token[tokindex]->token_num == semicolonsym){
				tokindex++;
				//printf("stmnt bgn pre-stmnt loop: %s %d\n", token[tokindex]->symname, tokindex);
				statement(L, M);
				//printf("stmnt bgn post-stmnt loop: %s %d\n", token[tokindex]->symname, tokindex);
				//tokindex++;
				if(errorNum != 0)
					break;
			}
			if(errorNum == 0){
				//printf("stmnt pre no-end: %s %d, %s \n", token[tokindex]->symname, tokindex, token[tokindex+1]->symname);
				if(token[tokindex]->token_num != endsym){
					errorNum = 27;
					return;
				}
			}
		}
		// "if" condition "then" statement ["else" statement]
		else if(token[tokindex]->token_num == ifsym){
			tokindex++;
			//printf("stmnt IF pre-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			condition();
			//printf("stmnt IF post-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				if(token[tokindex]->token_num != thensym){
					errorNum = 16;
					return;
				}

				tokindex++;
				int ctemp1 = cx;

				if(cx > CODE_SIZE){
					errorNum = 25;
					return;
				}
				else
					emit(JPC, 0, 0);      // JPC 0, 0
				//printf("stmnt THEN pre-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				statement(L, M);
				//printf("stmnt THEN post-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				if(errorNum == 0){

					if(token[tokindex]->token_num == elsesym){
						int ctemp2 = cx;
						emit(JMP, 0 , 0);

						code[ctemp1]->m = cx;				// changes JPC 0 0 to JPC 0 cx
						tokindex++;
					
						//printf("stmnt ELSE pre-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
						statement(L, M);
						//printf("stmnt ELSE post-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
						code[ctemp2]->m = cx;			// changes JPC 0 0 to JPC 0 cx
					}
					else{
						code[ctemp1]->m = cx;
					}
				}
			}
		}
		// "while" condition "do" statement
		else if(token[tokindex]->token_num == whilesym){
			int cx1 = cx;
			tokindex++;
			//printf("stmnt WHILE pre-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			condition();
			//printf("stmnt WHILE post-cndtn: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				int cx2 = cx;
				if(cx > CODE_SIZE){
					errorNum = 25;
					return;
				}else
					emit(JPC, 0, 0);      // JPC 0, 0

				if(token[tokindex]->token_num != dosym){
					errorNum = 18;
					return;
				}

				tokindex++;
				//printf("stmnt DO pre-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				statement(L, M);
				tokindex++;
				//printf("stmnt DO post-stmnt: %s %d\n", token[tokindex]->symname, tokindex);
				if(errorNum == 0){
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else{
						emit(JMP, 0, cx1);    // JMP 0, M
						code[cx2]->m = cx;
					}
				}
			}
		}
		// "read"
		else if(token[tokindex]->token_num == readsym){
			//printf("stmnt READ pre-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
			tokindex++;
			emit(RED, 0, 2);        	// RED 0, 2
			//printf("stmnt READ post-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
		}
		// "write"
		else if(token[tokindex]->token_num == writesym){
			tokindex++;
			//printf("stmnt WRITE pre-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
			//expression();
   			emit(WRT, 0, 1);            // WRT 0, 1
   			//printf("stmnt WRITE post-EMIT: %s %d\n", token[tokindex]->symname, tokindex);
		}
		// "in" ident
		else if(token[tokindex]->token_num == insym){
			tokindex++;

			if(token[tokindex]->token_num != identsym){
				errorNum = 4;
				return;
			}else{
				int sto_addr = validateIdent(token[tokindex]->ident);
				int sto_level = getLevel(token[tokindex]->ident);

				if(errorNum == 0){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else{
						emit(RED, 0, 2);            		// RED 0, 2
						emit(STO, sto_level, sto_addr);		// STO L, M
						tokindex++;
					}
				}
			}
		}
		// "out" ident
		else if(token[tokindex]->token_num == outsym){
			tokindex++;

			if(token[tokindex]->token_num != identsym){
				errorNum = 4;
				return;
			}else{
				int lod_addr = validateIdent(token[tokindex]->ident);
				int lod_level = getLevel(token[tokindex]->ident);

				if(!lod_addr){
					errorNum = 11;
					//printf("Error: %d\n", errorNum);
					return;
				}
				else{
					emit(LOD, lod_level, lod_addr);     // LOD L, M
					emit(WRT, 0, 1);            		// WRT 0, 1
					tokindex++;
				}
			}
		}
	}
}

// "odd" expression | expression rel-op expression
void condition(){
	int relation = 0;
	if(errorNum ==0){
		//printf("cndtn NEW CONDITION: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == oddsym){
			int oddop = token[tokindex]->token_num;
			tokindex++;
			//printf("cndtn ODD pre-expr: %s %d\n", token[tokindex]->symname, tokindex);
			expression();
			//printf("cndtn ODD post-expr: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				if(cx > CODE_SIZE){
					errorNum = 25;
					return;
				}
				else
					emit(OPR, 0, OPR_ODD);		// ODD - OPR 0, 6
			}
		}
		else {
			//printf("cndtn RELOP pre-1st expr: %s %d\n", token[tokindex]->symname, tokindex);
			expression();
			//printf("cndtn RELOP post-1st expr: %s %d\n", token[tokindex]->symname, tokindex);
			if(errorNum == 0){
				relation = checkRelOp(token[tokindex]->token_num);
				tokindex++;
				//printf("cndtn RELOP pre-2nd expr: %s %d\n", token[tokindex]->symname, tokindex);
				expression();
				//printf("cndtn RELOP post-2nd expr: %s %d\n", token[tokindex]->symname, tokindex);

				if(relation > 0){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else
						emit(OPR, 0, relation);   //Creates code for either EQL, NEQ, LSS, LEQ, GTR, or GEQ
				}else{
					errorNum = 20;
					return;
				}
			}
		}
	}
}

//[ "+"|"-"] term {("+"|"-") term}
void expression(){
	int addop;

	if(errorNum == 0){
		//printf("expr NEW_EXPRESSION: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == plussym || token[tokindex]->token_num == minussym){
			addop = token[tokindex]->token_num;
			tokindex++;
			term();
			if(errorNum == 0){
				if(addop == minussym){
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0 , OPR_NEG); // negate
				}
			}
		}
		//printf("expr pre-term: %s \n", token[tokindex]->symname);
		term();
		//tokindex++;
		//printf("expr pre-(+,-): %s %d\n", token[tokindex]->symname, tokindex);
		while(token[tokindex]->token_num == plussym || token[tokindex]->token_num == minussym){
			addop = token[tokindex]->token_num;
			tokindex++;
			//printf("expr pre-2nd term: %s %d\n", token[tokindex]->symname, tokindex);
			term();
			//printf("expr post-2nd term: %s %d\n", token[tokindex]->symname, tokindex);

			//if(token[tokindex]->token_num == rparentsym)
			//	tokindex++;

			if(errorNum == 0){
				if(addop == plussym){
					if(cx > CODE_SIZE)
						errorNum = 25;
					else{
						emit(OPR, 0, OPR_ADD); // addition
					}
				}else{
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0, OPR_SUB); // subtraction
				}
			}else
				break;
		}
	}
}

// factor {("*")|("/") factor}
void term(){
	int mulop;

	if(errorNum == 0){
		//printf("term NEW_TERM: %s %d\n", token[tokindex]->symname, tokindex);
		factor();
		tokindex++;
		//printf("term pre-(*,/): %s %d\n", token[tokindex]->symname, tokindex);
		while(token[tokindex]->token_num == multsym || token[tokindex]->token_num == slashsym){
			mulop = token[tokindex]->token_num;
			tokindex++;
			//printf("term pre-2nd fctr: %s %d\n", token[tokindex]->symname, tokindex);
			factor();
			//printf("term post-2nd fctr: %s %d\n", token[tokindex]->symname, tokindex);
			tokindex++;
			if(errorNum == 0){
				if(mulop == multsym){
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0, OPR_MUL); // multiplication
				}else{
					if(cx > CODE_SIZE){
						errorNum = 25;
						return;
					}
					else
						emit(OPR, 0, OPR_DIV); // division
				}
			}else
				break;
		}
	}
}

// ident | number | "(" expression ")"
void factor(){
	int value;
	//char* ident;
	if(errorNum == 0){
		//printf("fctr NEW_FACTOR: %s %d\n", token[tokindex]->symname, tokindex);
		if(token[tokindex]->token_num == identsym){      	// ident
			//printf("fctr ident: %s \n", token[tokindex]->symname);
			int lod_addr;
			int lod_level;
			lod_addr = validateIdent(token[tokindex]->ident);
			lod_level = getLevel(token[tokindex]->ident);

			if(!lod_addr){
				errorNum = 11;
				//printf("Error: %d\n", errorNum);
				return;
			}
			else
				emit(LOD, lod_level, lod_addr);

			//tokindex++;
		}
		else if(token[tokindex]->token_num == numbersym){   // number
			//printf("fctr nmbr: %s %d\n", token[tokindex]->symname, tokindex);
			value = token[tokindex]->number;
			if(cx > CODE_SIZE){
				errorNum = 25;
				return;
			}
			else
				emit(LIT, 0, value);     // LIT 0, M
			//tokindex++;
		}
		else if(token[tokindex]->token_num == lparentsym){  // "("
			tokindex++;
			//printf("fctr post-lparen: %s \n", token[tokindex]->symname);
			expression(); // expression
			//tokindex++;
			//printf("fctr pre-rparen: %s \n", token[tokindex]->symname);
			if(errorNum == 0){
				if(token[tokindex]->token_num != rparentsym){    // ")"
					errorNum = 22;
					return;
				}
				//tokindex++;
			}
		}
		else{
			errorNum = 19;
			return;
		}
	}
}

int validateIdent(char* ident){
	int i;
	int M;
	for(i = 0; i < symdex; i++){
		if(strcmp(ident, symbol[i]->name) == 0){
			/*if(symbol[i]->kind == 1){
				M = symbol[i]->val;
				//printf("symmatch: %s Value:%d\n", symbol[i]->name, symbol[i]->val);
			}
			else{
				M = symbol[i]->addr;
				//printf("symmatch: %s L:%d  M:%d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			}*/
			M = symbol[i]->addr;
			//printf("M:%d\n", M);
			return M;
		}
	}
	return 0;
}

// Returns the L level of the identifier in question
int getLevel(char* ident){
	int i;
	int L;
	for(i = 0; i < symdex; i++){
		if(strcmp(ident, symbol[i]->name) == 0){
			//printf("symmatch: %s L:%d  M:%d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			L = symbol[i]->level;
			//printf("L:%d\n", L);
			return L;
		}
	}
	return 0;
}

// Generate assembly code for VM
void emit(int op, int l, int m){
		//printf("cx: %d \n", cx);
		code[cx]->op = op;
		code[cx]->l = l;
		code[cx]->m = m;
		cx++;
}

void insertSymbol(char* name, int kind, int L, int M, int val){

 	switch(kind){
		case 1:		// const (kind, name, value)
			symbol[symdex]->kind = kind;
			strcpy(symbol[symdex]->name, name);
			symbol[symdex]->level = L;
			symbol[symdex]->addr = M;
			symbol[symdex]->val = val;
			break;
		case 2:     // var  (kind, name, L, M)
			symbol[symdex]->kind = kind;
			strcpy(symbol[symdex]->name, name);
			symbol[symdex]->level = L;
			symbol[symdex]->addr = M;
			break;
		case 3:     // proc (kind, name, L, M)
			symbol[symdex]->kind = kind;
			strcpy(symbol[symdex]->name, name);
			symbol[symdex]->level = L;
			symbol[symdex]->addr = M;
			break;
	}
	symdex++;
}

int checkRelOp(int relsym){

	switch(relsym){
		case eqlsym: // EQL
   			return OPR_EQL;
			break;
		case neqsym: // NEQ
   			return OPR_NEQ;
			break;
		case lessym: // LSS
   			return OPR_LSS;
			break;
		case leqsym: // LEQ
   			return OPR_LEQ;
			break;
		case gtrsym: // GTR
   			return OPR_GTR;
			break;
		case geqsym: // GEQ
   			return OPR_GEQ;
			break;
		default: // not a relation symbol
			return 0;
			break;
	}
}

void printTokens(FILE* fout){
	int i;

	printf("\nAnd its symbolic representation:\n");
	fprintf(fout, "\nAnd its symbolic representation:\n");
	for(i = 0; i < tokindex; i++){
		printf("%s %d ", token[i]->symname , i);
		fprintf(fout, "%s ", token[i]->symname);
	}
	printf("\n\n");
	fprintf(fout, "\n");
}

void printSymbols(FILE* fout){
	int i;

	printf("\nSymbol Table:\n"
		"KIND  NAME          L   M\n");
	fprintf(fout, "\nSymbol Table:\n"
		"KIND  NAME          L   M\n");
	for(i = 0; i < symdex; i++){
		switch(symbol[i]->kind){
		case 1:		// const (kind, name, value)
			fprintf(fout, "const %-11s   %-d\n", symbol[i]->name, symbol[i]->val);
			printf("const %-11s   %-d\n", symbol[i]->name, symbol[i]->val);
			break;
		case 2:     // var  (kind, name, L, M)
			fprintf(fout, "var   %-11s   %-2d  %-2d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			printf("var   %-11s   %-2d  %-2d\n", symbol[i]->name , symbol[i]->level, symbol[i]->addr);
			break;
		case 3:     // proc (kind name, L, M)
			fprintf(fout, "proc    %-11s   %-2d  %-2d\n", symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			printf("proc  %-11s   %-2d  %-2d\n",  symbol[i]->name, symbol[i]->level, symbol[i]->addr);
			break;
		}
	}
	//printf("\n");
	//fprintf(fout, "\n");
}

void printCode(FILE* fout){
	FILE* vmout;
	vmout = fopen("vminput.txt", "w");
	int i;

	// Seeds a '0' in front of generated code into VM input file
	// Notifies compiler driver of successful parsing
	//fprintf(vmout,"%d\n", errorNum);
	if(lex_flag)
		printf("\n\nGenerated Assembly Code:\n");
	fprintf(fout, "\n\nGenerated Assembly Code:\n");
	for(i = 0; i < cx; i++){
		fprintf(fout,"%-2d %-2d %d\n", code[i]->op, code[i]->l, code[i]->m);
		fprintf(vmout,"%d %d %d\n", code[i]->op, code[i]->l, code[i]->m);
		if(assem_flag)
			printf("%-2d %-2d %d\n", code[i]->op, code[i]->l, code[i]->m);

	}
	//printf("\n");
	//fprintf(fout, "\n");

	fclose(vmout);
}

// If there is an error, inserts error number into VM input file
// This notifies the compiler driver and thus the user, of a parser error
void seedError(){
	FILE* vmout;
	vmout = fopen("vminput.txt", "w");

	fprintf(vmout,"%d\n", errorNum);
	fclose(vmout);
}

void getSymname(int i){

	switch(token[i]->token_num){
		case 1:	// nulsym
			strcpy(token[i]->symname, "nulsym");
			break;
		case 2: // identsym
			strcpy(token[i]->symname, "identsym");
			break;
		case 3: // numbersym
			strcpy(token[i]->symname, "numbersym");
			break;
		case 4: // plussym
			strcpy(token[i]->symname, "plussym");
			break;
		case 5: // minussym
			strcpy(token[i]->symname, "minussym");
			break;
		case 6: // multsym
			strcpy(token[i]->symname, "multsym");
			break;
		case 7: // slashsym
			strcpy(token[i]->symname, "slashsym");
			break;
		case 8: // oddsym
			strcpy(token[i]->symname, "oddsym");
			break;
		case 9: // eqlsym
			strcpy(token[i]->symname, "eqlsym");
			break;
		case 10: // neqsym
			strcpy(token[i]->symname, "neqsym");
			break;
		case 11: // lessym
			strcpy(token[i]->symname, "lessym");
			break;
		case 12: // leqsym
			strcpy(token[i]->symname, "leqsym");
			break;
		case 13: // gtrsym
			strcpy(token[i]->symname, "gtrsym");
			break;
		case 14: // geqsym
			strcpy(token[i]->symname, "geqsym");
			break;
		case 15: // lparentsym
			strcpy(token[i]->symname, "lparentsym");
			break;
		case 16: // rparentsym
			strcpy(token[i]->symname, "rparentsym");
			break;
		case 17: // commasym
			strcpy(token[i]->symname, "commasym");
			break;
		case 18: // semicolonsym
			strcpy(token[i]->symname, "semicolonsym");
			break;
		case 19: // periodsym
			strcpy(token[i]->symname, "periodsym");
			break;
		case 20: // becomessym
			strcpy(token[i]->symname, "becomessym");
			break;
		case 21: // beginsym
			strcpy(token[i]->symname, "beginsym");
			break;
		case 22: // endsym
			strcpy(token[i]->symname, "endsym");
			break;
		case 23: // ifsym
			strcpy(token[i]->symname, "ifsym");
			break;
		case 24: // thensym
			strcpy(token[i]->symname, "thensym");
			break;
		case 25: // whilesym
			strcpy(token[i]->symname, "whilesym");
			break;
		case 26: // dosym
			strcpy(token[i]->symname, "dosym");
			break;
		case 27: // callsym
			strcpy(token[i]->symname, "callsym");
			break;
		case 28: // contsym
			strcpy(token[i]->symname, "constsym");
			break;
		case 29: // intsym
			strcpy(token[i]->symname, "intsym");
			break;
		case 30: // procsym
			strcpy(token[i]->symname, "procsym");
			break;
		case 31: // outsym
			strcpy(token[i]->symname, "outsym");
			break;
		case 32: // insym
			strcpy(token[i]->symname, "insym");
			break;
		case 33: // elsesym
			strcpy(token[i]->symname, "elsesym");
			break;
		case 34: // writesym
			strcpy(token[i]->symname, "writesym");
			break;
		case 35: // readsym
			strcpy(token[i]->symname, "readsym");
			break;
		case 36: // lcommentsym
			strcpy(token[i]->symname, "lcommentsym");
			break;
		case 37: // rcommentsym
			strcpy(token[i]->symname, "rcommentsym");
			break;
		default:
			strcpy(token[i]->symname, "UNKNOWN");
			break;
	}
}

void error(FILE* fout){

	if(errorNum){
		printf("\nErrors Found:\n");
		fprintf(fout, "\nErrors Found:\n");
	}

	switch(errorNum){
		case 0:
			//fputs("No errors, program is syntactically correct\n", fout);
			//puts("No errors, program is syntactically correct\n");
			break;
		case 1:
			fputs("Error number 1: used = instead of :=\n", fout);
			puts("Error number 1: used = instead of :=\n");
			break;
		case 2:
			fputs("Error number 2: = must be followed by a number\n", fout);
			puts("Error number 2: = must be followed by a number\n");
			break;
		case 3:
			fputs("Error number 3: identifier expected", fout);
			puts("Error number 3: identifier expected");
			break;
		case 4:
			fputs("Error number 4: 'const', 'int', 'procedure' must be followed by an identifier\n", fout);
			puts("Error number 4: 'const', 'int', 'procedure' must be followed by an identifier\n");
			break;
		case 5:
   			fputs("Error number 5, semicolon or comma missing\n", fout);
			puts("Error number 5, semicolon or comma missing\n");
			break;
		case 6:
			fputs("Error number 6, incorrect symbol after procedure declaration\n", fout);
			puts("Error number 6, incorrect symbol after procedure declaration\n");
			break;
		case 7:
			fputs("Error number 7, statement expected\n", fout);
			puts("Error number 7, statement expected\n");
			break;
		case 8:
			fputs("Error number 8, incorrect symbol after statement part in block\n", fout);
			puts("Error number 8, incorrect symbol after statement part in block\n");
			break;
		case 9:
			fputs("Error number 9, period expected\n", fout);
			puts("Error number 9, period expected\n");
			break;
		case 10:
			fputs("Error number 10,  semicolon between statements missing\n", fout);
			puts("Error number 10,  semicolon between statements missing\n");
			break;
		case 11:
			fputs("Error number 11, undeclared identifier\n", fout);
			puts("Error number 11, undeclared identifier\n");
			break;
		case 12:
			fputs("Error number 12, assignment to constant or procedure is not allowed\n", fout);
			puts("Error number 12, assignment to constant or procedure is not allowed\n");
			break;
		case 13:
			fputs("Error number 13, assignment operator expected\n", fout);
			puts("Error number 13, assignment operator expected\n");
			break;
		case 14:
			fputs("Error number 14, 'call' must be followed by an identifier\n", fout);
			puts("Error number 14, 'call' must be followed by an identifier\n");
			break;
		case 15:
			fputs("Error number 15, call of a constant or variable is meaningless\n", fout);
			puts("Error number 15, call of a constant or variable is meaningless\n");
			break;
		case 16:
			fputs("Error number 16, 'then' or 'else' expected\n", fout);
			puts("Error number 16, 'then' or 'else' expected\n");
			break;
		case 17:
			fputs("Error number 17, semicolon or right bracket expected\n", fout);
			puts("Error number 17, semicolon or right braket expected\n");
			break;
		case 18:
			fputs("Error number 18, 'do' expected\n", fout);
			puts("Error number 18, 'do' expected\n");
			break;
		case 19:
			fputs("Error number 19, incorrect symbol following statement\n", fout);
			puts("Error number 19, incorrect symbol following statement\n");
			break;
		case 20:
			fputs("Error number 20, relational operator expected\n", fout);
			puts("Error number 20, relational operator expected\n");
			break;
		case 21:
			fputs("Error number 21, expression must not contain a procedure identifier\n", fout);
			puts("Error number 21, expression must not contain a procedure identifier\n");
			break;
		case 22:
			fputs("Error number 22, right parenthesis missing\n", fout);
			puts("Error number 22, right parenthesis missing\n");
			break;
		case 23:
			fputs("Error number 23, the preceding factor cannot begin with this symbol\n", fout);
			puts("Error number 23, the preceding factor cannot begin with this symbol\n");
			break;
		case 24:
			fputs("Error number 24, an expression cannot begin with ths symbol\n", fout);
			puts("Error number 24, an expression cannot begin with ths symbol\n");
			break;
		case 25:
			fputs("Error number 25, this number is too large\n", fout);
			puts("Error number 25, this number is too large\n");
			break;
		case 26:
			fputs("Error number 26, Identifier name is too long\n", fout);
			puts("Error number 26, Identifier name is too long\n");
			break;
		case 27:
			fputs("Error number 27, 'end' expected\n", fout);
			puts("Error number 27, 'end' expected\n");
			break;
		default:
			fputs("Unknown error occurred\n", fout);
			puts("Unknown error occurred\n");
			break;
	}
}
//////// eof Parser Functions ////////

//////// VM Functions ////////
// Searches for the BP L levels down the stack
int base(int l,int base,int* stack){		// l stand for L in the instruction format
	int b1;									// find base L levels down
	b1 = base;
	while(l > 0){
		b1 = stack[b1];
		l--;
	}
	return b1;
}

// Reads in a value from the user
int read(){
	int input;

	printf("Please enter a value to add to the stack:\n");
	scanf("%d", &input);
	if(input > 99999){
		printf("\n%d is too large!\n", input);
		return read();
	}else
		return input;
}
//////// eof VM functions ////////
