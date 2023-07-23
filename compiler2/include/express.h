typedef enum stmtType
{
    DECLARATION,
    ASSIGNMENT,
    FUNCTION
} stmtType;
typedef enum Datatype
{
    INT = 0
} Datatype;
typedef enum functions
{
    PRINT = 0
} functions;

struct expr
{
    struct expr *left;
    struct expr *right;
    float val;
    char op;
    int isLeaf;
    int isVar;
    char *var_name;
};

typedef struct SymbolTable
{
    int maxIndex;
    struct symbol *start;
    struct symbol *end;
} SymbolTable;

typedef struct declAttribute
{
    Datatype Type;
} declAttribute;

typedef struct assignAttribute
{
    char *var_name;
} assignAttribute;

typedef struct funcAttribute
{
    functions fName;
} funcAttribute;

typedef struct instruct
{
    stmtType Type;
    void *lst;
    void *attr;
    struct instruct *nxtInst;
} instruct;

typedef struct symbol
{
    int num; // to store the position of the symbol in the symbol table
    Datatype type;
    char *name;
    int val;      // to store the value of the integer
    int isValSet; // is set to 1 when the variable  o/w 0
    struct symbol *nextSymbol;
} symbol;

// currently only used for making print statement list
typedef struct exprList
{
    struct expr *expr;
    struct exprList *next;
} exprList;

struct expr *addNonLeafExpr(struct expr *left, char op, struct expr *right);
struct expr *addLeafExpr(float val);
struct expr *addLeafVariable(char *name);
float evaluate(struct expr *tree);
symbol *addSymbol(char *name, symbol *sym);
void printSymbols(symbol *sym);
instruct *returnDeclaration(symbol *syms, Datatype Type);
void printinstruct(instruct *decl);
void addinstruct(instruct *ins);
void printCode();
instruct *returnAssignment(char *name, struct expr *exp);
instruct *returnFunction(functions fname, exprList *syms);
exprList *addExpression(struct expr *exp, exprList *lst);
void printExprList(exprList *lst);
int add_newVar(SymbolTable *table, symbol *sym, Datatype type);
symbol *getSymbol(char *var_name, SymbolTable *table);
int assignValue(char *var_name, int val, SymbolTable *table);
int lookUpvar(char *var_name, SymbolTable *table);
int addVars(SymbolTable *table, symbol *lst, Datatype type);
int call_print(exprList *vals);
int execute_instruct(instruct *ins);
int exec_code(instruct *codeStart);