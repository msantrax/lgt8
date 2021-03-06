//
// ex: set ro:
// DO NOT EDIT.
// generated by smc (http://smc.sourceforge.net/)
// from file : servo.sm
//

#ifndef SERVO_SM_H
#define SERVO_SM_H


#include <statemap.h>

namespace antares
{
    // Forward declarations.
    class ServoAdm;
    class ServoAdm_ColdStart;
    class ServoAdm_Idle;
    class ServoAdm_Default;
    class ServoState;
    class ServoContext;
    class Servo;

    class ServoState :
        public statemap::State
    {
    public:

        ServoState(const char * const name, const int stateId)
        : statemap::State(name, stateId)
        {};

        virtual void Entry(ServoContext&) {};
        virtual void Exit(ServoContext&) {};

        virtual void JobDone(ServoContext& context);
        virtual void Tick(ServoContext& context);

    protected:

        virtual void Default(ServoContext& context);
    };

    class ServoAdm
    {
    public:

        static ServoAdm_ColdStart ColdStart;
        static ServoAdm_Idle Idle;
    };

    class ServoAdm_Default :
        public ServoState
    {
    public:

        ServoAdm_Default(const char * const name, const int stateId)
        : ServoState(name, stateId)
        {};

        virtual void JobDone(ServoContext& context);
    };

    class ServoAdm_ColdStart :
        public ServoAdm_Default
    {
    public:
        ServoAdm_ColdStart(const char * const name, const int stateId)
        : ServoAdm_Default(name, stateId)
        {};

        virtual void Entry(ServoContext&);
        virtual void Tick(ServoContext& context);
    };

    class ServoAdm_Idle :
        public ServoAdm_Default
    {
    public:
        ServoAdm_Idle(const char * const name, const int stateId)
        : ServoAdm_Default(name, stateId)
        {};

        virtual void Entry(ServoContext&);
        virtual void Tick(ServoContext& context);
    };

    class ServoContext :
        public statemap::FSMContext
    {
    public:

        explicit ServoContext(Servo& owner)
        : FSMContext(ServoAdm::ColdStart),
          _owner(owner)
        {};

        ServoContext(Servo& owner, const statemap::State& state)
        : FSMContext(state),
          _owner(owner)
        {};

        virtual void enterStartState()
        {
            getState().Entry(*this);
            return;
        }

        inline Servo& getOwner()
        {
            return (_owner);
        };

        inline ServoState& getState()
        {
            if (_state == NULL)
            {
                throw statemap::StateUndefinedException();
            }

            return dynamic_cast<ServoState&>(*_state);
        };

        inline void JobDone()
        {
            getState().JobDone(*this);
        };

        inline void Tick()
        {
            getState().Tick(*this);
        };

    private:
        Servo& _owner;
    };
}


#endif // SERVO_SM_H

//
// Local variables:
//  buffer-read-only: t
// End:
//
