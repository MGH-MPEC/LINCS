#pragma once

#include "include.h"

/**
 * MaternalBiologyUpdater handles transitions related to Maternal biology with CMV infection during pregnancy and updates statistics.
 * It covers weekly incident infection of the mother.
*/

class PregnancyUpdater : public StateUpdater {
public:
    /* Constructor and destructor */
    PregnancyUpdater(Patient *patient);
    ~PregnancyUpdater(void);

    /* performInitialization performs all of the state and statistics updates upon patient creation */
    void performInitialization();
    /* peformWeeklyUpdates perform all of the state and statistics updates for a simulated week DURING pregnancy */
    void performWeeklyUpdates();


private:

};