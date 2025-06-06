#pragma once

#include "include.h"

class PrenatalTestUpdater : public StateUpdater {
public:
    /* Constructor and destructor */
    PrenatalTestUpdater(Patient *patient);
    ~PrenatalTestUpdater(void);


    /* performInitialization performs all of the state and statistics updates upon patient creation */
    void performInitialization();
    /* peformWeeklyUpdates perform all of the state and statistics updates for a simulated week DURING pregnancy */
    void performUpdates();
    /** peformWeeklyUpdates perform all of the prenatal tests state and statistics updates for a simulated week DURING pregnancy */
    void performWeeklyUpdates();

    /** determineIfScheduledVisit returns a boolean for whether there is a scheduled clinical visit during a particular time period */
    bool determineIfScheduledVisit();
    /** determineIfAllTestsPerformed returns a boolean for whether all scheduled tests for the current week have been performed.*/
    bool determineIfAllTestsPerformed(vector<SimContext::PrenatalTestInstance> testsThisWeek);


private:
    
    /** Vector containing the tests that are left to administer. Updated as each test is performed. */
    /** Vector containing the additional tests this week that were scheduled in the same week. Will be recursively read into prenatalTestsToPerform */
    vector<SimContext::PrenatalTestInstance> additionalTestsThisWeek;
    /** getTestsThisWeek gets a vector containing the tests that are administered in a particular week */
    vector<SimContext::PrenatalTestInstance> getTestsThisWeek();

    /** containsTestType function returns a boolean for whether a vector containing prenatal test states contains a particular test type. 
     * This is a polymorphic function that works regardless of whether the test in question is given as a test state or just the test type number.
    */
    bool containsTestType(vector< SimContext::PrenatalTestInstance> vect, int prenatalTestType);
    bool containsTestType(vector< SimContext::PrenatalTestInstance> vect, SimContext::PrenatalTestInstance prenatalTestState);

    /** initializePrenatalTestState initializes the PrenatalTestState object for a scheduled test */
    SimContext::PrenatalTestInstance initializePrenatalTestInstance(SimContext::PrenatalTestingInputs::PrenatalTest prenatalTest, int testID);

    /** initializePrenatalFollowUpState initializes the PrenatalTestState object for a follow-up test */
    SimContext::PrenatalTestInstance initializePrenatalFollowUpState(SimContext::PrenatalTestingInputs::PrenatalTest prenatalTest, SimContext::PrenatalTestInstance testFollowingUpFrom, int weeksDelay, int testID);

    /** iniitalizeConfirmatoryTestState intializes the PrenatalTestState object for a confirmatory test */
    SimContext::PrenatalTestInstance initializePrenatalConfirmatoryState(SimContext::PrenatalTestingInputs::PrenatalTest prenatalTest, SimContext::PrenatalTestInstance testFollowingUpFrom, int weeksDelay, int testID);

    /** Weekly checking function that checks the diagnostic outcome from the IgM/IgG assay combination */
    void antibodyTestsOutcomesUpdater();

    /** Weekly checking function that checks the diagnostic outcome after Avidity test is returned */
    void postAvidityTestOutcomesUpdater(SimContext::PrenatalTestInstance prenatalTestInstance);

    /** Weekly checking function that checks the diagnostic outcome after Maternal CMV test is returned */
    void postMaternalPCROutcomesUpdater(SimContext::PrenatalTestInstance prenatalTestInstance);

    /* Checking function that checks the diagnostic outcome after the Amniocentesis CMV test is returned */
    void postAmniocentesisTest(SimContext::PrenatalTestInstance prenatalTestInstance);

    /* PERFORMING THE TESTS */
    /** General performTest function that specifies events during a prenatal test */
    SimContext::PrenatalTestInstance performTest(SimContext::PrenatalTestInstance prenatalTestInstance);
    /** How specific tests should be performed */
    /** performRoutineUltrasound specifies events that occur during an ultrasound test */
    int performRoutineUltrasound();

    int performDetailedUltrasound();

    int performMaternalPCR();

    int performIgM();

    int performIgG();

    int performIgGAvidity();

    int performAmniocentesis();

    /** endTest function creates the follow-up and confirmatory test objects and adds results to the patient and runStats objects */
    void endTest(SimContext::PrenatalTestInstance prenatalTestInstance);

    
};

