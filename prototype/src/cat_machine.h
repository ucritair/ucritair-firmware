typedef enum CAT_machine_signal
{
	CAT_MACHINE_SIGNAL_ENTER,
	CAT_MACHINE_SIGNAL_TICK,
	CAT_MACHINE_SIGNAL_EXIT
} CAT_machine_signal;

typedef void (*CAT_machine_state)(CAT_machine_signal);
extern CAT_machine_state machine;

void CAT_machine_transition(CAT_machine_state state)
{
	if(machine != NULL)
		machine(CAT_MACHINE_SIGNAL_EXIT);
	machine = state;
	if(machine != NULL)
		machine(CAT_MACHINE_SIGNAL_ENTER);
}

void CAT_machine_tick()
{
	if(machine != NULL)
		machine(CAT_MACHINE_SIGNAL_TICK);
}
