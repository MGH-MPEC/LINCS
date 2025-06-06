#include "include.h"
/** \brief Constructor takes run name and associated simulation context pointer as parameters
 * \param runName a string representing the run name associated with this RunStats object
 * \param simContext a pointer to the SImContext representing the inputs associated with this RunStats object
*/
RunStats::RunStats(string runName, SimContext *simContext) {
    statsFileName = runName;
    statsFileName.append(CmvUtil::FILE_EXTENSION_FOR_OUTPUT);
    this->simContext = simContext;

    initRunStats();

} /* end constructor*/

/** \brief Destructor clears vectors and frees the allocated objects */
RunStats::~RunStats(void) {
    
}

/** \brief initRunsStats initializes all the run stats 
 * 
*/
void RunStats::initRunStats() {
    initPopulationSummary();
    initMaternalCohortSummary();
    initChildCohortSummary(); 
    //initMaternalDeathStats(); // Nothing here yet
    initChildDeathStats();

    patients.clear();
    for (vector<MaternalTimeSummary *>::iterator s = maternalTimeSummaries.begin(); s != maternalTimeSummaries.end(); s++) {
        MaternalTimeSummary *maternalSummary = *s;
        initMaternalTimeSummary(maternalSummary);
    }
    for (vector<ChildTimeSummary *>::iterator s = childTimeSummaries.begin(); s != childTimeSummaries.end(); s++) {
        ChildTimeSummary *childSummary = *s;
        initChildTimeSummary(childSummary);
    }
    //initChildTimeSummary(ChildTimeSummary* currChildStats;);
}

/** \brief finalizeStats calculate all aggregate statistics and values to be outputted
*/
void RunStats::finalizeStats() {
    finalizePopulationSummary();
    finalizeMaternalCohortSummary();
    finalizeChildCohortSummary();
    finalizeMaternalTimeSummary();
    finalizeChildTimeSummary();
    finalizeChildDeathStats();
}

void RunStats::writeStatsFile() {
    CmvUtil::changeDirectoryToResults();
    statsFile = CmvUtil::openFile(statsFileName.c_str(), "w");
    if (statsFile == NULL) {
        statsFileName.append("-tmp");
        statsFile = CmvUtil::openFile(statsFileName.c_str(), "w");
        if (statsFile == NULL) {
            string errorString = "  ERROR - Could not write stats or temporary stats file";
            throw errorString;
        }
    }
    writePopulationSummary();
    writePrenatalTimeSummaries();
}

void RunStats::initPrenatalTimeSummary(PrenatalTimeSummary *currTime) {
    // Initialize all the prenatal time summary values
    currTime->timePeriod = 0;
    currTime->activeMaternalCMV = 0;
    currTime->activePrimaryMaternalCMV = 0;
    currTime->activeSecondaryMaternalCMV = 0;
    currTime->numVerticalTransmissions = 0;
    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
        currTime->numFetalCMVInfections[i] = 0;
        currTime->numCMVBirthsByPhenotype[i] = 0;
        currTime->numCMVFetalDeathsByPhenotype[i] = 0;
    }
    currTime->numDiagnosedMaternalCMV = 0;
    for ( int i = 0; i < SimContext::INFECTION_TYPE_MAX; i++) {
        currTime->numDiagnosedMaternalCMVType[i] = 0;
    }
    currTime->numDiagnosedFetalCMV = 0;
    currTime->numAliveFetuses = 0;
    currTime->totalNumLiveBirths = 0;
    currTime->numLiveBirths = 0;
    currTime->numFetalDeaths = 0;

    // treatment outputs
    for (int i = 0; i < SimContext::NUM_TREATMENTS; i++) {
        currTime->numOnTreatment[i] = 0;
        currTime->numCMVMothersOnTreatment[i] = 0;
        currTime->numCMVFetusesOnTreatment[i] = 0;
    }


    currTime->maternalNoCMV = 0;
    currTime->maternalCMVNoTreatment = 0;
    for (int i = 0; i < SimContext::NUM_TREATMENT_POLICIES; i++) {
        currTime->maternalCMVOnTreatment[i] = 0;
        currTime->maternalNoCMVOnTreatment[i] = 0;
        currTime->numOnTreatment[i] = 0;
        currTime->fetalCMVOnTreatment[i] = 0;
        currTime->fetalNoCMVOnTreatment[i] = 0;
    }
    currTime->fetalNoCMV = 0;
    currTime->numMiscarriagesNoFetalCMV = 0;
    currTime->numMiscarriagesFetalCMV = 0;
    currTime->numMiscarriagesFetalCMVNoTreatment = 0;
    for (int i = 0; i < SimContext::NUM_TREATMENT_POLICIES; i++) {
        currTime->numMiscarriagesFetalCMVTreatment[i] = 0;
    }
    currTime->numSymptomaticTriggeredAppointmentsAttended = 0;
    currTime->numMildIllnessTriggeredAppointmentsAttended = 0;
    
    currTime->maternalCMVNoFetalCMV = 0;
    currTime->numScheduledPrenatalAppointments = 0;
    currTime->numAttendedPrenatalAppointments = 0;
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        currTime->numTestPerfomed[i] = 0;
        currTime->numTruePositives[i] = 0;
        currTime->numTrueNegatives[i] = 0;
        currTime->numFalsePositives[i] = 0;
        currTime->numFalseNegatives[i] = 0;
        for (int j = 0; j < SimContext::NUM_CMV_PHENOTYPES; j++) {
            currTime->numTruePositivesFetalCMV[i][j] = 0;
            currTime->numFalseNegativesFetalCMV[i][j] = 0;
        }
    }
    currTime->numNewMaternalCMVInfections = 0;
    currTime->numNewFetalCMVInfections = 0;
    currTime->numMaternalMildIllness = 0;
    currTime->numVerticalTransmissions = 0;
    currTime->birthsCMVNegative = 0;
    currTime->birthsCMVPositive = 0;
    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
        currTime->newFetalInfectionsByPhenotype[i] = 0;
    }
}
void RunStats::initMaternalTimeSummary(MaternalTimeSummary *maternalCurrStats) {
    maternalCurrStats->numInfections = 0;
    maternalCurrStats->newInfections = 0;
    maternalCurrStats->lostInfections = 0;
    maternalCurrStats->numSymptomatic = 0;
    maternalCurrStats->newSymptomatic = 0;
    maternalCurrStats->lostSymptomatic = 0;
    maternalCurrStats->numMildIllness = 0;
    maternalCurrStats->newMildIllness = 0;
    maternalCurrStats->lostMildIllness = 0;
    maternalCurrStats->totalNumMiscarriagesCMV = 0;
    maternalCurrStats->numMiscarriagesNoCMV = 0;
    maternalCurrStats->totalNumMiscarriagesNoCMV = 0;
    maternalCurrStats->numMiscarriagesCMV = 0;
    maternalCurrStats->totalNumVerticalTransmissions = 0;
    maternalCurrStats->numVerticalTransmissions = 0;

}

void RunStats::initChildTimeSummary(ChildTimeSummary *childCurrStats) {
    childCurrStats->totalBirthsCMVNegative = 0;
    childCurrStats->birthsCMVNegative = 0;
    childCurrStats->totalBirthsCMVNegative = 0;
    childCurrStats->birthsCMVPositive = 0;
    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
        childCurrStats->numberInfectionsByPhenotype[i] = 0;
    }
}

/** \brief initPopulationSummary initializes the PopulationSummary object 
 * 
 * 
*/
void RunStats::initPopulationSummary() {
    int i;

    popSummary.numCohorts = 0;
    popSummary.totalClinicVisitsPregnancy = 0;
    popSummary.numSymptomaticTriggeredVisits = 0;
    popSummary.numMildIllnessTriggeredVisits = 0;
    popSummary.totalNumEverOnAnyTreatment = 0;
    for (i = 0; i < SimContext::NUM_TREATMENT_POLICIES; i++) {
        popSummary.totalNumTreatment[i] = 0;
        popSummary.totalWeeksOnEachTreatment[i] = 0;
        popSummary.numMothersWithTreatmentBirthedCMVPositveChild[i] = 0;
        popSummary.numMothersWithTreatmentBirthedCMVNegativeChild[i] = 0;
        popSummary.numCMVPositiveChildrenBornWithTreatment = 0;
        popSummary.numCMVPositiveChildrenBornNeverTreatment = 0;
    }
    for (i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        popSummary.totalNumPrenatalTest[i] = 0;
        popSummary.totalNumPrenatalTestTruePositives[i] = 0;
        popSummary.totalNumPrenatalTestTrueNegatives[i] = 0;
        popSummary.totalNumPrenatalTestFalsePositives[i] = 0;
        popSummary.totalNumPrenatalTestFalseNegatives[i] = 0;
        popSummary.proportionTruePositives[i] = 0;
        popSummary.proportionTrueNegatives[i] = 0;
        popSummary.proportionFalsePositives[i] = 0;
        popSummary.proportionFalseNegatives[i] = 0;
    }
    for (i = 0; i < SimContext::NUM_TRIMESTERS; i++) {
        popSummary.numMaternalPrimaryInfectionInTrimester[i] = 0;
        popSummary.numMaternalSecondaryInfectionInTrimester[i] = 0;
        popSummary.numPrimaryVTByTrimester[i] = 0;
        popSummary.numSecondaryVTByTrimester[i] = 0;
    }
    popSummary.totalNumVT[SimContext::TRUE_NO_INFECTION] = 0;
    popSummary.totalNumVT[SimContext::TRUE_PRIMARY] = 0;
    popSummary.totalNumVT[SimContext::TRUE_SECONDARY] = 0;
    popSummary.runDate = "";
    popSummary.runName = "";
    popSummary.runSetName = "";
    popSummary.runTime = "";
}

void RunStats::initMaternalCohortSummary() {


    maternalSummary.numCMVInfections = 0;
    maternalSummary.numPrimaryInfections = 0;
    maternalSummary.numSecondaryInfections = 0;
    maternalSummary.numPrimaryInfectionsAbleToVT = 0;
    maternalSummary.numSecondaryInfectionsAbleToVT = 0;
    maternalSummary.numMildIllness = 0;
    maternalSummary.numCMVMildIllness = 0;
    maternalSummary.numCMVNoMildIllness = 0;
    maternalSummary.numMiscarriageNoCMV = 0;
    maternalSummary.numMiscarriageWithCMV = 0;
    
    maternalSummary.numBirthWithCMV = 0;
    maternalSummary.numBirthNoCMV = 0;
    maternalSummary.numDiagnosedCMV = 0;
    maternalSummary.numTruePositiveDiagnosedCMV = 0;
    maternalSummary.numFalsePositiveDiagnosedCMV = 0;
    for (int i = 0; i < SimContext::INFECTION_TYPE_MAX; i++) {
        maternalSummary.numDiagnosedByInfectionStatus[i] = 0;
        maternalSummary.numTruePositiveDiagnosedByInfectionStatus[i] = 0;
        maternalSummary.numFalsePositiveDiagnosedByInfectionStatus[i] = 0;
    }
    for (int i =0;i < SimContext::NUM_TRIMESTERS; i++) {
        maternalSummary.numCMVInfectionsByTrimester[i] = 0;
    }

}

void RunStats::initChildCohortSummary() {
    for (int i = 0; i < SimContext::NUM_TRIMESTERS; i++) {
        childSummary.numCMVInfectionsByTrimester[i] = 0;
        childSummary.numCMVInfectionsFromPrimaryByTrimester[i] = 0;
        childSummary.numCMVInfectionsFromPrimaryByTrimester[i] = 0;
    }
    childSummary.numCMVInfections = 0;
    childSummary.numMildIllness = 0;
    childSummary.sumWeekBirthNoCMV = 0;
    childSummary.sumWeekBirthWithCMV = 0;
    childSummary.averageWeekBirthNoCMV = 0;
    childSummary.averageWeekBirthCMV = 0;
    childSummary.numBirthNoCMV = 0;
    childSummary.numBirthWithCMV = 0;

    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
        childSummary.numWithCMVByType[i] = 0;
        childSummary.numBornWithCMVByType[i] = 0;
        childSummary.numMiscarriagesWithCMVByType[i] = 0;
    }
    childSummary.numMiscarriagesWithCMV = 0;
    childSummary.numMiscarriagesNoCMV = 0;
    childSummary.numDiagnosedCMV = 0;
    childSummary.numFetalDeathsAtBirth[0] = 0;
    childSummary.numFetalDeathsAtBirth[1] = 0;
}

void RunStats::initMaternalDeathStats() {
    // empty for now
}

void RunStats::initChildDeathStats() {
    childDeathStats.fetalDeaths = 0;
    childDeathStats.fetalDeathsCMV = 0;
    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
        childDeathStats.fetalDeathsByType[i] = 0;
    }
}

/* Finalizing run statistics */
/** */
void RunStats::finalizePopulationSummary() {
    // Copy the run/set name 
    const SimContext::RunSpecsInputs *runSpecs = simContext->getRunSpecsInputs();
    popSummary.runSetName = runSpecs->runSetName;
    popSummary.runName = runSpecs->runName;

}

void RunStats::finalizeMaternalCohortSummary() {

}

void RunStats::finalizeChildCohortSummary() {
    /* ERROR: PROBLEMATIC CODE, NEED TO FIX BEFORE SHIPPING 2023-12-29*/
    /* Note: Fixed 2023-12-29 19:36 EST but still need to test. */
    /** Calculating average week of birth for patients without CMV (NEED TO CHANGE DENOMINATOR TO JUST ALIVE PATIENTS WITHOUT CMV)*/
    /** update 01/02/2024: fixed. Still need to test. */

    childSummary.averageWeekBirthNoCMV = (double)childSummary.sumWeekBirthNoCMV/(double)childSummary.numBirthNoCMV;
    
    /** Calculating average week of birth for patients with CMV (NEED TO CHANGE DENOMINATOR TO JUST ALIVE PATIENTS WITH CMV)*/

    childSummary.averageWeekBirthCMV = (double)childSummary.sumWeekBirthWithCMV/(double)childSummary.numBirthWithCMV;
    
}

void RunStats::finalizeMaternalDeathStats() {
    // empty for now
}

void RunStats::finalizeChildDeathStats() {
    // empty for now
}

void RunStats::finalizeMaternalTimeSummary() {
    // empty for now
}

void RunStats::finalizeChildTimeSummary() {
    // empty for now
}

/** \brief writePopulationSummary outputs the PopulationSummary statistics to the stats file */
void RunStats::writePopulationSummary() {
	int i;

	// Print out the section header
	fprintf(statsFile, "POPULATION SUMMARY MEASURES (run completed %s,", popSummary.runDate.c_str());
    fprintf(statsFile, "%s)\n[Program version %s, build %s]", popSummary.runTime.c_str(),
        CmvUtil::CMV_VERSION_STRING, CmvUtil::CMV_EXECUTABLE_COMPILED_DATE);
    fprintf(statsFile, "\n\tRun Size\t%d", simContext->getRunSpecsInputs()->numCohorts);
    fprintf(statsFile, "\n\tNumber of Maternal CMV Infections\t%d", maternalSummary.numCMVInfections);
    fprintf(statsFile, "\n\tNumber of Fetal CMV Infections\t%d", childSummary.numCMVInfections);
    //fprintf(statsFile, "\n\tProportion of Incident Maternal CMV Infections\t%lf");
    //fprintf(statsFile, "\n\tProportion of Incident Child CMV Infections\t%lf");
    fprintf(statsFile, "\n\tNumber of Live Births\t%d", childSummary.numBirthNoCMV + childSummary.numBirthWithCMV);
    fprintf(statsFile, "\n\tNumber of Miscarriages/Stillbirths\t%d", childDeathStats.fetalDeaths);
    fprintf(statsFile, "\n\tTotal # of Appointments Attended\t%d", popSummary.totalClinicVisitsPregnancy);
    fprintf(statsFile, "\n\t# Appointments Attended Triggered by Symptomatic Maternal CMV or Mild Illness\t%d\t", popSummary.numSymptomaticTriggeredVisits + popSummary.numMildIllnessTriggeredVisits);
    fprintf(statsFile, "\n\t# Appointments Attended Triggered by Symptomatic Maternal CMV\t%d\t", popSummary.numSymptomaticTriggeredVisits);
    fprintf(statsFile, "\n\t# Appointments Attended Triggered by Mild Illness\t%d\t", popSummary.numMildIllnessTriggeredVisits);
    fprintf(statsFile, "\n\t\tTotal\tPrimary\tNonprimary\tUnknown - Recent\tUnknown - Long Ago\t");
    fprintf(statsFile, "\n\tDiagnosed Maternal CMV Infections\t%d\t%d\t%d\t%d\t%d", maternalSummary.numDiagnosedCMV, maternalSummary.numDiagnosedByInfectionStatus[SimContext::PRIMARY], 
        maternalSummary.numDiagnosedByInfectionStatus[SimContext::SECONDARY], maternalSummary.numDiagnosedByInfectionStatus[SimContext::RECENT_UNKNOWN], maternalSummary.numDiagnosedByInfectionStatus[SimContext::LONG_AGO_UNKNOWN]);
    fprintf(statsFile, "\n\tTrue Positive Diagnosed Maternal CMV\t%d\t%d\t%d\t%d\t%d\t", maternalSummary.numTruePositiveDiagnosedCMV, maternalSummary.numTruePositiveDiagnosedByInfectionStatus[SimContext::PRIMARY],
        maternalSummary.numTruePositiveDiagnosedByInfectionStatus[SimContext::SECONDARY], maternalSummary.numTruePositiveDiagnosedByInfectionStatus[SimContext::RECENT_UNKNOWN], maternalSummary.numTruePositiveDiagnosedByInfectionStatus[SimContext::LONG_AGO_UNKNOWN]);
    fprintf(statsFile, "\n\tFalse Positive Diagnosed Maternal CMV\t%d\t%d\t%d\t%d\t%d\t", maternalSummary.numFalsePositiveDiagnosedCMV, maternalSummary.numFalsePositiveDiagnosedByInfectionStatus[SimContext::PRIMARY], 
        maternalSummary.numFalsePositiveDiagnosedByInfectionStatus[SimContext::SECONDARY], maternalSummary.numFalsePositiveDiagnosedByInfectionStatus[SimContext::RECENT_UNKNOWN], maternalSummary.numFalsePositiveDiagnosedByInfectionStatus[SimContext::LONG_AGO_UNKNOWN]);
    fprintf(statsFile, "\n\tDiagnosed Fetal CMV Infections\t%d", childSummary.numDiagnosedCMV);

    fprintf(statsFile, "\n\n\tOutcome\tTotal");//\tStd Dev\tLB\tUB");
    //fprintf(statsFile, "\n\tWeek of Maternal CMV Infection\t%lf\t%lf\t%lf\t%lf", 0,0,0,0);
    //fprintf(statsFile, "\n\tWeek of Fetal CMV Infection\t%lf\t%lf\t%lf\t%lf", 0,0,0,0);
    fprintf(statsFile, "\n\t\tRUS\tDUS\tAMN\tPCR\tIGM\tIGG\tAVD\t");
    fprintf(statsFile, "\n\tTotal # Performed");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        fprintf(statsFile, "\t%d", popSummary.totalNumPrenatalTest[i]);
    }
    fprintf(statsFile, "\n\tTotal True Positives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        fprintf(statsFile, "\t%d", popSummary.totalNumPrenatalTestTruePositives[i]);
    }
    fprintf(statsFile, "\n\tTotal True Negatives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        fprintf(statsFile, "\t%d", popSummary.totalNumPrenatalTestTrueNegatives[i]);
    }
    fprintf(statsFile, "\n\tTotal False Positives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        fprintf(statsFile, "\t%d", popSummary.totalNumPrenatalTestFalsePositives[i]);
    }
    fprintf(statsFile, "\n\tTotal False Negatives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        fprintf(statsFile, "\t%d", popSummary.totalNumPrenatalTestFalseNegatives[i]);
    }
    fprintf(statsFile, "\n\tProportion True Positives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        if (popSummary.totalNumPrenatalTest[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.totalNumPrenatalTestTruePositives[i]/(double)popSummary.totalNumPrenatalTest[i]);
        }
        else if (popSummary.totalNumPrenatalTestTruePositives[i] == 0) {
            fprintf(statsFile, "\t0");
        }
        else {
            fprintf(statsFile, "\tZeroDivError");
        }
    }
    fprintf(statsFile, "\n\tProportion True Negatives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        if (popSummary.totalNumPrenatalTest[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.totalNumPrenatalTestTrueNegatives[i]/(double)popSummary.totalNumPrenatalTest[i]);
        }
        else if (popSummary.totalNumPrenatalTestTrueNegatives[i] == 0) {
            fprintf(statsFile, "\t0");
        }
        else {
            fprintf(statsFile, "\tZeroDivError");
        }
    }
    fprintf(statsFile, "\n\tProportion False Positives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        if (popSummary.totalNumPrenatalTest[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.totalNumPrenatalTestFalsePositives[i]/(double)popSummary.totalNumPrenatalTest[i]);
        }
        else if (popSummary.totalNumPrenatalTestFalsePositives[i] == 0) {
            fprintf(statsFile, "\t0");
        }
        else {
            fprintf(statsFile, "\tZeroDivError");
        }
    }
    fprintf(statsFile, "\n\tProportion False Negatives");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        if (popSummary.totalNumPrenatalTest[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.totalNumPrenatalTestFalseNegatives[i]/(double)popSummary.totalNumPrenatalTest[i]);
        }
        else if (popSummary.totalNumPrenatalTestFalseNegatives[i] == 0) {
            fprintf(statsFile, "\t0");
        }
        else {
            fprintf(statsFile, "\tZeroDivError");
        }
    }
    fprintf(statsFile, "\n\tSensitivity");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        if (popSummary.totalNumPrenatalTestTruePositives[i]+popSummary.totalNumPrenatalTestFalseNegatives[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.totalNumPrenatalTestTruePositives[i]/((double)(popSummary.totalNumPrenatalTestTruePositives[i]+popSummary.totalNumPrenatalTestFalseNegatives[i])));
        }
        else if (popSummary.totalNumPrenatalTestTruePositives[i] == 0) {
            fprintf(statsFile, "\t0");
        }
        else {
            fprintf(statsFile, "\tZeroDivError");
        }
        
    }
    fprintf(statsFile, "\n\tSpecificity");
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        if (popSummary.totalNumPrenatalTestTrueNegatives[i]+popSummary.totalNumPrenatalTestFalsePositives[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.totalNumPrenatalTestTrueNegatives[i]/((double)(popSummary.totalNumPrenatalTestTrueNegatives[i]+popSummary.totalNumPrenatalTestFalsePositives[i])));
        }
        else if (popSummary.totalNumPrenatalTestTrueNegatives[i] == 0) {
            fprintf(statsFile, "\t0");
        }
        else {
            fprintf(statsFile, "\tZeroDivError");
        }
        
    }
    fprintf(statsFile, "\n\tVertical Transmission");
    fprintf(statsFile, "\n\t\t1st Trimester\t2nd Trimester\t3rd Trimester");
    fprintf(statsFile, "\n\tVT from Primary");
    for (int i = 1; i < SimContext::NUM_TRIMESTERS; i++) {
        fprintf(statsFile, "\t%d", popSummary.numPrimaryVTByTrimester[i]);
    }
    fprintf(statsFile, "\n\tVT from Secondary");
    for (int i = 1; i < SimContext::NUM_TRIMESTERS; i++) {
        fprintf(statsFile, "\t%d", popSummary.numSecondaryVTByTrimester[i]);
    }
    fprintf(statsFile, "\n\tPrimary Maternal Infection by Trimester");
    for (int i = 1; i < SimContext::NUM_TRIMESTERS; i++) {
        fprintf(statsFile, "\t%d", popSummary.numMaternalPrimaryInfectionInTrimester[i]);
    }
    
    fprintf(statsFile, "\n\tSecondary Maternal Infection by Trimester");
    for (int i = 1; i < SimContext::NUM_TRIMESTERS; i++) {
        fprintf(statsFile, "\t%d", popSummary.numMaternalSecondaryInfectionInTrimester[i]);
    }

    fprintf(statsFile, "\n\tProportion VT from Primary");
    for (int i = 1; i < SimContext::NUM_TRIMESTERS; i++) {
        if (popSummary.numMaternalPrimaryInfectionInTrimester[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.numPrimaryVTByTrimester[i]/popSummary.numMaternalPrimaryInfectionInTrimester[i]);
            
        }
        else {
            if (popSummary.numPrimaryVTByTrimester[i] == 0) {
                fprintf(statsFile, "\t0");
            }
            else {
                fprintf(statsFile, "\tZero Div Error");
            }
        }
    }
    fprintf(statsFile, "\n\tProportion VT from Secondary");
    for (int i = 1; i < SimContext::NUM_TRIMESTERS; i++) {
        if (popSummary.numMaternalSecondaryInfectionInTrimester[i] > 0) {
            fprintf(statsFile, "\t%lf", (double)popSummary.numSecondaryVTByTrimester[i]/(double)popSummary.numMaternalSecondaryInfectionInTrimester[i]);
        }
        else {
            if (popSummary.numSecondaryVTByTrimester[i] == 0) {
                fprintf(statsFile, "\t0");
            }
            else {
                fprintf(statsFile, "\tZero Div Error");
            }
            
        }
    }
    // clinically relevant proportion of VT from PI and NPI
    fprintf(statsFile, "\n\tTotal Clinically Relevant Proportion VT from Primary");
    if (maternalSummary.numPrimaryInfectionsAbleToVT > 0) {
        fprintf(statsFile, "\t%lf", (double)popSummary.totalNumVT[SimContext::PRIMARY]/(double)maternalSummary.numPrimaryInfectionsAbleToVT);
    }
    else {
        fprintf(statsFile, "\t0");
    }
    
    fprintf(statsFile, "\n\tTotal Clinically Relevant Proportion VT from Secondary");
    if (maternalSummary.numSecondaryInfectionsAbleToVT > 0) {
        fprintf(statsFile, "\t%lf", (double)popSummary.totalNumVT[SimContext::SECONDARY]/(double)maternalSummary.numSecondaryInfectionsAbleToVT);
    }
    else {
        fprintf(statsFile, "\t0");
    }

    // ALL total proportion of VT from PI and NPI (regardless of clinical relevance/ability to VT)
    fprintf(statsFile, "\n\tOverall Proportion VT from Primary (ALL)");
    if (maternalSummary.numPrimaryInfections > 0) {
        fprintf(statsFile, "\t%lf", (double)popSummary.totalNumVT[SimContext::PRIMARY]/(double)maternalSummary.numPrimaryInfections);
    }
    else {
        fprintf(statsFile, "\t0");
    }

    fprintf(statsFile, "\n\tOverall Proportion VT from Secondary (ALL)");
    if (maternalSummary.numSecondaryInfections > 0) {
        fprintf(statsFile, "\t%lf", (double)popSummary.totalNumVT[SimContext::SECONDARY]/(double)maternalSummary.numSecondaryInfections);
    }

    fprintf(statsFile, "\n\t\tType 1\tType 2\tType 3\tType 4\tType 5");

    fprintf(statsFile, "\n\tNumber CMV+ Births");
    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
        fprintf(statsFile, "\t%d", childSummary.numBornWithCMVByType[i]);
    }
    fprintf(statsFile, "\n\tNumber CMV+ Miscarriages/Stillbirths");
    for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
        fprintf(statsFile, "\t%d", childSummary.numMiscarriagesWithCMVByType[i]);
    }

    /** Overall treatment outputs */
    fprintf(statsFile, "\n\tNum moms who received any treatment\t%d\t", popSummary.totalNumEverOnAnyTreatment);
    fprintf(statsFile, "\n\t\tTreatment 1\tTreatment 2\tTreatment 3\t");
    fprintf(statsFile, "\n\tNum who received treatment\t%d\t%d\t%d\t", popSummary.totalNumTreatment[SimContext::TREATMENT_1], popSummary.totalNumTreatment[SimContext::TREATMENT_2], popSummary.totalNumTreatment[SimContext::TREATMENT_3]);
    fprintf(statsFile, "\n\tGave birth to cCMV+ child on treatment\t%d\t%d\t%d\t", popSummary.numMothersWithTreatmentBirthedCMVPositveChild[SimContext::TREATMENT_1],
        popSummary.numMothersWithTreatmentBirthedCMVPositveChild[SimContext::TREATMENT_2], popSummary.numMothersWithTreatmentBirthedCMVPositveChild[SimContext::TREATMENT_3]);
    fprintf(statsFile, "\n\tGave birth to non cCMV child on treatment\t%d\t%d\t%d\t", popSummary.numMothersWithTreatmentBirthedCMVNegativeChild[SimContext::TREATMENT_1],
        popSummary.numMothersWithTreatmentBirthedCMVNegativeChild[SimContext::TREATMENT_2], popSummary.numMothersWithTreatmentBirthedCMVNegativeChild[SimContext::TREATMENT_3]);
    fprintf(statsFile, "\n\tcCMV+ children born to moms who were on treatment\t%d\t", popSummary.numCMVPositiveChildrenBornWithTreatment);
    fprintf(statsFile, "\n\tcCMV+ children born to moms who were NEVER on treatment\t%d", popSummary. numCMVPositiveChildrenBornNeverTreatment);
}

void RunStats::writePrenatalTimeSummaries() {
    int j, k;
    const SimContext::RunSpecsInputs *runSpecs = simContext->getRunSpecsInputs();
    for (vector<PrenatalTimeSummary *>::iterator t = prenatalTimeSummaries.begin(); t != prenatalTimeSummaries.end(); t++) {
        PrenatalTimeSummary *currTime = *t;

        fprintf(statsFile, "\nCOHORT SUMMARY FOR WEEK %d", currTime->timePeriod);
        fprintf(statsFile, "\n\tNum Alive Fetuses\t%d", currTime->numAliveFetuses);
        fprintf(statsFile, "\n\tNum Vertical Transmissions\t%d", currTime->numVerticalTransmissions);
        //fprintf(statsFile, "\n\t# Diagnosed\tMothers\tChildren");
        //fprintf(statsFile, "\n\t\t%d\t%d", currTime->numDiagnosedMaternalCMV, currTime->numDiagnosedFetalCMV);
        fprintf(statsFile, "\n\t\tTotal\tPrimary\tNonprimary\tUnknown - Recent\tUnknown - Long Ago\t");
        fprintf(statsFile, "\n\tDiagnosed Maternal CMV Infections\t%d\t%d\t%d\t%d\t%d", currTime->numDiagnosedMaternalCMV, currTime->numDiagnosedMaternalCMVType[SimContext::PRIMARY],
            currTime->numDiagnosedMaternalCMVType[SimContext::SECONDARY], currTime->numDiagnosedMaternalCMVType[SimContext::RECENT_UNKNOWN], currTime->numDiagnosedMaternalCMVType[SimContext::LONG_AGO_UNKNOWN]);
        fprintf(statsFile, "\n\tDiagnosed Fetal CMV Infections\t%d", currTime->numDiagnosedFetalCMV);
        fprintf(statsFile, "\n\tLive Births\t%d", currTime->numLiveBirths);
        fprintf(statsFile, "\n\t# Appointments attended this week\t%d", currTime->numAttendedPrenatalAppointments);
        fprintf(statsFile, "\n\t# Appointments attended triggered by symptomatic maternal CMV this week\t%d", currTime->numSymptomaticTriggeredAppointmentsAttended);
        fprintf(statsFile, "\n\t# Appointments attended triggered by mild illness this week\t%d", currTime->numMildIllnessTriggeredAppointmentsAttended);
        fprintf(statsFile, "\n\tCMV+ Births By Phenotype\tType 1\tType 2\tType 3\tType 4\tType 5");
        fprintf(statsFile, "\n\t");
        for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
            fprintf(statsFile, "\t%d", currTime->numCMVBirthsByPhenotype[i]);
        }
        fprintf(statsFile, "\n\t#Fetal Deaths\t%d", currTime->numFetalDeaths);
        fprintf(statsFile, "\n\t#CMV Fetal Deaths By Phenotype\tType 1\tType 2\tType 3\tType 4\tType 5");
        fprintf(statsFile, "\n\t");
        for (int i = 0; i < SimContext::NUM_CMV_PHENOTYPES; i++) {
            fprintf(statsFile, "\t%d", currTime->numCMVFetalDeathsByPhenotype[i]);
        }

        fprintf(statsFile, "\n\tOn Treatment\tTreatment 1\tTreatment 2\tTreatment 3");
        fprintf(statsFile, "\n\tMothers on Treatment (ALL)");
        for (int i = 0; i < SimContext::NUM_TREATMENTS; i++) {
            fprintf(statsFile, "\t%d", currTime->numOnTreatment[i]);
        }
        fprintf(statsFile, "\n\tCMV+ Mothers on Treatment");
        for (int i = 0; i < SimContext::NUM_TREATMENTS; i++) {
            fprintf(statsFile, "\t%d", currTime->numCMVMothersOnTreatment[i]);
        }
        fprintf(statsFile, "\n\tCMV+ Children on Treatment");
        for (int i = 0; i < SimContext::NUM_TREATMENTS; i++) {
            fprintf(statsFile, "\t%d", currTime->numCMVFetusesOnTreatment[i]);
        }
        /*
        fprintf(statsFile, "\n\tCare State\t#\tNum Mild Illness\tMild Illness Triggered Visit\tNum Symptomatic CMV\tSymptomatic CMV Triggered Visit");
        fprintf(statsFile, "\n\tMaternal CMV-\t%d\t%d\t%lf\t-\t-");
        fprintf(statsFile, "\n\tMaternal CMV+ untreated\t%d\t%d\t%lf\t%d\t%lf");
        fprintf(statsFile, "\n\tMaternal CMV+ treatment 1\t%d\t%d\t%lf\t%d\t%lf");
        fprintf(statsFile, "\n\tMaternal CMV+ treatment 2\t%d\t%d\t%lf\t%d\t%lf");
        fprintf(statsFile, "\n\tMaternal CMV+ treatment 3\t%d\t%d\t%lf\t%d\t%lf");

        fprintf(statsFile, "\n\n\tChild State\t#\tDeaths");
        fprintf(statsFile, "\n\tChild CMV-\t%d\t%d");
        fprintf(statsFile, "\n\tChild CMV+ no treatment\t%d\t%d");
        fprintf(statsFile, "\n\tChild CMV+ treatment 1\t%d\t%d");
        fprintf(statsFile, "\n\tChild CMV+ treatment 2\t%d\t%d");
        fprintf(statsFile, "\n\tChild CMV+ treatment 3\t%d\t%d");

        fprintf(statsFile, "\n\n\tMaternal CMV+ and Child CMV-\t%d");
        fprintf(statsFile, "\n\n\tPrenatal Appointments Scheduled\t%d");
        fprintf(statsFile, "\n\tProportion of Appointments Attended\t%lf");
        fprintf(statsFile, "\n\tNum Live Births\t%d");
        fprintf(statsFile, "\n\tNum Miscarriages\t%d");
        fprintf(statsFile, "\n\t\tType 1\tType 2\tType 3\tType 4\tType 5");
        fprintf(statsFile, "\n\tNum CMV+ Births\t%d\t%d\t%d\t%d\t%d");
        fprintf(statsFile, "\n\tNum CMV+ Miscarriages\t%d\t%d\t%d\t%d\t%d");
        */

        // Tests performed statistics - ADD THE ACTUAL OUTPUTS TO END OF FPRINTF FUNCTIONS
        fprintf(statsFile, "\n\n\t\tRUS\tDUS\tAMN\tPCR\tIGM\tIGG\tAVD");
        fprintf(statsFile, "\n\tTests Performed\t%d\t%d\t%d\t%d\t%d\t%d\t%d", currTime->numTestPerfomed[SimContext::ROUTINE_US], currTime->numTestPerfomed[SimContext::DETAILED_US], currTime->numTestPerfomed[SimContext::AMNIOCENTESIS], currTime->numTestPerfomed[SimContext::MATERNAL_PCR],
            currTime->numTestPerfomed[SimContext::IGM], currTime->numTestPerfomed[SimContext::IGG], currTime->numTestPerfomed[SimContext::IGG_AVIDITY]);
        fprintf(statsFile, "\n\tTrue Positives\t%d\t%d\t%d\t%d\t%d\t%d\t%d", currTime->numTruePositives[SimContext::ROUTINE_US], currTime->numTruePositives[SimContext::DETAILED_US], currTime->numTruePositives[SimContext::AMNIOCENTESIS], currTime->numTruePositives[SimContext::MATERNAL_PCR],
            currTime->numTruePositives[SimContext::IGM], currTime->numTruePositives[SimContext::IGG], currTime->numTruePositives[SimContext::IGG_AVIDITY]);
        fprintf(statsFile, "\n\tTrue Negatives\t%d\t%d\t%d\t%d\t%d\t%d\t%d", currTime->numTrueNegatives[SimContext::ROUTINE_US], currTime->numTrueNegatives[SimContext::DETAILED_US], currTime->numTrueNegatives[SimContext::AMNIOCENTESIS], currTime->numTrueNegatives[SimContext::MATERNAL_PCR],
            currTime->numTrueNegatives[SimContext::IGM], currTime->numTrueNegatives[SimContext::IGG], currTime->numTrueNegatives[SimContext::IGG_AVIDITY]);
        fprintf(statsFile, "\n\tFalse Positives\t%d\t%d\t%d\t%d\t%d\t%d\t%d", currTime->numFalsePositives[SimContext::ROUTINE_US], currTime->numFalsePositives[SimContext::DETAILED_US], currTime->numFalsePositives[SimContext::AMNIOCENTESIS], currTime->numFalsePositives[SimContext::MATERNAL_PCR],
            currTime->numFalsePositives[SimContext::IGM], currTime->numFalsePositives[SimContext::IGG], currTime->numFalsePositives[SimContext::IGG_AVIDITY]);
        fprintf(statsFile, "\n\tFalse Negatives\t%d\t%d\t%d\t%d\t%d\t%d\t%d", currTime->numFalseNegatives[SimContext::ROUTINE_US], currTime->numFalseNegatives[SimContext::DETAILED_US], currTime->numFalseNegatives[SimContext::AMNIOCENTESIS], currTime->numFalseNegatives[SimContext::MATERNAL_PCR],
            currTime->numFalseNegatives[SimContext::IGM], currTime->numFalseNegatives[SimContext::IGG], currTime->numFalseNegatives[SimContext::IGG_AVIDITY]);
    }
}