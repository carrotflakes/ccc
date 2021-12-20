#ifndef CCC_H
#define CCC_H

enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EQ,
  TK_NEQ,
  TK_EOF,
  TK_GE,
  TK_LE,

  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
  TK_RETURN,
};

typedef struct {
  int ty;
  int val;
  char *ident;
  char *input;
} Token;

enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_EQ,
  ND_NEQ,
  ND_EOF,
  ND_GT,
  ND_GE,
  ND_LT,
  ND_LE,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_RETURN,
  ND_STMTS,
};

union NodeBody {
  struct Ope {
    struct Node *lhs;
    struct Node *rhs;
  } ope;
  struct If {
    struct Node *cond;
    struct Node *then;
    struct Node *els;
  } if_stmt;
  struct Node **stmts;
  int val;
  char *ident;
};

typedef struct Node {
  int ty;
  union NodeBody body;
} Node;

Token *get_token(int pos);
void tokenize(char *p);
Node **program();

void gen_all(Node **node);

#endif
