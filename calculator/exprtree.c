#include <stdio.h>
#include <stdlib.h>

struct node 
{
	double value;
    char op;
    struct node *left;
    struct node *right;
};

struct node* addnonleaf(struct node *left,char op,struct node *right) 
{
    struct node *node=(struct node *) malloc(sizeof(struct node));
    if(node!=NULL)
    {node->op=op;
    node->left=left;
    node->right=right;
    return node;}
}

struct node* addleaf(double value) 
{
    struct node *node=(struct node *) malloc(sizeof(struct node));
    if(node!=NULL)
    {node->value=value;
    node->left=NULL;
    node->right=NULL;
    return node;}
}

double eval(struct node *node) 
{
    if (node->op==0) 
        return node->value;
    double val_left=eval(node->left);
    double val_right=eval(node->right);
    switch (node->op) 
	{
        case '+':
            return val_left+val_right;
        case '-':
            return val_left-val_right;
        case '*':
            return val_left*val_right;
        case '/':
            return val_left/val_right;
    }
    return 0;
}
