#pragma once
#include "include.h"

/**
 * CMVInfetionUpdater handles transitions related to CMV Infection and Mild Illness and updates statistics. During pregnancy, 
 * it covers weekly incident infection of the mother. After birth, it covers monthly incident infection of the child. 
*/

class CMVInfectionUpdater : public StateUpdater {
public:
    /* Constructor and destructor */
    CMVInfectionUpdater(Patient *patient);
    ~CMVInfectionUpdater(void);

    /* performInitialization performs all of the state and statistics updates upon patient creation */
    void performInitialization();
    /* peformWeeklyUpdates perform all of the state and statistics updates for a simulated week DURING pregnancy */
    void performWeeklyUpdates();
    /* performMonthlyUpdates performs all of the state and statistics updates for a simulated month AFTER birth */
    void performMonthlyUpdates();

private:
    /* drawPhenotype draws for the type of CMV that the patient (mother or child) is infected with 
    CHANGED: FUNCTION CURRENTLY DEFINED IN PATIENT OBJECT INITIALIZATION.*/
    //int drawPhenotype();
};