class RuntimeClass {
public:
  virtual ~RuntimeClass();
  virtual int method();
};

RuntimeClass::~RuntimeClass() {}
int RuntimeClass::method() { return 42; }

int runtime_data = 42;
