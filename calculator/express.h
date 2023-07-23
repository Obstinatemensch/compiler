struct node 
{
	double value;
    char op;
    struct node *left;
    struct node *right;
};
struct node* addnonleaf(struct node *left,char op,struct node *right); 

struct node* addleaf(double value);

double eval(struct node *node) ;