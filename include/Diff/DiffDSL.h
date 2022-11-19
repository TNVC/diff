#pragma once

#define NUM(VALUE) createNumber  (VALUE)
#define VAR(VALUE) createVariable(VALUE)

#define NUM_VAL(NODE) NODE->value.number
#define OP_VAL(NODE)  NODE->value.operat
#define VAR_VAL(NODE) NODE->value.variable

#define Left  node->left
#define Right node->right

#define CopyLeft  (node->left  ? createNode(Left)  : nullptr)
#define CopyRight (node->right ? createNode(Right) : nullptr)

#define dLeft  (node->left  ? diff(Left)  : nullptr)
#define dRight (node->right ? diff(Right) : nullptr)

#define CREATE_OPERATOR(TYPE, LEFT, RIGHT)         \
  db::createNode(                                  \
                 {db::OPERATOR_ ## TYPE},          \
                 db::type_t::OPERATOR,             \
                 (LEFT),                           \
                 (RIGHT)                           \
                )


#define ADD(LEFT, RIGHT) CREATE_OPERATOR(ADD , LEFT   , RIGHT)
#define SUB(LEFT, RIGHT) CREATE_OPERATOR(SUB , LEFT   , RIGHT)
#define MUL(LEFT, RIGHT) CREATE_OPERATOR(MUL , LEFT   , RIGHT)
#define DIV(LEFT, RIGHT) CREATE_OPERATOR(DIV , LEFT   , RIGHT)
#define SIN(VALUE)       CREATE_OPERATOR(SIN , nullptr, VALUE)
#define COS(VALUE)       CREATE_OPERATOR(COS , nullptr, VALUE)
#define SQRT(VALUE)      CREATE_OPERATOR(SQRT, nullptr, VALUE)
#define POW(LEFT, RIGHT) CREATE_OPERATOR(POW , LEFT   , RIGHT)
#define LOG(LEFT, RIGHT) CREATE_OPERATOR(LOG , LEFT   , RIGHT)
#define LN(VALUE)        CREATE_OPERATOR(LN  , nullptr, VALUE)

#define IS_NUM(NODE)      (NODE->type == db::type_t::NUMBER)
#define IS_OPERATOR(NODE) (NODE->type == db::type_t::OPERATOR)
#define IS_VAR(NODE)      (NODE->type == db::type_t::VARIABLE)

#define IS_IT_OPERATOR(NODE, OPERATOR_NAME)                                \
  (IS_OPERATOR(NODE) && OPERATOR(NODE) == db::OPERATOR_NAME)

#define NUMBER(NODE)   (NODE->value.number)
#define OPERATOR(NODE) (NODE->value.operat)
#define VARIABLE(NODE) (NODE->value.variable)

#define SET_TO_NUMBER(NODE)   NODE->type = db::type_t::NUMBER
#define SET_TO_VARIABLE(NODE) NODE->type = db::type_t::VARIABLE
#define SET_TO_OPERATOR(NODE) NODE->type = db::type_t::OPERATOR

#define REMOVE_CHILDREN(NODE)                   \
  do                                            \
    {                                           \
      if (Left ) db::removeNode(Left );         \
      Left  = nullptr;                          \
      if (Right) db::removeNode(Right);         \
      Right = nullptr;                          \
    } while (0)

#define IS_EQUAL(NODE, VALUE) (IS_NUM(NODE) && isEqual(NUMBER(NODE), VALUE))

#define CALC_CONST(EXPRESION)                   \
  do                                            \
    {                                           \
      SET_TO_NUMBER(node);                      \
                                                \
      NUMBER(node) = (EXPRESION);               \
                                                \
      REMOVE_CHILDREN(node);                    \
                                                \
      *wasChange = true;                        \
    } while (0)

#define UPPER_NODE(NODE)                        \
  do                                            \
    {                                           \
      db::TreeNode *nodeForUpping   = (NODE);   \
      db::TreeNode *nodeForDeliting =           \
        ((NODE) == Left ? Right : Left);        \
                                                \
      Left = Right = nullptr;                   \
                                                \
      db::removeNode(node           );          \
      db::removeNode(nodeForDeliting);          \
                                                \
      node = nodeForUpping;                     \
                                                \
      *wasChange = true;                        \
    } while (0)

#define TO_NUMBER(VALUE)                        \
  do                                            \
    {                                           \
      SET_TO_NUMBER(node);                      \
                                                \
      NUMBER(node) = VALUE;                     \
                                                \
      db::removeNode(Left );                    \
      db::removeNode(Right);                    \
                                                \
      Left = Right = nullptr;                   \
                                                \
      *wasChange = true;                        \
    } while (0)
