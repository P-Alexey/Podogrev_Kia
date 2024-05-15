//V 1.1 - добавил проверку потребления тока подогрева водителя

/* Китайские подогревы подача питания на каждый мат - 12В
1е-нажатие 100% Ток 1ого мата-1,2А
2е-нажатие частота сигнала 95Гц заполнение ШИМ 64% Ток 1ого мата-0.8
3е-нажатие частота сигнала 95Гц заполнение ШИМ 35% Ток 1ого мата-0.4

Включение Ардуино когда включен Замок зажигания (ACC +12V подано на блок подогрева)
  Включение подогрева:
    На выходы D9 или D13 +5V (включение подогрева подтягивание к GND на минимум 45мс)
    Выкл  - выходы D4,D3 и D7,D6 - включены (подтянуты к +5V) (лампочки не горят)
    1е нажатие Мах - вход D9 или D13 подтягивается к GND на 75мс, 
          выходы D4,D3 или D7,D6 - выключены - подтянуты к GND (лампочки горят)
    2е нажатие Mid - вход D9 или D13 подтягивается к GND на 75мс, 
          выходы D4 или D6 - выключены - подтянуты к GND (лампочки горят)
    3е нажатие Low - вход D9 или D13 подтягивается к GND на 75мс, 
          выходы D3 или D7 - выключены - подтянуты к GND (лампочки горят)
    Двойной клик по кнопке - Выключаем подогрев
    
    Долгое нажатие на Кнопку (D5 или D2), вклчить режим Max после включения Ардуино через 11 мин (авто прогрев) или выключить режм
          Ответ - мигание D4,D3 или D7, D6 - 3 мигания при вкл, 2 мигания при выкл
          Вкл или Выкл проверять по регистру в EEROM Регистр 0 - для водителя, 1-для пассажира (значение 1-вкл, 0-выкл)
    
    Проверка питания борт сети - если напряжение меньше  12,5В, отключить подогрев - мигание 2раза
    Проверка потребления тока подогрева водителя - если ток при включении меньше 0.2А или больше 6А, отключить подогрев - мигание 3раза
Разъем питание основной платы
01 - габариты
02 - +12V
03 - +12V
04 - ACC замок зажигания
05 - GND
06 - + обогрева пассажира
07 - + обогрева водителя

Распайка
Arduino   RS232   на кнопку  
    D5    - 1       - 9       - кнопка водителя (черный) (добавить R10k на GND)
    D4    - 2       - 10      - mid водителя (желтый)
    D3    - 3       - 11      - low водителя (зеленый)
    D2    - 4       - 3       - кнопка пассажира (черный) (добавить R10k на GND)
    gnd   - 5       - 1       - gnd (черный)    
    D7    - 6       - 5       - low пассажира (желтый)
    D6    - 7       - 4       - mid пассажира (зеленый)
          - 8       - 12      - +5V
          - 9       - 7       - габариты + 12V (желтый) (№1 или №2 на плате)
    A3    - делитель напряжения (10к-3.3к) 10к на +12V разъема плты 3,3к на GND разъема платы
    D9    - кнопка водителя (коричневый №11 на плате)
    D10   - синий водителя (№10 на плате)
    D11   - желтый водителя (№9 на плате)
    D12   - белый водителя (№8 на плате)
    D13   - кнопка пассажира (коричневый №7 на плате)
    A0    - синий пассажира (№6 на плате)
    A1    - желтый пассажира (№5 на плате)
    A2    - белый пассажира (№4 на плате)
	  A6	  - OUT измерителя тока ACS712
    GND   - GND c платы (черный 3 или 12 на плате)
    RAW   - +5V питание (с С3 на плате ближе к С1)
	
	Разорвать питание подогрева водителя, в разрыв добавить измеритель тока
*/

#include <EEPROM.h> 

//Входные
#define But_In_1_pin 5 // Сигнал c кнопки водителя (с кнопок)   - +5V - D5
#define But_In_2_pin 2 // Сигнал c кнопки пассажира (с кнопок)  - +5V - D2
#define Voltage_pin A3 // Напряжение сети                       - +0-5V - A3
#define Current_1_pin A6 // Ток подогрева водителя             - +0-+5V - A6
#define Yellow_1_pin 11 // Желтый Сигнал с платы (водителя)     - +5V - D11
#define White_1_pin 12 // Белый Сигнал с платы (водителя)       - +5V - D12
#define Blue_1_pin 10 // Синий Сигнал с платы (водителя)        - +5V - D10
#define Yellow_2_pin A1 // Желтый Сигнал с платы (пассажир)     - +5V - A1
#define White_2_pin A2 // Белый Сигнал с платы (пассажир)       - +5V - A2
#define Blue_2_pin A0 // Синий Сигнал с платы (пассажир)        - +5V - A0

//Выходные
#define Led_Low_1_pin 3//Лампочка Low водителя (на кнопки)      - GND - D3
#define Led_Mid_1_pin 4//Лампочка Mid водителя (на кнопки)      - GND - D4
#define Led_Low_2_pin 7//Лампочка Low пассажира (на кнопки)     - GND - D7
#define Led_Mid_2_pin 6//Лампочка Mid пассажира (на кнопки)     - GND - D6
#define Power_Out_1_pin 9//Вкл подогрев водителя на плату       - GND - D9
#define Power_Out_2_pin 13//Вкл подогрев пассажира на плату     - GND - D13

#define Time_Start 660000//Задержка авто включения мСек 11мин=660 000
#define Time_drebezg 100 // 0.1 сек дребезг контакта
#define Time_UpKey 500 // 0,5 сек максимальное время двух щелчков
#define Time_LongPush 5000 //удержание кнопки

#define k_volt 4.13//Коэфициент делителя напряжения
#define num_Srednee 200// число точек для определния среднего значения
#define max_current 5.0// Максимальное значение тока отключения
#define min_current 0.2// Минимальное значение тока отключения


int8_t Power[2]={0,0}; //Мощьность прогрева 0-выкл, 1-100%, 2-75%, 3-50%
byte Satr_Progrev[2]={0,0};//байт из памяти вкл автовлючение или нет
bool Ferst_Start=1; //Флаг что включили первый раз
bool Progrev_On=0; // флаг что нагрев включали
unsigned long timing=0; // переменная хранения задержки времени автовключения

void setup() {
 //Входные
  pinMode(But_In_1_pin, INPUT);
  pinMode(But_In_2_pin, INPUT);
  pinMode(Voltage_pin, INPUT);
  pinMode(Yellow_1_pin, INPUT);
  pinMode(White_1_pin, INPUT);
  pinMode(Blue_1_pin, INPUT);
  pinMode(Yellow_2_pin, INPUT);
  pinMode(White_2_pin, INPUT);
  pinMode(Blue_2_pin, INPUT);

  //Выходные
  pinMode(Led_Low_1_pin, OUTPUT);
  pinMode(Led_Mid_1_pin, OUTPUT);
  digitalWrite(Led_Low_1_pin,true); 
  digitalWrite(Led_Mid_1_pin,true);
  pinMode(Led_Low_2_pin, OUTPUT);
  digitalWrite(Led_Low_2_pin,true); 
  digitalWrite(Led_Mid_2_pin,true);
  pinMode(Led_Mid_2_pin, OUTPUT);
  pinMode(Power_Out_1_pin, OUTPUT);
  digitalWrite(Power_Out_1_pin,true);
  pinMode(Power_Out_2_pin, OUTPUT);
  digitalWrite(Power_Out_2_pin,true);

  Satr_Progrev[0]=EEPROM.read(0);
  Satr_Progrev[1]=EEPROM.read(1);
//Serial.begin(9600);
}

uint32_t mil_press=0;//время нажатия кнопки
uint8_t num_blink[2]={0,0};
bool Led_ON[2]={0,0};

void loop() {
  if (Ferst_Start){
    // если первое включение
    if ((Satr_Progrev[0]==1||Satr_Progrev[1]==1)&&!Progrev_On){
      //Если автопрогрев и не трогали прогрев, включить после задержки
      if (millis()> Time_Start){
        if (Satr_Progrev[0]==1){
          Push_Short(0);          
        }
        if (Satr_Progrev[1]==1){
          Push_Short(1);
        }
        Ferst_Start=false;
      }
    }
    else{
      Ferst_Start=false;
    }
  }  
  
  Read_I_U(); //Проверка тока и напряжения   
  
  // задержка на успевание обработать нажатие кнопок
  if (millis()-mil_press>10){
    Push_Key (0,digitalRead(But_In_1_pin));//еслнажали кнопку водителя
    Push_Key (1,digitalRead(But_In_2_pin));//еслнажали кнопку пассажира
    mil_press=millis();
  }
  Read_Led();  
}
//__________________________________________________________________________________________________________
//Проверка тока и напряжения
uint32_t Current_val=0;//Значение с шунта
uint16_t Kol_Read=0;//Колличество чтений

void Read_I_U() {
  if (Power[0]!=0||Power[1]!=0){//если седухи включены
    float V_auto=(k_volt*5/1024)*analogRead(Voltage_pin);
//Serial.print ("  v_auto=");//для точного определения коэфициента делителя
//Serial.print (V_auto);
//Serial.print('\n');
    if (V_auto<12.5){
      // проверить питание если меньше 12.5В отключить
      if (Power[0]!=0){
        delay (75);//45 порог
        Push_2Clicks(0);
        delay (75);//45 порог
        Blink_led(0,2);// мигнуть 2 раза
      }
      if (Power[1]!=0){
        delay (75);//45 порог
        Push_2Clicks(1); 
        delay (75);      
        Blink_led(1,2);// мигнуть 2 раза
      }      
    }
  }
  if (Power[0]!=0){//если седуха водителя включена
  
    Current_val+=analogRead(Current_1_pin);
  
    if (Kol_Read>=num_Srednee){
      Current_val=Current_val/num_Srednee;
      float I_auto=(((Current_val*5.0/1024.0)-2.5146)/0.189)/(-2.2);                 
//Serial.print(I_auto);
//Serial.print('\n');
      if (I_auto<min_current||I_auto>max_current){
          //Выключить
          delay (75);//45 порог
          Push_2Clicks(0);
          delay (75);//45 порог
          Blink_led(0,3);// мигнуть 2 раза
      }          
      Current_val=0;       
      Kol_Read=0;
    }
    else{
      Kol_Read++; 
    }
  }
}
//__________________________________________________________________________________________________________
bool press_key[2]={0,0};   // Флажок нажатия кнопки
bool long_press[2]={0,0};  // Длинное нажатие
uint32_t last_press[2]={0,0};  // переменная хранит момента последнего нажатия на кнопку
uint32_t up_mils[2]={0,0};       // время между отпусканием кнопки
int8_t kol_Click[2]={0,0};   // колличество отпускний кнопки
// отсчлеживание нажатия кнопки
void Push_Key(int vod_pas,bool State_Key){
  
  if (State_Key&&!press_key[vod_pas]&&millis() - last_press[vod_pas] > Time_drebezg) {
    // нажатие
    press_key[vod_pas] = true;
    last_press[vod_pas] = millis();   
  }

  if (State_Key && press_key[vod_pas] && !long_press[vod_pas]&& millis() - last_press[vod_pas] > Time_LongPush) {
    // удержание 5с
    long_press[vod_pas] = true; 
    Push_Long(vod_pas);
  }
  
  if (!State_Key && press_key[vod_pas] && long_press[vod_pas]) {
    // отпушена после удержания
    press_key[vod_pas] = false; 
    long_press[vod_pas] = false; 
    last_press[vod_pas]=0;
  }

if (!State_Key && press_key[vod_pas] && !long_press[vod_pas]) {
    //отпущена после короткого нажатия
    press_key[vod_pas] = false; // отпустили кнопу для след фикс нажатия
    kol_Click[vod_pas]++;    // добавили клик
    if (kol_Click[vod_pas]==1)  {
      up_mils[vod_pas]=millis();// запомнили время отпускания
    }    
  }

  if (kol_Click[vod_pas]==1&&millis()-up_mils[vod_pas]>Time_UpKey){
    // если с момента 1 клика прошло более 0,5с
    Push_Short(vod_pas); // короткое нажатие
    kol_Click[vod_pas]=0;
    Progrev_On=true;
  }
  if (kol_Click[vod_pas]==2){
     // если успели 2 клика
    Push_2Clicks(vod_pas);
    kol_Click[vod_pas]=0;
  }
}
//__________________________________________________________________________________________________________
//Короткое нажатие - Вкл прогрев
void Push_Short(int vod_pas) { 
  if (vod_pas==0){
    digitalWrite(Power_Out_1_pin,false);
    delay (75);//45 порог
    digitalWrite(Power_Out_1_pin,true);
  }
  else{
    digitalWrite(Power_Out_2_pin,false);
    delay (75);//45 порог
    digitalWrite(Power_Out_2_pin,true);
  }  
}
//__________________________________________________________________________________________________________
//Длинное нажатие - Вкл/Выкл автопрогрев
void Push_Long(int vod_pas) { 
  
  byte val=EEPROM.read(vod_pas);//Текущее значение
    
  if (val==0){
    EEPROM.update(vod_pas,1);//Включить автонарев
    Blink_led(vod_pas,2);// мигнуть 3 раза    
  }
  else{
    EEPROM.update(vod_pas,0);//Выключить автонагрев
    Blink_led(vod_pas,0);// мигнуть 1 раза        
  }    
}
//__________________________________________________________________________________________________________
// два щелчка Выкл подогрев
void Push_2Clicks(int vod_pas){ 
   while (Power[vod_pas]!=0){
    Push_Short(vod_pas);
    delay (75);
    Power[vod_pas]--;
   }
}
//__________________________________________________________________________________________________________
//Считать данные с платы
void Read_Led(){
  bool Data_Led_1[3]={0,0,0};
  bool Data_Led_2[3]={0,0,0};  

  if (digitalRead(Yellow_1_pin)) Data_Led_1[0]=true;
  if (digitalRead(White_1_pin)) Data_Led_1[1]=true;
  if (digitalRead(Blue_1_pin)) Data_Led_1[2]=true;
  if (digitalRead(Yellow_2_pin)) Data_Led_2[0]=true;
  if (digitalRead(White_2_pin)) Data_Led_2[1]=true;
  if (digitalRead(Blue_2_pin)) Data_Led_2[2]=true;
  
  if (!Data_Led_1[0]&&!Data_Led_1[1]&&!Data_Led_1[2]){
    digitalWrite(Led_Low_1_pin,true); 
    digitalWrite(Led_Mid_1_pin,true);
    Power[0]=0;
  }
  if (!Data_Led_2[0]&&!Data_Led_2[1]&&!Data_Led_2[2]){
    digitalWrite(Led_Low_2_pin,true); 
    digitalWrite(Led_Mid_2_pin,true);
    Power[1]=0;
  }

  if (Data_Led_1[0]&&Data_Led_1[1]&&Data_Led_1[2]){
      digitalWrite(Led_Low_1_pin,true); 
      digitalWrite(Led_Mid_1_pin,true);
      Power[0]=0;
  }
  if (Data_Led_2[0]&&Data_Led_2[1]&&Data_Led_2[2]){
      digitalWrite(Led_Low_2_pin,true); 
      digitalWrite(Led_Mid_2_pin,true);
      Power[1]=0;
  }

  if (!Data_Led_1[0]&&Data_Led_1[1]&&!Data_Led_1[2]){
      digitalWrite(Led_Low_1_pin,false); 
      digitalWrite(Led_Mid_1_pin,false);
      Power[0]=3;
  }
  if (!Data_Led_2[0]&&Data_Led_2[1]&&!Data_Led_2[2]){
      digitalWrite(Led_Low_2_pin,false); 
      digitalWrite(Led_Mid_2_pin,false);
      Power[1]=3;
  }

  if (Data_Led_1[0]&&!Data_Led_1[1]&&!Data_Led_1[2]){
      digitalWrite(Led_Low_1_pin,true); 
      digitalWrite(Led_Mid_1_pin,false);
      Power[0]=2;
  }
  if (Data_Led_2[0]&&!Data_Led_2[1]&&!Data_Led_2[2]){
      digitalWrite(Led_Low_2_pin,true); 
      digitalWrite(Led_Mid_2_pin,false);
      Power[1]=2;
  }

  if (!Data_Led_1[0]&&!Data_Led_1[1]&&Data_Led_1[2]){
      digitalWrite(Led_Low_1_pin,false); 
      digitalWrite(Led_Mid_1_pin,true);
      Power[0]=1;
  }
  if (!Data_Led_2[0]&&!Data_Led_2[1]&&Data_Led_2[2]){
      digitalWrite(Led_Low_2_pin,false); 
      digitalWrite(Led_Mid_2_pin,true);
      Power[1]=1;
  }
}
//__________________________________________________________________________________________________________
//Мигание для обратной связи
void Blink_led(int vod_pas, int kol){
  for (int i=0; i<=kol; i++){
    if (vod_pas==0){
      digitalWrite(Led_Low_1_pin,false); 
      digitalWrite(Led_Mid_1_pin,false);
      delay (500);
      digitalWrite(Led_Low_1_pin,true); 
      digitalWrite(Led_Mid_1_pin,true);
      delay (500);
    }
    else{
      digitalWrite(Led_Low_2_pin,false); 
      digitalWrite(Led_Mid_2_pin,false);
      delay (500);
      digitalWrite(Led_Low_2_pin,true); 
      digitalWrite(Led_Mid_2_pin,true);
      delay (500);
    }    
  }
}