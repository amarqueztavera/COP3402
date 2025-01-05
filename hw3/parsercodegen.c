// Sophia Gnisci
// Andrea Marquez Tavera
// COP3402 Fall 2023
// HW3 (Tiny PL/0 Compiler)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MACROS
#define MAX_LEN 512
#define MAX_ID 11
#define MAX_NUM 5
#define MAX_SYMBOL_TABLE_SIZE 500

// Instruction Register
typedef struct InstructionRegister {
  int OP;
  int L;
  int M;
} InstructionRegister;

// token types
typedef enum {
  oddsym = 1,
  identsym = 2,
  numbersym = 3,
  plussym = 4,
  minussym = 5,
  multsym = 6,
  slashsym = 7,
  ifelsym = 8,
  eqlsym = 9,
  neqsym = 10,
  lessym = 11,
  leqsym = 12,
  gtrsym = 13,
  geqsym = 14,
  lparentsym = 15,
  rparentsym = 16,
  commasym = 17,
  semicolonsym = 18,
  periodsym = 19,
  becomessym = 20,
  beginsym = 21,
  endsym = 22,
  ifsym = 23,
  thensym = 24,
  whilesym = 25,
  dosym = 26,
  // callsym = 27,
  constsym = 28,
  varsym = 29,
  // procsym = 30,
  writesym = 31,
  readsym = 32,
  // elsesym = 33
} token_type;
int currentToken;
int tokenList[MAX_LEN];            // array to keep track of token list
char identifiers[MAX_LEN][MAX_ID]; // array to keep track of identifiers

// symbol table
typedef struct {
  int kind;      // const = 1, var = 2, proc = 3
  char name[10]; // name up to 11 chars
  int val;       // number (ASCII value)
  int level;     // L level
  int addr;      // M address
  int mark;      // to indicate unavailable or deleted
} symbol;
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
int tp; // symbol table pointer
int pc;

InstructionRegister text[MAX_LEN]; // array of text (code)
int cx;                            // code index

void program();
void block();
void constDeclaration();
int varDeclaration();
void statement();
void condition();
void expression();
void term();
void factor();
void printSymbolTable();
int symbolTableCheck(char *name);
void emit(int op, int L, int M);
void markChange();
char *translateOP(int op);
void printCode();

int main(int argc, char *argv[]) {
  FILE *filePointer;
  char *filename;

  // check if enough arguments were passed to the terminal
  if (argc < 2) {
    printf("Filename missing!\n");
    exit(1);

    // get file name
  } else
    filename = argv[1];

  // read file
  filePointer = fopen(filename, "r");

  // check if file opened properly
  if (!filePointer) {
    printf("Failed to open file\n");
    exit(1);
  }

  // initialize source program char array
  char sourceProgram[MAX_LEN];
  for (int i = 0; i < MAX_LEN; i++) {
    sourceProgram[i] = ' ';
  }

  // printf("Source Program:\n");
  int charCount = 0; // keeps track of index to fill up sourceProgram

  while (1) {
    char temp1 = fgetc(filePointer);

    // stop at the end of the file
    if (feof(filePointer))
      break;

    // print source program
    // else
    // printf("%c", temp1);

    // if not invisble character, add to array
    sourceProgram[charCount] = temp1;
    charCount++;
  }

  sourceProgram[charCount] = '\0';

  // printf("\n\nLexeme Table:");
  // printf("\n\n%-30stoken type\n", "lexeme");

  int idIdx = 0;    // identifiers index
  int wordLen = 0;  // used to make sure word length is valid
  int digitLen = 0; // used to make sure digit length is valid
  int tokenIdx = 0;
  int comment = 0; // flag to check if currently in a comment
  int wordFlag = 0;

  // tokenize program
  for (int i = 0; i < charCount; i++) {

    // check if letter
    if (comment == 0 && (isalpha(sourceProgram[i]) || wordFlag == 1)) {
      digitLen = 0;
      wordFlag = 1;
      wordLen++;

      // check if we are at the end of a word
      if (sourceProgram[i + 1] == '\0' ||
          (!isdigit(sourceProgram[i + 1]) && !isalpha(sourceProgram[i + 1]))) {
        wordFlag = 0;

        // check if the word is the correct length
        if (wordLen <= MAX_ID) {

          char tempWord[MAX_ID + 1] = "";
          int idx = 0;

          for (int j = i - wordLen + 1; j <= i; j++) {
            tempWord[idx] = sourceProgram[j];
            idx++;
          }
          tempWord[idx] = '\0';

          // check if reserved word
          /*if (strcmp(tempWord, "ifel") == 0) {
            tokenList[tokenIdx] = ifelsym;
            //printf("\n%-30s%d", "ifel", ifelsym);
            tokenIdx++;
          } else*/ if (strcmp(tempWord, "begin") == 0) {
            //printf("\n%-30s%d", "begin", beginsym);
            tokenList[tokenIdx] = beginsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "end") == 0) {
            //printf("\n%-30s%d", "end", endsym);
            tokenList[tokenIdx] = endsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "if") == 0) {
            //printf("\n%-30s%d", "if", ifsym);
            tokenList[tokenIdx] = ifsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "then") == 0) {
            //printf("\n%-30s%d", "then", thensym);
            tokenList[tokenIdx] = thensym;
            tokenIdx++;
          } else if (strcmp(tempWord, "while") == 0) {
            //printf("\n%-30s%d", "while", whilesym);
            tokenList[tokenIdx] = whilesym;
            tokenIdx++;
          } else if (strcmp(tempWord, "do") == 0) {
            //printf("\n%-30s%d", "do", dosym);
            tokenList[tokenIdx] = dosym;
            tokenIdx++;
            /*} else if (strcmp(tempWord, "call") == 0) {
              printf("\n%-30s%d", "call", callsym);
              tokenList[tokenIdx] = callsym;
              tokenIdx++;*/
          } else if (strcmp(tempWord, "const") == 0) {
            //printf("\n%-30s%d", "const", constsym);
            tokenList[tokenIdx] = constsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "var") == 0) {
           // printf("\n%-30s%d", "var", varsym);
            tokenList[tokenIdx] = varsym;
            tokenIdx++;
            /*} else if (strcmp(tempWord, "procedure") == 0) {
              printf("\n%-30s%d", "procedure", procsym);
              tokenList[tokenIdx] = procsym;
              tokenIdx++;*/
          } else if (strcmp(tempWord, "write") == 0) {
            //printf("\n%-30s%d", "write", writesym);
            tokenList[tokenIdx] = writesym;
            tokenIdx++;
          } else if (strcmp(tempWord, "read") == 0) {
            //printf("\n%-30s%d", "read", readsym);
            tokenList[tokenIdx] = readsym;
            tokenIdx++;
          /*} else if (strcmp(tempWord, "else") == 0) {
            printf("\n%-30s%d", "else", elsesym);
            tokenList[tokenIdx] = elsesym;
            tokenIdx++;*/
          } else if(strcmp(tempWord, "odd") == 0) {
            tokenList[tokenIdx] = oddsym;
            tokenIdx++;

          // identifiers
          } else {

            // add identifier token
            tokenList[tokenIdx] = identsym;
            tokenIdx++;

            // add value
            strcpy(identifiers[idIdx], tempWord);
            tokenList[tokenIdx] = idIdx;
            //printf("\n%-30s%d", identifiers[tokenList[tokenIdx]],
                  // tokenList[tokenIdx - 1]);
            tokenIdx++;
            idIdx++;
          }

          // invalid length
        } else {
          char tempWord[MAX_LEN] = "";
          int idx = 0;
          for (int j = i - MAX_ID + 1; j <= i; j++) {
            tempWord[idx] = sourceProgram[j];
            idx++;
          }
          tempWord[idx] = '\0';
          printf("Error: Identifier length too long (must be 11 characters at most)\n");
          exit(1);
        }
      }

      // check if number
    } else if (comment == 0 && isdigit(sourceProgram[i])) {
      wordLen = 0;
      wordFlag = 0;
      digitLen++;

      // check if we are at the end of the number
      if (sourceProgram[i + 1] == '\0' || !isdigit(sourceProgram[i + 1])) {

        // check if the number is valid
        if (digitLen <= MAX_NUM) {

          // add number token
          tokenList[tokenIdx] = numbersym;
          tokenIdx++;

          // add value
          char tempNum[MAX_NUM + 1] = "";
          int idx = 0;

          for (int j = i - digitLen + 1; j <= i; j++) {
            tempNum[idx] = sourceProgram[j];
            idx++;
          }
          tempNum[idx] = '\0';
          tokenList[tokenIdx] = atoi(tempNum);

          // print to lexeme table
          // printf("%-30d%d", tokenList[tokenIdx], tokenList[tokenIdx - 1]);
          tokenIdx++;

          // print error if number is invalid
        } else {
          char tempNum[MAX_LEN] = "";
          int idx = 0;
          for (int j = i - digitLen + 1; j <= i; j++) {
            tempNum[idx] = sourceProgram[j];
            idx++;
          }
          tempNum[idx] = '\0';
          printf("Error: Too many digits\n");
          exit(1);
        }
      }
    } else {
      wordLen = 0;
      wordFlag = 0;
      digitLen = 0;

      // check if opening comment
      if (comment == 0 && sourceProgram[i] == '/' &&
          sourceProgram[i + 1] == '*') {
        comment = 1;
        i++;
        // check if closing comment
      } else if (comment == 1) {
        if (sourceProgram[i] == '*' && sourceProgram[i + 1] == '/') {
          comment = 0;
          i++;
        }
        // check if special symbol
      } else if (comment == 0 && sourceProgram[i] != ' ' &&
                 sourceProgram[i] != '\t' && sourceProgram[i] != '\n' &&
                 sourceProgram[i] != '\r') {
        int invalidSymbol = 0;
        int twoSymbols = 0;

        if (sourceProgram[i] == '+')
          tokenList[tokenIdx] = plussym;
        else if (sourceProgram[i] == '-')
          tokenList[tokenIdx] = minussym;
        else if (sourceProgram[i] == '*')
          tokenList[tokenIdx] = multsym;
        else if (sourceProgram[i] == '/')
          tokenList[tokenIdx] = slashsym;
        else if (sourceProgram[i] == '(')
          tokenList[tokenIdx] = lparentsym;
        else if (sourceProgram[i] == ')')
          tokenList[tokenIdx] = rparentsym;
        else if (sourceProgram[i] == ',')
          tokenList[tokenIdx] = commasym;
        else if (sourceProgram[i] == '.')
          tokenList[tokenIdx] = periodsym;
        else if (sourceProgram[i] == ';')
          tokenList[tokenIdx] = semicolonsym;
        else if (sourceProgram[i] == '=')
          tokenList[tokenIdx] = eqlsym;
        else if (sourceProgram[i] == ':' && sourceProgram[i + 1] == '=') {
          tokenList[tokenIdx] = becomessym;
          twoSymbols = 1;
        } else if (sourceProgram[i] == '<') {
          if (sourceProgram[i + 1] == '=') {
            tokenList[tokenIdx] = leqsym;
            twoSymbols = 1;
          } else if (sourceProgram[i + 1] == '>') {
            tokenList[tokenIdx] = neqsym;
            twoSymbols = 1;
          } else
            tokenList[tokenIdx] = lessym;
        } else if (sourceProgram[i] == '>') {
          if (sourceProgram[i + 1] == '=') {
            tokenList[tokenIdx] = geqsym;
            twoSymbols = 1;
          } else
            tokenList[tokenIdx] = gtrsym;

          // invalid symbol
        } else {
          invalidSymbol = 1;
        }

        if (invalidSymbol == 0) {
          if (twoSymbols == 1) {
            //printf("\n%c%-29c%d", sourceProgram[i], sourceProgram[i + 1],
                   //tokenList[tokenIdx]);
            i++;
          } //else
            //printf("\n%-30c%d", sourceProgram[i], tokenList[tokenIdx]);

          tokenIdx++;
        } else {
          // printf("\n%-30c", sourceProgram[i]);
          printf("Error: Not a valid symbol\n");
          exit(1);
        }
      }
    }
  }
  // printf("\n\nToken List:\n");
  //  print token list
  for (int i = 0; i < tokenIdx; i++) {

    // check if identifier
    if (tokenList[i] == identsym) {
      //printf("%d %s ", tokenList[i], identifiers[tokenList[i + 1]]);
      i++;
      // check if number
    } else if (tokenList[i] == numbersym) {
      //printf("%d %d ", tokenList[i], tokenList[i + 1]);
      i++;
    } /*else {
      printf("%d ", tokenList[i]);
    }*/
  }

  pc = 0;
  tp = 0;
  currentToken = 0;
  program();
  printCode();
  markChange();
  printSymbolTable();

  printf("\n");
  return 0;
}

void program() {
  block();

  // handle error
  if (tokenList[currentToken] != periodsym) {
    printf("\nError: Program must end with period\n");
    exit(1);
  }

  // HALT
  emit(9, 0, 3);
}

void block() {

  // JMP
  emit(7, 0, 3);
  pc = 3;

  constDeclaration();
  int numVars = varDeclaration();

  // INC
  emit(6, 0, 3 + numVars);

  statement();
}

void constDeclaration() {
  char identName[12];
  symbol s;

  if (tokenList[currentToken] == constsym) {
    do {
      currentToken++;
      
      // handle error
      if (tokenList[currentToken] != identsym) {
        printf("\nError: const, var, and read keywords must be followed by an identifier\n");
        exit(1);
      }

      currentToken++;

      // handle error
      if (symbolTableCheck(identifiers[tokenList[currentToken]]) != -1) {
        printf("\nError: Identifier already exists\n");
        exit(1);
      }

      strcpy(identName, identifiers[tokenList[currentToken]]);
      currentToken++;

      // handle error
      if (tokenList[currentToken] != eqlsym) {
        printf("\nError: Constants must be assigned with =\n");
        exit(1);
      }

      currentToken++;

      // handle error
      if (tokenList[currentToken] != numbersym) {
        printf("\nError: Constants must be assigned an integer value\n");
        exit(1);
      }

      currentToken++;

      // add to symbol table
      s.kind = 1;
      strcpy(s.name, identName);
      s.val = tokenList[currentToken];
      s.level = 0;
      s.addr = 0;
      s.mark = 0;
      symbolTable[tp] = s;
      tp++;
      currentToken++;

    } while (tokenList[currentToken] == commasym);

    // handle error
    if (tokenList[currentToken] != semicolonsym) {
      printf("\nError: Constant and variable declarations must be followed by a semicolon\n");
      exit(1);
    }
    currentToken++;
  }
}

// returns number of variables in the program
int varDeclaration() { 
  int numVars = 0;
  symbol s;

  if (tokenList[currentToken] == varsym) {
    do {
      currentToken++;

      // handle error
      if (tokenList[currentToken] != identsym) {
        printf("\nError: const, var, and read keywords must be followed by an identifier\n");
        exit(1);
      }
      
      currentToken++;

      // handle error
      if (symbolTableCheck(identifiers[tokenList[currentToken]]) != -1) {
        printf("\nError: Symbol name has already been declared\n");
        exit(1);
      }

      // add to symbol table
      s.kind = 2;
      strcpy(s.name, identifiers[tokenList[currentToken]]);
      s.val = 0;
      s.level = 0;
      s.addr = pc;
      pc++;
      s.mark = 0;
      symbolTable[tp] = s;
      tp++;
      numVars++;
      currentToken++;

    } while (tokenList[currentToken] == commasym);

    // handle error
    if (tokenList[currentToken] != semicolonsym) {
      printf("\nError: Constant and variable declarations must be followed by a semicolon\n");
      exit(1);
    }
    currentToken++;
  }
  return numVars;
}

void statement() {
  int symIdx ;
  int jpcIdx;

  // assignment statement
  if (tokenList[currentToken] == identsym) {
    currentToken++;

    // check if identifier is valid
    symIdx = symbolTableCheck(identifiers[tokenList[currentToken]]);

    // handle errors
    if (symIdx == -1) {
      printf("\nError: Undeclared identifier %s\n",
             identifiers[tokenList[currentToken]]);
      exit(1);
    }
    if (symbolTable[symIdx].kind != 2) { // not a var
      printf("\nError: only variable values may be altered\n");
      exit(1);
    }

    currentToken++;

    // handle error
    if (tokenList[currentToken] != becomessym) {
      printf("\nError: assignment statements must use :=\n");
      exit(1);
    }

    currentToken++;
    expression();

    // STO
    emit(4, 0, symbolTable[symIdx].addr);
    return;
  }

  // begin
  if (tokenList[currentToken] == beginsym) {
    do {
      currentToken++;
      statement();

    } while (tokenList[currentToken] == semicolonsym);

    // handle error
    if (tokenList[currentToken] != endsym) {
      printf("\nError: begin must be followed by end\n");
      exit(1);
    }
    currentToken++;
    return;
  }

  // if statement
  if (tokenList[currentToken] == ifsym) {
    currentToken++;
    condition(tokenList);
    jpcIdx = cx;

    // JPC
    emit(8, 0, cx);

    // handle error
    if (tokenList[currentToken] != thensym) {
      printf("\nError: if must be followed by then\n");
      exit(1);
    }
    
    currentToken++;
    statement();
    text[jpcIdx].M = cx;
    return;
  }

  // while loop
  if (tokenList[currentToken] == whilesym) {
    currentToken++;
    int loopIdx = cx;
    condition();

    // handle error
    if (tokenList[currentToken] != dosym) {
      printf("\nError: while must be followed by do\n");
      exit(1);
    }
    
    currentToken++;
    jpcIdx = cx;

    // JPC
    emit(8, 0, cx);
    statement();

    // JMP
    emit(7, 0, loopIdx);
    text[jpcIdx].M = cx;
    return;
  }

  // read
  if (tokenList[currentToken] == readsym) {
    currentToken++;

    // handle error
    if (tokenList[currentToken] != identsym) {
      printf("\nError: const, var, and read keywords must be followed by an identifier\n");
      exit(1);
    }

    currentToken++;

    // check if identifier is valid
    symIdx = symbolTableCheck(identifiers[tokenList[currentToken]]);

    // handle errors
    if (symIdx == -1) {
      printf("\nError: Undeclared identifier %s\n",
             identifiers[tokenList[currentToken]]);
      exit(1);
    }
    if (symbolTable[symIdx].kind != 2) { //(not a var)
      printf("\nError: only variable values may be altered\n");
      exit(1);
    }
    currentToken++;
    
    // READ
    emit(9, 0, 1);

    // STO
    emit(4, 0, symbolTable[symIdx].addr);
    return;
  }

  // write
  if (tokenList[currentToken] == writesym) {
    currentToken++;
    expression();

    // WRITE
    emit(9, 0, 2);
    return;
  }
}

// handles conditional statements
void condition() {

  if (tokenList[currentToken] == oddsym) {
    currentToken++;
    expression();

    // ODD
    emit(2, 0, 11);

  } else {
    expression();

    if (tokenList[currentToken] == eqlsym) {
      currentToken++;
      expression();

      // EQL
      emit(2, 0, 5);

    } else if (tokenList[currentToken] == neqsym) {
      currentToken++;
      expression();

      // NEQ
      emit(2, 0, 6);

    } else if (tokenList[currentToken] == lessym) {
      currentToken++;
      expression();

      // LSS
      emit(2, 0, 7);

    } else if (tokenList[currentToken] == leqsym) {
      currentToken++;
      expression();

      // LEQ
      emit(2, 0, 8);

    } else if (tokenList[currentToken] == gtrsym) {
      currentToken++;
      expression();

      // GTR
      emit(2, 0, 9);

    } else if (tokenList[currentToken] == geqsym) {
      currentToken++;

      expression();

      // GEQ
      emit(2, 0, 10);

      // handle error
    } else {
      printf("\nError: condition must contain comparison operator\n");
      exit(1);
    }
  }
}

void expression() {
  term();

  while (tokenList[currentToken] == plussym ||
         tokenList[currentToken] == minussym) {
    if (tokenList[currentToken] == plussym) {
      currentToken++;
      term();

      // ADD
      emit(2, 0, 1);

    } else {
      currentToken++;
      term();

      // SUB
      emit(2, 0, 2);
    }
  }
}

void term() {
  factor();
  while (tokenList[currentToken] == multsym ||
         tokenList[currentToken] == slashsym) {
    if (tokenList[currentToken] == multsym) {
      currentToken++;
      factor();

      // MUL
      emit(2, 0, 3);

    } else {
      currentToken++;
      factor();

      // DIV
      emit(2, 0, 4);
    }
  }
}

void factor() {
  int symIdx;

  if (tokenList[currentToken] == identsym) {
    currentToken++;
    
    // check if identifier is valid
    symIdx = symbolTableCheck(identifiers[tokenList[currentToken]]);

    // handle error
    if (symIdx == -1) {
      printf("\nError: Undeclared identifier %s\n",
             identifiers[tokenList[currentToken]]);
      exit(1);
    }
    
    if (symbolTable[symIdx].kind == 1) { // const

      // LIT
      emit(1, 0, symbolTable[symIdx].val);

    } else { // var

      // LOD
      emit(3, 0, symbolTable[symIdx].addr);
    }

    currentToken++;

  } else if (tokenList[currentToken] == numbersym) {
    currentToken++;

    // LIT
    emit(1, 0, tokenList[currentToken]);
    currentToken++;

  } else if (tokenList[currentToken] == lparentsym) {
    currentToken++;
    expression();
    
    // handle error
    if (tokenList[currentToken] != rparentsym) {
      printf("\nError: right parenthesis must follow left parenthesis\n");
      exit(1);
    }

    currentToken++;

  } else {
    printf("\nError: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
    exit(1);
  }
}

void printSymbolTable() {
  printf("\n\n\nSymbol Table:\n");
  printf("\n%4s | %12s | %5s | %5s | %7s | %5s", "Kind", "Name", "Value",
         "Level", "Address", "Mark");
  printf("\n-----------------------------------------------------");

  for (int i = 0; i < tp; i++) {
    printf("\n%4d | %12s | %5d | %5d | %7d | %5d", symbolTable[i].kind,
           symbolTable[i].name, symbolTable[i].val, symbolTable[i].level,
           symbolTable[i].addr, symbolTable[i].mark);
  }
  printf("\n");
}

// returns index if symbol exists in the symbol table
// returns -1 if not
int symbolTableCheck(char *name) {
  for (int i = 0; i < tp; i++) {
    if (strcmp(symbolTable[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

// code generation
void emit(int op, int L, int M) {
  if (cx > MAX_LEN) {
    printf("\nError: Program too long\n");
    exit(1);
  } else {
    text[cx].OP = op; // opcode
    text[cx].L = L;   // lexicographical level
    text[cx].M = M;   // modifier
    cx++;
  }
}

// changes all marks to 1 at the end of a function
void markChange() {
  for (int i = 0; i < tp; i++) {
    symbolTable[i].mark = 1;
  }
}

// prints the assembly code
void printCode() {
  printf("%-5s %4s %2s %2s", "Line", "OP", "L", "M");

  for (int i = 0; i < cx; i++) {
    printf("\n%-5d %4s %2d %2d", i, translateOP(text[i].OP), text[i].L,
           text[i].M);
  }
}

char *translateOP(int op) {
  switch (op) {
  case 1:
    return "LIT";
    break;
  case 2:
    return "OPR";
    break;
  case 3:
    return "LOD";
    break;
  case 4:
    return "STO";
    break;
  case 5:
    return "CAL";
    break;
  case 6:
    return "INC";
    break;
  case 7:
    return "JMP";
    break;
  case 8:
    return "JPC";
    break;
  case 9:
    return "SYS";
    break;
  default:
    return " ";
    break;
  }
}