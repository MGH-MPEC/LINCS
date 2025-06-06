#include "include.h"

/** Constructor takes in the patient object */
MaternalBiologyUpdater::MaternalBiologyUpdater(Patient* patient) : StateUpdater(patient) {

}

/** Destructor empty. No cleanup required */
MaternalBiologyUpdater::~MaternalBiologyUpdater(void) {
    
}

/** \brief performInitialization performs all of the state and statistics updates upon patient creation */
void MaternalBiologyUpdater::performInitialization() {
    StateUpdater::performInitialUpdates();
    /* MOVED TO PATIENT OBJECT
    weeksLowAvidity = (int)(CmvUtil::getRandomGaussian(simContext->getCohortInputs()->lowAvidityDurationMean, simContext->getCohortInputs()->lowAvidityDurationStdDev) + 0.5);
    if (weeksLowAvidity < 0) {
        weeksLowAvidity = 0;
    }
    */
}

/** \brief performWeeklyUpdates performs all of the state and statistics updates for a simulated week of pregnancy */
void MaternalBiologyUpdater::performWeeklyUpdates() {
    int week = patient->getGeneralState()->weekNum;
    /** 1 week after infection */
    if (week - patient->getMaternalDiseaseState()->weekLastCMV == patient->getMaternalDiseaseState()->weeksToViremiaFromInfection) {
        // set mother to be viremic 
        setViremia(true);
        setPCRWouldBePositive(true);
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "Mother is viremic, CMV PCR would be positive. \n");
        }
    }
    /** 2 weeks after infection */
    if (week - patient->getMaternalDiseaseState()->weekLastCMV == patient->getMaternalDiseaseState()->weeksToIgmPositiveFromInfection) {
        // Set maternal IgM to positive
        if (patient->getMaternalMonitoringState()->IgMStatus == false) {
            setMaternalIgM(true);
            if (patient->getGeneralState()->tracingEnabled){
                tracer->printTrace(1, "IgM positive\n");
            }
        }
    }
    /** 3 weeks after infection */
    if (week - patient->getMaternalDiseaseState()->weekLastCMV == patient->getMaternalDiseaseState()->weeksToIggPositiveFromInfection) {
        // Set maternal IgG to positive
        if (patient->getMaternalMonitoringState()->IgGStatus == false) {
            setMaternalIgG(true);
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "IgG positive\n");
            }
            setMaternalAvidity(SimContext::LOW_AVIDITY);
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Low Avidity\n");
            }
        }
    }
    /** 4 weeks after infection */
    if (week - patient->getMaternalDiseaseState()->weekLastCMV == (patient->getMaternalDiseaseState()->weeksToViremiaFromInfection + patient->getMaternalDiseaseState()->lengthOfViremia)) {
        // Set mother to no longer be viremic and PCR Would Be Positive to false
        setMaternalCMVSymptoms(false);
        setViremia(false);
        setPCRWouldBePositive(false);
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "Mother no longer viremic, CMV PCR would be negative. \n");
        }

    }
    /** 2 - 4 weeks after low avidity SHOULD BE MONTHS */
    if (week - patient->getMaternalDiseaseState()->weekLastCMV == patient->getMaternalDiseaseState()->weeksToLowAvidityFromInfection + patient->getMaternalDiseaseState()->weeksToHighAvidityFromLowAvidity) {
        setMaternalAvidity(SimContext::HIGH_AVIDITY);
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "High Avidity\n");
        }
    }
    /** 12 months (52 weeks) after infection */
    if (week - patient->getMaternalDiseaseState()->weekLastCMV == (patient->getMaternalDiseaseState()->weeksToIgmPositiveFromInfection + patient->getMaternalDiseaseState()->lengthOfIgmPositivity)) {
        setMaternalIgM(false);
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "IgM no longer positive\n");
        }
    }
}