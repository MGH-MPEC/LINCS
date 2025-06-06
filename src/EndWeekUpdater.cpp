#include "include.h"



/* END WEEK UPDATER FUNCTIONS */
/** \brief Constructor takes in the patient object and determines if updates can occur */
EndWeekUpdater::EndWeekUpdater(Patient *patient) : StateUpdater(patient) {

}

/* \brief Destructor is empty, no cleanup required */
EndWeekUpdater::~EndWeekUpdater(void) {
    // void
}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void EndWeekUpdater::performInitialization() {
    StateUpdater::performInitialUpdates();
}

/** \brief performWeeklyUpdates performs all of the state and statistics updates for a simulated week */
void EndWeekUpdater::performWeeklyUpdates() {
    double randNum;

    /* TREATMENT UPDATERS */
    /** If currently on treatment, check if still within the duration of treatment */
    if (patient->getGeneralState()->onTreatment) {
        if (patient->getGeneralState()->weekNum - patient->getGeneralState()->weekTreatmentStarted[patient->getGeneralState()->mostRecentTreatmentNumber]
        >= simContext->getPrenatalTreatmentInputs()->treatmentDurationBasedOnPolicy[patient->getGeneralState()->mostRecentTreatmentNumber]) {
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "No longer on Treatment %d, past duration of treatment. Effects of treatment still in place.\n", patient->getGeneralState()->mostRecentTreatmentNumber + 1);
            }
            setOnTreatment(false);
        }
    }

    /* Determine if current diagnostic state matches any of the prenatal treatment policies. If so, begin treatment.*/
    // NOTE: Treatment 1 is mapped to index 0 in the code, Treatment 2 is mapped to index 1, Treatmnet 3 is mapped to index 2, and so on. 
    int NoDiagnosisTreatment = simContext->getPrenatalTreatmentInputs()->treatmentNumberBasedOnPolicy[SimContext::NO_CMV_DIAGNOSES] - 1;
    int MaternalCMVTreatment = simContext->getPrenatalTreatmentInputs()->treatmentNumberBasedOnPolicy[SimContext::MATERNAL_CMV_DIAGNOSIS] - 1;
    int FetalCMVTreatment = simContext->getPrenatalTreatmentInputs()->treatmentNumberBasedOnPolicy[SimContext::FETAL_CMV_DIAGNOSIS] - 1;

    if (!patient->getMaternalMonitoringState()->knownHadCMV && !patient->getChildMonitoringState()->knownCMV && simContext->getPrenatalTreatmentInputs()->treatmentBasedOnPolicy[SimContext::NO_CMV_DIAGNOSES] != false && 
    patient->getGeneralState()->hadTreatment[NoDiagnosisTreatment] == false) {
        setOnTreatment(true);
        setMostRecentTreatmentNumber(NoDiagnosisTreatment);;
        setWeekTreatmentStarted(patient->getGeneralState()->weekNum, NoDiagnosisTreatment);
        setHadTreatment(true,  NoDiagnosisTreatment);
        
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "Treatment %d started\n",  NoDiagnosisTreatment + 1);
        }
        if (patient->getChildDiseaseState()->hadCMV){
            symptomReductionAlgorithm();
        }
    }
    else if (patient->getMaternalMonitoringState()->knownHadCMV && !patient->getChildMonitoringState()->knownCMV && simContext->getPrenatalTreatmentInputs()->treatmentNumberBasedOnPolicy[SimContext::MATERNAL_CMV_DIAGNOSIS]!= false &&
    patient->getGeneralState()->hadTreatment[MaternalCMVTreatment] == false) {
        setOnTreatment(true);
        setMostRecentTreatmentNumber(MaternalCMVTreatment);
        setWeekTreatmentStarted(patient->getGeneralState()->weekNum, MaternalCMVTreatment);
        setHadTreatment(true, MaternalCMVTreatment);
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "Treatment %d started\n", MaternalCMVTreatment + 1);
        }
        if (patient->getChildDiseaseState()->hadCMV){
            symptomReductionAlgorithm();
        }
    }
    else if (patient->getChildMonitoringState()->knownCMV && simContext->getPrenatalTreatmentInputs()->treatmentNumberBasedOnPolicy[SimContext::FETAL_CMV_DIAGNOSIS] != false &&
    patient->getGeneralState()->hadTreatment[FetalCMVTreatment] == false) {
        setOnTreatment(true);
        setMostRecentTreatmentNumber(FetalCMVTreatment);
        setWeekTreatmentStarted(patient->getGeneralState()->weekNum, FetalCMVTreatment);
        setHadTreatment(true, FetalCMVTreatment);
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "Treatment %d started\n", FetalCMVTreatment + 1);
        }
        if (patient->getChildDiseaseState()->hadCMV){
            symptomReductionAlgorithm();
        }
        
    }

    /* Incrementing relevant RunStats statistics */
    /** If mother was diagnosed this week */
    if (patient->getMaternalMonitoringState()->diagnosedCMV && patient->getMaternalMonitoringState()->weekCMVDiagnosed == patient->getGeneralState()->weekNum) {
        // increment number of mothers diagnosed with CMV
        incrementWeeklyMaternalNumDiagnosedCMV();
        // increment number of mothers diagnosed with CMV stratified by infection type (primary, nonprimary, recent unknown, or long ago unknown)
        incrementWeeklyMaternalNumDiagnosedCMVPrimaryOrSecondary(patient->getMaternalMonitoringState()->diagnosedCMVStatus);
        
    }

    /** If an appointment was attended this week */
    if (patient->getGeneralState()->prenatalAppointmentThisWeek) {
        incrementNumAppointmentsAttended();
    }

    if (patient->getGeneralState()->onTreatment) {
        incrementNumCurrOnTreatment(patient->getGeneralState()->mostRecentTreatmentNumber);
        /** If mother has or has had CMV before, increment number of CMV mothers on treatment */
        if (patient->getMaternalDiseaseState()->hadCMV) {
            incrementNumCMVMothersOnTreatment(patient->getGeneralState()->mostRecentTreatmentNumber);
        }
        /** If Child/Fetus has CMV and on treatment, increment CMV Fetuses on treatment */
        if (patient->getChildDiseaseState()->hadCMV) {
            incrementCMVFetusesOnTreatment(patient->getGeneralState()->mostRecentTreatmentNumber);
        }
    }
    /* END TREATMENT UPDATERS */

    incrementWeek(); // increment the week of pregnancy the mother will be in after this week.
    setPregnancyTrimester(); // set the pregnancy trimester that the mother will be in after this week. 
    /** updating total live births so far runstats statistics for weekly outputs */
    //setTotalLiveBirthsSoFar(runStats->getChildCohortSummary()->numBirthNoCMV + runStats->getChildCohortSummary()->numBirthWithCMV);
}

void EndWeekUpdater::performEndPatientUpdates() {
    /* Finalizing Patient Diagnosis Outputs */
    // if mother is diagnosed, increment the number of mothers diagnosed in runstats
    if (patient->getMaternalMonitoringState()->diagnosedCMV) {
        incrementMaternalNumDiagnosedCMV();
        incrementMaternalNumDiagnosedCMVPrimaryOrSecondary(patient->getMaternalMonitoringState()->diagnosedCMVStatus);
        // if mother actually did have CMV at any point before or during simulation, increment number of true positive diagnosed CMV cases
        if (patient->getMaternalDiseaseState()->hadCMV) {
            incrementMaternalNumDiagnosedTruePos();
            incrementMaternalNumDiagnosedTruePosPrimaryOrSecondary(patient->getMaternalMonitoringState()->diagnosedCMVStatus);
        }
        // if mother DID NOT actually have CMV at any point before or during simulation, increment number of false positive diagnosed CMV cases
        else {
            incrementMaternalNumDiagnosedFalsePos();
            incrementMaternalNumDiagnosedFalsePosPrimaryOrSecondary(patient->getMaternalMonitoringState()->diagnosedCMVStatus);
        }
    }

    // determine whether the patient has had treatment throughout the simulation
    bool hadTreatment;
    int sum = std::accumulate(patient->getGeneralState()->hadTreatment, patient->getGeneralState()->hadTreatment + SimContext::NUM_TREATMENTS, 0);
    // if the sum of the had treatment array is NOT 0, then the patient (mother-child pair) has had at least one treatment throughout pregnancy
    if (sum != 0) {
        // if mother-child pair was on treatment, increment relevant runstats
        incrementNumEverOnAnyTreatment();
        for (int treatmentNum = 0; treatmentNum < SimContext::NUM_TREATMENTS; treatmentNum++) {
            // for each treatment, if mother was ever on that particular treatment
            if (patient->getGeneralState()->hadTreatment[treatmentNum]) {
                // increment number of patients that were ever on the treatmnet
                incrementNumEverOnTreatmentNumber(treatmentNum);
                // if baby was born
                if (patient->getChildState()->deliveryOcurred) {
                    if (patient->getChildDiseaseState()->hadCMV) {
                        incrementNumMothersWithTreatmentBirthedCMVPositiveChild(treatmentNum);
                    }
                    else{
                        incrementNumMothersWithTreatmentBirthedCMVNegativeChild(treatmentNum);
                    }
                }
            }
        }
        // if patient was on treatment and child was born cCMV+
        if (patient->getChildState()->deliveryOcurred && patient->getChildDiseaseState()->hadCMV) {
            incrementNumCMVPositiveChildrenBornWithTreatment();
        }
    }
    else {
        // if patient was never on treatment and child was born cCMV+
        if (patient->getChildState()->deliveryOcurred && patient->getChildDiseaseState()->hadCMV) {
            incrementNumCMVPositiveChildrenBornNeverTreatment();
        }
    }
}