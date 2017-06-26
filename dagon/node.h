#ifndef _DAGON_NODE_
#define _DAGON_NODE_

#define FOREACH_NODE(NODE) \
  NODE(OBJECT_INITIALIZE_NODE) \
  NODE(STATEMENTS_NODE) \
  NODE(INT_NODE) \
  NODE(CLASS_DEFINITION_NODE) \
  NODE(METHOD_DEFINITION_NODE) \
  NODE(ASSIGNMENT_NODE) \
  NODE(VARIABLE_NODE) \
  NODE(WHILE_STATEMENT_NODE) \
  NODE(CASE_STATEMENT_NODE) \
  NODE(IF_STATEMENT_NODE) \
  NODE(CONSTANT_NODE) \
  NODE(INSTANCE_VARIABLE_NODE) \
  NODE(METHOD_CALL_NODE) \
  NODE(SCOPED_METHOD_CALL_NODE) \
  NODE(CASE_NODE) \
  NODE(CATCHALL_CASE_NODE) \
  NODE(STRING_NODE) \
  NODE(ARRAY_NODE) \
  NODE(NATIVE_NODE) \
  NODE(COMBINED_STRING_NODE)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
  FOREACH_NODE(GENERATE_ENUM)
} node_type;

typedef struct {
  node_type type;
  union {
    void* ptr;
    int ival;
    const char *sval;
  } value;
} Node;

typedef struct ListNode {
  Node* current;
  struct ListNode* next;
} ListNode;

typedef struct {
  ListNode* items;
} ArrayNode;

typedef struct {
  const char *name;
  ListNode* methods;
} ClassDefinitionNode;

typedef struct {
  const char *name;
  ListNode* args;
  ListNode* statements;
} MethodDefinitionNode;

typedef struct {
  Node *conditional;
  ListNode* statements;
} WhileStatementNode;

typedef struct {
  Node *value;
  ListNode* cases;
} CaseStatementNode;

typedef struct {
  Node *expression;
  ListNode *true_statements;
  ListNode *false_statements;
} IfStatementNode;

typedef struct {
  Node* variable;
  Node* value;
} AssignmentNode;

typedef struct {
  const char *name;
} VariableNode;

typedef struct {
  const char *name;
} InstanceVariableNode;

typedef struct {
  const char *name;
} ConstantNode;

typedef struct {
  Node* object;
  const char *method;
  ListNode *args;
} MethodCallNode;

typedef struct {
  const char *method;
  ListNode *args;
} ScopedMethodCallNode;

typedef struct {
  ListNode* args;
  ListNode* statements;
} LambdaNode;

typedef struct {
  Node* value;
  ListNode* statements;
} CaseNode;

typedef struct {
  ListNode* statements;
} CatchallCaseNode;

typedef struct {
  const char *name;
  ListNode* args;
} ObjectInitializeNode;

typedef struct CombinedStringNode {
  Node* string;
  struct CombinedStringNode* next_part;
} CombinedStringNode;

void dagon_dump(Node* node);
void dagon_free_node(Node* node);

void dagon_list_node_append(Node* list, Node* item);
void dagon_combine_string_node_append(Node *combined_string, Node *string);
Node* dagon_list_node_new(Node* item);
Node* dagon_int_node_new(int val);
Node* dagon_class_definition_node_new(const char *name, Node* method_definitions);
Node* dagon_method_definition_node_new(const char *name, Node* arglist, Node* statements);
Node* dagon_assignment_node_new(Node* variable, Node* value);
Node* dagon_variable_node_new(const char *name);
Node* dagon_constant_node_new(const char *name);
Node* dagon_instance_variable_node_new(const char *name);
Node* dagon_array_node_new(Node* items);
Node* dagon_while_statement_node_new(Node* condition, Node* statements);
Node* dagon_case_statement_node_new(Node* value, Node* cases);
Node* dagon_if_statement_node_new(Node* expression, Node* true_statements, Node* false_statements);
Node* dagon_combine_string_node_new(Node* string);
Node* dagon_bin_op_node_new(const char* op, Node* lhs, Node* rhs);
Node* dagon_method_call_node_new(Node* object, const char *method, Node* args);
Node* dagon_scoped_method_call_node_new(const char *method, Node* args);
Node* dagon_case_node_new(Node* value, Node* statements);
Node* dagon_catchall_case_node_new(Node* statements);
Node* dagon_string_node_new(const char *name);
Node* dagon_object_initialize_node_new(const char *name, Node* args);

#endif
