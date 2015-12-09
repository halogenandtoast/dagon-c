typedef enum node_type {
  DUMMY_NODE,
  OBJECT_INITIALIZE_NODE,
  STATEMENTS_NODE,
  INT_NODE,
  CLASS_DEFINITION_NODE,
  METHOD_DEFINITION_NODE,
  ASSIGNMENT_NODE,
  VARIABLE_NODE,
  WHILE_STATEMENT_NODE,
  CASE_STATEMENT_NODE,
  IF_STATEMENT_NODE,
  CONSTANT_NODE,
  INSTANCE_VARIABLE_NODE,
  METHOD_CALL_NODE,
  SCOPED_METHOD_CALL_NODE,
  CASE_NODE,
  CATCHALL_CASE_NODE,
  STRING_NODE,
  ARRAY_NODE
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

void dagon_dump(Node* node);
// void dagon_dump_list_node(ListNode* list);
void dagon_list_node_append(Node* list, Node* item);
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
Node* dagon_bin_op_node_new(const char* op, Node* lhs, Node* rhs);
Node* dagon_method_call_node_new(Node* object, const char *method, Node* args);
Node* dagon_scoped_method_call_node_new(const char *method, Node* args);
Node* dagon_case_node_new(Node* value, Node* statements);
Node* dagon_catchall_case_node_new(Node* statements);
Node* dagon_string_node_new(const char *name);
Node* dagon_object_initialize_node_new(const char *name, Node* args);
Node* dagon_dummy_node_new(const char *name);
