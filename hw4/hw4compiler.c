// Sophia Gnisci
// Andrea Marquez Tavera
// COP3402 Fall 2023
// HW4 (Tiny PL/0 Compiler)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MACROS
#define MAX_LEN 512
#define MAX_ID 11
#define MAX_NUM 5
#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_LEVEL 5

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
  callsym = 27,
  constsym = 28,
  varsym = 29,
  procsym = 30,
  writesym = 31,
  readsym = 32,
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

InstructionRegister text[MAX_LEN]; // array of text (code)
int cx;                            // code index

void program();
void block(int lev);
void constDeclaration(int *dx, int lev);
int varDeclaration(int lev, int *dx);
void statement(int lev);
void condition(int lev);
void expression(int lev);
void term(int lev);
void factor(int lev);
void printSymbolTable();
int symbolTableCheck(char *name);
void emit(int op, int L, int M);
void markChange();
char *translateOP(int op);
void printCode();
void outputVM();

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

  int charCount = 0; // keeps track of index to fill up sourceProgram

  while (1) {
    char temp1 = fgetc(filePointer);

    // stop at the end of the file
    if (feof(filePointer))
      break;

    // print source program
    else
      printf("%c", temp1);

    // if not invisble character, add to array
    sourceProgram[charCount] = temp1;
    charCount++;
  }

  printf("\n");
  sourceProgram[charCount] = '\0';

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

          if (strcmp(tempWord, "begin") == 0) {
            tokenList[tokenIdx] = beginsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "end") == 0) {
            tokenList[tokenIdx] = endsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "if") == 0) {
            tokenList[tokenIdx] = ifsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "then") == 0) {
            tokenList[tokenIdx] = thensym;
            tokenIdx++;
          } else if (strcmp(tempWord, "while") == 0) {
            tokenList[tokenIdx] = whilesym;
            tokenIdx++;
          } else if (strcmp(tempWord, "do") == 0) {
            tokenList[tokenIdx] = dosym;
            tokenIdx++;
          } else if (strcmp(tempWord, "call") == 0) {
            tokenList[tokenIdx] = callsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "const") == 0) {
            tokenList[tokenIdx] = constsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "var") == 0) {
            tokenList[tokenIdx] = varsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "procedure") == 0) {
            tokenList[tokenIdx] = procsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "write") == 0) {
            tokenList[tokenIdx] = writesym;
            tokenIdx++;
          } else if (strcmp(tempWord, "read") == 0) {
            tokenList[tokenIdx] = readsym;
            tokenIdx++;
          } else if (strcmp(tempWord, "odd") == 0) {
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
          printf("Error number 26: Identifier too long.\n");
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
          printf("Error number 25: This number is too large.\n");
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
            i++;
          } 
          tokenIdx++;

        } else {
          printf("Error number 27: Invalid symbol.\n");
          exit(1);
        }
      }
    }
  }

  tp = 0;
  currentToken = 0;
  program();
  printf("\nNo errors, program is syntactically correct\n\n");
  printCode();
  outputVM();

  printf("\n");
  return 0;
}

void program() {
  block(0);

  // handle error
  if (tokenList[currentToken] != periodsym) {
    printf("\nError number 9: Period expected.\n");
    exit(1);
  }

  // HALT
  emit(9, 0, 3);
}

void block(int lev) {
  int tp0 = tp;// symbol index = symbol table index aka tx0 = tx
  int dx = 3;// space
  int cx0 = cx;
  symbolTable[tp].addr = cx; // current cx is stored at index tp0

  // JMP
  emit(7, 0, 0);
  // handle error
  if (lev > MAX_LEVEL) {
    printf("\nError number 28: Cannot compute more than 5 lexicographical "
           "levels\n");
    exit(1);
  }
  constDeclaration(&dx, lev);
  int numVars = varDeclaration(lev, &dx);

  while (tokenList[currentToken] == procsym) {
    symbol s;
    currentToken++;

        // handle error
        if (tokenList[currentToken] != identsym) {
          printf("\nError number 4: Const, var, and procedure must be "
                "followed by an "
                "identifier.\n");
          exit(1);
        }
        currentToken++;

        // handle error
        if (symbolTableCheck(identifiers[tokenList[currentToken]]) != -1 && symbolTable[symbolTableCheck(identifiers[tokenList[currentToken]])].level == lev) {
          printf("\nError number 29: Identifier already exists\n");
          exit(1);
        }
        // add to symbol table
        s.kind = 3;
        strcpy(s.name, identifiers[tokenList[currentToken]]);
        s.val = 0;
        s.level = lev;
        s.addr = cx+1;
        s.mark = 0;
        symbolTable[tp] = s;
        tp++;
        currentToken++;

        // handle error
        if (tokenList[currentToken] != semicolonsym) {
          printf("Error number 5: Semicolon or comma missing.\n");
          exit(1);
        }
        currentToken++;

        block(lev + 1);

        // handle error
        if (tokenList[currentToken] != semicolonsym) {
          printf("Error number 5: Semicolon or end expected.\n");
          exit(1);
        }
        currentToken++;
  }
  text[cx0].M = 3 * cx;
  cx0 = cx;

  // INC
  emit(6, 0, dx);

  statement(lev);

  // RTN
  if(lev > 0)
  {
    emit(2, 0, 0);
  }
  markChange(lev);
}

void constDeclaration(int *dx, int lev) {
  char identName[12];
  symbol s;

  if (tokenList[currentToken] == constsym) {
    do {
      currentToken++;

      // handle error
      if (tokenList[currentToken] != identsym) {
        printf("\nError number 4: Const, var, and read keywords must be "
               "followed by an "
               "identifier.\n");
        exit(1);
      }

      currentToken++;

      // handle error
      if (symbolTableCheck(identifiers[tokenList[currentToken]]) != -1 && symbolTable[symbolTableCheck(identifiers[tokenList[currentToken]])].level == lev) {
        printf("\nError number 29: Identifier already exists.\n");
        exit(1);
      }

      strcpy(identName, identifiers[tokenList[currentToken]]);
      currentToken++;

      // handle error
      if (tokenList[currentToken] != eqlsym) {
        printf("\nError number 3: Identifier must be followed by =.\n");
        exit(1);
      }

      currentToken++;

      // handle error
      if (tokenList[currentToken] != numbersym) {
        printf("\nError number 2: = must be followed by a number.\n");
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
      printf("\nError number 10: Semicolon between statements missing.\n");
      exit(1);
    }
    currentToken++;
  }
}

// returns number of variables in the program
int varDeclaration(int lev, int *dx) {
  int numVars = 0;
  symbol s;

  if (tokenList[currentToken] == varsym) {
    do {
      currentToken++;

      // handle error
      if (tokenList[currentToken] != identsym) {
        printf("\nError number 4: Const, var, and read keywords must be "
               "followed by an "
               "identifier.\n");
        exit(1);
      }

      currentToken++;

      // handle error
      if (symbolTableCheck(identifiers[tokenList[currentToken]]) != -1 && symbolTable[symbolTableCheck(identifiers[tokenList[currentToken]])].level == lev) {
        printf("\nError number 29: Identifier already exists\n");
        exit(1);
      }

      // add to symbol table
      s.kind = 2;
      strcpy(s.name, identifiers[tokenList[currentToken]]);
      s.val = 0;
      s.level = lev;
      s.addr = *dx;
      (*dx)++;
      s.mark = 0;
      symbolTable[tp] = s;
      tp++;
      numVars++;
      currentToken++;
    } while (tokenList[currentToken] == commasym);

    // handle error
    if (tokenList[currentToken] != semicolonsym) {
      printf("\nError number 10: Semicolon between statements missing.\n");
      exit(1);
    }
    currentToken++;
  }
  return numVars;
}

void statement(int lev) {
  int symIdx;
  int jpcIdx;

  // assignment statement
  if (tokenList[currentToken] == identsym) {
    currentToken++;

    // check if identifier is valid
    symIdx = symbolTableCheck(identifiers[tokenList[currentToken]]);

    // handle errors
    if (symIdx == -1 || symbolTable[symbolTableCheck(identifiers[tokenList[currentToken]])].mark == 1) {
      printf("\nError number 11: Undeclared identifier %s\n",
             identifiers[tokenList[currentToken]]);
      exit(1);
    }
    if (symbolTable[symIdx].kind != 2) { // not a var
      printf("\nError number 12: Assignment to constant or procedure is not "
             "allowed\n");
      exit(1);
    }
    currentToken++;

    // handle error
    if (tokenList[currentToken] != becomessym) {
      printf("\nError number 13: Assignment operator expected.\n");
      exit(1);
    }

    currentToken++;
    expression(lev);

    // STO
    emit(4, lev - symbolTable[symIdx].level, symbolTable[symIdx].addr);

    return;
  }

  // call
  if (tokenList[currentToken] == callsym) {
    currentToken++;

    // handle error
    if (tokenList[currentToken] != identsym) {
      printf("\nError number 14: Call must be followed by an identifier.\n");
      exit(1);
    } 
      currentToken++;

      // check if valid identifier
      int i = symbolTableCheck(identifiers[tokenList[currentToken]]);

      // handle error
      if (i == -1 || symbolTable[symbolTableCheck(identifiers[tokenList[currentToken]])].mark == 1) {
        printf("\nError number 11: Undeclared Identifier\n");
        exit(1);
      }

      if (symbolTable[i].kind != 3) {//(not a procedure)
        printf("\nError 15: Call of a constant or variable is meaningless\n");
        exit(1);
      }
        emit(5,lev- symbolTable[i].level, symbolTable[i].addr*3);
      currentToken++;
    return;
  }

  // begin
  if (tokenList[currentToken] == beginsym) {
    do {
      currentToken++;
      statement(lev);
    } while (tokenList[currentToken] == semicolonsym);

    // handle error
    if (tokenList[currentToken] != endsym) {

      printf("\nError number 17: Semicolon or end expected.\n");
      exit(1);
    }
    currentToken++;
    return;
  }

  // if statement
  if (tokenList[currentToken] == ifsym) {
    currentToken++;
    condition(lev);
    jpcIdx = cx;

    // JPC
    emit(8, 0, 0);
    // handle error
    if (tokenList[currentToken] != thensym) {
      printf("\nError number 16: then expected.\n");
      exit(1);
    }
    currentToken++;
    statement(lev);
    text[jpcIdx].M = cx * 3;
    return;
  }

  // while loop
  if (tokenList[currentToken] == whilesym) {
    currentToken++;
    int loopIdx = cx;
    condition(lev);

    // handle error
    if (tokenList[currentToken] != dosym) {
      printf("\nError number 18: do expected.\n");
      exit(1);
    }

    currentToken++;
    jpcIdx = cx;

    // JPC
    emit(8, 0, 0);
    statement(lev);

    // JMP
    emit(7, 0, loopIdx);
    text[jpcIdx].M = cx * 3;
    return;
  }

  // read
  if (tokenList[currentToken] == readsym) {
    currentToken++;

    // handle error
    if (tokenList[currentToken] != identsym) {
      printf("\nError number 4: const, var, and read keywords must be followed "
             "by an "
             "identifier\n");
      exit(1);
    }

    currentToken++;

    // check if identifier is valid
    symIdx = symbolTableCheck(identifiers[tokenList[currentToken]]);

    // handle errors
    if (symIdx == -1 || symbolTable[symbolTableCheck(identifiers[tokenList[currentToken]])].mark == 1) {
      printf("\nError number 11: Undeclared identifier %s\n",
             identifiers[tokenList[currentToken]]);
      exit(1);
    }
    if (symbolTable[symIdx].kind != 2) { //(not a var)
      printf("\nError number 12: Assignment to constant or procedure is not "
             "allowed\n");
      exit(1);
    }
    currentToken++;

    // READ
    emit(9, lev - symbolTable[symIdx].level, 0);

    // STO
    emit(4, lev - symbolTable[symIdx].level, symbolTable[symIdx].addr);
    return;
  }

  // write
  if (tokenList[currentToken] == writesym) {
    currentToken++;
    expression(lev);

    // WRITE
    emit(9, 0, 1);
    return;
  }
}

// handles conditional statements
void condition(int lev) {

  if (tokenList[currentToken] == oddsym) {
    currentToken++;
    expression(lev);

    // ODD
    emit(2, 0, 11);

  } else {
    expression(lev);

    if (tokenList[currentToken] == eqlsym) {
      currentToken++;

      if (tokenList[currentToken] != numbersym) {
        printf("\nError number 2: = must be followed by a number\n");
        exit(1);
      }

      expression(lev);

      // EQL
      emit(2, 0, 5);

    } else if (tokenList[currentToken] == neqsym) {
      currentToken++;
      expression(lev);

      // NEQ
      emit(2, 0, 6);

    } else if (tokenList[currentToken] == lessym) {
      currentToken++;
      expression(lev);

      // LSS
      emit(2, 0, 7);

    } else if (tokenList[currentToken] == leqsym) {
      currentToken++;
      expression(lev);

      // LEQ
      emit(2, 0, 8);

    } else if (tokenList[currentToken] == gtrsym) {
      currentToken++;
      expression(lev);

      // GTR
      emit(2, 0, 9);

    } else if (tokenList[currentToken] == geqsym) {
      currentToken++;

      expression(lev);

      // GEQ
      emit(2, 0, 10);

      // handle error
    } else if (tokenList[currentToken] == becomessym) {
      printf("\nError number 1: Use = instead of :=\n");
      exit(1);

      // handle error
    } else {
      printf("\nError number 20: Relational operator expected.\n");
      exit(1);
    }
  }
}

void expression(int lev) {
  term(lev);

  while (tokenList[currentToken] == plussym ||
         tokenList[currentToken] == minussym) {
    if (tokenList[currentToken] == plussym) {
      currentToken++;
      term(lev);

      // ADD
      emit(2, 0, 1);

    } else {
      currentToken++;
      term(lev);

      // SUB
      emit(2, 0, 2);
    }
  }
}

void term(int lev) {
  factor(lev);
  while (tokenList[currentToken] == multsym ||
         tokenList[currentToken] == slashsym) {
    if (tokenList[currentToken] == multsym) {
      currentToken++;
      factor(lev);

      // MUL
      emit(2, 0, 3);

    } else {
      currentToken++;
      factor(lev);

      // DIV
      emit(2, 0, 4);
    }
  }
}

void factor(int lev) {
  int symIdx;

  if (tokenList[currentToken] == identsym) {
    currentToken++;

    // check if identifier is valid
    symIdx = symbolTableCheck(identifiers[tokenList[currentToken]]);

    // handle error
    if (symIdx == -1 || symbolTable[symbolTableCheck(identifiers[tokenList[currentToken]])].mark == 1) {
      printf("\nError number 11: Undeclared identifier %s\n",
             identifiers[tokenList[currentToken]]);
      exit(1);
    }

    if (symbolTable[symIdx].kind == 1) { // const

      // LIT
      emit(1, 0, symbolTable[symIdx].val);

    } else if (symbolTable[symIdx].kind == 2) { // var

      // LOD
      emit(3, lev - symbolTable[symIdx].level, symbolTable[symIdx].addr);

    } else { // procedure
      printf("\nError number 21: Expression must not contain a procedure "
             "identifier\n");
      exit(1);
    }

    currentToken++;

  } else if (tokenList[currentToken] == numbersym) {
    currentToken++;

    // LIT
    emit(1, 0, tokenList[currentToken]);
    currentToken++;

  } else if (tokenList[currentToken] == lparentsym) {
    currentToken++;
    expression(lev);

    // handle error
    if (tokenList[currentToken] != rparentsym) {
      printf("\nError number 22: Right parenthesis missing.\n");
      exit(1);
    }

    currentToken++;

  } else {
    printf("\nError number 24: An expression cannot begin with this symbol\n");
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
    printf("\nError number 30: Program too long\n");
    exit(1);
  } else {
    text[cx].OP = op; // opcode
    text[cx].L = L;   // lexicographical level
    text[cx].M = M;   // modifier
    cx++;
  }
}

// changes all marks to 1 at the end of a function
void markChange(int lev) {
  for (int i = 0; i < tp; i++) {
    if (symbolTable[i].level == lev)
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

// prints assembly code to a file
void outputVM() {
  FILE *fptr;

  // create file
  fptr = fopen("elf.txt", "w");

  for (int i = 0; i < cx; i++) {
    fprintf(fptr, "%d %d %d\n", text[i].OP, text[i].L, text[i].M);
  }

  fclose(fptr);
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