//
// ex: set ro:
// DO NOT EDIT.
// generated by smc (http://smc.sourceforge.net/)
// from file : sweep.sm
//

#ifndef SWEEP_SM_H
#define SWEEP_SM_H


#include <statemap.h>

namespace antares
{
    // Forward declarations.
    class Adm;
    class Adm_ColdStart;
    class Adm_Idle;
    class Adm_setParked;
    class Adm_searchOut;
    class Adm_searchIn;
    class Adm_Default;
    class SweepState;
    class SweepContext;
    class Sweep;

    class SweepState :
        public statemap::State
    {
    public:

        SweepState(const char * const name, const int stateId)
        : statemap::State(name, stateId)
        {};

        virtual void Entry(SweepContext&) {};
        virtual void Exit(SweepContext&) {};

        virtual void JobDone(SweepContext& context);
        virtual void Park(SweepContext& context);
        virtual void Tick(SweepContext& context);

    protected:

        virtual void Default(SweepContext& context);
    };

    class Adm
    {
    public:

        static Adm_ColdStart ColdStart;
        static Adm_Idle Idle;
        static Adm_setParked setParked;
        static Adm_searchOut searchOut;
        static Adm_searchIn searchIn;
    };

    class Adm_Default :
        public SweepState
    {
    public:

        Adm_Default(const char * const name, const int stateId)
        : SweepState(name, stateId)
        {};

        virtual void JobDone(SweepContext& context);
    };

    class Adm_ColdStart :
        public Adm_Default
    {
    public:
        Adm_ColdStart(const char * const name, const int stateId)
        : Adm_Default(name, stateId)
        {};

        virtual void Entry(SweepContext&);
        virtual void Tick(SweepContext& context);
    };

    class Adm_Idle :
        public Adm_Default
    {
    public:
        Adm_Idle(const char * const name, const int stateId)
        : Adm_Default(name, stateId)
        {};

        virtual void Entry(SweepContext&);
        virtual void Park(SweepContext& context);
        virtual void Tick(SweepContext& context);
    };

    class Adm_setParked :
        public Adm_Default
    {
    public:
        Adm_setParked(const char * const name, const int stateId)
        : Adm_Default(name, stateId)
        {};

        virtual void Entry(SweepContext&);
        virtual void Tick(SweepContext& context);
    };

    class Adm_searchOut :
        public Adm_Default
    {
    public:
        Adm_searchOut(const char * const name, const int stateId)
        : Adm_Default(name, stateId)
        {};

        virtual void Entry(SweepContext&);
        virtual void JobDone(SweepContext& context);
    };

    class Adm_searchIn :
        public Adm_Default
    {
    public:
        Adm_searchIn(const char * const name, const int stateId)
        : Adm_Default(name, stateId)
        {};

        virtual void Entry(SweepContext&);
        virtual void JobDone(SweepContext& context);
    };

    class SweepContext :
        public statemap::FSMContext
    {
    public:

        explicit SweepContext(Sweep& owner)
        : FSMContext(Adm::ColdStart),
          _owner(owner)
        {};

        SweepContext(Sweep& owner, const statemap::State& state)
        : FSMContext(state),
          _owner(owner)
        {};

        virtual void enterStartState()
        {
            getState().Entry(*this);
            return;
        }

        inline Sweep& getOwner()
        {
            return (_owner);
        };

        inline SweepState& getState()
        {
            if (_state == NULL)
            {
                throw statemap::StateUndefinedException();
            }

            return dynamic_cast<SweepState&>(*_state);
        };

        inline void JobDone()
        {
            getState().JobDone(*this);
        };

        inline void Park()
        {
            getState().Park(*this);
        };

        inline void Tick()
        {
            getState().Tick(*this);
        };

    private:
        Sweep& _owner;
    };
}


#endif // SWEEP_SM_H

//
// Local variables:
//  buffer-read-only: t
// End:
//
