// This was built in Eclipse with GCC, you'll have to mud wrestle with the .ino compiler
// if you want it to work in Arduino
// Only ported for Uno variants
// Some stuff is roughed in because I'm lazy.
// This should probably be factored into a header file, but I'm just posting this
// for curiosity

#include <Arduino.h>

#include "wiring_private.h"
#include "pins_arduino.h"


typedef enum
{
   P_NONE = 0,
   PA = 1,
   PB = 2,
   PC = 3,
   PD = 4,
   PE = 5,
   PF = 6,
   PG = 7,
   PH = 8,
   PJ = 10,
   PK = 11,
   PL = 12,
} Ports;



// This could be templatized at some cost.  I'd use the original
// one, but it is also declared static...
static void turnOffPWMT(const uint8_t timer)
{
   switch (timer)
   {
      #if defined(TCCR1A) && defined(COM1A1)
      case TIMER1A:   cbi(TCCR1A, COM1A1);    break;
      #endif
      #if defined(TCCR1A) && defined(COM1B1)
      case TIMER1B:   cbi(TCCR1A, COM1B1);    break;
      #endif

      #if defined(TCCR2) && defined(COM21)
      case  TIMER2:   cbi(TCCR2, COM21);      break;
      #endif

      #if defined(TCCR0A) && defined(COM0A1)
      case  TIMER0A:  cbi(TCCR0A, COM0A1);    break;
      #endif

      #if defined(TIMER0B) && defined(COM0B1)
      case  TIMER0B:  cbi(TCCR0A, COM0B1);    break;
      #endif
      #if defined(TCCR2A) && defined(COM2A1)
      case  TIMER2A:  cbi(TCCR2A, COM2A1);    break;
      #endif
      #if defined(TCCR2A) && defined(COM2B1)
      case  TIMER2B:  cbi(TCCR2A, COM2B1);    break;
      #endif

      #if defined(TCCR3A) && defined(COM3A1)
      case  TIMER3A:  cbi(TCCR3A, COM3A1);    break;
      #endif
      #if defined(TCCR3A) && defined(COM3B1)
      case  TIMER3B:  cbi(TCCR3A, COM3B1);    break;
      #endif
      #if defined(TCCR3A) && defined(COM3C1)
      case  TIMER3C:  cbi(TCCR3A, COM3C1);    break;
      #endif

      #if defined(TCCR4A) && defined(COM4A1)
      case  TIMER4A:  cbi(TCCR4A, COM4A1);    break;
      #endif
      #if defined(TCCR4A) && defined(COM4B1)
      case  TIMER4B:  cbi(TCCR4A, COM4B1);    break;
      #endif
      #if defined(TCCR4A) && defined(COM4C1)
      case  TIMER4C:  cbi(TCCR4A, COM4C1);    break;
      #endif
      #if defined(TCCR4C) && defined(COM4D1)
      case TIMER4D:  cbi(TCCR4C, COM4D1); break;
      #endif

      #if defined(TCCR5A)
      case  TIMER5A:  cbi(TCCR5A, COM5A1);    break;
      case  TIMER5B:  cbi(TCCR5A, COM5B1);    break;
      case  TIMER5C:  cbi(TCCR5A, COM5C1);    break;
      #endif
   }
}

template <const uint8_t pin>
const uint8_t digitalPinToTimerT()
{
   switch(pin)
   {
   case 3: return TIMER2B;
   case 5: return TIMER0B;
   case 6: return TIMER0A;
   case 9:  return TIMER1A;
   case 10: return TIMER1B;
   case 11: return TIMER2A;
   default: return NOT_ON_TIMER;
   }
};

template <const uint8_t pin>
struct digitalPinToPortS {};
template <> struct digitalPinToPortS<LED_BUILTIN>   { static const Ports kPort = PB; };
template <> struct digitalPinToPortS<LED_BUILTIN-1> { static const Ports kPort = PB; };


// finally!  a reason to use the ellipsis!
template <const uint8_t pin>
inline const Ports digitalPinToPortT()
{
   switch(pin)
   {
   case 0 ... 7:
      return PD;
   case 8 ... 13:
      return PB;
   case 14 ... 19:
      return PC;
   }
   return P_NONE;
}

// a couple of notes.
// this is a really naive way of doing this, but because it is resolved at compile time,
// it can be pretty much as dumb as you want it to be.  Here, I copied the program table
// for these and just turned it into a switch statement.
// There is certainly a mathematical way of expressing this, but it wasn't worth the time
// to figure it out since it has no run-time effect.
template <const uint8_t pin>
const uint8_t digitalPinToBitMaskT()
{
   switch(pin)
   {
   case 0:  return _BV(0); /* 0, port D */
   case 1:  return _BV(1);
   case 2:  return _BV(2);
   case 3:  return _BV(3);
   case 4:  return _BV(4);
   case 5:  return _BV(5);
   case 6:  return _BV(6);
   case 7:  return _BV(7);
   case 8:  return _BV(0); /* 8, port B */
   case 9:  return _BV(1);
   case 10: return _BV(2);
   case 11: return _BV(3);
   case 12: return _BV(4);
   case 13: return _BV(5);
   case 14: return _BV(0); /* 14, port C */
   case 15: return _BV(1);
   case 16: return _BV(2);
   case 17: return _BV(3);
   case 18: return _BV(4);
   case 19: default: return _BV(5);
   }
};

// template time, but can't constexpr because of the way PORTxx is defined
// well... it could be constexpr, but we'd have to use a private declaration of the
// location of PORTxx.
template <Ports P>
volatile uint8_t* portToOutputT()
{
   switch(P)
   {
   case PD: return &PORTD;
   case PC: return &PORTC;
   case PB: return &PORTB;
   default:
      return NOT_A_PORT;
   }
};

template <const uint8_t pin>
void __attribute__((noinline)) digitalWriteT(uint8_t val)
{
   // In the original digitalWrite, these were implemented as a series of table lookups into
   // program space.  This is a perfectly legitimate, and flexible way to define the tables,
   // but it has some limitations.
   // In c++11, some of these can be declared constexpr, which allows them to be chained
   // into other templates or other constexprs which makes programming them less heinous.
   const uint8_t timer = digitalPinToTimerT<pin>();
   const uint8_t bit   = digitalPinToBitMaskT<pin>();
   const Ports   port  = digitalPinToPortT<pin>();

   // so this was a macro to start with, and reduced to loading an offset.  This takes three instructions.
   // This suffers from the same problem that the others did, in that lpm instructions cost more
   // and have longer run time than just an immediate load.  This resulting line of code takes up less space
   // to execute, and takes less time to execute because everything is resolved at compile time.
   volatile uint8_t * const out = portToOutputT<digitalPinToPortS<pin>::kPort>(); //portOutputRegister(port);

   if (port == NOT_A_PIN) return;

   // If the pin that support PWM output, we need to turn it off
   // before doing a digital write.
   if (timer != NOT_ON_TIMER) turnOffPWMT(timer);

   // Important: I wanted to leave digitalWriteT with the same inteface contract as digitalWrite.
   // If a user has PWM on, we disable that and set the value.  If they have other routines that
   // might modify the same port at ISR time, it needs to still work correctly.
   // I'm sure with some more template mojo, that this can be optimized down even further, and
   // with some more work could be made transactional - hit bits in an arbitrary sequence all while
   // holding off ISRs for maximum throughput (and maximum interrupt jitter)
   const uint8_t oldSREG = SREG;
   cli();  // hmm.  I wonder if any ISRs modify SREG...

   if (val == LOW) {
      *out &= ~bit; // not an atomic operation, this is a read, modify, write.
   } else {
      *out |= bit; // not an atomic operation, this is a read, modify, write.
   }

   SREG = oldSREG;
}

void setup(void)
{
   pinMode(LED_BUILTIN, OUTPUT);

}

void loop(void)
{
   digitalWrite(LED_BUILTIN, LOW);
   int state = 0;
   while (1)
   {
#if 1
      digitalWriteT<LED_BUILTIN>(state); // 389 kHz toggle rate
#else
      digitalWrite(LED_BUILTIN, state); // 122 kHz toggle rate
#endif
      state ^= 1;
   }
}
