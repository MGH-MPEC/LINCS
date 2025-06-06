#pragma once

#include "include.h"

/**
 * MaternalBiologyUpdater handles transitions related to Maternal biology with CMV infection during pregnancy and updates statistics.
 * It covers weekly incident infection of the mother.
*/

class MaternalBiologyUpdater : public StateUpdater {
public:
    /* Constructor and destructor */
    MaternalBiologyUpdater(Patient *patient);
    ~MaternalBiologyUpdater(void);

    /* performInitialization performs all of the state and statistics updates upon patient creation */
    void performInitialization();
    /* peformWeeklyUpdates perform all of the state and statistics updates for a simulated week DURING pregnancy */
    void performWeeklyUpdates();

    /* WEEKS AFTER INFECTION */
    static const int WEEK_PCR_POSITIVE = 1;
    static const int WEEK_IGM_POSITIVE = 2;
    static const int WEEK_IGG_POSITIVE = 3;
    static const int WEEK_PCR_NEGATIVE = 4;
    static const int WEEKS_IN_A_YEAR = 52;


private:
    //int weeksLowAvidity; MOVED TO PATIENT CLASS UNDER MATERNAL DISEASE STATE

};