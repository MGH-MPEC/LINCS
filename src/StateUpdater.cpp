#include "include.h"

/** Constructor takes in the patient object */
StateUpdater::StateUpdater(Patient *patient) :
        patient(patient)
{

}

/**Destructor is empty, no cleanup required */
StateUpdater::~StateUpdater(void) {

}

/**
 * \brief Virtual function to perform the initial updates upon patient creation.
 * Sets the simContext, and tracer to match that of this->patient */
void StateUpdater::performInitialization() {
	// Copy the pointers to the simContext and tracer objects
	this->simContext = this->patient->simContext;
    this->runStats = this->patient->runStats;
    this->costStats = this->patient->costStats;
    this->tracer = this->patient->tracer;
}

/**
 * \brief VIrtual function to perform the initial updates upon patient creation
 * Sets the simContext, runStats, and tracer to match that of this->patient
*/
void StateUpdater::performInitialUpdates() {
    // Copy the pointers to the simContext, runStats, and tracer objects
    this->simContext = this->patient->simContext;
    this->runStats = this->patient->runStats;
    this->costStats = this->patient->costStats;
    this->tracer = this->patient->tracer;
}

/**
 * \brief Virtual function to perform all updates for a simulated week. 
 * Empty for now, no actions to perform if child does not override
*/
void StateUpdater::performWeeklyUpdates() {
    // Empty for now, no actions to perform if child does not override. 
}
void StateUpdater::performMonthlyUpdates() {
    // Empty for now, no actions to perform if child does not override. 
}

/** Virtual function changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model */
void StateUpdater::setSimContext(SimContext *newSimContext){
	this->simContext = newSimContext;
}

void StateUpdater::resetMonthNum() {
    patient->generalState.monthNum = 0;
}
void StateUpdater::resetWeekNum() {
    patient->generalState.weekNum = 0;
}

void StateUpdater::setMaternalAgeMonths(int ageMonths) {
    patient->maternalState.maternalAgeMonths = ageMonths;
    patient->maternalState.maternalAgeYearsExact = (double)ageMonths/12;
    patient->maternalState.maternalAgeYears = (int) (patient->maternalState.maternalAgeYearsExact + 0.5);
}


void StateUpdater::setTracingEnabled(bool tracingEnabled) {
    patient->generalState.tracingEnabled = tracingEnabled;
}

void StateUpdater::setBreastFeeding(bool breastfeedingStatus) {
    patient->generalState.breastfeedingStatus = breastfeedingStatus;
}


void StateUpdater::incrementWeek() {
    patient->generalState.weekNum++;
    patient->maternalState.maternalAgeYearsExact = patient->maternalState.maternalAgeYearsExact + (double)1/52;
    patient->maternalState.maternalAgeMonths = (int)(patient->maternalState.maternalAgeYearsExact/12 + 0.5);
    patient->maternalState.maternalAgeYears = (int)(patient->maternalState.maternalAgeYearsExact + 0.5);
}

void StateUpdater::incrementMonth() {
    patient->generalState.monthNum++;
    patient->maternalState.maternalAgeMonths++;
    patient->maternalState.maternalAgeYearsExact = patient->maternalState.maternalAgeMonths/12;
    patient->maternalState.maternalAgeYears = (int)(patient->maternalState.maternalAgeYearsExact + 0.5);
    
}


void StateUpdater::incrementCohortSize(){
    runStats->popSummary.numCohorts++;
    costStats->popSummary.numPatients++;
}

void StateUpdater::incrementPrenatalTestID() {
    patient->generalState.prenatalTestID++;
}

void StateUpdater::setPregnancyTrimester() {
    patient->generalState.trimester = CmvUtil::getTrimester(patient);
}

void StateUpdater::setPrenatalTestsPerformed(bool performed, int testIndex) {
    patient->generalState.prenatalTestsPerformed[testIndex] = performed;
}

void StateUpdater::setPrenatalTestsLastResult(int result, int testIndex) {
    patient->generalState.prenatalTestsLastResult[testIndex] = result;
}

void StateUpdater::setPendingFollowUpTestPerformed(bool performed, SimContext::PrenatalTestInstance followUpInstance) {
    for (int i = 0; i < patient->generalState.pendingFollowUpTests[followUpInstance.weekToTest].size(); i++) {
        if (followUpInstance.testID == patient->generalState.pendingFollowUpTests[followUpInstance.weekToTest][i].testID) {
            patient->generalState.pendingFollowUpTests[followUpInstance.weekToTest][i].testPerformed = true;
        }
    }
}

void StateUpdater::setPendingConfirmatoryTestPerformed(bool performed, SimContext::PrenatalTestInstance confirmatoryInstance) {
    for (int i = 0; i < patient->generalState.pendingConfirmatoryTests[confirmatoryInstance.weekToTest].size(); i++ ) {
        if (confirmatoryInstance.testID == patient->generalState.pendingConfirmatoryTests[confirmatoryInstance.weekToTest][i].testID) {
            patient->generalState.pendingConfirmatoryTests[confirmatoryInstance.weekToTest][i].testPerformed = true;
        }
    }
}

void StateUpdater::setWeekTestLastPerformed(int week, int testIndex) {
    patient->generalState.weekTestLastPerformed[testIndex] = week;
}

void StateUpdater::setAppointmentThisWeek(bool prenatalAppointmentThisWeek) {
    patient->generalState.prenatalAppointmentThisWeek = prenatalAppointmentThisWeek;
}

void StateUpdater::setPrenatalTestResultReturn(int week, SimContext::PrenatalTestInstance prenatalTestInstance) {
    int result = prenatalTestInstance.result;
    int testIndex = prenatalTestInstance.testAssay;
    if (result < -1 || result > 2) {
        std::cout << "INVALID RESULT, NOT A VALID OUTCOME. CHECK PRENATAL TEST SPECS. " << endl; cin.get();
    }
    else {
        patient->generalState.prenatalTestResultReturn[week].push_back(prenatalTestInstance);
    }
}

void StateUpdater::setOnTreatment(bool onTreatment) {
    patient->generalState.onTreatment = onTreatment;
    
}

void StateUpdater::setMostRecentTreatmentNumber(int mostRecentTreatmentNumber) {
    patient->generalState.mostRecentTreatmentNumber = mostRecentTreatmentNumber;
    if (mostRecentTreatmentNumber >=0 ) {
        RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
        if (currTime) {
            currTime->numOnTreatment[mostRecentTreatmentNumber]++;
        }
    }
}

void StateUpdater::setWeekTreatmentStarted(int week, int treatment) {
    patient->generalState.weekTreatmentStarted[treatment] = week;
}

void StateUpdater::setHadTreatment(bool hadTreatment, int treatment) {
    patient->generalState.hadTreatment[treatment] = hadTreatment;
}

/** Maternal Status updaters */
void StateUpdater::setMaternalIsAlive(bool isAlive) {
    patient->maternalState.isAlive = isAlive;
}


/** Maternal Disease State Updater */
void StateUpdater::setActiveCMV(bool activeCMV) {
    patient->maternalDiseaseState.activeCMV = activeCMV;
}
void StateUpdater::setViremia(bool viremia) {
    patient->maternalDiseaseState.viremia = viremia;
}
void StateUpdater::setMaternalIgG(bool IgG) {
    patient->maternalDiseaseState.IgG = IgG;
}
void StateUpdater::setMaternalIgM(bool IgM) {
    patient->maternalDiseaseState.IgM = IgM;
}
void StateUpdater::setMaternalAvidity(int avidity) {
    patient->maternalDiseaseState.avidity = avidity;
}
void StateUpdater::setPrevalentCMVCase(bool isPrevalentCMVCase) {
    patient->maternalDiseaseState.isPrevalentCMVCase = isPrevalentCMVCase;
}
void StateUpdater::setWeekOfMaternalCMVInfection(int weekOfMaternalCMVInfection) {
    patient->maternalDiseaseState.weekOfMaternalCMVInfection = weekOfMaternalCMVInfection;
}
void StateUpdater::setPrevCMVCase(bool prevCMVCase) {
    patient->maternalDiseaseState.prevCMVCase = prevCMVCase;
}
void StateUpdater::setPrevCMVStratum(int prevCMVStratum) {
    patient->maternalDiseaseState.prevCMVStratum = prevCMVStratum;
}
void StateUpdater::setPrevCMVPrimaryOrSecondary(int prevCMVPrimaryOrSecondary) {
    patient->maternalDiseaseState.prevCMVPrimaryOrSecondary = prevCMVPrimaryOrSecondary;
}
void StateUpdater::setHadCMV(bool hadCMV) {
    patient->maternalDiseaseState.hadCMV = hadCMV;
}
void StateUpdater::setHadCMVDuringSim(bool hadCMVDuringSim) {
    patient->maternalDiseaseState.hadCMVDuringSim = hadCMVDuringSim;
}
void StateUpdater::setWeekLastCMV(int weekLastCMV) {
    patient->maternalDiseaseState.weekLastCMV = weekLastCMV;
}
void StateUpdater::setMildIllness(bool mildIllness) {
    patient->maternalDiseaseState.mildIllness = mildIllness;
}
void StateUpdater::setHadMildIllness(bool hadMildIllness) {
    patient->maternalDiseaseState.hadMildIllness = hadMildIllness;
}
void StateUpdater::setWeekOfMildIllness(int weekMildIllness) {
    patient->maternalDiseaseState.weekMildIllness = weekMildIllness;
}
void StateUpdater::setVerticalTransmission(bool verticalTransmission) {
    patient->maternalDiseaseState.verticalTransmission = verticalTransmission;
}
void StateUpdater::setPrimaryOrSecondary(int primaryOrSecondary) {
    patient->maternalDiseaseState.primaryOrSecondary = primaryOrSecondary;
}


/** Maternal Monitoring State updater */
void StateUpdater::setExposureStatus(bool exposureStatus) {
    patient->maternalMonitoringState.exposureStatus = exposureStatus;
}
void StateUpdater::setKnownCMV(bool knownCMV) {
    patient->maternalMonitoringState.knownCMV = knownCMV;
}
void StateUpdater::setKnownHadCMV(bool knownHadCMV) {
    patient->maternalMonitoringState.knownHadCMV = knownHadCMV;
}
void StateUpdater::setDiagnosedMaternalCMV(bool diagnosedCMV) {
    if (diagnosedCMV) {
        patient->maternalMonitoringState.diagnosedCMV = diagnosedCMV;
        patient->maternalMonitoringState.weekCMVDiagnosed = patient->generalState.weekNum;
    }
    else {
        patient->maternalMonitoringState.diagnosedCMV = diagnosedCMV;
    }
        
}
void StateUpdater::setKnownPrimaryOrSecondary(int maternalInfectionCondition) {
    patient->maternalMonitoringState.diagnosedCMVStatus = maternalInfectionCondition;
}
void StateUpdater::setMildIllnessSymptoms(bool mildIllnessSymptoms) {
    patient->maternalMonitoringState.mildIllnessSymptoms = mildIllnessSymptoms;
}
void StateUpdater::setWeekMaternalCMVDiagnosed(int weekCMVDiagnosed) {
    patient->maternalMonitoringState.weekCMVDiagnosed;
}
void StateUpdater::setMonthCMVDetected(int month) {
    patient->maternalMonitoringState.monthCMVDetected = month;
}
void StateUpdater::setKnownMaternalViremia(bool viremiaStatus) {
    patient->maternalMonitoringState.viremiaStatus = viremiaStatus;
}
void StateUpdater::setKnownMaternalIgM(bool IgM) {
    patient->maternalMonitoringState.IgMStatus = IgM;
}
void StateUpdater::setKnownMaternalIgG(bool IgG) {
    patient->maternalMonitoringState.IgGStatus = IgG;
}
void StateUpdater::setKnownAvidity(int avidity) {
    patient->maternalMonitoringState.avidity;
}
void StateUpdater::setPCRWouldBePositive(bool PCRWouldBePositive) {
    patient->maternalDiseaseState.PCRWouldBePositive = PCRWouldBePositive;
}
void StateUpdater::setMaternalCMVSymptoms(bool CMVSymptoms) {
    patient->maternalMonitoringState.CMVSymptoms = CMVSymptoms;
}
void StateUpdater::setNumRegularAppointments(int numRegularAppointments) {
    patient->maternalMonitoringState.numRegularAppointments = numRegularAppointments;
}
void StateUpdater::incrementNumRegularAppointments() {
    patient->maternalMonitoringState.numRegularAppointments++;
}
void StateUpdater::setNumMissedAppointments(int numMissedAppointments) {
    patient->maternalMonitoringState.numMissedAppointments = numMissedAppointments;
}
void StateUpdater::incrementNumMissedAppointments() {
    patient->maternalMonitoringState.numMissedAppointments++;
}
void StateUpdater::setNumUltrasounds(int numUltrasounds) {
    patient->maternalMonitoringState.numUltrasounds = numUltrasounds;
}
void StateUpdater::incrementNumUltrasounds() {
    patient->maternalMonitoringState.numUltrasounds++;
}

void StateUpdater::pushBackPrenatalFollowUp(SimContext::PrenatalTestInstance prenatalTestInstance) {
    patient->generalState.pendingFollowUpTests[prenatalTestInstance.weekToTest].push_back(prenatalTestInstance);
}
void StateUpdater::pushBackPrenatalConfirmatory(SimContext::PrenatalTestInstance prenatalTestInstance) {
    patient->generalState.pendingConfirmatoryTests[prenatalTestInstance.weekToTest].push_back(prenatalTestInstance);
}
void StateUpdater::pushBackPrenatalSpecialCase(SimContext::PrenatalTestInstance prenatalTestInstance) {
    patient->generalState.specialCaseTests[prenatalTestInstance.weekToTest].push_back(prenatalTestInstance);
}
void StateUpdater::markPendingFollowUpTestAsDone(int testID) {
    for (int i = 0; i < patient->generalState.pendingFollowUpTests[patient->generalState.weekNum].size(); i++) {
        if (patient->generalState.pendingFollowUpTests[patient->generalState.weekNum][i].testID == testID) {
            patient->generalState.pendingFollowUpTests[patient->generalState.weekNum][i].testPerformed = true;
        }
    }
}
void StateUpdater::markPendingConfirmatoryTestAsDone(int testID) {
    for (int i = 0; i < patient->generalState.pendingConfirmatoryTests[patient->generalState.weekNum].size(); i++) {
        if (patient->generalState.pendingConfirmatoryTests[patient->generalState.weekNum][i].testID == testID) {
            patient->generalState.pendingConfirmatoryTests[patient->generalState.weekNum][i].testPerformed = true;
        }
    }
}

void StateUpdater::setSuspectedCMVStatus(int suspectedCMVStatus) {
    patient->maternalMonitoringState.suspectedCMVStatus = suspectedCMVStatus;
}
void StateUpdater::setSuspectedCMVTrimester(int suspectedCMVTrimester) {
    patient->maternalMonitoringState.suspectedCMVTrimester = suspectedCMVTrimester;
}
void StateUpdater::setPerformWeeklyIgmIggAvidityOutcomeCheck(bool perform) {
    patient->maternalMonitoringState.performWeeklyIgmIggAvidityOutcomeCheck = perform;
}
void StateUpdater::setIgmToIggDelay(int weeks) {
    patient->maternalMonitoringState.IgmToIggDelay = weeks;
}
void StateUpdater::setIggToAvidityDelay(int weeks) {
    patient->maternalMonitoringState.IggToAvidityDelay = weeks;
}

/* Child State Updaters */
void StateUpdater::setChildIsAlive(bool isAlive) {
    patient->childState.isAlive = isAlive;
}
void StateUpdater::setChildAgeWeeks(int ageWeeks) {
    patient->childState.ageWeeks = ageWeeks;
}
void StateUpdater::setChildAgeMonths(int ageMonths) {
    patient->childState.ageMonths = ageMonths;
}
void StateUpdater::setChildAgeYears(int ageYears) {
    patient->childState.ageYears = ageYears;
}
void StateUpdater::setDeliveryOccurred(bool deliveryOccured) {
    patient->childState.deliveryOcurred = deliveryOccured;
}
void StateUpdater::setBiologicalSex(int biologicalSex) {
    patient->childState.biologicalSex = biologicalSex;
}

/* Child Disease State Updaters */
void StateUpdater::setChildInfectedCMV(bool infectedCMV) {
    patient->childDiseaseState.infectedCMV = infectedCMV;
}
void StateUpdater::setChildDetectableCMV(bool detectableCMV) {
    patient->childDiseaseState.detectableCMV = detectableCMV;
}
void StateUpdater::setChildHadCMV(bool hadCMV) {
    patient->childDiseaseState.hadCMV = hadCMV;
}
void StateUpdater::setCMVAfterBirth(bool CMVAfterBirth) {
    patient->childDiseaseState.CMVAfterBirth = CMVAfterBirth;
}
void StateUpdater::setPhenotypeCMV(int phenotypeCMV) {
    patient->childDiseaseState.phenotypeCMV = phenotypeCMV;
}
void StateUpdater::setChildCMVInfectionTime(int infectionTime) {
    patient->childDiseaseState.infectionTime = infectionTime;
}
void StateUpdater::setInfectionTimeInMonths(bool infectionTimeInMonths) {
    patient->childDiseaseState.infectionTimeInMonths = infectionTimeInMonths;
    patient->childDiseaseState.infectionTrimester = CmvUtil::getTrimester(patient);
}
void StateUpdater::setChildCMVPrimaryOrSecondary(int primaryOrSecondary){
    patient->childDiseaseState.primaryOrSecondary = primaryOrSecondary;
}

/* Child Monitoring State Updaters */
void StateUpdater::setHadUltrasound(bool ultrasound) {
    patient->childMonitoringState.ultrasound = ultrasound;
}
void StateUpdater::setHadAmniocentesis(bool amniocentesis) {
    patient->childMonitoringState.amniocentesis = amniocentesis;
}
void StateUpdater::setChildObservedSymptomaticCMV(bool observedSymptomatic) {
    patient->childMonitoringState.observedSymptomatic = observedSymptomatic;
}
void StateUpdater::setChildKnownCMV(bool knownCMV) {
    patient->childMonitoringState.knownCMV = knownCMV;
}

/* TREATMENT SYMPTOM UPDATER */
void StateUpdater::symptomReductionAlgorithm() {
    // only applies if child does indeed have CMV
    if (patient->getChildDiseaseState()->hadCMV && patient->getGeneralState()->mostRecentTreatmentNumber >=0) {
        int phenotype = patient->getChildDiseaseState()->phenotypeCMV;
        int mostRecentTreatmentNumber = patient->getGeneralState()->mostRecentTreatmentNumber;
        // phenotype cannot be reduced if it's already at phenotype 0
        if (phenotype > 0) {
            vector<double> switchingProbabilityVector(0,SimContext::NUM_CMV_PHENOTYPES);
            for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
                switchingProbabilityVector.push_back(simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[mostRecentTreatmentNumber].reductionOfSymptoms[phenotype][i]);
            }
            
            double randNum = CmvUtil::getRandomDouble();
            int updatedPhenotype = CmvUtil::selectFromDist(switchingProbabilityVector, randNum);
            setPhenotypeCMV(updatedPhenotype);
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Phenotype changed from Type %d to Type %d due to Treatment %d\n", phenotype, updatedPhenotype, mostRecentTreatmentNumber+1);
            }
        }
    }
    else {
        cout << "Child Symptom Reduction is being applied even when fetus has not had CMV or has not had treatment administered. Check StateUpdater::symptomReductionAlgorithm function and where it's called." << endl; cin.get();
    }
}

/** RUN STATS UPDATERS */
/* Functions to update the Prenatal Time Summary runstats statistics */
void StateUpdater::incrementActiveMaternalCMV(int primaryOrSecondary) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->activeMaternalCMV++;
        if (primaryOrSecondary == SimContext::PRIMARY) {
            currTime->activePrimaryMaternalCMV++;
        }
        else {
            currTime->activeSecondaryMaternalCMV++;
        }
    }
}
/*
void StateUpdater::incrementNumDiagnosedFetalCMV() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numDiagnosedFetalCMV++;
    }
}
*/

void StateUpdater::incrementNumAliveFetuses() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numAliveFetuses++;
    }
}

void StateUpdater::setTotalLiveBirthsSoFar(int totalNumLiveBirths) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->totalNumLiveBirths = totalNumLiveBirths;
    }
}

void StateUpdater::incrementNumLiveBirths() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numLiveBirths++;
    }
}
/*
void StateUpdater::incrementNumCMVBirthsByPhenotype(int phenotype) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numCMVBirthsByPhenotype[phenotype]++;
    }
}
*/
/*
void StateUpdater::incrementNumFetalDeaths() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numFetalDeaths++;
        if (patient->getChildDiseaseState()->hadCMV && patient->getChildDiseaseState()->phenotypeCMV >= 0 && patient->getChildDiseaseState()->phenotypeCMV < SimContext::NUM_CMV_PHENOTYPES) {
            currTime->numCMVFetalDeathsByPhenotype[patient->getChildDiseaseState()->phenotypeCMV]++;
        }
        else {
            cout << "WARNING: Trying to increment num fetal CMV deaths when there is no fetal CMV. Check StateUpdater::incrementNumFetalDeaths and where it's being called. Press enter to continue." << endl; cin.get();
        }
    }
}
*/
void StateUpdater::incrementNumEverOnAnyTreatment() {
    runStats->popSummary.totalNumEverOnAnyTreatment++;
}

void StateUpdater::incrementNumEverOnTreatmentNumber(int treatment) {
    runStats->popSummary.totalNumTreatment[treatment]++;
}

void StateUpdater::incrementNumMothersWithTreatmentBirthedCMVPositiveChild(int treatment) {
    runStats->popSummary.numMothersWithTreatmentBirthedCMVPositveChild[treatment]++;
}

void StateUpdater::incrementNumMothersWithTreatmentBirthedCMVNegativeChild(int treatment) {
    runStats->popSummary.numMothersWithTreatmentBirthedCMVNegativeChild[treatment]++;
}

void StateUpdater::incrementNumCMVPositiveChildrenBornWithTreatment() {
    runStats->popSummary.numCMVPositiveChildrenBornWithTreatment++;
}

void StateUpdater::incrementNumCMVPositiveChildrenBornNeverTreatment() {
    runStats->popSummary.numCMVPositiveChildrenBornNeverTreatment++;
}

void StateUpdater::incrementNumCurrOnTreatment(int treatment) {
    runStats->popSummary.totalWeeksOnEachTreatment[treatment]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numOnTreatment[treatment]++;
    }
}


void StateUpdater::incrementNumCMVMothersOnTreatment(int treatment) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numCMVMothersOnTreatment[treatment]++;
    }
}

void StateUpdater::incrementCMVFetusesOnTreatment(int treatment) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numCMVFetusesOnTreatment[treatment]++;
    }
}


void StateUpdater::incrementMaternalNoCMV() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->maternalNoCMV++;
    }
}

void StateUpdater::incrementMaternalCMVNoTreatment() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->maternalCMVNoTreatment++;
    }
}

void StateUpdater::incrementMaternalCMVOnTreatment(int treatment) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->maternalCMVOnTreatment[treatment]++;
    }
}

void StateUpdater::incrementMaternalNoCMVOnTreatment(int treatment) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->maternalNoCMVOnTreatment[treatment]++;
    }
}

void StateUpdater::incrementNumFetalNoCMV() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->fetalNoCMV++;
    }
}

void StateUpdater::incrementNumMiscarriagesNoFetalCMV() {
    runStats->childSummary.numMiscarriagesNoCMV++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numMiscarriagesNoFetalCMV++;
    }
}

void StateUpdater::incrementNumMiscarriagesFetalCMV() {
    runStats->childSummary.numMiscarriagesWithCMV++;
    runStats->childSummary.numMiscarriagesWithCMVByType[patient->getChildDiseaseState()->phenotypeCMV]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numMiscarriagesFetalCMV++;
    }
}

void StateUpdater::incrementNumMiscarriagesNoTreatment() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numMiscarriagesFetalCMVNoTreatment++;
    }
}

void StateUpdater::incrementNumMiscarriagesFetalCMVOnTreatment(int treatment) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numMiscarriagesFetalCMVTreatment[treatment]++;
    }
}

void StateUpdater::incrementNumAppointmentsAttended() {
    runStats->popSummary.totalClinicVisitsPregnancy++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numAttendedPrenatalAppointments++;
    }
}

void StateUpdater::incrementNumSymptomaticAppointmentsAttended() {
    runStats->popSummary.numSymptomaticTriggeredVisits++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numSymptomaticTriggeredAppointmentsAttended++;
    }
}

void StateUpdater::incrementNumMildIllnessAppointmentsAttended() {
    runStats->popSummary.numMildIllnessTriggeredVisits++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numMildIllnessTriggeredAppointmentsAttended++;
    }
}

void StateUpdater::incrementNumScheduledPrenatalAppointment() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numScheduledPrenatalAppointments++;
    }
}

void StateUpdater::incrementMaternalCMVNoFetalCMV() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->maternalCMVNoFetalCMV++;
    }
}

void StateUpdater::incrementNumTestPerformed(int testNum) {
    runStats->popSummary.totalNumPrenatalTest[testNum]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numTestPerfomed[testNum]++;
    }
}

void StateUpdater::incrementNumTruePositives(int testNum) {
    runStats->popSummary.totalNumPrenatalTestTruePositives[testNum]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numTruePositives[testNum]++;
    }
}

void StateUpdater::incrementNumTrueNegatives(int testNum) {
    runStats->popSummary.totalNumPrenatalTestTrueNegatives[testNum]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numTrueNegatives[testNum]++;
    }
}

void StateUpdater::incrementNumFalsePositives(int testNum) {
    runStats->popSummary.totalNumPrenatalTestFalsePositives[testNum]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numFalsePositives[testNum]++;
    }
}

void StateUpdater::incrementNumFalseNegatives(int testNum) {
    runStats->popSummary.totalNumPrenatalTestFalseNegatives[testNum]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numFalseNegatives[testNum]++;
    }
}

void StateUpdater::incrementNumMaternalPrimaryInfectionInTrimester(int trimester) {
    runStats->popSummary.numMaternalPrimaryInfectionInTrimester[trimester]++;
}

void StateUpdater::incrementNumMaternalSecondaryInfectionInTrimester(int trimester) {
    runStats->popSummary.numMaternalSecondaryInfectionInTrimester[trimester]++;
}
void StateUpdater::incrementPrimaryVTByTrimester(int trimester) {
    runStats->popSummary.numPrimaryVTByTrimester[trimester]++;
}
void StateUpdater::incrementSecondaryVTByTrimester(int trimester) {
    runStats->popSummary.numSecondaryVTByTrimester[trimester]++;
}

void StateUpdater::incrementNumTruePositivesFetalCMV(int testNum, int phenotype) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numTruePositivesFetalCMV[testNum][phenotype]++;
    }
}

void StateUpdater::incrementNumFalseNegativesFetalCMV(int testNum, int phenotype) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numFalseNegativesFetalCMV[testNum][phenotype]++;
    }
}

void StateUpdater::incrementNumNewMaternalCMV() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numNewMaternalCMVInfections++;
    }
}

void StateUpdater::incrementNumNewFetalCMV() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numNewFetalCMVInfections++;
    }
}

void StateUpdater::incrementNumMaternalMildIllness() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numMaternalMildIllness++;
    }
}

void StateUpdater::incrementNumNewMaternalMildIllness() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numNewMaternalMildIllness++;
    }
}

void StateUpdater::incrementLostMaternalMildIllness() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->lostMaternalMildIllness++;
    }
}

void StateUpdater::incrementWeeklyNumVerticalTransmissions() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numVerticalTransmissions++;
    }
}

void StateUpdater::incrementBirthsNegativeMaternalCMV() {
    runStats->childSummary.numBirthNoCMV++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->birthsCMVNegative++;
    }
}

void StateUpdater::incrementBirthsPositiveMaternalCMV() {
    runStats->childSummary.numBirthWithCMV++;
    runStats->childSummary.numBornWithCMVByType[patient->getChildDiseaseState()->phenotypeCMV]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->birthsCMVPositive++;
        currTime->numCMVBirthsByPhenotype[patient->getChildDiseaseState()->phenotypeCMV]++;
    }
}

void StateUpdater::incrementNewFetalInfectionsByPhenotype(int phenotype) {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->newFetalInfectionsByPhenotype[phenotype]++;
    }
}

/* Functions to update the Maternal Summary runstats statistics */
/** incrementMaternalCMVInfections increments the number of maternal CMV infections for a particular subcohort by 1*/
void StateUpdater::incrementMaternalCMVinfections() {
    runStats->maternalSummary.numCMVInfections++;
}

void StateUpdater::incrementMaternalCMVInfectionsByTrimester(int trimester) {
    runStats->maternalSummary.numCMVInfectionsByTrimester[trimester]++;
}


/** incrementVerticalTransmissionsPrimaryOrSecondary increments the number of vertical transmissions stratified by whether the VT came from primary or nonprimary infection (or none, but that shouldn't be happening if this function is being called.) */
void StateUpdater::incrementVerticalTransmissionsPrimaryOrSecondary(int primaryOrSecondary) {
    if (primaryOrSecondary == SimContext::NONE) {
        cout << "ERROR: TRYING TO INCREMENT VERTICAL TRANSMISSIONS WHEN MOTHER IS NOT INFECTED (INFECTION TYPE LISTED AS NONE). CHECK CMVInfectionUpdater MODULE." << endl;
        cin.get();
    }
    else {
        runStats->popSummary.totalNumVT[primaryOrSecondary]++;
    }
}

/** incrementNumPrimaryInfections increments the number of primary CMV infection cases seen in mothers (before and during simulation) */
void StateUpdater::incrementMaternalPrimaryInfections() {
    runStats->maternalSummary.numPrimaryInfections++;
}
/** incrementNumSecondaryInfections increments the number of nonprimary CMV infection cases seen in mothers (before and during simulation) */
void StateUpdater::incrementMaternalSecondaryInfections() {
    runStats->maternalSummary.numSecondaryInfections++;
}
/** incrementNumPrimaryInfectionsAbleToVT() increments the number of primary CMV infections that are able to vertically transmit during the course of pregnancy */
void StateUpdater::incrementMaternalPrimaryInfectionsAbleToVT() {
    runStats->maternalSummary.numPrimaryInfectionsAbleToVT++;
}
/**  incrementNumPrimaryInfectionsAbleToVT() increments the number of primary CMV infections that are able to vertically transmit during the course of pregnancy */
void StateUpdater::incrementMaternalSecondaryInfectionsAbleToVT() {
    runStats->maternalSummary.numSecondaryInfectionsAbleToVT++;
}

/** incrementMaternalMildIllness increments the number of mothers that had mild illness during the simulation by 1*/
void StateUpdater::incrementMaternalMildIllness() {
    runStats->maternalSummary.numMildIllness++;
}
/** setMaternalCMVMildIllness sets the number of mothers that had mild illness and CMV during the simulation*/
void StateUpdater::setMaternalCMVMildIllness(int numCMVMildIllness) {
    runStats->maternalSummary.numCMVMildIllness = numCMVMildIllness;
}
/** setMaternalCMVNoMildIllness sets the number of mothers that had CMV but no mild illness during the simulation */
void StateUpdater::setMaternalCMVNoMildIllness(int numCMVNoMildIllness) {
    runStats->maternalSummary.numCMVNoMildIllness = numCMVNoMildIllness;
}
/** incrementMiscarriageNoCMV increments the number of mothers that had a miscarriage without CMV */
void StateUpdater::incrementMisacarriageNoCMV() {
    runStats->maternalSummary.numMiscarriageNoCMV++;
    runStats->childDeathStats.fetalDeaths++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numFetalDeaths++;
    }
}
/** incrementMiscarriageWithCMV increments the number of mothers that had a miscarriage with CMV */
void StateUpdater::incrementMiscarriageWithCMV() {
    runStats->maternalSummary.numMiscarriageWithCMV++;
    runStats->childDeathStats.fetalDeathsCMV++;
    runStats->childDeathStats.fetalDeaths++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numFetalDeaths++;
        currTime->numCMVFetalDeathsByPhenotype[patient->getChildDiseaseState()->phenotypeCMV]++;
    }
}
/** setMiscarriageNoCMV sets the number of mothers that had a miscarriage without CMV */
void StateUpdater::setMiscarriageNoCMV(int numMiscarriageNoCMV) {
    runStats->maternalSummary.numMiscarriageNoCMV = numMiscarriageNoCMV;
}
/** setMiscarriageWithCMV sets the number of mothers that had a miscarriage with CMV */
void StateUpdater::setMiscarriageWithCMV(int numMiscarriageWithCMV) {
    runStats->maternalSummary.numMiscarriageWithCMV = numMiscarriageWithCMV;
}

/** incrementNumDiagnosedCMV increments the number of mothers diagnosed with current or previous CMV */
void StateUpdater::incrementMaternalNumDiagnosedCMV() {
    runStats->maternalSummary.numDiagnosedCMV++;
}

/** incrementWeeklyNumDiagnosedCMV increments the WEEKLY number of mothers diagnosed with current or previous CMV */
void StateUpdater::incrementWeeklyMaternalNumDiagnosedCMV() {
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numDiagnosedMaternalCMV++;
    }
}



/**  */
/** incrementMaternalNumDiagnosedCMVPrimaryOrSecondary increments the number of mothers diagnosed with current or previous CMV stratified by 
 * primary, nonprimary, recent unknown, or long-ago-unknown CMV infection.
 */
void StateUpdater::incrementMaternalNumDiagnosedCMVPrimaryOrSecondary(int primaryOrSecondary) {
    runStats->maternalSummary.numDiagnosedByInfectionStatus[primaryOrSecondary]++;
}

 /** incrementWeeklyMaternalNumDiagnosedCMVPrimaryOrSecondary increments the WEEKLY number of mothers diagnosed with current or previous CMV stratified by 
 * primary, nonprimary, recent unknown, or long-ago-unknown CMV infection.
 */
void StateUpdater::incrementWeeklyMaternalNumDiagnosedCMVPrimaryOrSecondary(int primaryOrSecondary) {
    if (primaryOrSecondary == SimContext::NO_CMV_DIAGNOSES) {
        cout << "ERROR: TRYING TO INCREMENT WEEKLY MATERNAL NUM DIAGNOSED CMV BY STATUS WHEN THERE IS NO STATUS (NO PRIMARY, SECONDARY, UNKNOWN, BUT THERE IS DIAGNOSIS)" << endl; cin.get();
    }
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numDiagnosedMaternalCMVType[primaryOrSecondary]++;
    }
}

/** incrementMaternalNumDiagnosedTruePos() increments the number of TRUE POSITIVE maternal CMV diagnoses */
void StateUpdater::incrementMaternalNumDiagnosedTruePos() {
    runStats->maternalSummary.numTruePositiveDiagnosedCMV++;
}

/** incrementMaternalNumDiagnosedTruePosPrimaryOrSecondary increments the number of TRUE POSITIVE maternal CMV diagnoses stratified by infection status */
void StateUpdater::incrementMaternalNumDiagnosedTruePosPrimaryOrSecondary(int primaryOrSecondary) {
    runStats->maternalSummary.numTruePositiveDiagnosedByInfectionStatus[primaryOrSecondary]++;
}

/** StateUpdater::incrementMaternalNumDiagnosedFalsePos() increments the number of FALSE POSITIVE maternal CMV diagnoses */
void StateUpdater::incrementMaternalNumDiagnosedFalsePos() {
    runStats->maternalSummary.numFalsePositiveDiagnosedCMV++;
}

/** incrementMaternalNumDiagnosedFaLSEPosPrimaryOrSecondary increments the number of FALSE POSITIVE maternal CMV diagnoses stratified by infection status */
void StateUpdater::incrementMaternalNumDiagnosedFalsePosPrimaryOrSecondary(int primaryOrSecondary) {
    runStats->maternalSummary.numFalsePositiveDiagnosedByInfectionStatus[primaryOrSecondary]++;
}

/* Functions to update the Child Cohort Summary runstats statistics */
/** incrementChildCMVInfections increments the number of children that had CMV infection during simulation (by subcohort)*/
void StateUpdater::incrementChildCMVInfections(int trimester) {
    runStats->childSummary.numCMVInfections++;
    runStats->childSummary.numCMVInfectionsByTrimester[trimester]++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        if (patient->getChildDiseaseState()->hadCMV && patient->getChildDiseaseState()->phenotypeCMV >= 0 && patient->getChildDiseaseState()->phenotypeCMV < SimContext::NUM_CMV_PHENOTYPES) {
            currTime->numFetalCMVInfections[patient->getChildDiseaseState()->phenotypeCMV]++;
        }
        else {
            cout << "WARNING: Trying to increment num fetal CMV infections when there is no fetal CMV. Check in Cohort Inputs that the Phenotype Odds all add up to 1. Check StateUpdater::incrementNumFetalCMVInfections and where it's being called. Press enter to continue." << endl; cin.get();

        }
    }
}

/** DO WE NEED THIS? */
/** incrementChildMildIllness increments the number of children that had mild illness during simulation (by subcohort)*/
void StateUpdater::incrementChildMildIllness() {
    runStats->childSummary.numMildIllness++;
}
/** incrementNumWithCMVByType increments the number of children that had CMV infection during simulation (by phenotype) */
void StateUpdater::incrementNumWithCMVByType(int phenotype) {
    runStats->childSummary.numWithCMVByType[phenotype]++;
}
/** incrementSumWeekBirthNoCMV increments */
void StateUpdater::incrementSumWeekBirthNoCMV(int weeks) {
    runStats->childSummary.sumWeekBirthNoCMV += weeks;
}
void StateUpdater::incrementSumWeekBirthWithCMV(int weeks) {
    runStats->childSummary.sumWeekBirthWithCMV+= weeks ;
}
void StateUpdater::incrementNumBirthsNoCMV() {
    runStats->childSummary.numBirthNoCMV++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numLiveBirths++;
    }
}
/*
void StateUpdater::incrementNumBirthsWithCMV() {
    runStats->childSummary.numBirthWithCMV++;
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numLiveBirths++;
        currTime->numCMVBirthsByPhenotype[patient->getChildDiseaseState()->phenotypeCMV]++;
    }
}
*/
/** incrementNumFetalDiagnosedCMV increments the number of children diagnosed with CMV */
void StateUpdater::incrementNumFetalDiagnosedCMV() {
    runStats->childSummary.numDiagnosedCMV++;
    
    RunStats::PrenatalTimeSummary *currTime = getPrenatalTimeSummaryForUpdate();
    if (currTime) {
        currTime->numDiagnosedFetalCMV++;
    }
}

/** incrementNumFetalDeathsAtBirth increments the number of fetuses that die during birth 
 (not counted toward stillbirth or miscarriage, since baby was alive prior to birthing process). */
void StateUpdater::incrementNumFetalDeathsAtBirth() {
    runStats->childSummary.numFetalDeathsAtBirth[patient->getChildDiseaseState()->hadCMV]++;
}



/** \brief getPrenatalTimeSummaryForUpdate returns a non-const pointer to the PrenatalTimeSummary object for the current time period.*/
RunStats::PrenatalTimeSummary *StateUpdater::getPrenatalTimeSummaryForUpdate() {
    int timePeriod;
    timePeriod = patient->generalState.weekNum;
    if (timePeriod < (int) runStats->prenatalTimeSummaries.size()) {
        return runStats->prenatalTimeSummaries[timePeriod];
    }
    for (int i = (int) runStats->prenatalTimeSummaries.size(); i <= timePeriod; i++) {
        RunStats::PrenatalTimeSummary *currTime = new RunStats::PrenatalTimeSummary();
        runStats->initPrenatalTimeSummary(currTime);
        currTime->timePeriod = i;
        runStats->prenatalTimeSummaries.push_back(currTime);
    }
    return runStats->prenatalTimeSummaries[timePeriod];
} /* end getPrenatalTimeSummaryForUpdate */