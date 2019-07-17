#ifndef _H_STOPLIGHT
#define _H_STOPLIGHT


#include <vector>
#include <string>
#include <queue>

#include "sweepdefs.h"
#include "sweep_sm.h"
#include "antares.h"

namespace antares{
  

    class Sweep{

	SweepContext _fsm;  
	  
	  
    // Member functions.
    public:

        // Destructor.
        virtual ~Sweep(){};

        static Sweep* Initialize();
        void start();

        // Signal handles
        inline void Tick(){_fsm.Tick();};
        void TickCallback();



        // Services
        void ShowStatus(char * mes);
        void ToggleFlag();
        void ClearQueue();
        void SetupHardware();
        void JobisDone();

        void Slide(int step, int speed);
        void Goto(int step, int speed);

        void Park();
        void readSensors();
		bool isParked();
		bool isInside();
		bool isOutside();

		void SweepJob();

        // Events
        void serviceEvents();
        void pushEvent(event_t evt);

        void serviceBoard();


    private:
        Sweep(const statemap::State& state);

        void clearMap();


        inline uint32_t getIntap(){return intap;};
        inline uint32_t getOuttap(){return outtap;};


        void setEnable(bool set);
        void setOutDir(bool set);
        void setPulse(bool high);
        void PulseBurst();

        uint8_t setFlag32(uint8_t mon, uint8_t mask, uint32_t value);
        uint8_t setFlagB(uint8_t mon, uint8_t mask, bool value);

        std::queue<event_t> events;
        event_t *dummyevt;

        uint8_t sweepmap[4000];
        uint8_t *sweepmapinit;
        uint8_t *sweepmapptr;
        int sweeppos;
        uint8_t pulsegap;

        bool enable;
        bool outdir;
        bool pulse;

        uint32_t intap;
        uint8_t *intapptr;
        uint32_t outtap;
        uint8_t *outtapptr;
        uint32_t temptap;
        bool parked;
        uint8_t *parkedptr;
        bool tparked;
        uint32_t overcurrent;

        int sweepjob_in;
        int sweepjob_out;
        int sweepjob_speed;
        bool sweepjob_outdir;
        bool sweepjob_enabled;

    }; // end of class Sweep
}; // end of namespace cpp_ex4

#endif
