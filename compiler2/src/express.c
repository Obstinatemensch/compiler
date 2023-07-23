#include "../include/express.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern SymbolTable symTable;
extern instruct *codeStart;
extern instruct *codeEnd;

struct expr *addNonLeafExpr(struct expr *left, char op, struct expr *right)
{
    struct expr *newNode = (struct expr *)malloc(sizeof(struct expr));
    if (newNode != NULL)
    {
        // initializing values of the new node
        newNode->val = 0;
        newNode->left = left;
        newNode->op = op;
        newNode->right = right;
        newNode->isLeaf = 0;

        return newNode;
    }
    // case that malloc fails
    else
    {
        return NULL;
    }
}
struct expr *addLeafExpr(float val)
{
    struct expr *newNode = (struct expr *)malloc(sizeof(struct expr));

    // stores the value of the leaf and mark it as a leaf
    if (newNode != NULL)
    {
        newNode->val = val;
        newNode->isLeaf = 1;
        newNode->isVar = 0;
        return newNode;
    }
    // in case malloc doesn't work
    else
    {
        printf("Lack of space...malloc failed\n");
        exit(1);
        return NULL;
    }
}
struct expr *addLeafVariable(char *name)
{
    struct expr *newNode = (struct expr *)malloc(sizeof(struct expr));

    if (newNode != NULL)
    {

        newNode->val = 0;
        newNode->isLeaf = 1;
        newNode->var_name = strndup(name, strlen(name));
        newNode->isVar = 1;
        return newNode;
    }
    // if malloc failed
    else
    {
        printf("Lack of space...malloc failed\n");
        exit(1);
        return NULL;
    }
}
// function to go through the tree of each expression and recursively evaluate
float evaluate(struct expr *tree)
{

    if (tree == NULL)
    {
        return -1;
    }
    if (tree->isLeaf == 1)
    {
        if (tree->isVar == 0)
        {
            return tree->val;
        }
        else
        {
            return lookUpvar(tree->var_name, &symTable);
        }
    }
    else if (tree->isLeaf == 0)
    {
        switch (tree->op)
        {
        case '+':
            return evaluate(tree->left) + evaluate(tree->right);
            break;
        case '-':
            return evaluate(tree->left) - evaluate(tree->right);
            break;
        case '*':
            return evaluate(tree->left) * evaluate(tree->right);
            break;
        case '/':
            return evaluate(tree->left) / evaluate(tree->right);
            break;
        default:
            return -1;
        }
    }
    else
    {
        return -1;
    }
}
// a function to add store the variables details in a symbol tree
symbol *addSymbol(char *name, symbol *sym)
{
    symbol *newNode = (symbol *)malloc(sizeof(symbol));
    newNode->nextSymbol = sym;
    newNode->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    newNode->isValSet = 0;
    newNode->val = 0;
    newNode->num = 0;
    strcpy(newNode->name, name);
    return newNode;
}
// function to print the symbols
void printSymbols(symbol *sym)
{
    while (sym != NULL)
    {
        printf("%s ", sym->name);
        sym = sym->nextSymbol;
        if (sym->nextSymbol == NULL)
        {
            break;
        }
    }
    printf("%s ", sym->name);
}

instruct *returnDeclaration(symbol *syms, Datatype Type)
{
    instruct *newIns = (instruct *)malloc(sizeof(instruct));
    declAttribute *newAttr = (declAttribute *)malloc(sizeof(declAttribute));
    newAttr->Type = Type;
    newIns->nxtInst = NULL;
    newIns->lst = (void *)syms;
    newIns->attr = (void *)newAttr;
    newIns->Type = DECLARATION;
    return newIns;
}
// function to go throught the expression tree of the assignments and print it in preorder traversal
void print_expres(struct expr *tree)
{
    if (tree == NULL)
    {
        return;
    }
    if (tree->isLeaf == 1)
    {
        int value = tree->val;
        if (tree->isVar == 0)
        {
            printf("%d ", value);
        }
        else
        {
            printf("%s ", tree->var_name);
        }
    }
    else if (tree->isLeaf == 0)
    {
        switch (tree->op)
        {
        case '+':
            printf("PLUS ");
            print_expres(tree->left);
            print_expres(tree->right);
            break;
        case '-':
            printf("SUB ");
            print_expres(tree->left);
            print_expres(tree->right);
            break;
        case '*':
            printf("MUL ");
            print_expres(tree->left);
            print_expres(tree->right);
            break;
        case '/':
            printf("DIV ");
            print_expres(tree->left);
            print_expres(tree->right);
            break;
        default:
            break;
        }
    }
}
// function to check the type of the instruction and do appropriate statements
void printinstruct(instruct *ins)
{

    if (ins->Type == DECLARATION)
    {
        printf("DECL ");
        printSymbols(ins->lst);
        printf(";\n");
    }
    else if (ins->Type == ASSIGNMENT)
    {
        assignAttribute *attr = ins->attr;
        printf("ASSIGN %s ", attr->var_name);
        print_expres(ins->lst);
        printf(";\n");
    }
    else if (ins->Type == FUNCTION)
    {
        funcAttribute *attr = ins->attr;
        if (attr->fName == PRINT)
        {
            printf("CALL print ");
            printExprList(ins->lst);
            printf(";\n");
        }
    }
}

void addinstruct(instruct *ins)
{
    if (codeStart == NULL)
    {
        codeStart = ins;
        codeEnd = ins;
    }
    else
    {
        codeEnd->nxtInst = ins;
        codeEnd = ins;
    }
}

void printCode()
{
    printf("--------------------------\n");
    printf("Printing the AST output...\n\n");
    instruct *ins = codeStart;
    while (ins != NULL)
    {
        // printf("reached fing here\n");
        printinstruct(ins);
        ins = ins->nxtInst;
    }
}

instruct *returnAssignment(char *name, struct expr *exp)
{
    instruct *newIns = (instruct *)malloc(sizeof(instruct));
    assignAttribute *newAttr = (assignAttribute *)malloc(sizeof(assignAttribute));
    newAttr->var_name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(newAttr->var_name, name);

    newIns->nxtInst = NULL;
    newIns->lst = (void *)exp;
    newIns->attr = (void *)newAttr;
    newIns->Type = ASSIGNMENT;
    return newIns;
}

instruct *returnFunction(functions fname, exprList *syms)
{
    instruct *newIns = (instruct *)malloc(sizeof(instruct));
    funcAttribute *newAttr = (funcAttribute *)malloc(sizeof(funcAttribute));
    newAttr->fName = fname;

    newIns->nxtInst = NULL;
    newIns->lst = (void *)syms;
    newIns->attr = (void *)newAttr;
    newIns->Type = FUNCTION;
    return newIns;
}

exprList *addExpression(struct expr *exp, exprList *lst)
{
    exprList *newExprListNode = (exprList *)malloc(sizeof(exprList));
    if (newExprListNode != NULL)
    {
        newExprListNode->expr = exp;
        newExprListNode->next = lst;
    }
    else
    {
        printf("malloc failed in addExpression function \n");
    }
    return newExprListNode;
}

void printExprList(exprList *lst)
{
    exprList *ptr = lst;
    while (ptr != NULL)
    {
        print_expres((ptr->expr));
        ptr = ptr->next;
    }
}
// function to see if a variable name is already declared or not
int isPresent(char *var_name, SymbolTable *table)
{

    symbol *var = table->start;
    while (var != NULL)
    {
        if (strcmp(var->name, var_name) == 0)
        {
            return 1;
        }
        else
        {
            var = var->nextSymbol;
        }
    }
    return 0;
}
// function to add a new variables to the SymbolTable if it already doesn't exist
int add_newVar(SymbolTable *table, symbol *sym, Datatype type)
{

    if (isPresent(sym->name, table) == 1)
    {
        printf("Multiple delcaration of the variable '%s'\n", sym->name);
        exit(1);
    }
    else
    {
        if (table->start == NULL)
        {
            table->start = sym;
            table->end = sym;
            table->maxIndex += 1;
        }
        else
        {
            table->end->nextSymbol = sym;
            table->end = sym;
            table->maxIndex += 1;
        }

        sym->num = table->maxIndex;
        sym->type = type;
        sym->val = 0;
        sym->isValSet = 0;
        sym->nextSymbol = NULL;
        return 1;
    }
}

symbol *getSymbol(char *var_name, SymbolTable *table)
{

    symbol *var = table->start;

    while (var != NULL)
    {
        if (strcmp(var->name, var_name) == 0)
        {
            return var;
        }
        var = var->nextSymbol;
    }
    return NULL;
}
// function to assign a value to variables in the SymbolTable
int assignValue(char *var_name, int val, SymbolTable *table)
{
    symbol *var = getSymbol(var_name, table);

    if (var != NULL)
    {
        var->isValSet = 1;
        var->val = val;
        return 1;
    }
    else
    {
        if (isPresent(var_name, table) == 0)
        {
            printf("Variable '%s' not declared.\n", var_name);
            exit(1);
        }
        return 0;
    }
}

// This look up function returns zero value even if the values is not assigned to the variable ...
// ... if the variable is present in the table .
int lookUpvar(char *var_name, SymbolTable *table)
{
    symbol *var = getSymbol(var_name, table);

    if (var != NULL)
    {

        return var->val;
    }
    else
    {
        if (isPresent(var_name, table) == 0)
        {
            printf("Variable '%s' not declared.\n", var_name);
            exit(1);
        }
        return 0;
    }
}

symbol *copySymbolTo(symbol *sym1)
{
    symbol *newNode = (symbol *)malloc(sizeof(symbol));

    if (newNode == NULL)
    {
        printf("Lack of space...malloc failed\n");
        exit(1);
    }

    newNode->isValSet = sym1->isValSet;
    newNode->name = sym1->name;
    newNode->nextSymbol = sym1->nextSymbol;
    newNode->num = sym1->num;
    newNode->type = sym1->type;
    newNode->val = sym1->val;
    return newNode;
}
// function to add add variables to the SymbolTable
int addVars(SymbolTable *table, symbol *lst, Datatype type)
{
    symbol *sym = lst, *temp, *newNode;
    while (sym != NULL)
    {
        newNode = copySymbolTo(sym);
        newNode->nextSymbol = NULL;
        add_newVar(table, newNode, type);
        sym = sym->nextSymbol;
    }
    return 1;
}
// function to evaluate the expressions...
int call_print(exprList *vals)
{
    exprList *exprs = vals;
    int val;
    while (exprs != NULL)
    {
        val = evaluate(exprs->expr);
        printf("%d ", val);
        exprs = exprs->next;
    }
    printf("\n");
    return 1;
}

int execute_instruct(instruct *ins)
{
    if (ins->Type == DECLARATION)
    {
        declAttribute *attr = ins->attr;
        addVars(&symTable, ins->lst, attr->Type);
        return 1;
    }
    else if (ins->Type == ASSIGNMENT)
    {
        assignAttribute *attr = ins->attr;
        struct expr *val = ins->lst;

        assignValue(attr->var_name, evaluate(val), &symTable);
        return 1;
    }
    else if (ins->Type == FUNCTION)
    {
        funcAttribute *attr = ins->attr;
        if (attr->fName == PRINT)
        {
            call_print(ins->lst);
        }
        return 1;
    }
}

int exec_code(instruct *codeStart)
{
    instruct *ins = codeStart;

    while (ins != NULL)
    {
        execute_instruct(ins);
        ins = ins->nxtInst;
    }
}
