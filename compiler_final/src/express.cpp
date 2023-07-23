#include "../include/express.hpp"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <map>
#include <string>
using namespace std;

map<string, value *> symbol_table;
map<string, funcDetails *> func_table;

Program prog = {NULL, NULL, NULL};

void addVar(char *name, types type, map<string, value *> &sym_tab);
void addVar(char *name, types type, indexList *startIndex, map<string, value *> &sym_tab);

void update(char *name, result *res, map<string, value *> &sym_tab);
void update(char *name, indexList *startIndex, result *res, map<string, value *> &sym_tab);

result *lookup(char *name, map<string, value *> &sym_tab);
result *lookup(char *name, indexList *startIndex, map<string, value *> &sym_tab);

void printExpr(expr *node);

result *solve(expr *node, map<string, value *> local_sym_tab);

bool isError = true;

varExpr *createVarExpr(char *varName)
{
    varExpr *a = new varExpr;
    a->varName = strndup(varName, strlen(varName));
    a->isArray = false;
    a->startIndex = NULL;
    a->endIndex = NULL;

    return a;
}

varExpr *createVarExpr(varExpr *varExp, expr *index)
{
    exprList *newPtr = new exprList;
    newPtr->exp = index;
    newPtr->next = NULL;

    if (varExp->startIndex == NULL)
    {
        varExp->startIndex = newPtr;
        varExp->endIndex = newPtr;
    }
    else
    {
        varExp->endIndex->next = newPtr;
        varExp->endIndex = newPtr;
    }
    varExp->isArray = true;
    return varExp;
}

expr *addLeafExpr(int val)
{
    intExpr *a = new intExpr;
    a->type = Int;
    a->val = val;

    expr *newPtr = new expr;
    newPtr->attr = a;
    newPtr->left = NULL;
    newPtr->right = NULL;
    newPtr->type = Leaf;
    newPtr->valType = Int;

    return newPtr;
}

expr *addLeafExpr(bool val)
{
    boolExpr *a = new boolExpr;
    a->type = Bool;
    a->val = val;

    expr *newPtr = new expr;
    newPtr->attr = a;
    newPtr->left = NULL;
    newPtr->right = NULL;
    newPtr->type = Leaf;
    newPtr->valType = Bool;

    return newPtr;
}

expr *addLeafExpr(func_call *_func_call)
{
    expr *newPtr = new expr;
    newPtr->attr = _func_call;

    newPtr->left = NULL;
    newPtr->right = NULL;
    newPtr->type = FunCall;
    newPtr->valType = Unknown;

    return newPtr;
}

expr *addLeafExpr(varExpr *var)
{
    expr *newPtr = new expr;
    newPtr->attr = var;
    newPtr->left = NULL;
    newPtr->right = NULL;
    newPtr->type = Leaf;
    newPtr->valType = Variable;

    return newPtr;
}

exprList *createExprList(expr *exp)
{
    exprList *newexprnode = new exprList;
    newexprnode->exp = exp;
    newexprnode->next = NULL;
    return newexprnode;
}

exprList *createExprList(expr *exp, exprList *exprlist)
{
    exprList *newexprnode = new exprList;
    newexprnode->exp = exp;
    newexprnode->next = exprlist;
    return newexprnode;
}

types opType(char sym)
{
    switch (sym)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '<':
    case '>':
    case 'g':
    case 'l':
    case 'n':
    case 'e':
        return Int;
    case '!':
    case '&':
    case '|':
        return Bool;
    default:
        cerr << "Invalid Operator";
        isError = false;
    }
    return Unknown;
}

expr *addNonLeafExpr(expr *left, char opSym, expr *right)
{
    opSymbol *a = new opSymbol;
    a->type = opType(opSym);
    a->sym = opSym;

    expr *newPtr = new expr;
    newPtr->attr = a;
    newPtr->left = left;
    newPtr->right = right;
    newPtr->type = NonLeaf;
    newPtr->valType = Unknown;

    return newPtr;
}

int sizeOfExprList(exprList *start)
{
    int i = 0;
    while (start != NULL)
    {
        i += 1;
        start = start->next;
    }
    return i;
}

indexList *exprListToindexList(exprList *expList, map<string, value *> local_sym_tab)
{
    indexList *start = NULL, *end = NULL;
    result *res;
    int size = sizeOfExprList(expList);

    for (int i = 0; i < size; i++)
    {
        if (start == NULL)
        {

            start = new indexList;
            start->next = NULL;
            end = start;
            res = solve(expList->exp, local_sym_tab);
            if (res->type == Bool)
            {
                cerr << "Invalid expression type in the "
                     << "i+1 "
                     << "th index. The index expression should be of INTERGER type." << endl;
                isError = false;
            }
            else if (res->type == Int)
            {
                start->i = *(int *)(res->value);
            }
            delete res;
        }
        else
        {
            end->next = new indexList;
            end = end->next;
            end->next = NULL;
            res = solve(expList->exp, local_sym_tab);

            if (res->type == Bool)
            {
                cerr << "Invalid expression type in the "
                     << "i+1 "
                     << "th index. The index expression should be of INTERGER type." << endl;
                isError = false;
                delete ((bool *)(res->value));
            }
            else if (res->type == Int)
            {
                end->i = *(int *)(res->value);
                delete ((int *)(res->value));
            }
            delete res;
        }
        expList = expList->next;
    }
    return start;
}

result *solve(expr *node, map<string, value *> local_sym_tab)
{
    result *a = new result;
    if (node->type == Leaf)
    {
        if (node->valType == Int)
        {
            a->type = ((intExpr *)(node->attr))->type;
            a->value = &((intExpr *)(node->attr))->val;
        }
        else if (node->valType == Bool)
        {
            a->type = ((boolExpr *)(node->attr))->type;
            a->value = &((boolExpr *)(node->attr))->val;
        }
        else if (node->valType == Variable)
        {
            varExpr *varE = (varExpr *)(node->attr);
            if (!(varE->isArray))
            {
                result *val = lookup(varE->varName, local_sym_tab);
                a = val;
            }
            else
            {
                indexList *indexlist = exprListToindexList(varE->startIndex, local_sym_tab);

                result *val = lookup(varE->varName, indexlist, local_sym_tab);
                a = val;
            }
        }
        else
        {
            cerr << "invalid Type for a Leaf";
        }
    }
    else if (node->type == FunCall)
    {
        func_call *funCall = (func_call *)(node->attr);
        if (func_table.count(funCall->funName) > 0)
        {
            funcDetails *funcdetails = func_table[funCall->funName];
            if (!(funcdetails->isDefSet && funcdetails != NULL))
            {
                cerr << "Function \"" << funCall->funName << "\" definition not found.>" << endl;
                isError = false;
            }

            a->type = funcdetails->declType;
            if (a->type == Int)
                a->value = new int(1);
            else
                a->value = new bool(false);
        }
        else
        {
            cerr << "Function \"" << funCall->funName << "\" not declared." << endl;
            isError = false;
            a->type = Int;
            a->value = new int(1);
        }
    }
    else if (node->type == NonLeaf)
    {
        if (((opSymbol *)(node->attr))->type == Bool)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            case '&':
                a->value = new bool((*(bool *)(solve(node->left, local_sym_tab)->value)) & (*(bool *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            case '|':
                a->value = new bool((*(bool *)(solve(node->left, local_sym_tab)->value)) | (*(bool *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            case '!':
                a->value = new bool(!(*(bool *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            default:
                cerr << "Invalid operator. Operator not compatible with operand.";
                isError = false;
            }
        }
        else if (((opSymbol *)(node->attr))->type == Int)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            // operations that yield int itself
            case '+':
                a->value = new int((*(int *)(solve(node->left, local_sym_tab)->value)) + (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Int;
                break;
            case '-':
                a->value = new int((*(int *)(solve(node->left, local_sym_tab)->value)) - (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Int;
                break;
            case '/':
                a->value = new int((*(int *)(solve(node->left, local_sym_tab)->value)) / (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Int;
                break;
            case '*':
                a->value = new int((*(int *)(solve(node->left, local_sym_tab)->value)) * (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Int;
                break;
            case '%':
                a->value = new int((*(int *)(solve(node->left, local_sym_tab)->value)) % (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Int;
                break;

            // operation that yeild bool
            case '>':
                a->value = new bool((*(int *)(solve(node->left, local_sym_tab)->value)) > (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            case '<':
                a->value = new bool((*(int *)(solve(node->left, local_sym_tab)->value)) < (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            case 'g':
                a->value = new bool((*(int *)(solve(node->left, local_sym_tab)->value)) >= (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            case 'l':
                a->value = new bool((*(int *)(solve(node->left, local_sym_tab)->value)) <= (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            case 'n':
                a->value = new bool((*(int *)(solve(node->left, local_sym_tab)->value)) != (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            case 'e':
                a->value = new bool((*(int *)(solve(node->left, local_sym_tab)->value)) == (*(int *)(solve(node->right, local_sym_tab)->value)));
                a->type = Bool;
                break;
            default:
                cerr << "Invalid operator. Operator not compatible with operand.";
                isError = false;
            }
        }
        else
        {
            cerr << "Invalid Expr Node detected";
            isError = false;
        }
    }
    return a;
}
void printExprList(exprList *start)
{
    while (start != NULL)
    {
        cout << "[ ";
        printExpr(start->exp);
        cout << "]";
        start = start->next;
    }
}

void printIndexList(indexList *start)
{
    while (start != NULL)
    {
        cout << "[ " << start->i << "]";
        start = start->next;
    }
}

void printExpr(expr *node)
{
    result *a = new result;

    if (node->type == Leaf)
    {
        if (node->valType == Int)
        {
            cout << "NUM ";
        }
        else if (node->valType == Bool)
        {
            cout << "BOOL ";
        }
        else if (node->valType == Variable)
        {
            varExpr *varE = (varExpr *)(node->attr);
            if (!(varE->isArray))
            {
                cout << "VAR ";
            }
            else
            {
                cout << "ARRREF VAR ";
                printExprList(varE->startIndex);
            }
        }
        else
        {
            cout << "invalid Type for a Leaf";
            exit(0);
        }
    }
    else if (node->type == NonLeaf)
    {
        if (((opSymbol *)(node->attr))->type == Bool)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            case '&':
                cout << "LOGICAL_AND ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case '|':
                cout << "LOGICAL_OR ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case '!':
                cout << "LOGICAL_NOT ";
                printExpr(node->right);
                break;
            default:
                cout << "Invalid operator";
                exit(1);
            }
        }
        else if (((opSymbol *)(node->attr))->type == Int)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            // operations that yield int itself
            case '+':
                cout << "PLUS ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case '-':
                cout << "SUB ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case '/':
                cout << "DIV ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case '*':
                cout << "MUL ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case '%':
                cout << "MOD ";
                printExpr(node->left);
                printExpr(node->right);
                break;

            // operation that yeild bool
            case '>':
                cout << "MORE ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case '<':
                cout << "LESS ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case 'g':
                cout << "GREATERTHANOREQUAL ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case 'l':
                cout << "LESSTHANOREQUAL ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case 'n':
                cout << "NOTEQUAL ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            case 'e':
                cout << "EQUALEQUAL ";
                printExpr(node->left);
                printExpr(node->right);
                break;
            default:
                cout << "Invalid operator";
                exit(1);
            }
        }
        else
        {
            cout << "Invalid Expr Node detected";
            exit(0);
        }
    }
}

// Symbol table functions
void addVar(char *name, types type, map<string, value *> &sym_tab)
{
    if (sym_tab.count(name) > 0)
    {
        cerr << "Multiple declaration of the variable " << name << endl;
        isError = false;
    }
    else
    {
        value *v = createValue(name, type);
        v->type = type;
        sym_tab[name] = v;
    }
}

void addVar(char *name, types type, indexList *startIndex, map<string, value *> &sym_tab)
{
    if (sym_tab.count(name) > 0)
    {
        cerr << "Multiple declaration of the variable " << name << endl;
        isError = false;
    }
    else
    {
        value *v = createValue(name, type, startIndex);
        v->type = type;
        sym_tab[name] = v;
    }
}

result *lookup(char *name, map<string, value *> &local_sym_tab)
{
    map<string, value *> *sym_tab;
    if (local_sym_tab.count(name) > 0)
    {
        sym_tab = &local_sym_tab;
    }
    else if (symbol_table.count(name) > 0)
    {
        sym_tab = &symbol_table;
    }
    else
    {
        cerr << "Undeclared Variable " << name << endl;
        isError = false;
        return NULL;
    }

    value *v = (*sym_tab)[name];
    result *res = new result;
    if (v->type == Int)
    {
        if (v->isArray)
        {
            cerr << name << " is an array. Its value cannot be used without index." << endl;
            isError = false;
        }
        else
        {
            res->type = Int;
            res->value = (int *)(v->val);
            return res;
        }
    }
    else if (v->type == Bool)
    {
        if (v->isArray)
        {
            cerr << name << " is an array. Its value cannot be used without index." << endl;
            isError = false;
        }
        else
        {
            res->type = Bool;
            res->value = (bool *)(v->val);
            return res;
        }
    }
    else
    {
        cerr << "Invalid type." << endl;
        isError = false;
    }
    return NULL;
}

int arrayDim(indexList *startIndex)
{
    int dim = 0;
    while (startIndex != NULL)
    {
        dim++;
        startIndex = startIndex->next;
    };

    return dim;
}

int totalArraySize(indexList *startIndex)
{
    int size = 1;
    indexList *start = startIndex;
    if (startIndex == NULL)
    {
        return 0;
    }
    else
    {
        while (start != NULL)
        {
            if (start->i <= 0)
            {
                cerr << "Invalid array index" << endl;
                isError = false;
            }
            size *= start->i;
            start = start->next;
        };
    }
    return size;
}

bool arrayRangeCheck(indexList *declIndex, indexList *index)
{
    if (declIndex == NULL || index == NULL)
    {
        return false;
    }
    else if (arrayDim(declIndex) != arrayDim(index))
    {
        cerr << "Array dimension mismatch." << endl;
        isError = false;
        return false;
    }

    int size = arrayDim(declIndex);
    for (int i = 0; i < size; i++)
    {
        if (declIndex->i <= index->i || index->i < 0)
        {
            cerr << i + 1 << " th index out of range" << endl;
            isError = false;
            return false;
        }

        declIndex = declIndex->next;
        index = index->next;
    }
    return true;
}

int convertnDto1D(indexList *declIndex, indexList *actualIndex)
{
    int index = 0;
    int size = arrayDim(declIndex);
    if (arrayRangeCheck(declIndex, actualIndex))
    {
        for (int i = 0; i < size - 1; i++)
        {
            index += (actualIndex->i) * (totalArraySize(declIndex->next));
            actualIndex = actualIndex->next;
            declIndex = declIndex->next;
        }
        index += actualIndex->i;
        return index;
    }
    else
    {
        isError = false;
    }
    return 0;
}

result *lookup(char *name, indexList *startIndex, map<string, value *> &local_sym_tab)
{
    map<string, value *> *sym_tab;
    if (local_sym_tab.count(name) > 0)
    {
        sym_tab = &local_sym_tab;
    }
    else if (symbol_table.count(name) > 0)
    {
        sym_tab = &symbol_table;
    }
    else
    {
        cerr << "Undeclared Variable " << name << endl;
        isError = false;
        return NULL;
    }

    value *v = (*sym_tab)[name];
    result *res = new result;
    if (v->type == Int)
    {
        if (!(v->isArray))
        {
            cerr << name << " is not indexable. Variable not an array. " << endl;
            isError = false;
            return NULL;
        }
        else
        {
            arrayInfo *arr = (arrayInfo *)(v->val);
            int index = convertnDto1D(arr->indexLst, startIndex);

            res->type = v->type;
            res->value = &(((int *)(arr->arr))[index]);
            return res;
        }
    }
    else if (v->type == Bool)
    {
        if (!(v->isArray))
        {
            cerr << name << " is not indexable. Variable not an array. " << endl;
            isError = false;
            return NULL;
        }
        else
        {
            arrayInfo *arr = (arrayInfo *)(v->val);
            int index = convertnDto1D(arr->indexLst, startIndex);

            res->type = v->type;
            res->value = &(((bool *)(arr->arr))[index]);
            return res;
        }
    }
    else
    {
        cerr << "Invalid Type " << endl;
        isError = false;
    }
    return NULL;
}

void update(char *name, result *res, map<string, value *> &local_sym_tab)
{
    map<string, value *> *sym_tab;
    if (local_sym_tab.count(name) > 0)
    {
        sym_tab = &local_sym_tab;
    }
    else if (symbol_table.count(name) > 0)
    {
        sym_tab = &symbol_table;
    }
    else
    {
        cerr << "Undeclared Variable " << name << endl;
        isError = false;
        return;
    }

    value *v = (*sym_tab)[name];
    if (v->type == Int)
    {
        if (res->type == Int)
        {
            if (v->isArray)
            {
                cerr << name << " is an array. Its value cannot be assigned value without an index. " << endl;
                isError = false;
            }
            else
            {
                *((int *)(v->val)) = *(int *)(res->value);
            }
        }
        else
        {
            cerr << "Type Error. Variable \"" << name << "\" is integer type. " << endl;
            isError = false;
        }
    }
    else if (res->type == Bool)
    {
        if (res->type == Bool)
        {
            if (v->isArray)
            {
                cerr << name << " is an array. Its value cannot be assigned value without an index. " << endl;
                isError = false;
            }
            else
            {
                *((bool *)(v->val)) = *(bool *)(res->value);
            }
        }
        else
        {
            cerr << "Type Error. Variable \"" << name << "\" is boolean type. " << endl;
            isError = false;
        }
    }
    else
    {
        cerr << "Invalid type " << endl;
        isError = false;
    }
}

void update(char *name, indexList *startIndex, result *res, map<string, value *> &local_sym_tab)
{
    map<string, value *> *sym_tab;
    if (local_sym_tab.count(name) > 0)
    {
        sym_tab = &local_sym_tab;
    }
    else if (symbol_table.count(name) > 0)
    {
        sym_tab = &symbol_table;
    }
    else
    {
        cerr << "Undeclared Variable " << name << endl;
        isError = false;
        return;
    }

    value *v = (*sym_tab)[name];
    if (v->type == Int)
    {
        if (res->type == Int)
        {
            if (!(v->isArray))
            {
                cerr << name << " is not indexable. Variable not an array." << endl;
                isError = false;
            }
            else
            {
                arrayInfo *arr = (arrayInfo *)(v->val);
                int index = convertnDto1D(arr->indexLst, startIndex);
                ((int *)(arr->arr))[index] = *(int *)(res->value);
            }
        }
        else
        {
            cerr << "Type mismatch" << endl;
            isError = false;
        }
    }
    else if (res->type == Bool)
    {
        if (res->type == Bool)
        {
            if (!(v->isArray))
            {
                cerr << name << " is not indexable. Variable not an array." << endl;
                isError = false;
            }
            else
            {
                arrayInfo *arr = (arrayInfo *)(v->val);
                int index = convertnDto1D(arr->indexLst, startIndex);
                ((bool *)(arr->arr))[index] = *(bool *)(res->value);
            }
        }
        else
        {
            cerr << "Type mismatch" << endl;
            isError = false;
        }
    }
    else
    {
        cerr << "Invalid type " << endl;
        isError = false;
    }
}

// declaration helper functions

value *createValue(char *varName, types type)
{
    value *v = new value;
    v->type = type;
    if (type == Int)
        v->val = new int(0);
    else if (type == Bool)
        v->val = new bool(false);

    v->isArray = false;
    v->name = strndup(varName, strlen(varName));
    return v;
}

value *createValue(char *varName, types type, indexList *startIndex)
{
    value *v = new value;
    v->name = strndup(varName, strlen(varName));
    v->type = type;
    v->isArray = true;

    arrayInfo *a = new arrayInfo;
    if (type == Int)
    {
        a->arr = new int[totalArraySize(startIndex)]();
    }
    else if (type == Bool)
    {
        a->arr = new bool[totalArraySize(startIndex)]();
    }
    a->indexLst = startIndex;
    a->arrayDim = arrayDim(startIndex);
    v->val = a;

    return v;
}

declVarNode *createDeclNode(char *varName)
{
    declVarNode *newPtr = new declVarNode;
    newPtr->name = strndup(varName, strlen(varName));
    newPtr->startIndex = NULL;
    newPtr->endIndex = NULL;
    newPtr->isArray = false;
    return newPtr;
}

declVarNode *createDeclNode(declVarNode *node, int size)
{
    indexList *a = new indexList;
    a->i = size;
    a->next = NULL;
    if (node->startIndex == NULL)
    {
        node->startIndex = a;
        node->endIndex = a;
    }
    else
    {
        node->endIndex->next = a;
        node->endIndex = a;
    }
    node->isArray = true;

    return node;
}

Glist *createGlist(declVarNode *node)
{
    Glist *newGlist = new Glist;
    newGlist->next = NULL;
    newGlist->isGid = true;
    newGlist->node = node;
    return newGlist;
}

Glist *createGlist(declVarNode *node, Glist *glist)
{
    Glist *newGlist = new Glist;
    newGlist->next = glist;
    newGlist->isGid = true;
    newGlist->node = node;
    return newGlist;
}

Glist *createGlist(func *node)
{
    Glist *newGlist = new Glist;
    newGlist->next = NULL;
    newGlist->isGid = false;
    newGlist->node = node;
    return newGlist;
}

Glist *createGlist(func *node, Glist *glist)
{
    Glist *newGlist = new Glist;
    newGlist->next = glist;
    newGlist->isGid = false;
    newGlist->node = node;
    return newGlist;
}

var_list *createVarList(char *name)
{
    var_list *newPtr = new var_list;
    newPtr->name = strndup(name, strlen(name));
    newPtr->next = NULL;
    return newPtr;
}

var_list *createVarList(char *name, var_list *varList)
{
    var_list *newPtr = new var_list;
    newPtr->name = strndup(name, strlen(name));
    newPtr->next = varList;
    return newPtr;
}

arg *createArg(types type, var_list *varList)
{
    arg *newArg = new arg;
    newArg->type = type;
    newArg->varList = varList;
    return newArg;
}

arg_list *createArgList(arg *Arg)
{
    arg_list *argList = new arg_list;
    argList->Arg = Arg;
    argList->next = NULL;
    return argList;
}

arg_list *createArgList(arg *Arg, arg_list *argList)
{
    arg_list *newArgList = new arg_list;
    newArgList->Arg = Arg;
    newArgList->next = argList;
    return newArgList;
}

func *createFunc(char *name, arg_list *argList)
{
    func *newFunc = new func;
    newFunc->name = strndup(name, strlen(name));
    newFunc->argList = argList;
    return newFunc;
}

Fdef *createFdef(types ret_type, char *name, arg_list *argList, declaration *local_decl, statement *stmt_list, expr *ret_stmt)
{
    Fdef *newFdef = new Fdef;
    newFdef->retType = ret_type;
    newFdef->funcName = strndup(name, strlen(name));
    newFdef->FargList = argList;
    newFdef->LdeclSec = local_decl;
    newFdef->stmtList = stmt_list;
    newFdef->retStmt = ret_stmt;
    newFdef->next = NULL;
    return newFdef;
}

Fdef *createFdefSec(Fdef *fdef, Fdef *fdefSec)
{
    fdef->next = fdefSec;
    return fdef;
}

declaration *createDeclaration(types type, Glist *top)
{
    declaration *newPtr = new declaration;

    newPtr->type = type;
    newPtr->list = top;
    newPtr->next = NULL;

    return newPtr;
}

declaration *createDeclarationList(declaration *node, declaration *list)
{
    node->next = list;
    return node;
}

void addGlobalDeclarationBlock(declaration *root)
{
    prog.global_declarations = root;
}

void printDeclVarNodes(declVarNode *list)
{
    declVarNode *ptr = list;
    if (ptr == NULL)
    {
        cerr << "From printDeclNodes, we shouldn't be here" << endl;
        exit(0);
    }
    if (ptr->isArray)
    {
        cout << "ARR VAR ";
        printIndexList(ptr->startIndex);
    }
    else
    {
        cout << "VAR";
    }
}

void printVarList(var_list *varList)
{
    while (varList != NULL)
    {
        cout << "VAR";
        varList = varList->next;
        if (varList != NULL)
        {
            cout << ", ";
        }
    }
}

void printType(types type)
{
    if (type == Int)
    {
        cout << "integer";
    }
    else if (type == Bool)
    {
        cout << "boolean";
    }
}

void printArg(arg *Arg)
{
    if (Arg->type == Int)
    {
        cout << "INT";
    }
    else if (Arg->type == Bool)
    {
        cout << "BOOL";
    }
    cout << " ";
    printVarList(Arg->varList);
}

void printArgList(arg_list *argList)
{
    while (argList != NULL)
    {
        printArg(argList->Arg);
        argList = argList->next;
        if (argList != NULL)
        {
            cout << "; ";
        }
    }
}

void printFunc(func *Func)
{
    cout << "FUNCVAR ( ";
    printArgList(Func->argList);
    cout << ")";
}

void printGlist(Glist *glist)
{
    while (glist != NULL)
    {
        if (glist->isGid)
        {
            printDeclVarNodes((declVarNode *)(glist->node));
        }
        else
        {
            printFunc((func *)(glist->node));
        }
        glist = glist->next;
        if (glist != NULL)
            cout << ", ";
    }
}

void printDeclstmt(declaration *root)
{
    cout << "DECL ";
    if (root->type == Int)
    {
        cout << "INT ";
    }
    else if (root->type == Bool)
    {
        cout << "BOOL ";
    }
    else
    {
        cout << "invalid type ";
    }

    if (root->list->isGid)
    {
        printGlist(root->list);
    }
    else
    {
        cout << "declaration pending";
    }
    cout << endl;
}

void printGlobalDecl(declaration *root)
{
    declaration *ptr = root;
    while (ptr != NULL)
    {
        printDeclstmt(ptr);
        ptr = ptr->next;
    }
    if (root != NULL)
        cout << endl;
}

// assignment statement helper function

statement *createAssignStmt(varExpr *var, expr *exp)
{
    assign_stmt *newPtr = new assign_stmt;
    newPtr->var = var;
    newPtr->exp = exp;

    statement *newStmt = new statement;
    newStmt->stmt = newPtr;
    newStmt->stmtType = assign;
    newStmt->next = NULL;
    return newStmt;
}

void printAssignStmt(assign_stmt *stmt)
{
    cout << "ASSIGN ";
    if (stmt->var->isArray)
    {
        cout << "ARRREF VAR ";

        printExprList(stmt->var->startIndex);
    }
    else
    {
        cout << "VAR ";
    }

    printExpr(stmt->exp);
}
///----------------------------------------------///
str_expr *createStrExpr(char *name)
{
    str_expr *newPtr = new str_expr;
    newPtr->var = strndup((name), strlen(name));
    newPtr->next = NULL;
    return newPtr;
}

str_expr *createStrExpr(char *name, str_expr *strExp)
{
    str_expr *newPtr = new str_expr;
    newPtr->var = strndup((name), strlen(name));
    newPtr->next = strExp;
    return newPtr;
}

void printStrExprList(str_expr *list)
{
    if (list == NULL)
    {
        cerr << "List was NULL\n";
    }
    while (list != NULL)
    {
        if (list->var == NULL)
        {
            cerr << "string is null";
        }
        cout << list->var << " ";
        list = list->next;
    }
    cout << endl;
}

// write statement helper function
statement *createWriteStmt(expr *expr)
{
    write_stmt *newPtr = new write_stmt;
    newPtr->val = expr;
    newPtr->isExpr = true;

    statement *newStmt = new statement;
    newStmt->stmt = newPtr;
    newStmt->stmtType = write;
    newStmt->next = NULL;
    return newStmt;
}

statement *createReadStmt(varExpr *varexp)
{
    read_stmt *newPtr = new read_stmt;
    newPtr->varexp = varexp;

    statement *newStmt = new statement{newPtr, read, NULL};
    return newStmt;
}

statement *createWriteStmt(str_expr *strExpr)
{
    write_stmt *newPtr = new write_stmt;
    newPtr->val = strExpr;
    newPtr->isExpr = false;

    statement *newStmt = new statement;
    newStmt->stmt = newPtr;
    newStmt->stmtType = write;
    newStmt->next = NULL;
    return newStmt;
}

void printWriteStmt(write_stmt *stmt)
{
    cout << "FUNC ";
    if (stmt->isExpr)
    {
        printExpr((expr *)(stmt->val));
    }
    else
    {
        printStrExprList((str_expr *)(stmt->val));
    }
}

void printArgExprList(exprList *exprlist)
{
    for (exprList *list = exprlist; list != NULL; list = list->next)
    {
        cout << "VAR ";
        if (list->next != NULL)
            cout << ", ";
    }
}

void printFunCall(func_call *stmt)
{
    cout << "FUNCVAR ( ";
    printArgExprList(stmt->exprlist);
    cout << ")";
}

void printReadStmt(read_stmt *stmt)
{
    cout << "READ ";
    if (stmt->varexp->isArray)
    {
        cout << "ARRREF VAR ";

        printExprList(stmt->varexp->startIndex);
    }
    else
    {
        cout << "VAR ";
    }
}

// helper functions for statements

statement *createStmtList(statement *stmt, statement *stmtList)
{
    stmt->next = stmtList;
    return stmt;
}

void printStmt(statement *stmt)
{
    if (stmt->stmtType == write)
    {
        printWriteStmt((write_stmt *)(stmt->stmt));
    }
    else if (stmt->stmtType == assign)
    {
        printAssignStmt((assign_stmt *)(stmt->stmt));
    }
    else if (stmt->stmtType == cond)
    {
        printCondStmt((cond_stmt *)(stmt->stmt));
    }
    else if (stmt->stmtType == funccall)
    {
        printFunCall((func_call *)(stmt->stmt));
    }
    else if (stmt->stmtType == read)
    {
        printReadStmt((read_stmt *)(stmt->stmt));
    }
    else
    {
        cout << "invalid statement type" << endl;
    }
}

void printStmtList(statement *stmt)
{

    while (stmt != NULL)
    {
        printStmt(stmt);
        stmt = stmt->next;
        cout << endl;
    }
}

// helper functions for conditional statementss

statement *createIfStmt(expr *cond_expr, statement *stmt_list)
{
    if_stmt *newIf = new if_stmt;
    newIf->cond = cond_expr;
    newIf->stmtList = stmt_list;

    cond_stmt *newCondStmt = new cond_stmt;
    newCondStmt->cond_stmt_attrs = newIf;
    newCondStmt->type = If;

    statement *newStmt = new statement;
    newStmt->stmt = newCondStmt;
    newStmt->stmtType = cond;
    newStmt->next = NULL;
    return newStmt;
}

statement *createIfElseStmt(expr *cond_expr, statement *if_stmt_list, statement *else_stmt_list)
{
    if_else_stmt *newIfElse = new if_else_stmt;
    newIfElse->cond = cond_expr;
    newIfElse->if_stmtList = if_stmt_list;
    newIfElse->else_stmtList = else_stmt_list;

    cond_stmt *newCondStmt = new cond_stmt;
    newCondStmt->cond_stmt_attrs = newIfElse;
    newCondStmt->type = IfElse;

    statement *newStmt = new statement;
    newStmt->stmt = newCondStmt;
    newStmt->stmtType = cond;
    newStmt->next = NULL;
    return newStmt;
}

statement *createWhileStmt(expr *cond_expr, statement *stmt_list)
{
    while_stmt *newWhile = new while_stmt;
    newWhile->cond = cond_expr;
    newWhile->stmtList = stmt_list;

    cond_stmt *newCondStmt = new cond_stmt;
    newCondStmt->cond_stmt_attrs = newWhile;
    newCondStmt->type = While;

    statement *newStmt = new statement;
    newStmt->stmt = newCondStmt;
    newStmt->stmtType = cond;
    newStmt->next = NULL;
    return newStmt;
}

statement *createForStmt(statement *startAssign, expr *condition, statement *updateAssign, statement *stmtList)
{
    for_loop *newForLoop = new for_loop{(assign_stmt *)(startAssign->stmt), condition, (assign_stmt *)(updateAssign->stmt), stmtList};

    cond_stmt *newCondStmt = new cond_stmt{For, newForLoop};

    statement *newStmt = new statement{newCondStmt, cond, NULL};
    return newStmt;
}

void printIfStmt(if_stmt *stmt)
{
    cout << "\nIF ";
    printExpr(stmt->cond);
    cout << endl
         << " THEN" << endl;
    printStmtList(stmt->stmtList);
    cout << "ENDIF" << endl;
}

void printIfElseStmt(if_else_stmt *stmt)
{
    cout << "\nIF ";
    printExpr(stmt->cond);
    cout << " THEN " << endl;
    printStmtList(stmt->if_stmtList);
    cout << "ELSE" << endl;
    printStmtList(stmt->else_stmtList);
    cout << "ENDIF" << endl;
}

void printWhileStmt(while_stmt *stmt)
{
    cout << "\nWHILE ";
    printExpr(stmt->cond);
    cout << " DO " << endl;
    printStmtList(stmt->stmtList);
    cout << "ENDWHILE" << endl;
}

void printFor_loop(for_loop *stmt)
{
    cout << "FOR ";
    printAssignStmt(stmt->startAssign);
    cout << ";";
    printExpr(stmt->cond);
    cout << ";";
    printAssignStmt(stmt->updateAssign);
    cout << endl;

    printStmtList(stmt->stmtList);
    cout << "END FOR" << endl;
}

void printCondStmt(cond_stmt *stmt)
{
    if (stmt->type == If)
    {
        printIfStmt((if_stmt *)(stmt->cond_stmt_attrs));
    }
    else if (stmt->type == IfElse)
    {
        printIfElseStmt((if_else_stmt *)(stmt->cond_stmt_attrs));
    }
    else if (stmt->type == While)
    {
        printWhileStmt((while_stmt *)(stmt->cond_stmt_attrs));
    }
    else
    {
        printFor_loop((for_loop *)(stmt->cond_stmt_attrs));
    }
}

func_call *createfunc_call(char *name, exprList *exprlist)
{
    func_call *newFuncall = new func_call{name, exprlist};
    return newFuncall;
}

statement *createFuncStmt(func_call *funcCall)
{
    statement *newStmt = new statement{funcCall, funccall, NULL};
    return newStmt;
}

// main block functions

main_block *createMainBlock(types ret_type, declaration *local_decl, statement *stmt_list, expr *ret_stmt)
{
    main_block *mainBlock = new main_block{ret_type, local_decl, stmt_list, ret_stmt};
    return mainBlock;
}

void addMainBlock(main_block *mainBlock)
{
    prog.mainBlock = mainBlock;
}
void addFdef_sec(Fdef *fdefSec)
{
    prog.fdef_sec = fdefSec;
}

// function for execution of code
bool compare_argList(arg_list *argList1, arg_list *argList2);

void addFunctionDef(Fdef *fdef)
{
    if (func_table.count(fdef->funcName) <= 0)
    {
        cerr << "No declaration of the function " << fdef->funcName << endl;
        isError = false;
    }
    else
    {
        funcDetails *newFunDetails = func_table[fdef->funcName];
        if (newFunDetails->isDefSet)
        {
            cerr << "Multiple definition of function \"" << newFunDetails->Func->name << "\"." << endl;
            isError = false;
        }
        else
        {
            if (!compare_argList(fdef->FargList, newFunDetails->Func->argList))
            {
                isError = false;
            }

            if (fdef->retType != newFunDetails->declType)
            {
                cerr << "Function \"" << fdef->funcName << "\" have different return type at declaration and definition." << endl;
            }

            newFunDetails->fdef = fdef;
            newFunDetails->isDefSet = true;
        }
    }
}

void addFunctionDefSec()
{
    Fdef *fdef = prog.fdef_sec;
    while (fdef != NULL)
    {
        addFunctionDef(fdef);
        fdef = fdef->next;
    }
}

void insertFunctionDecl(func *f, types type)
{
    if (func_table.count(f->name) > 0)
    {
        cerr << "Multiple declaration of the function " << f->name << endl;
        isError = false;
    }
    else
    {
        funcDetails *newFunDetails = new funcDetails;
        newFunDetails->declType = type;
        newFunDetails->Func = f;
        newFunDetails->fdef = NULL;
        newFunDetails->isDefSet = false;
        func_table[f->name] = newFunDetails;
    }
}

void addDeclaration(map<string, value *> &sym_tab, declaration *decl)
{
    Glist *glist = decl->list;
    while (glist != NULL)
    {
        if (glist->isGid)
        {
            declVarNode *node = (declVarNode *)(glist->node);
            if (!(node->isArray))
            {
                addVar(node->name, decl->type, sym_tab);
            }
            else
            {
                addVar(node->name, decl->type, node->startIndex, sym_tab);
            }
        }
        else
        {
            func *node = (func *)(glist->node);
            insertFunctionDecl(node, decl->type);
        }
        glist = glist->next;
    }
}

void addDeclarationList(map<string, value *> &sym_tab, declaration *decl_list)
{
    while (decl_list != NULL)
    {
        addDeclaration(sym_tab, decl_list);
        decl_list = decl_list->next;
    }
}

void printBool(bool b)
{
    if (b)
    {
        cout << "true ";
    }
    else
    {
        cout << "false ";
    }
}

void printBoolArray(bool *lst, int size)
{
    for (int i = 0; i < size; i++)
    {
        printBool(lst[i]);
    }
}

void printIntArray(int *lst, int size)
{
    for (int i = 0; i < size; i++)
    {
        cout << lst[i] << " ";
    }
}

int len_varList(var_list *list1)
{
    int i1 = 0;
    for (var_list *list = list1; list != NULL; list = list->next)
    {
        i1++;
    }
    return i1;
}

bool compare_arg(arg *arg1, arg *arg2)
{
    bool truth = true;
    if (arg1->type != arg2->type)
    {
        cerr << "Argument type mismatch." << endl;
        truth = false;
    }
    if (len_varList(arg1->varList) != len_varList(arg2->varList))
    {
        cerr << "Arguments count mismatch." << endl;
        return false;
    }

    // argument name should be same,

    for (var_list *l1 = arg1->varList, *l2 = arg2->varList; l1 != NULL && l2 != NULL; l1 = l1->next, l2 = l2->next)
    {
        if (strcmp(l1->name, l2->name) != 0)
        {
            cerr << "Argument mismatch: " << l1->name << " and " << l2->name << "." << endl;
            truth = false;
        }
    }

    return truth;
}

int len_arg_list(arg_list *argList1)
{
    int i = 0;
    for (arg_list *list = argList1; list != NULL; list = list->next)
    {
        i++;
    }
    return i;
}

bool compare_argList(arg_list *argList1, arg_list *argList2) // function that compares the argument list in the function definition and declaration
{
    bool truth = true;
    if (len_arg_list(argList1) != len_arg_list(argList2))
    {
        cerr << "Argument list count mismatch" << endl;
        return false;
    }

    for (arg_list *arL1 = argList1, *arL2 = argList2; arL1 != NULL && arL2 != NULL; arL1 = arL1->next, arL2 = arL2->next)
    {
        if (!compare_arg(arL1->Arg, arL2->Arg))
        {
            truth = false;
        }
    }
    return truth;
}

bool isPresent(char *name, map<string, value *> sym_tab)
{
    if (sym_tab.count(name) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void print_result_list(result_list *reslist)
{
    for (result_list *list = reslist; list != NULL; list = list->next)
    {
        printResult(list->res);
        cout << ", ";
    }
    cout << endl;
}

bool insertArg(arg *Arg, map<string, value *> &sym_tab)
{
    bool truth = true;
    for (var_list *list = Arg->varList; list != NULL; list = list->next)
    {
        if (isPresent(list->name, sym_tab))
        {
            cerr << "Duplicate arguments '" << list->name << "' in function definition." << endl;
            truth = false;
        }
        else
        {
            addVar(list->name, Arg->type, sym_tab);
        }
    }
    return truth;
}

bool insertArgList(arg_list *argList, map<string, value *> &sym_tab)
{
    for (arg_list *list = argList; list != NULL; list = list->next)
    {
        if (!insertArg(list->Arg, sym_tab))
        {
            return false;
        }
    }
    return true;
}

result_list *exprList_to_resultList(exprList *explist, map<string, value *> local_sym_tab)
{
    result_list *start = NULL, *end = NULL;
    for (exprList *list = explist; list != NULL; list = list->next)
    {
        if (start == NULL)
        {
            start = new result_list{solve(list->exp, local_sym_tab), NULL};
            end = start;
        }
        else
        {
            end->next = new result_list{solve(list->exp, local_sym_tab), NULL};
            end = end->next;
        }
    }
    return start;
}

int resutltList_length(result_list *resultList)
{
    int i = 0;
    for (result_list *list = resultList; list != NULL; list = list->next)
    {
        i++;
    }
    return i;
}

int total_formal_arguments(arg_list *arglist)
{
    int i = 0;
    for (arg_list *list = arglist; list != NULL; list = list->next)
    {
        i += len_varList(list->Arg->varList);
    }
    return i;
}

bool compare_actual_formal_param(result_list *actualParams, arg_list *arglist, map<string, value *> local_sym_tab)
{
    int len = resutltList_length(actualParams);
    if (len != total_formal_arguments(arglist))
    {
        cout << "Formal and actual parameter count mismatch" << endl;
        return false;
    }

    result_list *start = actualParams;
    for (arg_list *arglst = arglist; arglst != NULL; arglst = arglst->next)
    {
        int varListlen = len_varList(arglst->Arg->varList);
        for (int i = 0; i < varListlen; i++)
        {
            if (start->res->type != arglst->Arg->type)
            {
                cout << "Formal and actual parameter type mismatch" << endl;
                return false;
            }
            start = start->next;
        }
    }

    return true;
}

void printTypeForAST(types type)
{
    if (type == Int)
    {
        cout << "INT";
    }
    else if (type == Bool)
    {
        cout << "BOOL";
    }
    else
    {
        cout << "Invalid Type" << endl;
        exit(0);
    }
}

void printMainBlock()
{
    main_block *mainBlock = prog.mainBlock;

    cout << "FUN ";

    printTypeForAST(mainBlock->return_type);

    cout << "MAIN " << endl;

    printGlobalDecl(mainBlock->local_declaration);

    printStmtList(mainBlock->stmt_list);

    cout << "RET ";

    printExpr(mainBlock->return_stmt);
    cout << endl;

    cout << "END MAIN" << endl;
}
void printFdef(Fdef *fdef)
{
    cout << "FUN ";
    printTypeForAST(fdef->retType);
    cout << " ";
    cout << fdef->funcName << " ";
    cout << "(";
    printArgList(fdef->FargList);
    cout << ")" << endl;

    printGlobalDecl(fdef->LdeclSec);
    printStmtList(fdef->stmtList);

    cout << "RET ";

    printExpr(fdef->retStmt);

    cout << endl;

    cout << "END " << fdef->funcName << endl
         << endl;
}

void printFDefSec()
{
    Fdef *fdef = prog.fdef_sec;
    while (fdef != NULL)
    {
        printFdef(fdef);
        fdef = fdef->next;
    }
}

void PrintProgram()
{
    cout << "----------------------------" << endl;
    cout << "   Abstract Syntax Tree           \n";
    cout << "----------------------------" << endl;
    printGlobalDecl(prog.global_declarations);
    printFDefSec();
    printMainBlock();
    cout << "----------------------------" << endl;
    cout << "    C code Generation             \n";
    cout << "----------------------------" << endl;
}

// some printing and test functions
void printResult(result *val)
{
    if (val->type == Int)
    {
        cout << *(int *)(val->value);
    }
    else if (val->type == Bool)
    {
        cout << *(bool *)(val->value);
    }
}

//------------c code generation---------------------------------
void generate_c_type(types type)
{
    if (type == Int)
    {
        cout << "int ";
    }
    else if (type == Bool)
    {
        cout << "bool ";
    }
}

void generate_c_VarList(var_list *varList, types type)
{
    while (varList != NULL)
    {
        generate_c_type(type);

        cout << varList->name;
        varList = varList->next;
        if (varList != NULL)
        {
            cout << ", ";
        }
    }
}

void generate_c_Arg(arg *Arg)
{
    generate_c_VarList(Arg->varList, Arg->type);
}

void generate_c_ArgList(arg_list *argList)
{
    while (argList != NULL)
    {
        generate_c_Arg(argList->Arg);
        argList = argList->next;
        if (argList != NULL)
        {
            cout << ", ";
        }
    }
}

void generate_c_IndexList(indexList *start)
{
    while (start != NULL)
    {
        cout << "[ " << start->i << "]";
        start = start->next;
    }
}

void generate_c_DeclVarNodes(declVarNode *list)
{
    declVarNode *ptr = list;
    if (ptr == NULL)
    {
        cout << "From printDeclNodes, we shouldn't be here" << endl;
        exit(0);
    }
    if (ptr->isArray)
    {
        cout << ptr->name;
        generate_c_IndexList(ptr->startIndex);
    }
    else
    {
        cout << ptr->name;
    }
}

void generate_c_Func(func *Func)
{
    cout << Func->name << " (";
    generate_c_ArgList(Func->argList);
    cout << ")";
}

void generate_c_Glist(Glist *glist)
{
    while (glist != NULL)
    {
        if (glist->isGid)
        {
            generate_c_DeclVarNodes((declVarNode *)(glist->node));
        }
        else
        {
            generate_c_Func((func *)(glist->node));
        }
        glist = glist->next;
        if (glist != NULL)
            cout << ", ";
    }
}

void generate_c_declaration(declaration *decl)
{
    generate_c_type(decl->type);

    generate_c_Glist(decl->list);

    cout << ";";
    cout << endl;
}

void generate_c_declaration_list(declaration *decl_list)
{
    for (declaration *start = decl_list; start != NULL; start = start->next)
    {
        generate_c_declaration(start);
    }
}

void generate_c_Expr(expr *node);

void generate_c_ExprIndexList(exprList *start)
{
    while (start != NULL)
    {
        cout << "[ ";
        generate_c_Expr(start->exp);
        cout << "]";
        start = start->next;
    }
}

void generate_c_FunCall(func_call *stmt, int flag = 1);

void generate_c_Expr(expr *node)
{
    result *a = new result;

    if (node->type == Leaf)
    {
        if (node->valType == Int)
        {
            cout << ((intExpr *)(node->attr))->val << " ";
        }
        else if (node->valType == Bool)
        {
            cout << boolalpha << ((boolExpr *)(node->attr))->val << " ";
        }
        else if (node->valType == Variable)
        {
            varExpr *varE = (varExpr *)(node->attr);
            if (!(varE->isArray))
            {
                cout << varE->varName << " ";
            }
            else
            {
                cout << varE->varName << " ";
                generate_c_ExprIndexList(varE->startIndex);
            }
        }
        else
        {
            cout << "invalid Type for a Leaf";
            exit(0);
        }
    }
    else if (node->type == FunCall)
    {
        generate_c_FunCall((func_call *)(node->attr), 0);
    }
    else if (node->type == NonLeaf)
    {
        if (((opSymbol *)(node->attr))->type == Bool)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            case '&':
                generate_c_Expr(node->left);
                cout << "&& ";
                generate_c_Expr(node->right);
                break;
            case '|':
                generate_c_Expr(node->left);
                cout << "|| ";
                generate_c_Expr(node->right);
                break;
            case '!':
                cout << "! ";
                generate_c_Expr(node->right);
                break;
            default:
                cout << "Invalid operator";
                exit(1);
            }
        }
        else if (((opSymbol *)(node->attr))->type == Int)
        {
            switch (((opSymbol *)(node->attr))->sym)
            {
            // operations that yield int itself
            case '+':
                generate_c_Expr(node->left);
                cout << "+ ";
                generate_c_Expr(node->right);
                break;
            case '-':
                generate_c_Expr(node->left);
                cout << "- ";
                generate_c_Expr(node->right);
                break;
            case '/':
                generate_c_Expr(node->left);
                cout << "/ ";
                generate_c_Expr(node->right);
                break;
            case '*':
                generate_c_Expr(node->left);
                cout << "* ";
                generate_c_Expr(node->right);
                break;
            case '%':
                generate_c_Expr(node->left);
                cout << "% ";
                generate_c_Expr(node->right);
                break;

            // operation that yeild bool
            case '>':
                generate_c_Expr(node->left);
                cout << "> ";
                generate_c_Expr(node->right);
                break;
            case '<':
                generate_c_Expr(node->left);
                cout << "< ";
                generate_c_Expr(node->right);
                break;
            case 'g':
                generate_c_Expr(node->left);
                cout << ">= ";
                generate_c_Expr(node->right);
                break;
            case 'l':
                generate_c_Expr(node->left);
                cout << "<= ";
                generate_c_Expr(node->right);
                break;
            case 'n':
                generate_c_Expr(node->left);
                cout << "!= ";
                generate_c_Expr(node->right);
                break;
            case 'e':
                generate_c_Expr(node->left);
                cout << "== ";
                generate_c_Expr(node->right);
                break;
            default:
                cout << "Invalid operator";
                exit(1);
            }
        }
        else
        {
            cout << "Invalid Expr Node detected";
            exit(0);
        }
    }
}

void generate_c_StrExprList(str_expr *list)
{
    // cout << "StrExpr list was called\n";
    if (list == NULL)
    {
        cout << "List was NULL\n";
    }
    while (list != NULL)
    {
        if (list->var == NULL)
        {
            cout << "string is null";
        }
        cout << list->var << " ";
        list = list->next;
    }
}

void generate_c_WriteStmt(write_stmt *stmt)
{
    if (stmt->isExpr)
    {
        cout << "printf(\"\%d\", ";
        generate_c_Expr((expr *)(stmt->val));
    }
    else
    {
        cout << "printf(\"";
        generate_c_StrExprList((str_expr *)(stmt->val));
        cout << "\"";
    }
    cout << ");";
}

void generate_c_AssignStmt(assign_stmt *stmt, int flag = 1)
{
    if (stmt->var->isArray)
    {
        cout << stmt->var->varName;
        generate_c_ExprIndexList(stmt->var->startIndex);
    }
    else
    {
        cout << stmt->var->varName;
    }

    cout << " = ";
    generate_c_Expr(stmt->exp);
    if (flag == 1)
        cout << ";";
}

void generate_c_StmtList(statement *stmt);

void generate_c_IfStmt(if_stmt *stmt)
{
    cout << "if (";
    generate_c_Expr(stmt->cond);
    cout << ") ";

    cout << "{" << endl
         << endl;
    generate_c_StmtList(stmt->stmtList);
    cout << endl
         << "}";
}

void generate_c_IfElseStmt(if_else_stmt *stmt)
{
    cout << "if (";
    generate_c_Expr(stmt->cond);
    cout << ") ";

    cout << "{" << endl
         << endl;
    generate_c_StmtList(stmt->if_stmtList);
    cout << "}" << endl;

    cout << "else " << endl;
    cout << "{" << endl;
    generate_c_StmtList(stmt->else_stmtList);
    cout << endl
         << "}";
}

void generate_c_WhileStmt(while_stmt *stmt)
{
    cout << "while (";
    generate_c_Expr(stmt->cond);
    cout << ") ";

    cout << "{ " << endl
         << endl;
    generate_c_StmtList(stmt->stmtList);
    cout << endl
         << "}";
}

void generate_c_For_loop(for_loop *stmt)
{
    cout << "for (";
    generate_c_AssignStmt(stmt->startAssign, 0);
    cout << ";";
    generate_c_Expr(stmt->cond);
    cout << ";";
    generate_c_AssignStmt(stmt->updateAssign, 0);
    cout << ") ";

    cout << "{ " << endl
         << endl;
    generate_c_StmtList(stmt->stmtList);
    cout << endl
         << "}";
}

void generate_c_CondStmt(cond_stmt *stmt)
{
    if (stmt->type == If)
    {
        generate_c_IfStmt((if_stmt *)(stmt->cond_stmt_attrs));
    }
    else if (stmt->type == IfElse)
    {
        generate_c_IfElseStmt((if_else_stmt *)(stmt->cond_stmt_attrs));
    }
    else if (stmt->type == For)
    {
        generate_c_For_loop((for_loop *)(stmt->cond_stmt_attrs));
    }
    else if (stmt->type == While)
    {
        generate_c_WhileStmt((while_stmt *)(stmt->cond_stmt_attrs));
    }
}

void generate_c_ExprList(exprList *exprlist)
{
    for (exprList *list = exprlist; list != NULL; list = list->next)
    {
        generate_c_Expr(list->exp);
        if (list->next != NULL)
            cout << ", ";
    }
}

void generate_c_FunCall(func_call *stmt, int flag)
{
    cout << stmt->funName << " ( ";
    generate_c_ExprList(stmt->exprlist);
    if (flag == 1)
        cout << ");";
    else
        cout << ") ";
}

void generate_c_ReadStmt(read_stmt *stmt)
{
    cout << "scanf(\"\%d\",&";
    if (stmt->varexp->isArray)
    {
        cout << stmt->varexp->varName;
        generate_c_ExprIndexList(stmt->varexp->startIndex);
    }
    else
    {
        cout << stmt->varexp->varName;
    }

    cout << ");";
}

void generate_c_Stmt(statement *stmt)
{
    if (stmt == NULL)
    {
        cout << "statement is null" << endl;
        return;
    }

    switch (stmt->stmtType)
    {
    case write:
        generate_c_WriteStmt((write_stmt *)(stmt->stmt));
        break;
    case assign:
        generate_c_AssignStmt((assign_stmt *)(stmt->stmt));
        break;
    case cond:
        generate_c_CondStmt((cond_stmt *)(stmt->stmt));
        break;
    case funccall:
        generate_c_FunCall((func_call *)(stmt->stmt));
        break;
    case read:
        generate_c_ReadStmt((read_stmt *)(stmt->stmt));
        break;
    default:
        cout << "Invalid statement type" << endl;
        break;
    }
}

void generate_c_StmtList(statement *stmt)
{

    while (stmt != NULL)
    {
        generate_c_Stmt(stmt);
        stmt = stmt->next;
        cout << endl;
    }
}

void generate_c_Fdef(Fdef *fdef)
{
    generate_c_type(fdef->retType);
    cout << " ";
    cout << fdef->funcName << " ";
    cout << "(";
    generate_c_ArgList(fdef->FargList);
    cout << ")" << endl;

    cout << "{" << endl;
    generate_c_declaration_list(fdef->LdeclSec);
    generate_c_StmtList(fdef->stmtList);

    cout << "return ";
    generate_c_Expr(fdef->retStmt);
    cout << "; " << endl;

    cout << "} " << endl;
}

void generate_c_FDefSec()
{
    Fdef *fdef = prog.fdef_sec;
    while (fdef != NULL)
    {
        generate_c_Fdef(fdef);
        fdef = fdef->next;
    }
}

void generate_c_MainBlock()
{
    main_block *mainBlock = prog.mainBlock;

    generate_c_type(mainBlock->return_type);

    cout << "main ()" << endl;

    cout << "{" << endl;
    generate_c_declaration_list(mainBlock->local_declaration);

    generate_c_StmtList(mainBlock->stmt_list);

    cout << "return ";
    generate_c_Expr(mainBlock->return_stmt);
    cout << "; " << endl;

    cout << "} " << endl;
}

void generate_c_code()
{
    isError = true;

    if (isError)
    {
        cout << "#include <stdio.h>" << endl;
        cout << "#include <stdbool.h>" << endl
             << endl;
        generate_c_declaration_list(prog.global_declarations);
        generate_c_FDefSec();
        generate_c_MainBlock();
    }
}