const int CONNECTION_SPEED = 9600;

const int DEFAULT_CAPACITY = 4;
const int MAX_QUEUE_SIZE = 100;

enum ResponseCodes { RC_OK, RC_BAD_KEY, RC_BAD_VALUE, RC_EMPTY_MESSAGE, RC_EMPTY_BUFFER, RC_SIZE };
enum KeyvalIds { KI_KEY, KI_VALUE, KI_SIZE };
enum KeyCodes { KC_SIZE }; // discuss with Anton
enum StateInfo { SI_VOLTAGE_ID, SI_CURENT_ID, SI_PRESSURE_ID, SI_GAUGE1_ID, SI_GAUGE2_ID, SI_OP_ERR_CODE, SI_SIZE };

class SystemInfo {
  public:
    SystemInfo(): _values(NULL) { _values = new int [SI_SIZE]; on_start(); }
    ~SystemInfo() { delete[] _values; }
    int update();
  private:
    int on_start(); // on-start indicators check (added by Asya's request)
    int deserialize(char* message); // Garrick's SPI reading algorithm here
    int serialize(char*& message); // send data to display
    int* _values;
};


class InputQueue {
  public:
    InputQueue(): _head(0), _tail(0), _keys(NULL), _values(NULL) { _keys = new int[MAX_QUEUE_SIZE]; _values = new int[MAX_QUEUE_SIZE]; }
    ~InputQueue() { delete[] _keys; delete[] _values; }
    int push(const int& key, const int& value);
    int pop(int*& key_val_pair);
    int head(int*& key_val_pair) const;
    boolean empty() const { return _head == _tail || (_keys == NULL && _values == NULL); }
    boolean full() const { return ((_head + 1) % MAX_QUEUE_SIZE) == _tail; }
  private:
    int _head;
    int _tail;
    int* _keys;
    int* _values;
};

class UserInterface {
  public:
    UserInterface() {}
    ~UserInterface() {}
    int update();
  private:
    int check_input(); // checking all active input ports and receiving a key-value pairs to _user_input
    int serialize(int* input, char*& message);
    boolean input_empty() { return _user_input.empty(); }
    
    InputQueue _user_input;
};

int SystemInfo::on_start() {
   
}

int InputQueue::push(const int& key, const int& value) {
  // temporary measures - will be removed and we'll just rewrite the old commands
  if (full()) {
    return RC_QUEUE_FULL;
  }
  _keys[_tail] = key;
  _values[_tail] = value;
  ++_tail;
  _tail %= MAX_QUEUE_SIZE;
  return RC_OK;
}

int InputQueue::pop(int*& key_val_pair) {
  if (empty()) {
    return RC_BUFFER_EMPTY;
  }
  delete[] key_val_pair;
  key_val_pair = new int[KI_SIZE];
  key_val_pair[KI_KEY] = _keys[_head];
  key_val_pair[KI_VALUE] = _value[_head];
  ++_head;
  _head %= MAX_QUEUE_SIZE;
  return RC_OK;
}

int InputQueue::head(int*& key_val_pair) const {
  if (empty()) {
    return RC_BUFFER_EMPTY;
  }
  delete[] key_val_pair;
  key_val_pair = new int[KI_SIZE];
  key_val_pair[KI_KEY] = _keys[_head];
  key_val_pair[KI_VALUE] = _value[_head];
  return RC_OK;
}

int UserInterface::update() {
  return RC_OK;
}
