#include "include.h"

class BeginWeekUpdater : public StateUpdater {
public:
    /* Constructor and destructor */
    BeginWeekUpdater(Patient *patient);
    ~BeginWeekUpdater(void);

    /* performInitialization performs all of the state and statistics updates upon patient creation */
    void performInitialization();
    /* peformWeeklyUpdates perform all of the state and statistics updates for a simulated week DURING pregnancy */
    void performWeeklyUpdates();
};
