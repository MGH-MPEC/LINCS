#include "include.h"

enum PREGNANCY_CONTINUATION {MISCARRIAGE, BIRTH, CONTINUE};
/** Constructor takes in the patient object */
PregnancyUpdater::PregnancyUpdater(Patient* patient) : StateUpdater(patient) {

}

/** Destructor empty. No cleanup required */
PregnancyUpdater::~PregnancyUpdater(void) {
    
}

/** \brief performInitialization performs all of the state and statistics updates upon patient creation */
void PregnancyUpdater::performInitialization() {
    StateUpdater::performInitialUpdates();
    
}

void PregnancyUpdater::performWeeklyUpdates() {
    double randNum;
    /* MISCARRIAGE/DELIVERY/CONTINUE PREGNANCY UPDATERS */

    /** Roll for spontaneous abortion/miscarriage/IUFD/Stillbirth, Delivery, or continue to next month */
    double probMiscarriage = simContext->getPrenatalNatHistInputs()->backgroundMiscarriage[patient->getMaternalState()->maternalAgeYears][patient->getGeneralState()->weekNum];
    double probDelivery = simContext->getCohortInputs()->weeklyProbBirth[patient->getMaternalState()->maternalAgeYears][patient->getGeneralState()->weekNum];
    if (patient->getMaternalDiseaseState()->activeCMV) {
        probMiscarriage = simContext->getPrenatalNatHistInputs()->CMVPositiveMiscarriage[patient->getMaternalState()->maternalAgeYears][patient->getGeneralState()->weekNum];
        probDelivery = simContext->getCohortInputs()->CMVPositiveWeeklyProbBirth[patient->getMaternalState()->maternalAgeYears][patient->getGeneralState()->weekNum];
    }
    double probContinue = 1 - probMiscarriage - probDelivery;
    if (probMiscarriage + probDelivery > 1) {
        probDelivery = 1 - probMiscarriage;
        probContinue = 0;
        if ((probMiscarriage + probDelivery > 1) && patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "WARNING: Sum of probability of Miscarriage and Probability of Delivery greater than 1. May yield inaccurate results.\n");
        }
    }
    vector<double> miscarriageOrDelivery = {probMiscarriage, probDelivery, probContinue};
    randNum = CmvUtil::getRandomDouble();
    int pregnancyContinuation = CmvUtil::selectFromDist(miscarriageOrDelivery, randNum);
    /** if it's max week pregnant and "continue" is selected for, just make it birth because birth has to occur by week 41.*/
    if (pregnancyContinuation == CONTINUE && patient->getGeneralState()->weekNum == SimContext::MAX_WEEKS_PREGNANT - 1) {
        pregnancyContinuation = BIRTH;
    }

    /** Conditionals for pregnancy continuation status */
    if (pregnancyContinuation == CONTINUE) {
        incrementNumAliveFetuses();
    }
    // if pregnancy does not continue, it's either due to miscarriage or birth. Increment relevant run statistics. 
    else { 
        if (pregnancyContinuation == MISCARRIAGE) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Miscarriage/Spontaneous Abortion.\n");
            }
            setChildIsAlive(false); // set child to not alive
            /** Incrementing maternal miscarriage statistics */
            if (patient->getMaternalDiseaseState()->hadCMV) {
                incrementMiscarriageWithCMV();
                
            }
            else {
                incrementMisacarriageNoCMV();
            }
            /** Incrementing fetal miscarriage statistics */
            if (patient->getChildDiseaseState()->hadCMV) {
                    incrementNumMiscarriagesFetalCMV();
            }
            else {
                incrementNumMiscarriagesNoFetalCMV();
            }
        }
        else if (pregnancyContinuation == BIRTH) {
            // roll for death at birth
            randNum = CmvUtil::getRandomDouble();
            if (randNum < simContext->getPrenatalNatHistInputs()->fetalDeathRiskDuringBirth[patient->getGeneralState()->weekNum][patient->getChildDiseaseState()->hadCMV]) {
                // baby dies during birth
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "Fetal Death During Childbirth");
                }
                setChildIsAlive(false); // MAKE INCREMENTERS
                incrementNumFetalDeathsAtBirth();
            }
            // If no death at birth, mark as delivery occurred and update number of live births statistics.
            else {
                incrementNumLiveBirths();
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "Delivery Occurred.\n");
                }
                setDeliveryOccurred(true); // set delivery occurred to true
                if (patient->getChildDiseaseState()->hadCMV) {
                    incrementSumWeekBirthWithCMV(patient->getGeneralState()->weekNum);
                    //incrementNumBirthsWithCMV();
                    incrementBirthsPositiveMaternalCMV();
                }
                else {
                    incrementSumWeekBirthNoCMV(patient->getGeneralState()->weekNum);
                    incrementBirthsNegativeMaternalCMV();
                }
            }
        }
        else {
            // we have an issue. cout error message.
            cout << "ERROR: Pregnancy continuation parameter not valid. Check performWeeklyUpdates() function under PregnancyUpdater.cpp." << endl; cin.get();
        }
        /* Increment runstats associated with pregnancy end */
        // increment total number of diagnosed mothers
        // increment total number of diagnosed mothers by type/status of diagnosis (PI, NPI, etc)
        // increment total number of diagnosed children
    }
    /*\
        if (pregnancyContinuation == MISCARRIAGE) {
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "Miscarriage/Spontaneous Abortion.\n");
        }
        setChildIsAlive(false); // set child to not alive
        // Incrementing maternal miscarriage statistics 
        if (patient->getMaternalDiseaseState()->hadCMV) {
            incrementMiscarriageWithCMV();
            
        }
        else {
            incrementMisacarriageNoCMV();
        }
        // Incrementing fetal miscarriage statistics
        if (patient->getChildDiseaseState()->hadCMV) {
                incrementNumMiscarriagesFetalCMV();
        }
        else {
            incrementNumMiscarriagesNoFetalCMV();
        }
    }
    else if (pregnancyContinuation == BIRTH) {
        // roll for death at birth
        randNum = CmvUtil::getRandomDouble();
        if (randNum < simContext->getPrenatalNatHistInputs()->fetalDeathRiskDuringBirth[patient->getGeneralState()->weekNum][patient->getChildDiseaseState()->hadCMV]) {
            // baby dies during birth
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Fetal Death During Childbirth");
            }
            setChildIsAlive(false); // MAKE INCREMENTERS
            incrementNumFetalDeathsAtBirth();
        }
        // If no death at birth, mark as delivery occurred and update number of live births statistics.
        else {
            incrementNumLiveBirths();
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Delivery Occurred.\n");
            }
            setDeliveryOccurred(true); // set delivery occurred to true
            if (patient->getChildDiseaseState()->hadCMV) {
                incrementSumWeekBirthWithCMV(patient->getGeneralState()->weekNum);
                //incrementNumBirthsWithCMV();
                incrementBirthsPositiveMaternalCMV();
            }
            else {
                incrementSumWeekBirthNoCMV(patient->getGeneralState()->weekNum);
                incrementBirthsNegativeMaternalCMV();
            }
        }
        /// Increment other  
        
    }
    else {
        incrementNumAliveFetuses();
    }
     */
    
    //continue pregnancy in all other cases (other than birth or miscarriage)
}