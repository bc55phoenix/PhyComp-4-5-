/* Barrier synchronization
 * 
 * A common pattern used in parallel programs is to have processing in two stages: computation and communication.
 * First, all threads do computation or IO in parallel.  When every thread has finished,
 * the threads communicate their computed values with one another, and based on those
 * values, another round of computation takes place, followed by another communication
 * step.
 * 
 * In order to facilitate this style of computation, it's useful to use a synchronization
 * construct known as a barrier.  Barriers cause all threads to wait until all other
 * threads have also called wait.  Then all threads are allowed to continue processing.
 * The barrier prevents threads from communicating before other threads are finished,
 * or from starting the next round of computation before other threads have communicated.
 * 
 * In this example, we have three threads
 */

#include <proc.h>

// constants won't change. They're uuuset here.
const int dataPin = 11;   // data pin (pin14 on 74ch595)
const int clockPin = 12;  // clock pin (pin11 on 74ch595)
const int latchPin = 10;  // latch pin (pin12 on 74ch595)
const int NUM_COLS = 5;  // 5 cathode columns

//                  0, 1, 2, 3, 4 -> 1 is Sum, 2 is Mean, 3 is Printer
const int COLS[] = {2, 3, 4 ,5, 6};   // arduino pins connected through resistor to the 5 cathode columns

// Iteration numbers
// 00000001,00000010,00000100,00001000,00010000,00100000,01000000
int rows[7] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40};
int rowNum = 0;

void lightPixel(int _id, int _iter) {
  digitalWrite(COLS[_id],LOW);
  digitalWrite(latchPin,LOW);
  rowNum = _iter%7;
  shiftOut(dataPin,clockPin,MSBFIRST,rows[rowNum]);  
  delay(200);          // So that pixel is visible
  digitalWrite(COLS[_id],HIGH);
  digitalWrite(latchPin,HIGH);
}


class Barrier {

    /* We have declared one lock and one condition
     * variable here.  You may (or may not) find it
     * useful to declare additional condition variables
     * to solve the problem.  We just left these here as
     * a reminder on the syntax. */

    // We have a counter integer variable to keep 
    // track of the number of threads
    Lock* _l;
    Cond* _c;
    int _n;
    int numTh;

  public:
    Barrier (int n) {  //Constructor for club
      _l = new Lock();
      _c = new Cond(_l);
      _n = n;
      numTh = 0;
    }

    /*
     * Every time a thread enters the wait function, it acquires 
     * the lock and increments the counter. 
     * 
     * Until our counter integer reaches one less than the 
     * maximum number of threads (3  in this case), each thread 
     * unlocks and then goes into waiting. 
     * 
     * The last thread comes along and instead of going into
     * waiting, decrements the counter since it signals the other
     * waiting thread so that they can all output their results 
     * and unlock and exit the scene.
     * 
     * When this other thread is signaled, it decrements the counter 
     * too and signals the other waiting thread  (if one exists),
     * so that it can decrement the counter too and they can 
     * unlock and exit the scene.
     * 
    */

    void wait() {
      /*TODO: Only return when _n other threads have also called wait*/
      _l->lock();
      numTh++;
      if(numTh < _n){
        _l->unlock();
        _c->wait();  
        numTh--;
        if(_c->waiting()){
           _c->signal();  
        }else{
          _l->unlock();
        }
      }else{
          numTh--;
          _c->signal(); 
      }
      _l->unlock();
    }
};

Barrier B(3);

class CalcThread : Process {
  public:
  virtual int get_num();
};

CalcThread* threads [3];

class Sum : public CalcThread {

  int _id;
  int _iteration;
  int _num;
  
  public:
    Sum (int id) {
      _id = id;
      _iteration = 0;
      _num = 0;
    }

    int get_num() {
      return _num;
    }
    void loop () {
      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(1,100);
      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      
      B.wait();

      int output = 0;
      for (int i = 0; i < 3; ++i)
        output += threads[i]->get_num();

      lightPixel(_id, _iteration);
      Serial.print("Sum = ");
      Serial.println(output);

      B.wait();
    }
};

class Mean : public CalcThread {

  int _id;
  int _iteration;
  int _num;
  
  public:
    Mean (int id) {
      _id = id;
      _iteration = 0;
      _num = 0;
    }

    int get_num() {
      return _num;
    }
    void loop () {
      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(1, 100);
      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      
      B.wait();

      float output = 0.0;
      for (int i = 0; i < 3; ++i)
        output += threads[i]->get_num()/3.0;

      lightPixel(_id, _iteration);
      Serial.print("Mean = ");
      Serial.println(output);

      B.wait();
    }
};

class Printer : public CalcThread {

  int _id;
  int _iteration;
  int _num;
  
  public:
    Printer (int id) {
      _id = id;
      _iteration = 0;
      _num = 0;
    }

    int get_num() {
      return _num;
    }
    void loop () {
      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(300, 1500);
      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      
      B.wait();

      lightPixel(_id, _iteration);
      Serial.print("Numbers: [");
      for (int i = 0; i < 3; ++i) {
        Serial.print(threads[i]->get_num());
        Serial.print(",");
      }
      Serial.println("]");

      B.wait();
    }
};



// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(9600); // open serial terminal
  Process::Init();  // start the threading library

  pinMode(latchPin, OUTPUT);              // set latch as output pin
  pinMode(clockPin, OUTPUT);              // set clock as output pin
  pinMode(dataPin, OUTPUT);               // set data as output pin

  for(int i = 0; i < NUM_COLS; i++) {     // loop through each col
    pinMode(COLS[i], OUTPUT);             // set col pin as output
//    digitalWrite(COLS[i], HIGH);          // turn cathodes off
  }
    for(int i = 0; i < NUM_COLS; i++) {     // loop through each col
//    pinMode(COLS[i], OUTPUT);             // set col pin as output
    digitalWrite(COLS[i], HIGH);          // turn cathodes off
  }

  threads[0] = new Sum(1); //start first thread
  threads[1] = new Mean(2); //start second thread
  threads[2] = new Printer(3); //start third thread
}

// the loop routine runs over and over again forever:
void loop() {
  Process::Start();
}
