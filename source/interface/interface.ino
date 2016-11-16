// A system interface module for coursework

/*======================*/
/* list of enumerations */
/*======================*/

enum ResponseCodes { RC_OK, RC_BAD_INDEX, RC_BAD_PID, RC_BAD_INPUT, RC_EMPTY_STRING, RC_UNKNOWN_CODE, RC_WRONG_STAT_RANGE, RC_SIZE };

enum SystemInfo { SI_SOURCE, SI_VOLTAGE, SI_CURRENT, SI_PRESSURE, SI_MECHANIC, SI_TURBO, SI_VALVE1, SI_VALVE2, SI_GAUGE1, SI_GAUGE2, SI_PUMP_VALVE, SI_OP_ERR_CODE, SI_SIZE };
enum IndicatorPids { IP_SOURCE, IP_SENSOR_WIDE, IP_SENSOR_LOW, IP_MECHANIC, IP_TURBO, IP_VALVE1, IP_VALVE2, IP_PUMP_VALVE, IP_SIZE };

enum UserInput { UI_PRESSURE, UI_GAUGE, UI_CURRENT, UI_SIZE };

/*==========================*/
/* list of enumerations end */
/*==========================*/

/*===================*/
/* list of constants */
/*===================*/

const int CONNECTION_SPEED = 9600;
const int TIMEOUT_VALUE = 2000;

const int PRESSURE_SENSOR_SWITCH_VALUE = 100; // ! TWEAK IT LATER ! //

const int INIT_SYSTEM_VALUE = 0;
const int INDICATOR_LOW = 0;

const boolean UI_WRITE = true;
const boolean UI_SEND = false;

/*=======================*/
/* list of constants end */
/*=======================*/

/*====================*/
/* class declarations */
/*====================*/

// класс, ответственный только за чтение данных о состоянии с мастер-платы и индикацию/печать на дисплей
class SystemInfo {
  public:
    SystemInfo(): system_state(NULL) { _system_state = new int[SI_SIZE] { INIT_SYSTEM_VALUE }; }
    ~SystemInfo() { delete[] _system_state; }
    int update();
  private:
    int set_diode(const int& pid, const int& value); // посылает на указанный PID HIGH или LOW в зависимости от value
    int light_diodes(); // использует set_diode() со значениями из _system_state и PIDами из IndicationPids
    int on_start(); // функция инициализации (сюда можно впихнуть проверку диодов и дисплеев)
    int serialize(); // отправляет сообщение в дисплей
    int deserialize(); // читает пакет от мастер-платы
    
    int* _system_state;
};

// класс для обработки пользовательского ввода, умеет выводить данные в мастер-плату (нужно доработать)
class UserInput {
  public:
    UserInput(): _tmp_state_vector(NULL), _state_vector(NULL), _mode(UI_SEND) { _tmp_state_vector = new int[UI_SIZE]; _state_vector = new int[UI_SIZE]; }
    ~UserInput() { delete[] _tmp_state_vector; delete[] _state_vector; }
    int toggle_mode() { _mode = !_mode; return RC_OK; } // переключение режима работы (изменяем данные или отправляем)
    boolean mode() { return _mode; }
    int set_tmp_elem(const int& index, const int& value); // изменение элемента в буферном векторе
    int update();
  private:
    int on_start(); // функция инициализации
    int serialize(); // отправляет сообщение на мастер-плату
    
    boolean _mode; // режим работы
    int* _tmp_state_vector; // буферный вектор состояний
    int* _state_vector; // вектор состояний, отсылаемый на мастер-плату
};

// класс панели управления, объединяет описанные ранее классы (вероятно, стоит отдать дисплей ему под контроль, т.к. и инндикация и юзер ввод могут что-то туда писать)
// подумать, как научить юзер ввод и индикацию делить дисплей без "гонок" --16.11.2016--
class ControlPanel {
  public:
    ControlPanel() {}
    ~ControlPanel() {}
    int update();
  private:
    SystemInfo _indication;
    UserInput _input; 
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
  return RC_OK; 
}

int UserInput::set_tmp_elem(const int& index, const int& value) {
  if (index >= UI_SIZE) {
    return RC_BAD_INDEX;
  }
  _tmp_state_vector[index] = value;
  return RC_OK; 
}

int UserInput::update() {
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

/*================================*/
/* ! ControlPanel class methods ! */
/*================================*/

int ControlPanel::update() {
  int response = _indication.update();
  response = _input.update();
  return RC_OK; 
}

/*====================================*/
/* ! ControlPanel class methods end ! */
/*====================================*/

