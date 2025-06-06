#include "include.h"

/* BEGIN MONTH UPDATER FUNCTIONS */
/** \brief Constructor takes in the patient object and determines if updates can occur */
PrenatalTestUpdater::PrenatalTestUpdater(Patient *patient) : StateUpdater(patient) {
    
}



/* \brief Destructor is empty, no cleanup required */
PrenatalTestUpdater::~PrenatalTestUpdater(void) {
    //delete testState;
}

void PrenatalTestUpdater::performInitialization() {
    StateUpdater::performInitialUpdates();
}


/*
PrenatalTestUpdater::PrenatalTestState::~PrenatalTestState(void) {
    //void
}
*/

// Initializing Prenatal Test State for Base Test

SimContext::PrenatalTestInstance PrenatalTestUpdater::initializePrenatalTestInstance(SimContext::PrenatalTestingInputs::PrenatalTest prenatalTest, int testID) {
    double randNum;
    int testAssay = prenatalTest.testArray;
    int testType = SimContext::PN_TEST_BASE;
    int confirmatoryNumber = SimContext::PN_NOT_CONF;
    int testFollowingUpFrom = -1;
    int weekScheduled = patient->getGeneralState()->weekNum;
    int weekToTest = patient->getGeneralState()->weekNum;
    bool testPerformed = false;
    bool result = false;
    bool returnToPatient = false;
    randNum = CmvUtil::getRandomDouble();
    if (randNum < prenatalTest.probResultReturn) {
        returnToPatient = true;
    }
    int weekToReturn = patient->getGeneralState()->weekNum + prenatalTest.weeksToResultReturn;
    // Return all tests by end of pregnancy. 
    if (weekToReturn >= SimContext::MAX_WEEKS_PREGNANT) {
        weekToReturn = SimContext::MAX_WEEKS_PREGNANT - 1;
    }
    SimContext::PrenatalTestInstance baseTestInstance = {testID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom, weekScheduled, 
        weekToTest, testPerformed, result, weekToReturn, returnToPatient};

    
    StateUpdater::incrementPrenatalTestID();
    return baseTestInstance;
}


// Initiatializing Prenatal Test State for Follow-up test
SimContext::PrenatalTestInstance PrenatalTestUpdater::initializePrenatalFollowUpState(SimContext::PrenatalTestingInputs::PrenatalTest prenatalTest, SimContext::PrenatalTestInstance testFollowingUpFrom, int weeksDelay, int testID) {
    
    int weekScheduled = patient->getGeneralState()->weekNum;
    int weekToTest = patient->getGeneralState()->weekNum + weeksDelay;
    bool returnToPatient = false;
    double randNum = CmvUtil::getRandomDouble();
    if (randNum < prenatalTest.probResultReturn) {
        returnToPatient = true;
    }
    int weekToReturn = patient->getGeneralState()->weekNum + prenatalTest.weeksToResultReturn;
    SimContext::PrenatalTestInstance followUpTestInstance = {testID, prenatalTest.testArray, SimContext::PN_NOT_CONF, testFollowingUpFrom.testAssay, weekScheduled, weekToTest,
        false, false, weekToReturn, returnToPatient};
    StateUpdater::incrementPrenatalTestID();
    return followUpTestInstance;
}

// Initializing Prenatal Test State for Confirmatory test
SimContext::PrenatalTestInstance PrenatalTestUpdater::initializePrenatalConfirmatoryState(SimContext::PrenatalTestingInputs::PrenatalTest prenatalTest, SimContext::PrenatalTestInstance testFollowingUpFrom, int weeksDelay, int testID) {
    int testAssay = prenatalTest.testArray;
    int testType = SimContext::PN_TEST_CONF;
    int confirmatoryNumber = testFollowingUpFrom.confirmatoryNumber + 1;
    int weekScheduled = patient->getGeneralState()->weekNum;
    int weekToTest = patient->getGeneralState()->weekNum + weeksDelay;
    bool testPerformed = false;
    bool result = false;
    /** Rolling for whether to return the result to the patient */
    bool returnToPatient = false;
    double randNum = CmvUtil::getRandomDouble();
    if (randNum < prenatalTest.probResultReturn) {
        returnToPatient = true;
    }
    int weekToReturn = patient->getGeneralState()->weekNum + prenatalTest.weeksToResultReturn;
    SimContext::PrenatalTestInstance confirmatoryTestInstance = {testID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom.testAssay, weekScheduled,
        weekToTest, testPerformed, result, weekToReturn, returnToPatient};
    StateUpdater::incrementPrenatalTestID();
    
    return confirmatoryTestInstance;
}

void PrenatalTestUpdater::performUpdates() {
    //void
}


/* determines whether there is a scheduled appointment this week*/

bool PrenatalTestUpdater::determineIfScheduledVisit() {
    bool visitThisWeek = false;
    int i;
    int interval;
    int timeBucketMin;
    int weekNum = patient->getGeneralState()->weekNum;
    bool observedSymptomatic;
    int maternalInfectionCondition;
    
    /** Can make the following if/else if/else statement into a for loop to reduce redundancy but this seems more readable */
    /* if between month N0 and N1 on input sheet (C25 and C26 on Prenatal tab)*/
    if (patient->getChildMonitoringState()->knownCMV) {

        observedSymptomatic = patient->getChildMonitoringState()->observedSymptomatic;
        if (simContext->getBackgroundScreeningInputs()->weeksWithVisitsUponChildDiagnosis[weekNum][observedSymptomatic] == true) {
            visitThisWeek = true;
        }
        
    }
    else if (patient->getMaternalMonitoringState()->knownCMV) {
        maternalInfectionCondition = patient->getMaternalMonitoringState()->diagnosedCMVStatus;
        if (simContext->getBackgroundScreeningInputs()->weeksWithVisitsUponMaternalDiagnosis[weekNum][maternalInfectionCondition] == true) {
            visitThisWeek = true;
        }
    }
    else {
        if (simContext->getBackgroundScreeningInputs()->weeksWithScheduledAppointments[weekNum] == true) {
            visitThisWeek = true;
        }
    }
    return visitThisWeek;
}

/* determines if all tests this week have been performed */
bool PrenatalTestUpdater::determineIfAllTestsPerformed(vector<SimContext::PrenatalTestInstance> testsThisWeek) {
    bool allTestsPerformed = true;
    for (int i = 0; i < testsThisWeek.size(); i++) {
        if (testsThisWeek[i].testPerformed == false) {
            allTestsPerformed = false;
        }
    }
    return allTestsPerformed;
}


vector<SimContext::PrenatalTestInstance> PrenatalTestUpdater::getTestsThisWeek() {
    int i;
    double randNum;
    int weekNum = patient->getGeneralState()->weekNum;
    bool scheduledVisit;
    vector<SimContext::PrenatalTestInstance> testsThisWeek;
    testsThisWeek.clear();

    /** FOLLOW-UP TESTS */
    // looping over all scheduled follow-up tests for this week. Add to testsThisWeek. 
    for (i = 0; i < patient->getGeneralState()->pendingFollowUpTests[weekNum].size(); i++) {
        SimContext::PrenatalTestInstance followUpTest = patient->getGeneralState()->pendingFollowUpTests[weekNum][i];
        //Checking for tests that are scheduled for the current week
        if (followUpTest.weekToTest == patient->getGeneralState()->weekNum) {
            // if week num is within the weeks that the test is available, continue.
            if (patient->getGeneralState()->weekNum >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[followUpTest.testAssay].availableStartingWeek &&
            patient->getGeneralState()->weekNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[followUpTest.testAssay].notAvailableAfter) {
                // If weeks since test was last performed is greater than the test's minimum test interval, continue. Otherwise, don't.
                if ((weekNum - patient->getGeneralState()->weekTestLastPerformed[followUpTest.testAssay]) >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[followUpTest.testAssay].minimumTestInterval) {
                    randNum = CmvUtil::getRandomDouble();
                    // Roll for whether test is given based on probability the test is offered and probability the test is performed. If yes, add to testsThisWeek vector.
                    if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[followUpTest.testAssay].probOfferedAndAccept) {
                        testsThisWeek.push_back(followUpTest);
                    }
                }
            }
        }
    }

    /** CONFIRMATORY TESTS*/
    // looping over all scheduled confirmatory tests for this week. Add to testsThisWeek ONLY IF scheduled follow-up tests don't override them. 
    for ( i = 0; i < patient->getGeneralState()->pendingConfirmatoryTests[weekNum].size(); i++) {
        SimContext::PrenatalTestInstance confirmatoryTest = patient->getGeneralState()->pendingConfirmatoryTests[weekNum][i];
        // Checking for tests that are scheduled for the current week
        if (confirmatoryTest.weekToTest == patient->getGeneralState()->weekNum) {
            // if week num is within the weeks that the test is available, continue.
            if (patient->getGeneralState()->weekNum >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[confirmatoryTest.testAssay].availableStartingWeek &&
            patient->getGeneralState()->weekNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[confirmatoryTest.testAssay].notAvailableAfter) {

                // Checking if test type is already in the pending Follow-Up test vectors. Only add the test to testsThisWeek if it's NOT already in the scheduled follow-up vector. 
                if (!containsTestType(patient->getGeneralState()->pendingFollowUpTests[weekNum], confirmatoryTest)) {
                    // Checking if testsThisWeek already contains this test type
                    if (!containsTestType(testsThisWeek, confirmatoryTest)) {
                        // If weeks since test was last performed is greater than the test's minimum test interval, continue. Otherwise, don't.
                        if ((patient->getGeneralState()->weekNum - patient->getGeneralState()->weekTestLastPerformed[confirmatoryTest.testAssay]) >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[confirmatoryTest.testAssay].minimumTestInterval) {
                            // Roll for whether test is given based on probability the test is offered and probability the test is performed. If yes, add to testsThisWeek vector.
                            randNum = CmvUtil::getRandomDouble();
                            if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[confirmatoryTest.testAssay].probOfferedAndAccept) {
                                testsThisWeek.push_back(confirmatoryTest);
                            }
                        }
                    }
                }
            }
        }
    }

    /** SCHEDULED BASE TESTS */
    // Checking to see if there is a scheduled visit this week (base tests will only be done if there is a scheduled visit)
    if (determineIfScheduledVisit()) {
        // looping over all scheduled base tests, add to tests this week ONLY IF scheduled follow-up and confirmatory tests don't override them.
        for (i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
            bool scheduledTest;
            if (patient->getChildMonitoringState()->knownCMV) {
                scheduledTest = simContext->getBackgroundScreeningInputs()->testingUponChildDiagnosis[patient->getGeneralState()->weekNum][i][patient->getChildMonitoringState()->observedSymptomatic];
            }
            else if (patient->getMaternalMonitoringState()->knownCMV) {
                scheduledTest = simContext->getBackgroundScreeningInputs()->testingUponMaternalDiagnosis[patient->getGeneralState()->weekNum][i][patient->getMaternalMonitoringState()->diagnosedCMVStatus];
            }
            else {
                scheduledTest = simContext->getBackgroundScreeningInputs()->backgroundTesting[patient->getGeneralState()->weekNum][i];
            }
            
            // Checking if test type is already in the pending Follow-Up and Confirmatory test vectors. Only add the test to testsThisWeek if it's NOT already in either of these vectors. 
            if (!containsTestType(patient->getGeneralState()->pendingFollowUpTests[weekNum], i) && !containsTestType(patient->getGeneralState()->pendingConfirmatoryTests[weekNum], i)){
                if (patient->getGeneralState()->weekNum >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[i].availableStartingWeek &&
                patient->getGeneralState()->weekNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[i].notAvailableAfter) {
                    // Checking if testsThisWeek already contains this test type
                    if (!containsTestType(testsThisWeek, i)) {
                        if (scheduledTest) {
                            // If weeks since test was last performed is greater than the test's minimum test interval, continue. Otherwise, don't.
                            if ((patient->getGeneralState()->weekNum - patient->getGeneralState()->weekTestLastPerformed[i]) >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[i].minimumTestInterval) {
                                // Roll for whether test is given based on probability the test is offered and probability the test is performed.
                                randNum = CmvUtil::getRandomDouble();
                                if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[i].probOfferedAndAccept) {
                                    SimContext::PrenatalTestInstance scheduledTestState = initializePrenatalTestInstance(simContext->getPrenatalTestingInputs()->PrenatalTestsVector[i], patient->getGeneralState()->prenatalTestID);
                                    testsThisWeek.push_back(scheduledTestState);
                                }
                                else {
                                    //std::cout << "test " << i << " not offered or accepted." << endl;
                                }
                            }
                            else {
                                //std::cout << "test " << i << " within minimum test interval" << endl;
                            }
                        }
                        
                    }
                    else {
                        //std::cout << "test " << i << " already in testThisWeek vector." << endl;
                    }
                }
                else {
                    //std::cout << "test " << i << " not in test available time range." << endl;"
                }
            }
            else {
                //std::cout << "test " << i << " already in pending follow-up or confirmatory test vectors." << endl;
            }
            
        }
        
    }
    else {
    }
    
    

    /* Getting special case tests from special case triggering */
    for (i = 0; i < patient->getGeneralState()->specialCaseTests[weekNum].size(); i++) {
        SimContext::PrenatalTestInstance specialCaseTest = patient->getGeneralState()->specialCaseTests[weekNum][i];
        // checking for special case tests scheduled to be performed this week
        if (specialCaseTest.weekToTest == patient->getGeneralState()->weekNum) {
            if (patient->getGeneralState()->weekNum >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[specialCaseTest.testAssay].availableStartingWeek &&
            patient->getGeneralState()->weekNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[specialCaseTest.testAssay].notAvailableAfter) {
                // Checking if test type is already in the pending Follow-Up test vectors. Only add the test to testsThisWeek if it's NOT already in the scheduled follow-up vector. 
                if (!containsTestType(testsThisWeek, specialCaseTest) ){
                    // If weeks since test was last performed is greater than the test's minimum test interval, continue. Otherwise, don't.
                    if ((patient->getGeneralState()->weekNum - patient->getGeneralState()->weekTestLastPerformed[specialCaseTest.testAssay]) >= simContext->getPrenatalTestingInputs()->PrenatalTestsVector[specialCaseTest.testAssay].minimumTestInterval) {
                        // Roll for whether test is given based on probability the test is offered and probability the test is performed. If yes, add to testsThisWeek vector.
                        randNum = CmvUtil::getRandomDouble();
                        if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[specialCaseTest.testAssay].probOfferedAndAccept) {
                            testsThisWeek.push_back(specialCaseTest);
                        }
                    }
                }
            }
        }
    }
    
    /** returning the vector containing the test states for the tests this week  */
    return testsThisWeek;
}


/** Returns true if a vector containing test states contains the same test type as an individual test state. Basically, ifVectorContains(vector, value)*/
bool PrenatalTestUpdater::containsTestType(vector< SimContext::PrenatalTestInstance> vect, int prenatalTestNum) {
	    bool contains = false;
        for (int i = 0; i < vect.size(); i++) {
            if (vect[i].testAssay == prenatalTestNum) {
                contains = true;
            }
        }
	    return contains;
}
bool PrenatalTestUpdater::containsTestType(vector< SimContext::PrenatalTestInstance> vect, SimContext::PrenatalTestInstance prenatalTestInstance) {
	    bool contains = false;
        for (int i = 0; i < vect.size(); i++) {
            if (vect[i].testAssay == prenatalTestInstance.testAssay) {
                contains = true;
            }
        }
	    return contains;
}

void PrenatalTestUpdater::antibodyTestsOutcomesUpdater() {
    // THIS FUNCTION SHOULD BE PERFORMED AFTER THE IGG AVIDITY TEST IS RETURNED, UNLESS IGM OR IGG TESTS HAVE NOT BEEN RETURNED, THEN WAIT UNTIL THEY BOTH ARE.
    /*
    if ((patient->getGeneralState()->weekNum - patient->getMaternalMonitoringState()->weekLastIgMTest) >= simContext->getPrenatalTestingInputs()->igmTest.weeksToResultReturn &&
    (patient->getGeneralState()->weekNum - patient->getMaternalMonitoringState()->weekLastIgGTest) >=  simContext->getPrenatalTestingInputs()->iggTest.weeksToResultReturn&& 
    (patient->getGeneralState()->weekNum - patient->getMaternalMonitoringState()->weekLastAvidityTest) >= simContext->getPrenatalTestingInputs()->avidityTest.weeksToResultReturn)
    {
    */
    // Checking what the combination of results show:
    if (patient->getMaternalMonitoringState()->IgMStatus == true) {
        if (patient->getMaternalMonitoringState()->IgGStatus == true) {
            setKnownCMV(true);
            setKnownHadCMV(true);
            setDiagnosedMaternalCMV(true);
            setKnownPrimaryOrSecondary(SimContext::RECENT_UNKNOWN);
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\tIgM and IgG both positive. CMV suspected. RECENT UNKNOWN PI/NPI Infection. Waiting for Avidity test for time of infection.\n");
            }
            setPerformWeeklyIgmIggAvidityOutcomeCheck(false);
            
        }
        else { // IgG is negative
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\tIgM positive, IgG negative. Inconclusive, needs more follow-up tests.\n");
            }
            // PCR is ordered (already set in follow-up tests)
            // No CMV diagnosed (yet). Nothing happens, continue.
        }
    }
    else { // IgM is negative
        if (patient->getMaternalMonitoringState()->IgGStatus == true) {
            setKnownHadCMV(true);
            setDiagnosedMaternalCMV(true);
            setKnownPrimaryOrSecondary(SimContext::LONG_AGO_UNKNOWN);
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\tIgM negative, IgG positive. LONG AGO UNKNOWN PI/NPI (Remote Previous Infection) suspected.\n");
            }
            // Current or previous CMV has been diagnosed. Stop this weekly IgM/IgG/Avidity outcomes checking function.
            setPerformWeeklyIgmIggAvidityOutcomeCheck(false);
        }
        else { // IgG is negative
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\tIgM and IgG both negative. No CMV suspected.\n");
            }
            // No CMV diagnosed. Nothing happens, continue.
        }
    }
}

void PrenatalTestUpdater::postAvidityTestOutcomesUpdater(SimContext::PrenatalTestInstance avidityTestInstance) {
    // Checking what the combination of results show:
    if (patient->getMaternalMonitoringState()->IgMStatus == true) {
        if (patient->getMaternalMonitoringState()->IgGStatus == true) {
            if (avidityTestInstance.result == SimContext::NO_AVIDITY) {
                setKnownPrimaryOrSecondary(SimContext::PRIMARY);
                setDiagnosedMaternalCMV(true);
                if (patient->getGeneralState()->tracingEnabled){
                    tracer->printTrace(1, "\tNo IgG Avidity. Likley FP IgG. PRIMARY infection suspected.\n");
                }
            }
            else if (avidityTestInstance.result == SimContext::LOW_AVIDITY) {
                setKnownPrimaryOrSecondary(SimContext::PRIMARY);
                setDiagnosedMaternalCMV(true);
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "\tLow IgG Avidity. PRIMARY infection suspected.\n");
                }
            }
            else { // High Avidity
                setKnownPrimaryOrSecondary(SimContext::SECONDARY);
                setDiagnosedMaternalCMV(true);
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "\tHigh IgG Avidity. NONPRIMARY infection suspected.\n");
                }
            }
            setPerformWeeklyIgmIggAvidityOutcomeCheck(false);
        }
        else { // IgG is negative
            // PCR is ordered (already set in follow-up tests)
            // No CMV diagnosed (yet). Nothing happens, continue.
        }
    }
    else { // IgM is negative
        if (patient->getMaternalMonitoringState()->IgGStatus == true) {
            if (avidityTestInstance.result == SimContext::NO_AVIDITY) {
                // Negative IgM, Positive IgG. Remote PI/Long Ago Unknown should have already been diagnosed
            }
            else if (avidityTestInstance.result == SimContext::LOW_AVIDITY) {
                // Negative IgM, Positive IgG. Remote PI/Long Ago Unknown should have already been diagnosed
            }
            else { // High Avidity
                // Negative IgM, Positive IgG. Remote PI/Long Ago Unknown should have already been diagnosed
            }
            // Current or previous CMV has been diagnosed. Stop this weekly IgM/IgG/Avidity outcomes checking function.
            setPerformWeeklyIgmIggAvidityOutcomeCheck(false);
        }
        else { // IgG is negative
            // No CMV diagnosed. Nothing happens, continue.
        }
    }
}

void PrenatalTestUpdater::postMaternalPCROutcomesUpdater(SimContext::PrenatalTestInstance maternalPCRInstance) {
    if (maternalPCRInstance.testAssay != SimContext::MATERNAL_PCR) {
        cout << "ERROR: postMaternalPCROutcomesUpdater is taking in a test that's not Maternal PCR as an argument. " <<endl;cin.get();
    }
    else {
        if (maternalPCRInstance.testType == SimContext::PN_TEST_FOLLOWUP && maternalPCRInstance.testFollowingUpFrom == SimContext::IGG) {
            if (maternalPCRInstance.result == true) {
                setKnownCMV(true);
                setKnownHadCMV(true);
                setDiagnosedMaternalCMV(true);
                setKnownPrimaryOrSecondary(SimContext::PRIMARY);
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "\tIgM positive, IgG negative, Maternal PCR positive. Primary CMV infection suspected.\n");
                }
            }
            else {

                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "\tIgM positive, IgG negative, Maternal  negative. False trigger likely. CMV not suspected.\n");
                }
            }
        }
    }
}

void PrenatalTestUpdater::postAmniocentesisTest(SimContext::PrenatalTestInstance amniocentesisInstance) {
    if (amniocentesisInstance.testAssay != SimContext::AMNIOCENTESIS) {
        cout << "ERROR: postAmniocentesisTest is taking in a test that's not Amniocentesis as an argument. " <<endl;cin.get();
    }
    else {
        if (amniocentesisInstance.result == true) {
            if (!patient->getChildMonitoringState()->knownCMV) {
                incrementNumFetalDiagnosedCMV();
            }
            setChildKnownCMV(true);
            

            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[FETAL DIAGNOSIS] CMV suspected in fetus\n");
            }

            // if has had ultrasound before (either routine or detailed), check if we need to classify the patient's CMV infection as symptomatic
            if (patient->getGeneralState()->prenatalTestsPerformed[SimContext::DETAILED_US]) {
                int detailedUltrasoundLastResult = patient->getGeneralState()->prenatalTestsLastResult[SimContext::DETAILED_US];
                if (detailedUltrasoundLastResult == SimContext::ABNORMAL) {
                    setChildObservedSymptomaticCMV(true);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "\tFetal CMV suspected to be symptomatic from previous detailed ultrasound\n");
                    }
                }
                else {
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "\tFetal CMV NOT suspected to be symptomatic from previous detailed ultrasound\n");
                    }
                }
            }
            else if (patient->getGeneralState()->prenatalTestsPerformed[SimContext::ROUTINE_US]) {
                int routineUltrasoundLastResult = patient->getGeneralState()->prenatalTestsLastResult[SimContext::ROUTINE_US];
                if (routineUltrasoundLastResult == SimContext::ABNORMAL) {
                    setChildObservedSymptomaticCMV(true);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "\tFetal CMV suspected to be symptomatic from previous routine ultrasound");
                    }
                }
                else {
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "\tFetal CMV NOT suspected to be symptomatic from previous routine ultrasound\n");
                    }
                }
            }
        }
    }
}

/* DEFINING TESTING EVENTS SEQUENCES */
SimContext::PrenatalTestInstance PrenatalTestUpdater::performTest(SimContext::PrenatalTestInstance prenatalTestInstance) {
    // perform ultrasound test 

    SimContext::PrenatalTestInstance prenatalTestInstanceReturn = prenatalTestInstance;
    
    bool positiveOrNegative = false;
    bool maternalPCRPositiveOrNegative = false;
    int testResult = 0;

    if (prenatalTestInstance.testAssay == SimContext::ROUTINE_US) {
        positiveOrNegative = performRoutineUltrasound();
    }
    else if (prenatalTestInstance.testAssay == SimContext::DETAILED_US) {
        positiveOrNegative = performDetailedUltrasound();
    }
    else if (prenatalTestInstance.testAssay == SimContext::MATERNAL_PCR) {
        bool PCRReturn = false;
        PCRReturn = performMaternalPCR();
        positiveOrNegative = PCRReturn;
        if (positiveOrNegative != false) {
            positiveOrNegative = true;
            //PROBLEM AREA. POSSIBLE MEMORY ISSUE. NEED THE COUT RIGHT NOW
            cout << ""; //<< positiveOrNegative << endl;
        }
        maternalPCRPositiveOrNegative = positiveOrNegative;
        
    }
    else if (prenatalTestInstance.testAssay == SimContext::IGM) {
        positiveOrNegative = performIgM();
    }
    else if (prenatalTestInstance.testAssay == SimContext::IGG) {
        positiveOrNegative = performIgG();
    }
    else if (prenatalTestInstance.testAssay == SimContext::IGG_AVIDITY) {
        positiveOrNegative = performIgGAvidity();
    }
    else if (prenatalTestInstance.testAssay == SimContext::AMNIOCENTESIS) {
        positiveOrNegative = performAmniocentesis();
    }
    else {
        std::cout << "ERROR: Using unspecified test. Please do not use tests 8 or 9." << std::endl;
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "ERROR: Using unspecified test.\n");
        }
    }

    if (prenatalTestInstance.testAssay == SimContext::IGG_AVIDITY) {
        if (positiveOrNegative == true) {
            /** If test result is that there is avidity but there really isn't, set result to low avidity.
             * If Test result is that there is avidity and there really is, set to true avidity
            */
            if (patient->getMaternalDiseaseState()->avidity == SimContext::NO_AVIDITY) {
                testResult = SimContext::LOW_AVIDITY;
            }
            else {
                testResult = patient->getMaternalDiseaseState()->avidity;
            }
            
        }
    }

    else {
        if (prenatalTestInstance.testAssay == SimContext::MATERNAL_PCR) {
            testResult = maternalPCRPositiveOrNegative;
        }
        else {
            testResult = (int)positiveOrNegative;
        }
        
        
    }
    
    //tracer->printTrace(1, "\t%s %s performed. ", SimContext::PRENATAL_TEST_NAMES[prenatalTestInstance.testAssay], SimContext::PRENATAL_TEST_TYPE_NAMES[prenatalTestInstance.testType]);
    // if it's rolled that result will return to patient, add the result to the result return array
    
    prenatalTestInstanceReturn.result = testResult;
    prenatalTestInstanceReturn.testPerformed = true;
    // set that test was performed
    if (prenatalTestInstanceReturn.testType == SimContext::PN_TEST_FOLLOWUP) {
        setPendingFollowUpTestPerformed(true, prenatalTestInstanceReturn);
    }
    else if (prenatalTestInstanceReturn.testType == SimContext::PN_TEST_CONF) {
        setPendingConfirmatoryTestPerformed(true, prenatalTestInstanceReturn);
    }
    if (prenatalTestInstanceReturn.returnToPatient) {
        setPrenatalTestResultReturn(prenatalTestInstanceReturn.weekToReturn, prenatalTestInstanceReturn);
    }
    
    return prenatalTestInstanceReturn;
}

/* Defining events that occur during Routine Ultrasound Test */
int PrenatalTestUpdater::performRoutineUltrasound() {
    int testResult;
    double randNum;
    /** Increment number of times this test was performed */
    incrementNumTestPerformed(SimContext::ROUTINE_US);
    // Set this test's index in the prenatalTestsPerformed vector to true
    setPrenatalTestsPerformed(true, SimContext::ROUTINE_US);
    // Set the current week as the week that this test was last performed
    setWeekTestLastPerformed(patient->getGeneralState()->weekNum, SimContext::ROUTINE_US);
    // If child is positive for CMV, roll for abnormal ultrasound result based on odds for SENSITIVITY (stratified by phenotype)
    if (patient->getChildDiseaseState()->hadCMV && patient->getChildDiseaseState()->detectableCMV) {
        randNum = CmvUtil::getRandomDouble();
        testResult = false; 
        if (randNum < simContext->getPrenatalTestingInputs()->routineUltrasound.testSensitivity[patient->getGeneralState()->trimester][patient->getChildDiseaseState()->phenotypeCMV]){
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative.
        if (testResult == true) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[ROUTINE ULTRASOUND PERFORMED] Result: Abnormal (true positive)\n");
            }
            incrementNumTruePositives(SimContext::ROUTINE_US);
            incrementNumTruePositivesFetalCMV(SimContext::ROUTINE_US, patient->getChildDiseaseState()->phenotypeCMV);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[ROUTINE ULTRASOUND PERFORMED] Result: Normal (false negative)\n");
            }
            incrementNumFalseNegatives(SimContext::ROUTINE_US);
            incrementNumFalseNegativesFetalCMV(SimContext::ROUTINE_US, patient->getChildDiseaseState()->phenotypeCMV);
        }

    }
    // If child is NOT positive for CMV, roll for abnormal ultrasound result base don odds for SPECIFICITY (stratified by phenotype)
    else {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (1 - simContext->getPrenatalTestingInputs()->routineUltrasound.testSpecificity[patient->getGeneralState()->trimester])){
            testResult = true;
        }
        // output trace. If positive, report that it's a false positive
        if (testResult == true) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[ROUTINE ULTRASOUND PERFORMED] Result: Abnormal (false positive)\n");
            }
            incrementNumFalsePositives(SimContext::ROUTINE_US);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[ROUTINE ULTRASOUND PERFORMED] Result: Normal (true negative)\n");
            }
            incrementNumTrueNegatives(SimContext::ROUTINE_US);
        }    
    }
    
    setPrenatalTestsLastResult((int)testResult, SimContext::ROUTINE_US);
    return testResult;
}

/* Defining events that occur during Detailed Ultrasound Test */
int PrenatalTestUpdater::performDetailedUltrasound() {
    
    int testResult;
    double randNum;
    /** Increment number of times this test was performed. */
    incrementNumTestPerformed(SimContext::DETAILED_US);
    // Set this test's index in the prenatalTestsPerformed vector to true
    setPrenatalTestsPerformed(true, SimContext::DETAILED_US);
    // Set the current week as the week that this test was last performed
    setWeekTestLastPerformed(patient->getGeneralState()->weekNum, SimContext::DETAILED_US);
    
    // If child is positive for CMV, roll for abnormal ultrasound result based on odds for SENSITIVITY (stratified by phenotype)
    if (patient->getChildDiseaseState()->hadCMV && patient->getChildDiseaseState()->detectableCMV) {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < simContext->getPrenatalTestingInputs()->detailedUltrasound.testSensitivity[patient->getGeneralState()->trimester][patient->getChildDiseaseState()->phenotypeCMV]){
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative.
        if (testResult == true) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[DETAILED ULTRASOUND PERFORMED] Result: Abnormal (true positive)\n");
            }
            incrementNumTruePositives(SimContext::DETAILED_US);
            incrementNumTruePositivesFetalCMV(SimContext::DETAILED_US, patient->getChildDiseaseState()->phenotypeCMV);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[DETAILED ULTRASOUND PERFORMED] Result: Normal (false negative)\n");
            }
            incrementNumFalseNegatives(SimContext::DETAILED_US);
            incrementNumFalseNegativesFetalCMV(SimContext::DETAILED_US, patient->getChildDiseaseState()->phenotypeCMV);
        }

    }
    // If child is NOT positive for CMV, roll for abnormal ultrasound result based on odds for SPECIFICITY (stratified by phenotype)
    else {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (1 - simContext->getPrenatalTestingInputs()->detailedUltrasound.testSpecificity[patient->getGeneralState()->trimester])) {
            testResult = true;
        }
        // output trace. If positive, report that it's a false positive
        if (testResult == true) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[DETAILED ULTRASOUND PERFORMED] Result: Abnormal (false positive)\n");
            }
            incrementNumFalsePositives(SimContext::DETAILED_US);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[DETAILED ULTRASOUND PERFORMED] Result: Normal (true negative)\n");
            }
            incrementNumTrueNegatives(SimContext::DETAILED_US);
        }    
    }
    setPrenatalTestsLastResult((int)testResult, SimContext::DETAILED_US);
    return testResult;
}

/* Defining events that occur during Maternal PCR Test */
int PrenatalTestUpdater::performMaternalPCR() {
    int testResult = false;
    double randNum;
    /** Increment number of times this test was performed. */
    incrementNumTestPerformed(SimContext::MATERNAL_PCR);

    // Set this test's index in the prenatalTestsPerformed vector to true
    setPrenatalTestsPerformed(true, SimContext::MATERNAL_PCR);
    // Set the current week as the week that this test was last performed
    setWeekTestLastPerformed(patient->getGeneralState()->weekNum, SimContext::MATERNAL_PCR);
    // If patient's disease state would have positive PCR indicators, roll for PCR positivity based on odds for SENSITIVITY
    if (patient->getMaternalDiseaseState()->PCRWouldBePositive) {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < simContext->getPrenatalTestingInputs()->maternalPCR.testSensitivity[patient->getGeneralState()->trimester][0]) {
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative. 
        if (testResult == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[MATERNAL PCR PERFORMED] Result: Positive (true positive)\n");
            }
            incrementNumTruePositives(SimContext::MATERNAL_PCR);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumTruePositivesFetalCMV(SimContext::MATERNAL_PCR, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[MATERNAL PCR PERFORMED] Result: Negative (false negative)\n");
            }
            incrementNumFalseNegatives(SimContext::MATERNAL_PCR);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumFalseNegativesFetalCMV(SimContext::MATERNAL_PCR, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
    }
    // If patient's disease state would have negative PCR indicators, roll for PCR positivity based on odds for SPECIFICITY
    else {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (1 - simContext->getPrenatalTestingInputs()->maternalPCR.testSpecificity[patient->getGeneralState()->trimester])) {
            testResult = true;
        }
        // output trace. If positive, report that it's a false positive under truth
        if (testResult == true) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[MATERNAL PCR PERFORMED] Result: Positive (false positive)\n");
            }
            incrementNumFalsePositives(SimContext::MATERNAL_PCR);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[MATERNAL PCR PERFORMED] Result: Negative (true negative)\n");
            }
            incrementNumTrueNegatives(SimContext::MATERNAL_PCR);
        }
    }

    // if test result is positive, update the patient and run statistics
    // return test result
    setPrenatalTestsLastResult((int)testResult, SimContext::MATERNAL_PCR);
    return testResult;
}

/* Defining events that occur during IgM antibody Test */
int PrenatalTestUpdater::performIgM() {
    int testResult;
    double randNum;
    /** Increment number of times this test was performed. */
    incrementNumTestPerformed(SimContext::IGM);
    

    // Set this test's index in the prenatalTestsPerformed vector to true
    setPrenatalTestsPerformed(true, SimContext::IGM);
    // Set the current week as the week that this test was last performed
    setWeekTestLastPerformed(patient->getGeneralState()->weekNum, SimContext::IGM);
    
    // If mother's TRUE IgM state is positive, roll for Positive Test result based on odds for SENSITIVITY
    if (patient->getMaternalDiseaseState()->IgM) {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (simContext->getPrenatalTestingInputs()->igmTest.testSensitivity[patient->getGeneralState()->trimester][0])) {
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative. 
        if (testResult == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgM TEST PERFORMED] Result: Positive (true positive)\n");
            }
            incrementNumTruePositives(SimContext::IGM);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumTruePositivesFetalCMV(SimContext::IGM, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgM TEST PERFORMED] Result: Negative (false negative)\n");
            }
            incrementNumFalseNegatives(SimContext::IGM);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumFalseNegativesFetalCMV(SimContext::IGM, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
    }
    // If mother's TRUE IgM state is negative, roll for positive test result based on odds for SPECIFICITY
    else {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (1 - simContext->getPrenatalTestingInputs()->igmTest.testSpecificity[patient->getGeneralState()->trimester])){
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative. 
        if (testResult == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgM TEST PERFORMED] Result: Positive (false positive)\n");
            }
            incrementNumFalsePositives(SimContext::IGM);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgM TEST PERFORMED] Result: Negative (true negative)\n");
            }
            incrementNumTrueNegatives(SimContext::IGM);
        }
    }
    //cout <<"patient " << patient->getGeneralState()->patientNum << "week " << patient->getGeneralState()->weekNum << " maternal PCR Result: " << testResult << endl;
    setPrenatalTestsLastResult((int)testResult, SimContext::IGM);
    return testResult;
    
}

/* Defining events that occur during IgG antibody Test */
int PrenatalTestUpdater::performIgG() {
    int testResult;
    double randNum;
    /** Increment number of times this test was performed. */
    incrementNumTestPerformed(SimContext::IGG);

    // Set this test's index in the prenatalTestsPerformed vector to true
    setPrenatalTestsPerformed(true, SimContext::IGG);
    // Set the current week as the week that this test was last performed
    setWeekTestLastPerformed(patient->getGeneralState()->weekNum, SimContext::IGG);
    // If mother's TRUE IgG state is positive, roll for Positive Test result based on odds for SENSITIVITY
    if (patient->getMaternalDiseaseState()->IgG) {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (simContext->getPrenatalTestingInputs()->iggTest.testSensitivity[patient->getGeneralState()->trimester][0])) {
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative. 
        if (testResult == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgG TEST PERFORMED] Result: Positive (true positive)\n");
            }
            incrementNumTruePositives(SimContext::IGG);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumTruePositivesFetalCMV(SimContext::IGG, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgG TEST PERFORMED] Result: Negative (false negative)\n");
            }
            incrementNumFalseNegatives(SimContext::IGG);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumFalseNegativesFetalCMV(SimContext::IGG, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
    }
    // If mother's TRUE IgG state is negative, roll for positive test result based on odds for SPECIFICITY
    else {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (1 - simContext->getPrenatalTestingInputs()->iggTest.testSpecificity[patient->getGeneralState()->trimester])){
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative. 
        if (testResult == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgG TEST PERFORMED] Result: Positive (false positive)\n");
            }
            incrementNumFalsePositives(SimContext::IGG);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[IgG TEST PERFORMED] Result: Negative (true negative)\n");
            }
            incrementNumTrueNegatives(SimContext::IGG);
        }
    }
    setPrenatalTestsLastResult((int)testResult, SimContext::IGG);
    return testResult;
}

/* Defining events that occur during IgG Avidity Test */
/* CHANGE TO ALLOW FOR NONE/LOW/HIGH*/
int PrenatalTestUpdater::performIgGAvidity() {
    int testResult = false;
    double randNum;
    int trueAvidityLevel = patient->getMaternalDiseaseState()->avidity;

    /** Increment number of times this test was performed. */
    incrementNumTestPerformed(SimContext::IGG_AVIDITY);

    // Set this test's index in the prenatalTestsPerformed vector to true
    setPrenatalTestsPerformed(true, SimContext::IGG_AVIDITY);
    // Set the current week as the week that this test was last performed
    setWeekTestLastPerformed(patient->getGeneralState()->weekNum, SimContext::IGG_AVIDITY);
    /* If mother's TRUE Avidity state is Low or High (there is Avidity), then use the sensitivity and specificity values. 
        Low avidity would be considered positive, because that means there was recent infection.
        High avidity would be considered negative, because that means the infection is not recent. 
        If there is actually no avidity, the result will always return "None", because it's pretty unambiguous when there is no avidity present.
    */
    // Low Avidity case (avidity is actually low according to biological truth)
    if (patient->getMaternalDiseaseState()->avidity == SimContext::LOW_AVIDITY) {
        bool result = false;
        randNum = CmvUtil::getRandomDouble();
        if (randNum < (simContext->getPrenatalTestingInputs()->avidityTest.testSensitivity[patient->getGeneralState()->trimester][0])) {
            result = true;
        }
        // output trace. If negative, report that it's a false negative.
        if (result == true) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AVIDITY TEST PERFORMED] Result: Low (true positive) \n");
            }
            testResult = SimContext::LOW_AVIDITY;
            incrementNumTruePositives(SimContext::IGG_AVIDITY);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumTruePositivesFetalCMV(SimContext::IGG_AVIDITY, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
        else { // High avidity
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AVIDITY TEST PERFORMED] Result: High (false negative) \n");
            }
            testResult = SimContext::HIGH_AVIDITY;
            incrementNumFalseNegatives(SimContext::IGG_AVIDITY);
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumFalseNegativesFetalCMV(SimContext::IGG_AVIDITY, patient->getChildDiseaseState()->phenotypeCMV);
            }
        }
    }

    // High Avidity case (avidity is actually high according to biological truth)
    else if (patient->getMaternalDiseaseState()->avidity == SimContext::HIGH_AVIDITY) {
        bool result = false;
        randNum = CmvUtil::getRandomDouble();
        if (randNum < (1 - simContext->getPrenatalTestingInputs()->avidityTest.testSpecificity[patient->getGeneralState()->trimester])) {
            result = true;
        }
        // output trace. If positive, report that it's a false positive
        if (result == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AVIDITY TEST PERFORMED] Result: Low (false positive)\n");
            }
            testResult = SimContext::LOW_AVIDITY;
            incrementNumFalsePositives(SimContext::IGG_AVIDITY);
        }
        else { // true negative
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AVIDITY TEST PERFORMED] Result: High (true negative)\n");
            }
            testResult = SimContext::HIGH_AVIDITY;
            incrementNumTrueNegatives(SimContext::IGG_AVIDITY);
        }

    }

    // No Avidity case (there is actually no avidity according to biological truth)
    else {
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace( 1, "\t[AVIDITY TEST PERFORMED] RESULT: No Avidity\n");
        }
        testResult = SimContext::NO_AVIDITY;
    }
    setPrenatalTestsLastResult((int)testResult, SimContext::IGG_AVIDITY);
    return testResult;
}

/* Defining events that occur during Amniocentesis test */
int PrenatalTestUpdater::performAmniocentesis() {
    int testResult = false;
    double randNum;

    /** Increment number of times this test was performed. */
    incrementNumTestPerformed(SimContext::AMNIOCENTESIS);

    // Set this test's index in the prenatalTestsPerformed vector to true
    setPrenatalTestsPerformed(true, SimContext::AMNIOCENTESIS);
    // Set the current week as the week that this test was last performed
    setWeekTestLastPerformed(patient->getGeneralState()->weekNum, SimContext::AMNIOCENTESIS);
    // If child's TRUE CMV state is positive, roll for Positive Test result based on odds for SENSITIVITY
    if (patient->getChildDiseaseState()->hadCMV && patient->getChildDiseaseState()->detectableCMV) {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (simContext->getPrenatalTestingInputs()->amniocentesis.testSensitivity[patient->getGeneralState()->trimester][patient->getChildDiseaseState()->phenotypeCMV])) {
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative. 
        if (testResult == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AMNIOCENTESIS TEST PERFORMED] Result: Positive (true positive)\n");
            }
            setPrenatalTestsLastResult((int)testResult, SimContext::AMNIOCENTESIS);
            incrementNumTruePositives(SimContext::AMNIOCENTESIS);
            incrementNumTruePositivesFetalCMV(SimContext::AMNIOCENTESIS, patient->getChildDiseaseState()->phenotypeCMV);
            
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AMNIOCENTESIS TEST PERFORMED] Result: Negative (false negative)\n");
            }
            setPrenatalTestsLastResult((int)testResult, SimContext::AMNIOCENTESIS);
            incrementNumFalseNegatives(SimContext::AMNIOCENTESIS);
            incrementNumFalseNegativesFetalCMV(SimContext::AMNIOCENTESIS, patient->getChildDiseaseState()->phenotypeCMV);
        }
    }
    // If child's TRUE CMV state is negative, roll for positive test result based on odds for SPECIFICITY
    else {
        randNum = CmvUtil::getRandomDouble();
        testResult = false;
        if (randNum < (1 - simContext->getPrenatalTestingInputs()->amniocentesis.testSpecificity[patient->getGeneralState()->trimester])) {
            testResult = true;
        }
        // output trace. If negative, report that it's a false negative. 
        if (testResult == true ) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AMNIOCENTESIS TEST PERFORMED] Result: Positive (false positive)\n");
            }
            setPrenatalTestsLastResult(SimContext::ABNORMAL, SimContext::AMNIOCENTESIS);
            incrementNumFalsePositives(SimContext::AMNIOCENTESIS);
        }
        else {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\t[AMNIOCENTESIS TEST PERFORMED] Result: Negative (true negative)\n");
            }
            setPrenatalTestsLastResult(SimContext::NORMAL, SimContext::AMNIOCENTESIS);
            incrementNumTrueNegatives(SimContext::AMNIOCENTESIS);
        }
    }
    return testResult;
}

/** endTest function creates the follow-up and confirmatory test objects and adds results to the patient and runStats objects */
void PrenatalTestUpdater::endTest(SimContext::PrenatalTestInstance prenatalTestInstance) {
    double randNum;
    /** Creating the follow-up tests */
    int currTestNumber = prenatalTestInstance.testAssay;
    /** Set appointment this week to be true, since test was performed */
    setAppointmentThisWeek(true);
    // If the test was triggered by a base test or special case (exposure/mild illness/symptoms). 
    if (prenatalTestInstance.testType == SimContext::PN_TEST_BASE || prenatalTestInstance.testType == SimContext::PN_TEST_SPECIAL) {
        // follow-up can only be created if the result is either true (1) or false (0), or in the case of IgG Avidity, high (0) or low (1). "None" is represented with the value 2, and no follow up occurs with "None" result. 
        if (prenatalTestInstance.result < 2) {
            // Create the appropriate FOLLOW-UP prenatal test state objects 
            for (int fuTestNum = 0; fuTestNum < SimContext::NUM_PRENATAL_TESTS; fuTestNum++) {
            
            
                if (simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].followUpTests[fuTestNum][prenatalTestInstance.result]) {
                    /* Setting Follow Up Test Instance specs */
                    int testAssay = fuTestNum; // the test number
                    int confirmatoryNumber = SimContext::PN_NOT_CONF; // not a confirmatory test 
                    int testType = SimContext::PN_TEST_FOLLOWUP; // test type is follow-up
                    
                    int result = false; // initialize result to false
                    int weekScheduled = patient->getGeneralState()->weekNum; // week that the test was scheduled (current week)
                    int testFollowingUpFrom = currTestNumber;
                    /* if the current test is IgM and the follow-up is IgG, the testFollowingUpFrom number is different depending on the IgM test result.*/
                    if (prenatalTestInstance.testAssay == SimContext::IGM && prenatalTestInstance.result == false && fuTestNum == SimContext::IGG) {
                        testFollowingUpFrom = prenatalTestInstance.testAssay + 1;
                    }
                    int weekToTest = patient->getGeneralState()->weekNum + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].followUpTestDelays[fuTestNum][prenatalTestInstance.result];
                    bool testPerformed = false;
                    int weekToReturn = weekToTest + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[fuTestNum].weeksToResultReturn;
                    bool returnToPatient = false;
                    randNum = CmvUtil::getRandomDouble();
                    if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[fuTestNum].probResultReturn) {
                        returnToPatient = true;
                    }

                    // Rolling for whether follow-up tests will occur based on probability of follow-up.
                    randNum = CmvUtil::getRandomDouble();
                    if (randNum < (simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].followUpTestsProbabilities[fuTestNum][prenatalTestInstance.result])){
                        
                        SimContext::PrenatalTestInstance followUpInstance = {patient->getGeneralState()->prenatalTestID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom,
                            weekScheduled, weekToTest, testPerformed, result, weekToReturn, returnToPatient};
                        
                        StateUpdater::incrementPrenatalTestID();
                        if (currTestNumber == SimContext::IGM) {
                            setIgmToIggDelay(simContext->getPrenatalTestingInputs()->igmTest.followUpTestDelays[SimContext::IGG][prenatalTestInstance.result]);
                        }
                        if (currTestNumber == SimContext::IGG) {
                            setIggToAvidityDelay(simContext->getPrenatalTestingInputs()->iggTest.followUpTestDelays[SimContext::IGG_AVIDITY][prenatalTestInstance.result]);
                        }
                        // push the prenatal test state to the patient vector containing current pending follow-ups
                        pushBackPrenatalFollowUp(followUpInstance);
                        // if follow-up test is supposed to happen in the current week, add it to additionalTestsThisWeek vector
                        if (followUpInstance.weekToTest == patient->getGeneralState()->weekNum) {
                            additionalTestsThisWeek.push_back(followUpInstance);
                        }
                        if (patient->getGeneralState()->tracingEnabled) {
                            tracer->printTrace(1,"\t%s Follow Up Test scheduled for week %d\n", SimContext::PRENATAL_TEST_NAMES[testAssay], weekToTest);
                        }
                    }
                    // if follow-up test was scheduled but rolls so that it won't happen, write in trace file that the FU test will not happen.
                    else {
                        if (patient->getGeneralState()->tracingEnabled) {
                            tracer->printTrace(1, "\t%s Follow Up Test NOT scheduled for week %d\n", SimContext::PRENATAL_TEST_NAMES[testAssay], weekToTest);
                        }
                    }
                }
            
            }
        }
        // Create the appropriate CONFIRMATORY prenatal test state objects
        if (simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].numRepeatedConfirmatory > 0) {
            
            int confirmatoryNumber = SimContext::PN_CONF_1;
            int testType = SimContext::PN_TEST_CONF;
            int testAssay = prenatalTestInstance.testAssay;
            bool result = false; // initialize result to false
            int weekScheduled = patient->getGeneralState()->weekNum;
            int testFollowingUpFrom = prenatalTestInstance.testAssay;
            int weekToTest = patient->getGeneralState()->weekNum + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].weeksBetweenConfirmatoryTests;
            bool testPerformed = false;
            int weekToReturn = weekToTest + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].weeksToResultReturn;
            bool returnToPatient = false;
            randNum = CmvUtil::getRandomDouble();
            if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].probResultReturn) {
                returnToPatient = true;
            }

            SimContext::PrenatalTestInstance confirmatoryInstance = {patient->getGeneralState()->prenatalTestID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom, 
                weekScheduled, weekToTest, testPerformed, result, weekToReturn, returnToPatient};
            incrementPrenatalTestID();
            pushBackPrenatalConfirmatory(confirmatoryInstance);
            // if confirmatory test is supposed to happen in the current week, add it to additonalTestsThisWeek vector
            if (confirmatoryInstance.weekToTest == patient->getGeneralState()->weekNum) {
                additionalTestsThisWeek.push_back(confirmatoryInstance);
            }
        }

    }
    // If the test was a confirmatory test
    else if (prenatalTestInstance.testType == SimContext::PN_TEST_CONF) {
        // Create the appropriate CONFIRMATORY prenatal test state objects (no follow-ups for confirmatory tests)
        if (prenatalTestInstance.confirmatoryNumber < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].numRepeatedConfirmatory) {
            // If confirmatory test is needed, create the confirmatory prenatal test instance
            int confirmatoryNumber = prenatalTestInstance.confirmatoryNumber + 1;
            int testType = SimContext::PN_TEST_CONF;
            int testAssay = prenatalTestInstance.testAssay;
            bool result = false; // intiialize result to false
            int weekScheduled = patient->getGeneralState()->weekNum;
            int testFollowingUpFrom = currTestNumber; // not applicable to this test type, but set it to current test number anyways
            int weekToTest = patient->getGeneralState()->weekNum + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].weeksBetweenConfirmatoryTests;
            bool testPerformed = false;
            int weekToReturn = weekToTest + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].weeksToResultReturn;
            bool returnToPatient = false;
            randNum = CmvUtil::getRandomDouble();
            if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].probResultReturn) {
                returnToPatient = true;
            }

            SimContext::PrenatalTestInstance confirmatoryInstance = {patient->getGeneralState()->prenatalTestID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom, 
                weekScheduled, weekToTest, testPerformed, result, weekToReturn, returnToPatient};
            incrementPrenatalTestID(); // CRITICAL. DO NOT DELETE.

            // push the confirmatory test state to the patient vector containing current pending confirmatory tests
            pushBackPrenatalConfirmatory(confirmatoryInstance);
            // if confirmatory test is supposed to happen in the current week, add it to additionalTestsThisWeek vector
            if (confirmatoryInstance.weekToTest == patient->getGeneralState()->weekNum) {
                additionalTestsThisWeek.push_back(confirmatoryInstance);
            }
        }
        markPendingConfirmatoryTestAsDone(prenatalTestInstance.testID);

    }
    // If the test was a follow-up test
    else {
        // If Result is 0 or 1 (less than 2), create the appropriate FOLLOW-UP prenatal test state objects 
        if (prenatalTestInstance.result < 2) {
            for (int fuTestNum = 0; fuTestNum < SimContext::NUM_PRENATAL_TESTS; fuTestNum++) {
                // if the specified follow-up test is activated
                if (simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].FUTriggeredByTest[prenatalTestInstance.testFollowingUpFrom][fuTestNum][prenatalTestInstance.result]) {
                    /* Setting Follow Up Test Instance specs */
                    int testType = SimContext::PN_TEST_FOLLOWUP;
                    int confirmatoryNumber = SimContext::PN_NOT_CONF;
                    int testAssay = fuTestNum;
                    bool result = false; //initialize result to false;
                    int weekScheduled = patient->getGeneralState()->weekNum;
                    int testFollowingUpFrom = currTestNumber;
                    // For IgG, follow-up stats from negative or positive IgM test may be different. Manual override for negative IgM test result here. 
                    if (fuTestNum == SimContext::IGG && prenatalTestInstance.testAssay == SimContext::IGM) {
                        if (prenatalTestInstance.result == false) {
                            testFollowingUpFrom = SimContext::IGM + 1;
                        }
                    }
                    // set scheduling delay
                    int weekToTest = patient->getGeneralState()->weekNum + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].FUTriggeredByTestDelays[prenatalTestInstance.testFollowingUpFrom][fuTestNum][prenatalTestInstance.result];
                    bool testPerformed = false;
                    
                    // rolling for whether follow-up tests will occur based on probability of follow-up.
                    randNum = CmvUtil::getRandomDouble();
                    if (randNum < (simContext->getPrenatalTestingInputs()->PrenatalTestsVector[currTestNumber].FUTriggeredByTestProbabilities[prenatalTestInstance.testFollowingUpFrom][fuTestNum][prenatalTestInstance.result])) {
                        
                        if (currTestNumber == SimContext::IGM) {
                            setIgmToIggDelay(simContext->getPrenatalTestingInputs()->igmTest.FUTriggeredByTestDelays[prenatalTestInstance.testFollowingUpFrom][SimContext::IGG][prenatalTestInstance.result]);
                        }
                        if (currTestNumber == SimContext::IGG) {
                            setIggToAvidityDelay(simContext->getPrenatalTestingInputs()->iggTest.FUTriggeredByTestDelays[prenatalTestInstance.testFollowingUpFrom][SimContext::IGG_AVIDITY][prenatalTestInstance.result]);
                        }
                        int weekToReturn = weekToTest + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[fuTestNum].weeksToResultReturn;
                        bool returnToPatient = false;
                        randNum = CmvUtil::getRandomDouble();
                        if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[fuTestNum].probResultReturn) {
                            returnToPatient = true;
                        }
                        
                        SimContext::PrenatalTestInstance followUpInstance = {patient->getGeneralState()->prenatalTestID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom,
                            weekScheduled, weekToTest, testPerformed, result, weekToReturn, returnToPatient};
                        incrementPrenatalTestID();

                        // push the prenatal test state to the patient vector containing current pending follow-ups
                        pushBackPrenatalFollowUp(followUpInstance);
                        // if follow-up test is supposed to happen in the current week, add it to additionalTestsThisWeek vector
                        if (followUpInstance.weekToTest == patient->getGeneralState()->weekNum) {
                            additionalTestsThisWeek.push_back(followUpInstance);
                        }
                        if (patient->getGeneralState()->tracingEnabled) {
                            tracer->printTrace(1,"\t%s Test scheduled for week %d\n", SimContext::PRENATAL_TEST_NAMES[testAssay], weekToTest);
                        }
                    }
                    // if follow-up test was scheduled but rolls so that it won't happen, write in trace file that the FU test will not happen.
                    else {
                        if (patient->getGeneralState()->tracingEnabled) {
                            tracer->printTrace(1, "\t%s Follow Up Test NOT scheduled for week %d\n", SimContext::PRENATAL_TEST_NAMES[testAssay], weekToTest);
                        }
                    }
                }
            }
        }
        markPendingFollowUpTestAsDone(prenatalTestInstance.testID);
    }

}


/* peformWeeklyUpdates performs all of the Prenatal Test state and statistics updates for a simulated week DURING pregnancy*/
void PrenatalTestUpdater::performWeeklyUpdates() {
    double randNum;
    int weekNum = patient->getGeneralState()->weekNum;
    bool visitThisWeek = false;

    bool attendVisit = false;
    if (determineIfScheduledVisit()) {
        incrementNumScheduledPrenatalAppointment();
        visitThisWeek = true;
    }
    // roll for whether MF pair attends ANC visit 
    double probAttendingAppointmentThisWeek = 0;
    /* If there is a scheduled visit this week, get probabilities of attending appointment and roll for whether the patient attends the appointment*/
    if (visitThisWeek) {
        if (patient->getChildMonitoringState()->knownCMV /*known child CMV*/) {
        probAttendingAppointmentThisWeek = simContext->getBackgroundScreeningInputs()->probAttendUponChildDiagnosis[weekNum][patient->getChildMonitoringState()->observedSymptomatic];
        //get probAttendingAppointmentThisWeek from child prob vector
        }
        else if (patient->getMaternalMonitoringState()->knownCMV /*known maternal CMV*/) {
            probAttendingAppointmentThisWeek = simContext->getBackgroundScreeningInputs()->probAttendUponMaternalDiagnosis[weekNum][patient->getMaternalMonitoringState()->diagnosedCMVStatus];
            // get probAttendingAppointmentThisWeek from maternal prob vector
        }
        else { // no known CMV yet
            probAttendingAppointmentThisWeek = simContext->getBackgroundScreeningInputs()->probAttendingRegularAppointmentsByWeek[weekNum];
            // get probAttendingAppointmentThisWeek from background screening vector
        }
        attendVisit = false;
        randNum = CmvUtil::getRandomDouble();
        if (randNum < probAttendingAppointmentThisWeek) {
            attendVisit = true;
        }
    }
    /* If no scheduled appointment, see if there is still a probability of appointment this week due to symptomatic CMV or mild illness */
    else {
        // if scheduled follow up or confirmatory test
        if (patient->getGeneralState()->pendingFollowUpTests[weekNum].size() > 0 || patient->getGeneralState()->pendingConfirmatoryTests[weekNum].size() > 0) {
            probAttendingAppointmentThisWeek = 1; // if follow up or confirmatory tests have already been scheduled, the appointment will definitely happen.
                                                  // we don't need to roll for follow up test because that happened at the time the test was scheduled
        }
        // else if symptomatic CMV infection this week
        else if (patient->getMaternalDiseaseState()->weekLastCMV == patient->getGeneralState()->weekNum && patient->getMaternalMonitoringState()->CMVSymptoms) {
            probAttendingAppointmentThisWeek = simContext->getBackgroundScreeningInputs()->seekingHealthcareSymptomatic;
        }
        // else if mild illness this week
        else if (patient->getMaternalDiseaseState()->weekMildIllness == patient->getGeneralState()->weekNum && patient->getMaternalDiseaseState()->mildIllness) {
            probAttendingAppointmentThisWeek = simContext->getBackgroundScreeningInputs()->seekingHealthcareMildIllness;
        }
        attendVisit = false;
        randNum = CmvUtil::getRandomDouble();
        if (randNum < probAttendingAppointmentThisWeek) {
            attendVisit = true;
        }
    }

    // if MF pair attends scheduled visit
    if (attendVisit) {
        setAppointmentThisWeek(true);
        // check for pending tests/results 
        vector<SimContext::PrenatalTestInstance> prenatalTestsToPerform = getTestsThisWeek();

        /* If special case (symptomatic CMV or mild illness) this week, roll for attend appointment and for special case triggered tests */
        // if new CMV infection this week and it's symptomatic, roll for attend appointment and for symptomatic cmv triggered tests
        if (patient->getMaternalDiseaseState()->weekLastCMV == patient->getGeneralState()->weekNum && patient->getMaternalMonitoringState()->CMVSymptoms) {
            // mark appointment as due only to symptomatic CMV IF no other tests were already supposed to happen this week
            if (prenatalTestsToPerform.size() == 0) {
                incrementNumSymptomaticAppointmentsAttended();
            }
            // set up the symptomatic CMV triggered tests
            for (int test = 0; test < SimContext::NUM_PRENATAL_TESTS; test++) {
                // if the prenatalTestsToPerform vector does not already contain this specific test type, continue rolling for symptomatically triggered prenatal test
                if (!containsTestType(prenatalTestsToPerform, test)) {
                    double probTest = simContext->getPrenatalTestingInputs()->PrenatalTestsVector[test].symptomaticCMVTrigger;
                    // roll for if test is set to occur. If yes, add it to prenatalTestsToPerform vector
                    randNum = CmvUtil::getRandomDouble();
                    if (randNum < probTest) {
                        int testType = SimContext::PN_TEST_SPECIAL;
                        int weekToReturn = patient->getGeneralState()->weekNum + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[test].weeksToResultReturn;
                        bool returnToPatient = false;
                        randNum = CmvUtil::getRandomDouble();
                        if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[test].probResultReturn) {
                            returnToPatient = true;
                        }
                        int result = false; // initializing result to false
                        SimContext::PrenatalTestInstance specialCaseTestInstance = {patient->getGeneralState()->prenatalTestID, test, testType, -1, -1,
                            weekNum, weekNum, false, result, weekToReturn, returnToPatient};
                        
                        StateUpdater::incrementPrenatalTestID();
                        prenatalTestsToPerform.push_back(specialCaseTestInstance);
                    }
                }
            }
        }
        else if (patient->getMaternalDiseaseState()->weekMildIllness == patient->getGeneralState()->weekNum && patient->getMaternalDiseaseState()->mildIllness) {
            // mark appointment as due only to mild illness IF no other tests were already supposed to happen this week
            if (prenatalTestsToPerform.size() == 0) {
                incrementNumMildIllnessAppointmentsAttended();
            }
            // set up the mild illness triggered tests
            for (int test = 0; test < SimContext::NUM_PRENATAL_TESTS; test++) {
                // if the prenatalTestsToPerform vector does not already contain this specific test type, continue rolling for symptomatically triggered prenatal test
                if (!containsTestType(prenatalTestsToPerform, test)) {
                    double probTest = simContext->getPrenatalTestingInputs()->PrenatalTestsVector[test].mildIllnessTrigger;
                    // roll for if test is set to occur. If yes, add it to prenatalTestsToPerform vector
                    randNum = CmvUtil::getRandomDouble();
                    if (randNum < probTest) {
                        int testType = SimContext::PN_TEST_SPECIAL;
                        int weekToReturn = patient->getGeneralState()->weekNum + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[test].weeksToResultReturn;
                        bool returnToPatient = false;
                        randNum = CmvUtil::getRandomDouble();
                        if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[test].probResultReturn) {
                            returnToPatient = true;
                        }
                        int result = false; // initializing result to false
                        SimContext::PrenatalTestInstance specialCaseTestInstance = {patient->getGeneralState()->prenatalTestID, test, testType, -1, -1,
                            weekNum, weekNum, false, result, weekToReturn, returnToPatient};
                        
                        StateUpdater::incrementPrenatalTestID();
                        prenatalTestsToPerform.push_back(specialCaseTestInstance);
                    }
                }
            }
        }
        /** While all scheduled tests this week haven't been performed yet, keep on performing the tests that haven't been performed. 
         * This is structured this way to allow for new tests to be scheduled the same week, in case there are any follow-up or confirmatory
         * tests that need to happen immediately.
        */
        while (determineIfAllTestsPerformed(prenatalTestsToPerform) == false) {
            // looping through testsThisWeek vector to perform all prenatal tests this week that haven't yet been performed.
            for (int test = 0; test < prenatalTestsToPerform.size(); test++) {
                if (!prenatalTestsToPerform[test].testPerformed) { // if the test has not been performed yet
                    SimContext::PrenatalTestInstance testState = prenatalTestsToPerform[test]; // potential for improper pointer. check this.
                    testState = performTest(testState);
                    prenatalTestsToPerform[test].testPerformed = true; // mark test performed as true
                    endTest(testState);
                }
            }
            // adding the additional tests this week to prenatalTestsToPerform vector
            for (int test = 0; test < additionalTestsThisWeek.size(); test++) {
                prenatalTestsToPerform.push_back(additionalTestsThisWeek[test]);
            }
            additionalTestsThisWeek.clear();
        }
        //prenatalTestsToPerform.clear();
    }

    // Look at all tests to return this week
    vector<SimContext::PrenatalTestInstance> prenatalTestsReturnThisWeek = patient->getGeneralState()->prenatalTestResultReturn[patient->getGeneralState()->weekNum];
    for (int test = 0; test < prenatalTestsReturnThisWeek.size(); test++) {
        bool result = prenatalTestsReturnThisWeek[test].result;
        int testAssay = prenatalTestsReturnThisWeek[test].testAssay;
        if (result > 1 ) {
            std::cout << "ERROR 001: INVALID RESULT. RESULT INDEX OUT OF RANGE." << endl; cin.get();
        }
        else if (result <= -1){
            std::cout << "ERROR 003: INVALID RESULT. RESULT INDEX OUT OF RANGE (LESS THAN -1)." << endl; cin.get();
        }
        else {
            /** Separate conditional for if test is avidity, since avidity result can be none (0), low avidity (1), or high avidity (2).
             * If avidity test result return is abnormal, whether it's returned as low or high avidity depends on whether the patient actually
             * has low or high IgG avidity.
            */
            if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::IGG_AVIDITY) {
                int avidityResult = (int)result;
                if (result == true) {
                    if (patient->getMaternalDiseaseState()->avidity == SimContext::HIGH_AVIDITY) {
                        avidityResult = SimContext::HIGH_AVIDITY;
                    }
                }
                setKnownAvidity(avidityResult);
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "\t[TEST RESULT RETURN] %s Test Result: %s\n", SimContext::PRENATAL_TEST_NAMES[testAssay], SimContext::AVIDITY_OUTCOME[avidityResult]);
                }
            }
            else {
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "\t[TEST RESULT RETURN] %s Test Result: %s\n", SimContext::PRENATAL_TEST_NAMES[testAssay], SimContext::TEST_OUTCOME[result]);
                }
                if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::IGM) {
                    setKnownMaternalIgM(result);
                }
                if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::IGG) {
                    setKnownMaternalIgG(result);
                }

            }

            /* Outcome Updater - checking for diagnostic combinations that result in CMV diagnosis */
            
            if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::IGG) {
                antibodyTestsOutcomesUpdater();
            }
            else if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::IGG_AVIDITY) {
                postAvidityTestOutcomesUpdater(prenatalTestsReturnThisWeek[test]);
            }
            else if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::MATERNAL_PCR) {
                postMaternalPCROutcomesUpdater(prenatalTestsReturnThisWeek[test]);
            }
            
            if (!patient->getChildMonitoringState()->knownCMV) { // if CMV has not yet been diagnosed in the child, perform child outcomes updater
                if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::AMNIOCENTESIS) {
                    postAmniocentesisTest(prenatalTestsReturnThisWeek[test]);
                }
                
            }
            // If CMV has been diagnosed in the child, see if we can determine if the infection is asymptomatic or symptomatic when RUS or DUS test occurs.
            if (patient->getChildMonitoringState()->knownCMV) {
                if (!patient->getChildMonitoringState()->observedSymptomatic) {
                    if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::ROUTINE_US) {
                        if (prenatalTestsReturnThisWeek[test].result == SimContext::ABNORMAL) {
                            // if no DUS currently scheduled, log as symptomatic
                            bool scheduledDetailedUltrasound = false;
                            for (int i = patient->getGeneralState()->weekNum; i < SimContext::MAX_WEEKS_PREGNANT; i++) {
                                if (containsTestType(patient->getGeneralState()->pendingFollowUpTests[i], SimContext::DETAILED_US) || 
                                containsTestType(patient->getGeneralState()->pendingConfirmatoryTests[i], SimContext::DETAILED_US)) {
                                    scheduledDetailedUltrasound = true;
                                }
                            }
                            if (scheduledDetailedUltrasound == false) {
                                setChildObservedSymptomaticCMV(true);
                                tracer->printTrace(1, "Fetal CMV suspected to be symptomatic from Routine Ultrasound");
                            }
                        }
                    }
                    else if (prenatalTestsReturnThisWeek[test].testAssay == SimContext::DETAILED_US) {
                        if (prenatalTestsReturnThisWeek[test].result == SimContext::ABNORMAL) {
                            setChildObservedSymptomaticCMV(true);
                            if (patient->getGeneralState()->tracingEnabled == true) {
                                tracer->printTrace(1, "Fetal CMV suspected to be symptomatic from Detailed Ultrasound");
                            }
                        }
                    }
                }
            }
            
        }
    }
}