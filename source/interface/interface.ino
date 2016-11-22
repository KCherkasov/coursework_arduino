// A system interface module for coursework

// добавить сюда #include для библиотеки под дисплей

/*======================*/
/* list of enumerations */
/*======================*/

enum ResponseCodes { RC_OK, RC_BAD_INDEX, RC_BAD_PID, RC_BAD_INPUT, RC_EMPTY_STRING, RC_UNKNOWN_CODE, RC_WRONG_STAT_RANGE, RC_SIZE };

enum SystemInfo { SI_SOURCE, SI_VOLTAGE, SI_CURRENT, SI_PRESSURE, SI_MECHANIC, SI_TURBO, SI_VALVE1, SI_VALVE2, SI_GAUGE1, SI_GAUGE2, SI_PUMP_VALVE, SI_OP_ERR_CODE, SI_SIZE };
enum IndicatorPids { IP_SOURCE, IP_SENSOR_WIDE, IP_SENSOR_LOW, IP_MECHANIC, IP_TURBO, IP_VALVE1, IP_VALVE2, IP_PUMP_VALVE, IP_SIZE };
enum ButtonPids { BP_CURRENT_UP, BP_CURRENT_DOWN, BP_FIX_PRESSURE, BP_FIX_GAUGE, BP_TOGGLE_MODE, BP_SIZE };
enum PotentiometerPids { PP_PRESSURE, PP_GAUGE, PP_SIZE };

enum User_Input { UI_PRESSURE, UI_GAUGE, UI_CURRENT, UI_SIZE };

/*==========================*/
/* list of enumerations end */
/*==========================*/

/*===================*/
/* list of constants */
/*===================*/

const int CONNECTION_SPEED = 9600;
const int TIMEOUT_VALUE = 2000;

const int PRESSURE_SENSOR_SWITCH_VALUE = 100; // ! TWEAK IT LATER ! //

const int INT_DEFAULT_VALUE = 0;
const int INIT_SYSTEM_VALUE = 0;
const int INDICATOR_LOW = 0;

const boolean UI_CHANGE = true;
const boolean UI_SEND = false;

const int STRING_LENGTH = 16;

const int CURRENT_CHANGE_STEP = 100; // ! TWEAK IT LATER ! //
const int PRESSURE_CHANGE_STEP = 100; // ! TWEAK IT LATER ! //
const int GAUGE_CHANGE_STEP = 100; // ! TWEAK IT LATER ! //

/*=======================*/
/* list of constants end */
/*=======================*/

/*====================*/
/* class declarations */
/*====================*/

// класс, ответственный только за чтение данных о состоянии с мастер-платы и индикацию/печать на дисплей
class SystemInfo {
  public:
    SystemInfo(): _mode(UI_SEND), _system_state(NULL) { _system_state = new int[SI_SIZE] { INIT_SYSTEM_VALUE }; }
    ~SystemInfo() { delete[] _system_state; }
    boolean mode() { return _mode; }
    int toggle_mode() { _mode = !_mode; return RC_OK; }
    int update();
  private:
    int set_diode(const int& pid, const int& value); // посылает на указанный PID HIGH или LOW в зависимости от value
    int light_diodes(); // использует set_diode() со значениями из _system_state и PIDами из IndicationPids
    int on_start(); // функция инициализации (сюда можно впихнуть проверку диодов и дисплеев)
    int serialize(); // отправляет сообщение в дисплей
    int deserialize(); // читает пакет от мастер-платы
    
    boolean _mode;
    int* _system_state;
};

// класс для обработки пользовательского ввода, умеет выводить данные в мастер-плату (нужно доработать)
class UserInput {
  public:
    UserInput(): _tmp_state_vector(NULL), _state_vector(NULL), _mode(UI_SEND) { _tmp_state_vector = new int[UI_SIZE]; _state_vector = new int[UI_SIZE]; }
    ~UserInput() { delete[] _tmp_state_vector; delete[] _state_vector; }
    int toggle_mode() { _mode = !_mode; return RC_OK; } // переключение режима работы (изменяем данные или отправляем)
    boolean mode() { return _mode; }
    int update();
  private:
    int on_start(); // функция инициализации
    int serialize(); // отправляет сообщение на мастер-плату
    int set_tmp_elem(const int& index, const int& value); // изменение элемента в буферном векторе
    int ask_input(); // опросить порты кнопок и внести изменения в tmp_state_vector
    boolean _mode; // режим работы
    int* _tmp_state_vector; // буферный вектор состояний
    int* _state_vector; // вектор состояний, отсылаемый на мастер-плату
};

// класс дисплея, отвечает за вывод присланных модулем данных на экран
class Display {
  public:
    Display(): _upper_string(NULL), _upper_id(INT_DEFAULT_VALUE), _lower_string(NULL), _lower_id(INT_DEFAULT_VALUE) { _upper_string = new char[STRING_LENGTH]; _lower_string = new char[STRING_LENGTH]; }
    ~Display() { delete[] _upper_string; delete[] _lower_string; }
    int update();
  private:
    int on_start();
    int deserialize(); // читает строки со входного порта
    int render(); // отрисовывает считанные строки на экране
    char* _upper_string;
    int _upper_id;
    char* _lower_string;
    int _lower_id;
};

// класс панели управления, объединяет описанные ранее классы (вероятно, стоит отдать дисплей ему под контроль, т.к. и инндикация и юзер ввод могут что-то туда писать)
// подумать, как научить юзер ввод и индикацию делить дисплей без "гонок" --16.11.2016-- || решение найдено, протестировать. --21.11.2016--
class ControlPanel {
  public:
    ControlPanel() {}
    ~ControlPanel() {}
    boolean mode() { return _mode; }
    int update();
  private:
    int toggle_mode() { _mode = !_mode; return RC_OK; }
    boolean _mode;
    SystemInfo _indication;
    UserInput _input; 
    Display _display;
}

/*========================*/
/* class declarations end */
/*========================*/

/*==============================*/
/* ! SystemInfo class methods ! */
/*==============================*/

int SystemInfo::set_diode(const int& pid, const int& value) {
  if (pid > IP_SIZE) {
    return RC_BAD_PID;
  }
  if (value > INDICATOR_LOW) {
    digitalWrite(pid, HIGH); 
  } else {
    digitalWrite(pid, LOW);
  }
  return RC_OK;
}

int SystemInfo::light_diodes() {
  set_diode(IP_SOURCE + 1, _system_state[SI_SOURCE]);
  if (_system_state[SI_PRESSURE] > PRESSURE_SENSOR_SWITCH_VALUE) {
    set_diode(IP_SENSOR_WIDE + 1, _system_state[SI_PRESSURE]);
  } else {
    set_diode(IP_SENSOR_LOW + 1, _system_state[SI_PRESSURE]);
  }
  set_diode(IP_MECHANIC + 1, _system_state[SI_MECHANIC]);
  set_diode(IP_TURBO + 1, _system_state[SI_TURBO]);
  set_diode(IP_VALVE1 + 1, _system_state[SI_VALVE1]);
  set_diode(IP_VALVE2 + 1, _system_state[SI_VALVE2]);
  set_diode(IP_PUMP_VALVE + 1, _system_state[SI_PUMP_VALVE]);
  return RC_OK; 
}

int SystemInfo::on_start() {
  return RC_OK; 
}

int SystemInfo::serialize() {
  // код отправки верхней строки на дисплей тут
  
  // если режим - чтение системных данных - послать на дисплей еще и нижнюю строку
  if (_mode == UI_SEND) {

  }
  return RC_OK; 
}

int SystemInfo::deserialize() {
  int index = SI_SOURCE;
  while (Serial.available() > 0 && index < SI_SIZE) {
    int buffer = Serial.parseInt();
    _system_state[index++] = buffer;
  }
  return RC_OK; 
}

int SystemInfo::update() {
  int response = deserialize();
  if (response != RC_OK) {
    return response; 
  }
  light_diodes();
  serialize();
  return RC_OK; 
}

/*==================================*/
/* ! SystemInfo class methods end ! */
/*==================================*/

/*=============================*/
/* ! UserInput class methods ! */
/*=============================*/

int UserInput::on_start() {
  return RC_OK; 
}

int UserInput::serialize() {
  for (int i = 0; i < UI_SIZE; ++i) {
    Serial.write(_state_vector[i]); // or Serial1 should be used here?
  }
  // если режим установлен в чтение пользовательского ввода - послать строку на дисплей
  if (_mode == UI_CHANGE) {
  
  }
  return RC_OK; 
}

int UserInput::set_tmp_elem(const int& index, const int& value) {
  if (index >= UI_SIZE) {
    return RC_BAD_INDEX;
  }
  _tmp_state_vector[index] = value;
  return RC_OK; 
}

int UserInput::ask_input() {
  if (digitalRead(BP_CURRENT_DOWN) == HIGH) {
    if (_tmp_state_vector[UI_CURRENT] >= CURRENT_CHANGE_STEP) {
      _tmp_state_vector[UI_CURRENT] -= CURRENT_CHANGE_STEP;
    } else {
      _tmp_state_vector[UI_CURRENT] = INT_DEFAULT_VALUE;
    }
    // _tmp_state_vector[UI_CURRENT] %= CURRENT_MAX;
  }
  if (digitalRead(BP_CURRENT_UP) == HIGH) {
    _tmp_state_vector[UI_CURRENT] += CURRENT_CHANGE_STEP;
    // _tmp_state_vector[UI_CURRENT] %= CURRENT_MAX;
  }
  if (digitalRead(BP_FIX_PRESSURE) == LOW) {
    // если не нажата кнопка фиксации давления - менять давление по потенциометру
  }
  if (digitalRead(BP_FIX_GAUGE) == LOW) {
    // если не нажата кнопка фиксации расхода - менять расход по потенциометру
  }
  return RC_OK;
}

int UserInput::update() {
  ask_input();
  if (_mode == UI_SEND) {
    for (int i = 0; i < UI_SIZE; ++i) {
      _state_vector[i] = _tmp_state_vector[i];
    }
  }
  serialize();
  return RC_OK;
}

/*=================================*/
/* ! UserInput class methods end ! */
/*=================================*/

/*===========================*/
/* ! Display class methods ! */
/*===========================*/

int Display::on_start() {
  return RC_OK;
}

int Display::deserialize() {
  _upper_id = INT_DEFAULT_VALUE;
  _lower_id = INT_DEFAULT_VALUE;
  // читать строки по 16 символов или пока не найдем сепаратор
  
  return RC_OK;
}

int Display::render() {
  return RC_OK;
}

int Display::update() {
  deserialize();
  render();
  return RC_OK;
}

/*===============================*/
/* ! Display class methods end ! */
/*===============================*/

/*================================*/
/* ! ControlPanel class methods ! */
/*================================*/

int ControlPanel::update() {
  // проверяем нажатие на кнопку смены режима - если да, изменяем режим работы панели
  if (digitalRead(BP_TOGGLE_MODE) == HIGH) {
    toggle_mode();
  }
  // синхронизируем режим панели с режимами модулей индикации и ввода
  if (_mode != _indication.mode()) {
    _indication.toggle_mode();
  }
  if (_mode != _input.mode()) {
    _input.toggle_mode();
  }
  int response = _indication.update();
  response = _input.update();
  response = _display.update();
  return RC_OK; 
}

/*====================================*/
/* ! ControlPanel class methods end ! */
/*====================================*/
