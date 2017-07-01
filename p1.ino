/*
 Time Waster Wars: Reddit vs. 4chan

 a. Add synchronization primitives to ensure that
    (a) the club is exclusively redditors or 4channers, i.e. no redditors
        should enter as long as there are 4channers in the club,
        and vice versa,
    (b) the club should always be used as long as there are
        customers

    Note that starvation is not something you need to worry
    about. If the club becomes redditors and remains exclusively
    redditors for all time, the waiting 4channers will just have
    to get old at the door.

 Modify only the code of the class Club to make the program
 correct. (but add the LED manipulation to the other classes)

 Place your synchronization variables inside the Club instance.
 */

#include <proc.h>


void hang_out() {
//  Serial.println("start hang_out");
  delay(random(300,1500)); //waste time
//  Serial.println("end hang_out");
}

class Club {
  
  /* We have declared one lock and one condition
   * variable here.  You may (or may not) find it
   * useful to declare additional condition variables
   * to solve the problem.  We just left these here as
   * a reminder on the syntax. */

  /*  We have two condition variables (one for redditors
   *  and one for 4channers).
   *  
   *  We also have two counter integer variables to keep 
   *  track of the number of redditors and 4channers that 
   *  are either waiting or are in the club.
  */
   
  Lock *_l;
  Cond *_c_redditors;
  Cond *_c_fourchanners; 
  int _redditor_count;
  int _fourchanner_count;
  
  public:
    Club () {  //Constructor for club
      _l = new Lock();
      _c_redditors = new Cond(_l);
      _c_fourchanners = new Cond(_l);
      _redditor_count = 0;
      _fourchanner_count = 0;
    }

    ~Club () { //Destructor for club
      delete _l;
      delete _c_redditors;
      delete _c_fourchanners;
    }

    /* 
     * Everytime a thread of a customer type is born,it enters 
     * its corresponding 'customer_enter' function. 
     * 
     * On doing so, it acquires the lock and if the number of 
     * customers of the other type is greater than 0, it means
     * that the club is infested by those not of its own kind.
     * Therefore, it releases the lock and waits.
     * 
     * On the flip side, it increments the counter for its own
     * kind whenever it enters the club and signals the other 
     * waiting thread of its kind if any exist. This happens either 
     * when there were no other type of customers in the club or 
     * when it is signaled by the other type of customers that they
     * have all left. 
     * 
     * When a thread finishes hanging out, it etners the 
     * 'customer_exit' function. It acquires the lock
     * and decrements the counter for its kind. If this counter 
     * becomes 0, it signals the waiting thread of the other 
     * customer type.
     * 
     */
    
    void redditor_enter() {
      _l->lock();
      if (_fourchanner_count > 0) {
        _l->unlock();
        _c_redditors->wait();
      } 
      _redditor_count++;
      if (_c_redditors->waiting()) {
          _c_redditors->signal();
      } else {
          _l->unlock();
      }
    }

    void redditor_exit() {  
      _l->lock();
      _redditor_count -= 1;
      if (_redditor_count == 0) {
        if (_c_fourchanners->waiting()) {
          _c_fourchanners->signal();
          return;
        }
      }
      _l->unlock();
    }

    void fourchanner_enter() {
      _l->lock();
      if (_redditor_count > 0) {
        _l->unlock();
        _c_fourchanners->wait();
      } 
      _fourchanner_count++;
      if (_c_fourchanners->waiting()) {
          _c_fourchanners->signal();
      } else {
          _l->unlock();
      }
    }

    void fourchanner_exit() {
      _l->lock();
      _fourchanner_count -= 1;
      if (_fourchanner_count == 0) {
        if (_c_redditors->waiting()) {
          _c_redditors->signal();
          return;
        }
      }
      _l->unlock();
    }
};

Club *daclub;

class Redditor: Process {
  int _id;

public:
  Redditor (int id) { 
    _id = id; 
  }

  void loop () {
    daclub->redditor_enter();
    Serial.print(F("Redditor "));
    Serial.print(_id);
    Serial.println(F(": in the club"));
    lightColumn(_id);
    hang_out();
    daclub->redditor_exit();
    delay(200);       // So that starvation doesn't occur and become exclusively redditors
  }
};

class Fourchanner: Process {
  int _id;

public:
  Fourchanner (int id) { 
    _id = id; 
  }

  void loop () {
    daclub->fourchanner_enter();
    Serial.print(F("Fourchanner "));
    Serial.print(_id);                  
    Serial.println(F(": in the club"));
    lightColumn(_id);
    hang_out();
    daclub->fourchanner_exit();
    delay(200);       // So that starvation doesn't occur and become exclusively 4channers
  }
};


// constants won't change. They're uuuset here.
const int dataPin = 11;   // data pin (pin14 on 74ch595)
const int clockPin = 12;  // clock pin (pin11 on 74ch595)
const int latchPin = 10;  // latch pin (pin12 on 74ch595)
const int NUM_COLS = 5;  // 5 cathode columns
const int COLS[] = {2, 3, 4 ,5, 6};   // arduino pins connected through resistor to the 5 cathode columns

// the setup routine runs once when you press reset:
void setup() {                

  Redditor *r;
  Fourchanner *f;
  
  Serial.begin(9600); // open serial terminal
  Process::Init();  // start the threading library
  daclub = new Club();
  r = new Redditor(1); //start first thread
  r = new Redditor(2); //start second thread
  f = new Fourchanner(4); //start third thread
  f = new Fourchanner(5); //start fourth thread
  
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
}

void loop() {
  Process::Start();
}

void lightColumn(int column) {
  digitalWrite(COLS[column-1],LOW);
  digitalWrite(latchPin,LOW);
  shiftOut(dataPin,clockPin,MSBFIRST,0x7F); // Hexadecimal of 01111111
  delay(200);                               // So that column is visible 
  digitalWrite(COLS[column-1],HIGH);
  digitalWrite(latchPin,HIGH);
}
