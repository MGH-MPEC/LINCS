#include "include.h"

/** Constructor takes in the patient object */
CMVInfectionUpdater::CMVInfectionUpdater(Patient* patient) : 
StateUpdater(patient)
{
}

/** Destructor empty. No cleanup required */
CMVInfectionUpdater::~CMVInfectionUpdater(void) {
    
}

/** \brief performInitialization performs all of the state and statistics updates upon patient creation */
void CMVInfectionUpdater::performInitialization() {
    StateUpdater::performInitialUpdates();
    if (patient->getMaternalDiseaseState()->prevCMVCase ) {
        incrementMaternalCMVinfections();
        // if previous case was primary, add to total primary infectoins
        if (patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary == SimContext::TRUE_PRIMARY) {
            // if there is a possibility of vertical transmission from the previous primary infection (based on the stratum), increment the total number of maternal primary infections that could lead to vertical transmission.
            if (simContext->getCohortInputs()->oneTimeVerticalTransmissionProb[SimContext::TRUE_PRIMARY][patient->getMaternalDiseaseState()->prevCMVStratum] > 0) {
                incrementMaternalPrimaryInfectionsAbleToVT();
            }
            incrementMaternalPrimaryInfections();
            incrementNumMaternalPrimaryInfectionInTrimester(SimContext::PRECONCEPTION);
        }
        else if (patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary == SimContext::TRUE_SECONDARY) {
            // if there is a possibility of vertical transmission from the previous non-primary infection (based on the stratum), increment the total number of maternal primary infections that could lead to vertical transmission.
            // If the probability of VT for that stratum is 0, we don't want to count those patients in that stratum because it would skew the results. 
            if (simContext->getCohortInputs()->oneTimeVerticalTransmissionProb[SimContext::TRUE_SECONDARY][patient->getMaternalDiseaseState()->prevCMVStratum] > 0) {
                incrementMaternalSecondaryInfectionsAbleToVT();
            }
            incrementMaternalSecondaryInfections();
            incrementNumMaternalSecondaryInfectionInTrimester(SimContext::PRECONCEPTION);
        }
        else {
            cout << "ERROR: PATIENT HAS NO prevCMVPrimaryOrSecondary ATTRIBUTE DESPITE INITIALIZED TO HAVING PREVIOUS CMV." << endl;
            cin.get();
        }
        // if previous case was nonprimary, add to total nonprimary infections
    }
}

/** \brief performWeeklyUpdates performs all of the state and statistics updates for a simulated week of pregnancy */
void CMVInfectionUpdater::performWeeklyUpdates() {
    int week = patient->getGeneralState()->weekNum;
    
    int trimester = CmvUtil::getTrimester(patient);
    double randNum;
    double probMaternalCMV = simContext->getPrenatalNatHistInputs()->weeklyPrimaryCMV;
    double probMaternalSymptomatic;
    double probVerticalTransmission;
    
    /* If mother does not currently have active CMV and has not had CMV during the simulation, roll for active CMV */
    if (!patient->getMaternalDiseaseState()->activeCMV && !patient->getMaternalDiseaseState()->hadCMVDuringSim) {
        /* Rolling for maternal CMV */
        /** If mother has had previous CMV before or during the simulation */
        if (patient->getMaternalDiseaseState()->prevCMVCase || patient->getMaternalDiseaseState()->hadCMV) {
            probMaternalCMV = simContext->getPrenatalNatHistInputs()->weeklySecondaryCMV;
            // if on treatment, apply reduction of weekly nonprimary/secondary infection multiplier
            if (patient->getGeneralState()->onTreatment){
                probMaternalCMV = probMaternalCMV * simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[patient->getGeneralState()->mostRecentTreatmentNumber].reductionOfWeeklySecondaryCMV;
            }            
            randNum = CmvUtil::getRandomDouble();
            if (randNum < probMaternalCMV) {
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "Secondary Maternal CMV Infection. \n");
                    
                }
                setWeekOfMaternalCMVInfection(week);
                
                /** Set maternal CMV Statuses and print to trace */
                setPrimaryOrSecondary(SimContext::TRUE_SECONDARY);
                incrementMaternalSecondaryInfections();
                incrementMaternalSecondaryInfectionsAbleToVT(); // all infections that occur during the course of pregnancy should theoretically be able to vertically transmit.
                setHadCMV(true);
                setHadCMVDuringSim(true);
                setWeekLastCMV(week);
                setActiveCMV(true);
                incrementNumMaternalSecondaryInfectionInTrimester(trimester);
                /** Rolling for whether mother has symptoms */
                randNum = CmvUtil::getRandomDouble();
                probMaternalSymptomatic = simContext->getCohortInputs()->probSymptomsWithSecondaryCMV[trimester];
                // if on treatment, apply multiplier for probability that mother is symptomatic
                if (patient->getGeneralState()->onTreatment) {
                    probMaternalSymptomatic = probMaternalSymptomatic * simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[patient->getGeneralState()->mostRecentTreatmentNumber].reductionOfWeeklySecondaryProportionSymptomatic;
                }
                if (randNum < probMaternalSymptomatic) {
                    setMaternalCMVSymptoms(true);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Mother's CMV case is symptomatic. \n");
                    }
                    // Roll for attending appointment triggered by symptomatic CMV -- think about generalizing this so we can use for symptomatic CMV, mild illness, and known exposure (in the future)
                    randNum = CmvUtil::getRandomDouble();
                    if (randNum < simContext->getBackgroundScreeningInputs()->seekingHealthcareSymptomatic) {
                        // patient attends appointment
                        // ADD NUMBER OF SYMPTOMATIC TRIGGERED APPOINTMENTS TO RUNSTATS, SEPARATE FROM NUMBER OF MILD ILLNESS TRIGGERED TESTS
                        // ONLY ADD IF THERE WASN'T ALREADY GOING TO BE AN APPOINTMENT
                        // DO THE SAME FOR MILD ILLNESS
                    }

                    // MOVE THIS TO PRENATAL TESTING UPDATER
                    // Roll for special case triggered test (special case is symptomatic maternal CMV)
                    for (int pnTest = 0; pnTest < SimContext::NUM_PRENATAL_TESTS; pnTest++) {
                        randNum = CmvUtil::getRandomDouble();
                        if ( randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[pnTest].symptomaticCMVTrigger) {
                            int confirmatoryNumber = SimContext::PN_NOT_CONF;
                            int testType = 0;//SimContext::PN_TEST_SPECIAL;
                            int testAssay = pnTest;
                            bool result = false; // initialize result to false
                            int weekScheduled = patient->getGeneralState()->weekNum;
                            int testFollowingUpFrom = pnTest; // not relevant for this test, not a follow-up
                            // set week that test will take place
                            int weekToTest = patient->getGeneralState()->weekNum; // will take place the same week as the special event
                            bool testPerformed = false;
                            bool returnToPatient = false;
                            randNum = CmvUtil::getRandomDouble();
                            if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[pnTest].probResultReturn) {
                                returnToPatient = true;
                            }
                            int weekToReturn = weekToTest + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[pnTest].weeksToResultReturn;

                            // set up prenatal test instance (counts as base case testing)
                            SimContext::PrenatalTestInstance specialCaseInstance = {patient->getGeneralState()->prenatalTestID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom, 
                                weekScheduled, weekToTest, testPerformed, result, weekToReturn, returnToPatient};
                            incrementPrenatalTestID(); // CRUCIAL -- DO NOT DELETE

                            // push the confirmatory test state to the patient vector containing current pending confirmatory tests
                            pushBackPrenatalSpecialCase(specialCaseInstance);
                        }
                    }
                }
                
                /** Rolling for whether mother knows about infection */
                randNum = CmvUtil::getRandomDouble();
                if (randNum < simContext->getPrenatalNatHistInputs()->weeklyProbKnownInfection) {
                    setKnownCMV(true);
                    setKnownHadCMV(true);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Mother knows about CMV exposure/infection.\n");
                    }
                }
                else {
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Mother DOES NOT know about CMV exposure/infection.\n");
                    }
                }
            }
        }
        /** if mother has not had CMV infection in the past, it is a primary infection. */
        else { 
            probMaternalCMV = simContext->getPrenatalNatHistInputs()->weeklyPrimaryCMV;
            // if on treatment, apply reduction of weekly primary infection multiplier
            if (patient->getGeneralState()->onTreatment) {
                probMaternalCMV = probMaternalCMV * simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[patient->getGeneralState()->mostRecentTreatmentNumber].reductionOfWeeklyPrimaryCMV;
            }
            randNum = CmvUtil::getRandomDouble();

            if (randNum < probMaternalCMV) {
                if (patient->getGeneralState()->tracingEnabled) {
                    tracer->printTrace(1, "Primary Maternal CMV Infection. \n");
                }
                setWeekOfMaternalCMVInfection(week);

                /** Set maternal CMV Statuses and print to trace */
                setPrimaryOrSecondary(SimContext::TRUE_PRIMARY);
                incrementMaternalPrimaryInfections();
                incrementMaternalPrimaryInfectionsAbleToVT(); // all infections that occur during pregnancy should be able to vertically transmit in theory.
                setHadCMV(true);
                setHadCMVDuringSim(true);
                setWeekLastCMV(week);
                setActiveCMV(true);

                /** increment runstats count for number of mothers in cohort that had CMV infections */
                incrementMaternalCMVinfections();
                incrementNumMaternalPrimaryInfectionInTrimester(trimester);
                /** Rolling for whether mother has symptoms */
                randNum = CmvUtil::getRandomDouble();
                probMaternalSymptomatic = simContext->getCohortInputs()->probSymptomsWithPrimaryCMV[trimester];
                // if on treatment, apply multiplier for probability that mother is symptomatic
                if (patient->getGeneralState()->onTreatment) {
                    probMaternalSymptomatic = probMaternalSymptomatic * simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[patient->getGeneralState()->mostRecentTreatmentNumber].reductionOfWeeklyPrimaryProportionSymptomatic;
                }
                if (randNum < probMaternalSymptomatic) {
                    setMaternalCMVSymptoms(true);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Mother's CMV case is symptomatic.\n");
                    }
                }
                /** Rolling for whether mother knows about infection */
                randNum = CmvUtil::getRandomDouble();
                if (randNum < simContext->getPrenatalNatHistInputs()->weeklyProbKnownInfection) {
                    //incrementMaternalNumDiagnosedCMV();
                    setKnownCMV(true);
                    setKnownHadCMV(true);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Mother knows about CMV exposure/infection.\n");
                    }
                }
                else {
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Mother DOES NOT know about CMV exposure/infection.\n");
                    }
                }
            }
        }
        /* End rolling for CMV Infection */
    }
    

    /* Set MaternalActive CMV to false if it's been more than 3 weeks of Active CMV */
    if (patient->getMaternalDiseaseState()->activeCMV && week - patient->getMaternalDiseaseState()->weekLastCMV >= 3) {
        setActiveCMV(false);
    }
    
    /* MATERNAL INFECTION TO VERTICAL TRANSMISSION */
    if (patient->getMaternalDiseaseState()->hadCMVDuringSim && week - patient->getMaternalDiseaseState()->weekOfMaternalCMVInfection == simContext->getPrenatalNatHistInputs()->maternalInfectionToVerticalTransmissionDelay) {
        if (!patient->getMaternalDiseaseState()->verticalTransmission) {
            /* rolling for vertical transmission from secondary maternal infection */
            randNum = CmvUtil::getRandomDouble();
            probVerticalTransmission = simContext->getPrenatalNatHistInputs()->primaryVerticalTransmission[trimester];
            if (patient->getMaternalDiseaseState()->primaryOrSecondary == SimContext::SECONDARY) {
                probVerticalTransmission = simContext->getPrenatalNatHistInputs()->secondaryVerticalTransmission[trimester];
            }
            // Adjust probability of vertical transmission if patient is on treatment
            if (patient->getGeneralState()->onTreatment) {
                probVerticalTransmission = probVerticalTransmission * simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[patient->getGeneralState()->mostRecentTreatmentNumber].reductionOfVerticalTransmissionFromSecondaryCMV[week];
            }
            if (randNum < probVerticalTransmission) { // if draw for vertical transmission to occur
                setVerticalTransmission(true);
                incrementWeeklyNumVerticalTransmissions();
                setChildHadCMV(true);
                setChildCMVInfectionTime(week);
                incrementNumWithCMVByType(patient->getChildDiseaseState()->phenotypeCMV);
                // if maternal infection was primary
                if (patient->getMaternalDiseaseState()->primaryOrSecondary == SimContext::TRUE_PRIMARY) {
                    setChildCMVPrimaryOrSecondary(SimContext::TRUE_PRIMARY);
                    incrementVerticalTransmissionsPrimaryOrSecondary(SimContext::TRUE_PRIMARY);
                    incrementPrimaryVTByTrimester(trimester);
                    vector<double> childPhenotypeProbs;
                    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
                        childPhenotypeProbs.push_back(simContext->getCohortInputs()->probPhenotypesUponVT[SimContext::TRUE_PRIMARY][trimester][i]);
                    }
                    randNum = CmvUtil::getRandomDouble();
                    int phenotype = CmvUtil::selectFromDist(childPhenotypeProbs, randNum);
                    setPhenotypeCMV(phenotype);
                    
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Primary vertical transmission occurred. Child infected with phenotype %d. Child infection will be detectable in %d weeks.\n", patient->getChildDiseaseState()->phenotypeCMV + 1, simContext->getPrenatalNatHistInputs()->verticalTransmissionToDetectableDelay);
                    }
                }
                // if maternal infection was nonprimary
                else if (patient->getMaternalDiseaseState()->primaryOrSecondary == SimContext::TRUE_SECONDARY) {
                    setChildCMVPrimaryOrSecondary(SimContext::TRUE_SECONDARY);
                    incrementVerticalTransmissionsPrimaryOrSecondary(SimContext::TRUE_SECONDARY);
                    incrementSecondaryVTByTrimester(trimester);
                    vector<double> childPhenotypeProbs;
                    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
                        childPhenotypeProbs.push_back(simContext->getCohortInputs()->probPhenotypesUponVT[SimContext::TRUE_SECONDARY][trimester][i]);
                    }
                    randNum = CmvUtil::getRandomDouble();
                    int phenotype = CmvUtil::selectFromDist(childPhenotypeProbs, randNum);
                    setPhenotypeCMV(phenotype);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Secondary vertical transmission occurred. Child infected with phenotype %d. Child infection will be detectable in %d weeks.\n", patient->getChildDiseaseState()->phenotypeCMV + 1, simContext->getPrenatalNatHistInputs()->verticalTransmissionToDetectableDelay);
                    }
                }
                else {
                    cout << "ERROR: Mother's CMV infection does not have an assigned Primary or Nonprimary status." << endl; cin.get();
                    exit;
                }
                incrementChildCMVInfections(trimester);
                /** If currently on or was on treatment, call Symptom Reduction Algorithm function */
                if (patient->getGeneralState()->mostRecentTreatmentNumber > 0) {
                    symptomReductionAlgorithm();
                }
            }
            else {
                if (patient->getGeneralState()->tracingEnabled){
                    tracer->printTrace(1, "No vertical transmission\n");
                }
            }
        }
    }

    /** VERTICAL TRANSMISSION TO DETECTABLE */
    if (patient->getChildDiseaseState()->hadCMV) {
        if (week - patient->getChildDiseaseState()->infectionTime == simContext->getPrenatalNatHistInputs()->verticalTransmissionToDetectableDelay) {
            setChildDetectableCMV(true);
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Child's CMV infection is now detectable.");
            }
        }
    }

    /** If the mother had prevalent CMV (CMV infection that started before model start), roll for one-time probability of vertical transmission */
    
    if (patient->getMaternalDiseaseState()->prevCMVCase && !patient->getChildDiseaseState()->hadCMV) {
        if (patient->getMaternalDiseaseState()->prevCMVStratum >=4) {
            cout << "ERROR: PREV CMV STRATUM OUT OF BOUNDS" << endl; cin.get();
            exit;
        }

        if (week == simContext->getCohortInputs()->oneTimeVerticalTansmissionWeek[patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary][patient->getMaternalDiseaseState()->prevCMVStratum]) {
            // if vertical transmission has not yet occurred, can continue rolling for one-time probability of vertical transmission
            if (!patient->getMaternalDiseaseState()->verticalTransmission) {
                double probVerticalTransmission = simContext->getCohortInputs()->oneTimeVerticalTransmissionProb[patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary][patient->getMaternalDiseaseState()->prevCMVStratum];
                // if treatment happening, reduce corresponding probability of vertical transmission 
                if (patient->getGeneralState()->mostRecentTreatmentNumber > 0) {
                    if (patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary == SimContext::PRIMARY) {
                        probVerticalTransmission = probVerticalTransmission * simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[patient->getGeneralState()->mostRecentTreatmentNumber].reductionOfPreviousVTFromPrimaryCMV[patient->getMaternalDiseaseState()->prevCMVStratum];
                    }
                    else if (patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary == SimContext::SECONDARY) {
                        probVerticalTransmission = probVerticalTransmission * simContext->getPrenatalTreatmentInputs()->PrenatalTreatmentsVector[patient->getGeneralState()->mostRecentTreatmentNumber].reductionOfPreviousVTFromSecondaryCMV[patient->getMaternalDiseaseState()->prevCMVStratum];
                    }
                }
                bool verticalTransmission = false;
                randNum = CmvUtil::getRandomDouble();
                if (randNum < simContext->getCohortInputs()->oneTimeVerticalTransmissionProb[patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary][patient->getMaternalDiseaseState()->prevCMVStratum]) {
                    verticalTransmission = true;
                }
                if (verticalTransmission) {
                    setVerticalTransmission(true);
                    incrementWeeklyNumVerticalTransmissions();
                    if (patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary == SimContext::PRIMARY) {
                        incrementPrimaryVTByTrimester(trimester);
                    }
                    else if (patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary == SimContext::SECONDARY) {
                        incrementSecondaryVTByTrimester(trimester);
                    }
                    vector<double> childPhenotypeProbs;
                    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
                        childPhenotypeProbs.push_back(simContext->getCohortInputs()->probPhenotypesUponVT[patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary][SimContext::PRECONCEPTION][i]);
                    }

                    randNum = CmvUtil::getRandomDouble();
                    int phenotype = CmvUtil::selectFromDist(childPhenotypeProbs, randNum);
                    setPhenotypeCMV(phenotype);
                    setChildHadCMV(true);
                    setChildInfectedCMV(true);
                    setChildCMVInfectionTime(week);
                    setChildCMVPrimaryOrSecondary(patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary);
                    setInfectionTimeInMonths(false); // infection time is in weeks, not months
                    incrementChildCMVInfections(trimester);
                    // incrementing total number of vertical transmissions in runStats stratified by whether infection was primary or nonprimary
                    incrementVerticalTransmissionsPrimaryOrSecondary(patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary);
                    
                    
                    //int phenotype = drawPhenotype();
                    //setPhenotypeCMV(phenotype);
                    incrementNumWithCMVByType(patient->getChildDiseaseState()->phenotypeCMV);
                    if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "Child infected with phenotype %d from previous %s maternal CMV case (before model start).\n", patient->getChildDiseaseState()->phenotypeCMV + 1, SimContext::PRIMARY_OR_SECONDARY_CHAR[patient->getMaternalDiseaseState()->prevCMVPrimaryOrSecondary]);
                    }
                    /** If currently on or was on treatment, call Symptom Reduction Algorithm function */
                    if (patient->getGeneralState()->mostRecentTreatmentNumber > 0) {
                        symptomReductionAlgorithm();
                    }
                }
            }
        }
        
    }

    /* Mild Illness */
    /** Rolling for Mild Illness */
    
    if (!patient->getMaternalDiseaseState()->mildIllness) {
        randNum = CmvUtil::getRandomDouble();
        if (randNum < simContext->getPrenatalNatHistInputs()->weeklyProbMildIllness) {
            setMildIllness(true);
            /** If patient's first time having mild illness, increment the number of mothers who have had mild illness */
            if (!patient->getMaternalDiseaseState()->hadMildIllness) {
                incrementMaternalMildIllness();
                // roll for tests triggered by mild illness, add to pending tests
            }
            setHadMildIllness(true);
            setWeekOfMildIllness(week);
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Mild Illness.\n");
            }
            // roll for appointment triggered by mild illness

            // Roll for special case triggered test (special case is symptomatic maternal CMV)
            for (int pnTest = 0; pnTest < SimContext::NUM_PRENATAL_TESTS; pnTest++) {
                randNum = CmvUtil::getRandomDouble();
                if ( randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[pnTest].mildIllnessTrigger) {
                    int confirmatoryNumber = SimContext::PN_NOT_CONF;
                    int testType = 0;//SimContext::PN_TEST_SPECIAL;
                    int testAssay = pnTest;
                    bool result = false; // initialize result to false
                    int weekScheduled = patient->getGeneralState()->weekNum;
                    int testFollowingUpFrom = pnTest; // not relevant for this test, not a follow-up
                    // set week that test will take place
                    int weekToTest = patient->getGeneralState()->weekNum; // will take place the same week as the special event
                    bool testPerformed = false;
                    bool returnToPatient = false;
                    randNum = CmvUtil::getRandomDouble();
                    if (randNum < simContext->getPrenatalTestingInputs()->PrenatalTestsVector[pnTest].probResultReturn) {
                        returnToPatient = true;
                    }
                    int weekToReturn = weekToTest + simContext->getPrenatalTestingInputs()->PrenatalTestsVector[pnTest].weeksToResultReturn;

                    // set up prenatal test instance (counts as base case testing)
                    SimContext::PrenatalTestInstance specialCaseInstance = {patient->getGeneralState()->prenatalTestID, testAssay, testType, confirmatoryNumber, testFollowingUpFrom, 
                        weekScheduled, weekToTest, testPerformed, result, weekToReturn, returnToPatient};
                    incrementPrenatalTestID(); // CRUCIAL -- DO NOT DELETE

                    // push the confirmatory test state to the patient vector containing current pending confirmatory tests
                    pushBackPrenatalSpecialCase(specialCaseInstance);
                }
            }
        }
    }
    
    /** After 3 weeks, patient no longer has mild illness. HOW LONG SHOULD PATIENT BE IN MILD ILLNESS? */
    if (patient->getMaternalDiseaseState()->mildIllness && week - patient->getMaternalDiseaseState()->weekMildIllness == 3) {
        setMildIllness(false);
        if (patient->getGeneralState()->tracingEnabled) {
            tracer->printTrace(1, "\tRecovered from Mild Illness.\n");
        }
    }

}

/** \brief performMonthlyUpdates performs all of the state and statistics updates for a simulated month AFTER pregnancy */
void CMVInfectionUpdater::performMonthlyUpdates() {

}


/** \brief drawPhenotype draws for the patient's CMV phenotype 
 * 
 * CHANGED: FUNCTION CURRENTLY DEFINED IN PATIENT OBJECT INITIALIZATION.
*/
/*
int CMVInfectionUpdater::drawPhenotype() {
    vector<double> proportionCMVPhenotypesVector = simContext->getRunSpecsInputs()->proportionCMVPhenotypesVector;
    int randNum = CmvUtil::getRandomDouble();
    int phenotype = selectFromDist(proportionCMVPhenotypesVector, randNum);
    return phenotype;
}
*/