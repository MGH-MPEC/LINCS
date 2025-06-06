#include "include.h"

/**
	The EndWeekUpdater class is a state updater that is run at the end of each simulated
	week.  It updates and accumulates all necessary patient state and statistics for that
	week, and generates the weekly tracing during pregnancy. 
*/
class EndWeekUpdater : public StateUpdater {
public:
    /* Constructor and destructor */
    EndWeekUpdater(Patient *patient);
    ~EndWeekUpdater(void);

    /* performInitialization performs all of the state and statistics updates upon patient creation */
    void performInitialization();
    /* peformWeeklyUpdates perform all of the state and statistics updates for a simulated week DURING pregnancy */
    void performWeeklyUpdates();
    /* performEndPatientUpdates updates the relevant runstats for the end of a patient's simulation */
    void performEndPatientUpdates();
};