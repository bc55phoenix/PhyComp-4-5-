#include <proc.h>
#include <proc_intr.h>

/*
 Life emulation problem
 Scientists have discovered alien life on planet Leviche 538X.
 Unlike earth-based lifeforms, Levicheans have three genders, he, she and it.
 On reaching adulthood, Levichean organisms go to a mating area in search of
 other Levicheans. When a Levichean of one gender comes together with two
 other Levicheans of the other two genders (for example, a she runs into a he
 and an it) in this area, they form a lifelong physical bond and attach
 themselves into a triad. Once in a triad, Levicheans wait sometime and then
 respawn, available to reproduce yet again. As an earth
 scientist, you have been tasked with simulating the mating habits of the
 Levicheans, using threads, locks and condition variables. Each Levichean
 is modeled by a thread. Fill in the missing code segments below such that
 a) the Levichean triad formation is modeled according to the specification
 above, paying special attention to make sure that the three-way join is
 simulated correctly,
 b) the code makes forward progress whenever possible to do so (i.e. your
 simulation should accommodate every mating opportunity that is present on
 Leviche 538X).
 */

#include <proc.h>

class MatingArea {

    /* We have declared one lock and one condition
     * variable here.  You may (or may not) find it
     * useful to declare additional condition variables
     * to solve the problem.  We just left these here as
     * a reminder on the syntax. */

     // We use 3 condition variables, one for each Levichean gender
    Lock* _l;
    Cond* _heCond;
    Cond* _sheCond;
    Cond* _itCond;
    
  public:
    MatingArea () {  // Constructor for club
      _l = new Lock();
      _heCond = new Cond(_l);
      _sheCond = new Cond(_l);
      _itCond = new Cond(_l);
    }

    /*
     * Everytime a Levichean is born and becomes an adult, 
     * it enters its corresponding Levichean_ready function. 
     * 
     * Each Levichean_ready function below behaves exactly like
     * this -
     * On entering it and acquiring the lock, if at least 2 
     * Levicheans of the other genders (different from its own)
     * aren't already waiting, it gives up the lock and waits
     * for someone to signal the time to form a triad.
     * 
     * On the other hand, if at least 2 Levicheans of the 
     * other genders (different from its own) are already
     * waiting, it signals them that it is the 3rd Levichean
     * that they've been waiting for, and the two come out
     * of waiting to form the triad with this Levichean.
     * 
     * That's all.
    */

    void he_ready() {
      /*TODO: Only return when there is a she and an it also ready.*/
      _l->lock();
      
      if(_sheCond->waiting() && _itCond->waiting()) {
        _sheCond->signal();
        _itCond->signal();
        _l->unlock();
        return;
      }
      _l->unlock();
      _heCond->wait();
      return;
    }
   
    void she_ready() {
      /*TODO: Only return when there is a he and an it also ready*/
      _l->lock();
      if(_heCond->waiting() && _itCond->waiting()) {
        _heCond->signal();
        _itCond->signal();
        _l->unlock();
        return;
      }
      _l->unlock();
      _sheCond->wait();
    }

    void it_ready() {
      /*TODO: Only return when there is a he and a she also ready*/
      _l->lock();
      if(_heCond->waiting() && _sheCond->waiting()) {
        _heCond->signal();
        _sheCond->signal();
        _l->unlock();
        return;
      }
      _l->unlock();
      _itCond->wait();
    }
};

MatingArea ma;

class He : Process {
  int _id;

  public:
    He (int id) {
      _id  = id;    // added to easily identify which He
    }

    void loop () {
      delay(random(300, 1500)); //waste time
      Serial.print(F("He: "));
      Serial.print(_id);    
      Serial.println(F(" I'm born!"));
      Serial.print(F("He: "));
      Serial.print(_id);
      Serial.println(F(" Adult now, time to form a triad!"));
      ma.he_ready(); //do not pass until there is a she and an it
      Serial.print(F("He: "));
      Serial.print(_id);
      Serial.println(F(" Yay, I'm part of a triad!"));
    }
};

class She : Process {

  public:
    She () {}
    void loop () {
      delay(random(300, 1500)); //waste time
      Serial.println(F("She: I'm born!"));
      Serial.println(F("She: Adult now, time to form a triad!"));
      ma.she_ready(); //do not pass until there is a he and an it
      Serial.println(F("She: Yay, I'm part of a triad!"));
    }
};

class It : Process {

  public:
    It () {}
    void loop () {
      delay(random(300, 1500)); //waste time
      Serial.println(F("It: I'm born!"));
      Serial.println(F("It: Adult now, time to form a triad!"));
      ma.it_ready(); //do not pass until there is a he and a she
      Serial.println(F("It: Yay, I'm part of a triad!"));
    }
};


// the setup routine runs once when you press reset:
void setup() {

  He *h;
  She *s;
  It *i;

  Serial.begin(9600); // open serial terminal
  Process::Init();  // start the threading library

  ma = MatingArea();
  h = new He(1); //start first thread
  h = new He(2); //start second thread
  s = new She(); //start third thread
  i = new It(); //start fourth thread
}

// the loop routine runs over and over again forever:
void loop() {
  Process::Start();
}
