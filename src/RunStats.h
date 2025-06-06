#pragma once

#include "include.h"

class RunStats
{
public:
    /** Make the StateUpdater class a friend class so it can modify the private data */
    friend class StateUpdater;
    
    /* Constructors and Destructor */
    RunStats(string runName, SimContext *simContext);
    ~RunStats(void);

    class PatientSummary {
    public:
        /** Total costs accrued by patient (entire pregnancy)*/
        double costs;
        /** Total life weeks lived by the mother*/
        double maternalLMs;
        /** Total quality adjusted life weks lived by the mother */
        double maternalQALMs;
    };

    class PopulationSummary {
    public:
        // Basic run and set information
        /** The name of the run set this summary belongs to */
        string runSetName;
        /** The name of the run corresponding ot this summary */
        string runName;
        /** The date of the run */
        string runDate;
        /** The time the run finished */
        string runTime;
        // Number of patient and clinic visit aggregates 
        /** The number of mother/child(ren) pairs in this cohort */
        int numCohorts;
        /** Total clinic visits during pregnancy in this run */
        int totalClinicVisitsPregnancy;
        /** Number of symptomatic-CMV-triggered clinic visits */
        int numSymptomaticTriggeredVisits;
        /** Number of mild ilness-triggered clinic visits */
        int numMildIllnessTriggeredVisits;
        /** Total number of patients (mother-child pair) that had treatment administered during pregnancy */
        int totalNumEverOnAnyTreatment;
        /** Total number of each type of treatment administered */
        int totalNumTreatment[SimContext::NUM_TREATMENTS];
        /** Total number of person-weeks on each treatment */
        int totalWeeksOnEachTreatment[SimContext::NUM_TREATMENTS];
        /** Number of mothers on treatment at any point who gave birth to cCMV+ child*/
        int numMothersWithTreatmentBirthedCMVPositveChild[SimContext::NUM_TREATMENTS];
        /** Number of mothers who gave birth to NON cCMV child WHILE ON TREATMENT */
        int numMothersWithTreatmentBirthedCMVNegativeChild[SimContext::NUM_TREATMENTS];
        /** Number of cCMV+ children born to moms who were ON TREATMENT */
        int numCMVPositiveChildrenBornWithTreatment;
        /** Number of cCMV+ children born to moms who were NEVER on treatment */
        int numCMVPositiveChildrenBornNeverTreatment;

        /** Total number of each type of prenatal test performed */
        int totalNumPrenatalTest[SimContext::NUM_PRENATAL_TESTS];
        /** Total number of true positives for each test performed */
        int totalNumPrenatalTestTruePositives[SimContext::NUM_PRENATAL_TESTS];
        /** Total number of true negatives for each test performed */
        int totalNumPrenatalTestTrueNegatives[SimContext::NUM_PRENATAL_TESTS];
        /** Total number of false positives for each test performed */
        int totalNumPrenatalTestFalsePositives[SimContext::NUM_PRENATAL_TESTS];
        /** Total number of false negatives for each test performed */
        int totalNumPrenatalTestFalseNegatives[SimContext::NUM_PRENATAL_TESTS];
        /** Proportions of true positives, true negatives, false positives, and false negatives */
        int proportionTruePositives[SimContext::NUM_PRENATAL_TESTS];
        int proportionTrueNegatives[SimContext::NUM_PRENATAL_TESTS];
        int proportionFalsePositives[SimContext::NUM_PRENATAL_TESTS];
        int proportionFalseNegatives[SimContext::NUM_PRENATAL_TESTS];
        /** Number of mothers who got infected with CMV during each trimester  */
        int numMaternalPrimaryInfectionInTrimester[SimContext::NUM_TRIMESTERS]; // stratified by trimester
        int numMaternalSecondaryInfectionInTrimester[SimContext::NUM_TRIMESTERS]; // stratified by trimester
        /** Number of vertical transmissions (primary or nonprimary) */
        int numPrimaryVTByTrimester[SimContext::NUM_TRIMESTERS]; // stratified by trimester
        int numSecondaryVTByTrimester[SimContext::NUM_TRIMESTERS];
        int totalNumVT[3]; // stratified by none, primary, and nonprimary. "none" is not technically applicable but still leaving it in for enumeration consistency.
        /**  */
    };

    /** MaternalCohortSummary holds summary aggregate statistics and other misc information for the mothers */
    class MaternalCohortSummary {
    public:
        /* Total Number of Maternal CMV Infections */
        int numCMVInfections;
        /* Total Number of PRIMARY Maternal CMV Infections */
        int numPrimaryInfections;
        /* Total Number of NONPRIMARY Maternal CMV Infections */
        int numSecondaryInfections;
        /* Total Number of PRIMARY Maternal CMV Infections that could have been vertically transmitted */
        int numPrimaryInfectionsAbleToVT;
        /* Total Number of NONPRIMARY Maternal CMV Infections that could have been vertically transmitted */
        int numSecondaryInfectionsAbleToVT;
        /* Total number of Maternal CMV Infections by trimester */
        int numCMVInfectionsByTrimester[SimContext::NUM_TRIMESTERS];
        /** Total number of Mild Illness */
        int numMildIllness;
        /** Proportion (of entire population) of CMV positive mothers with mild illness */
        int numCMVMildIllness;
        /** Proportion (of entire population) of CMV positive mothers without mild illness */
        int numCMVNoMildIllness;
        /** Proportion (of entire population) of CMV negative mothers who had a miscarriage */
        int numMiscarriageNoCMV;
        /** Proportion (of entire population) of mothers who were CMV positive and had a miscarriage */
        int numMiscarriageWithCMV;
        /** Number of mothers who gave birth while CMV Positive*/
        int numBirthWithCMV;
        /** Number of mothers who gave birth without CMV */
        int numBirthNoCMV;
        /** Number of mothers with diagnosed CMV or diagnosed previous CMV*/
        int numDiagnosedCMV;
        /** Number of mothers diagnosed with CMV from either PI, NPI, or unknown PI or NPI infection */
        int numDiagnosedByInfectionStatus[SimContext::INFECTION_TYPE_MAX];
        /** Number of mothers with TRUE POSITIVE diagnosed CMV or diagnosed previous CMV */
        int numTruePositiveDiagnosedCMV;
        /** Number of mothers TRUE POSITIVELY diagnosed with CMV from either PI, NPI, or unknown PI or NPI infection */
        int numTruePositiveDiagnosedByInfectionStatus[SimContext::INFECTION_TYPE_MAX];
        /** Number of mothers with FALSE POSITIVE diagnosed CMV or diagnosed previous CMV */
        int numFalsePositiveDiagnosedCMV;
        /** Number of mothers FALSE POSITIVELY diagnosed with CMV from either PI, NPI, or unknown PI or NPI infection */
        int numFalsePositiveDiagnosedByInfectionStatus[SimContext::INFECTION_TYPE_MAX];
    };

    /** ChildCohortSummary holds summary aggregate statistics and other misc information for the pediatric patients */
    class ChildCohortSummary {
    public:
        /** Total number of CMV infections */
        int numCMVInfections;
        /** Total number of CMV infections by trimester (timing of VT) */
        int numCMVInfectionsByTrimester[SimContext::NUM_TRIMESTERS];
        /** Number of CMV infections from primary maternal infection */
        int numCMVInfectionsFromPrimaryByTrimester[SimContext::NUM_TRIMESTERS];
        /** Number of CMV infections from non-primary maternal infection */
        int numCMVInfectionsFromSecondaryByTrimester[SimContext::NUM_TRIMESTERS];
        /** Total number of Mild Illness */
        int numMildIllness;
        /** Proportion (of all children) with CMV of each type */
        int numWithCMVByType[SimContext::NUM_CMV_PHENOTYPES];
        /** Sum of week of birth for CMV (-) children (for calculating average)*/
        int sumWeekBirthNoCMV;
        /** Sum of week of birth for CMV (+) children (for calculating average) */
        int sumWeekBirthWithCMV;
        /** Average week of birth for CMV (-) children */
        double averageWeekBirthNoCMV;
        /** Average week of birth for CMV (+) children */
        double averageWeekBirthCMV;
        /** Number of miscarriages with fetal CMV */
        int numMiscarriagesWithCMV;
        /** Number of miscarriages without fetal CMV */
        int numMiscarriagesNoCMV;
        /** Number of children birthed with CMV */
        int numBirthWithCMV;
        /** Number of children born with CMV of each type  */
        int numBornWithCMVByType[SimContext::NUM_CMV_PHENOTYPES];
        /** Number of children birthed without CMV */
        int numBirthNoCMV;
        /** Number of children diganosed with CMV */
        int numDiagnosedCMV;
        /** Number of miscarriages/stillbirths with CMV by type */
        int numMiscarriagesWithCMVByType[SimContext::NUM_CMV_PHENOTYPES];
        /** Number of deaths at/during birth (with or without CMV)*/
        int numFetalDeathsAtBirth[2];
    };

    /** MaternalDeathStats contains the statistics and distributions of causes of deaths for mothers */
    class MaternalDeathStats {
    public:
        //
    };

    /** ChildDeathStats contains the statistics and distributions of causes of deaths for pediatrics */
    class ChildDeathStats {
    public:
        /** Total number of fetal deaths */
        int fetalDeaths;
        /** Total number of fetal deaths by CMV Phenotype */
        int fetalDeathsByType[SimContext::NUM_CMV_PHENOTYPES];
        /** Total number of fetal deaths with CMV */
        int fetalDeathsCMV;
    };

    /** TimeSummary class contains weekly/monthly longitudinal stats */
    class PrenatalTimeSummary {
    public:
        /** Time period week */
        int timePeriod;
        /** Number of Active Maternal CMV Infections */
        int activeMaternalCMV;
        /** Number of Active Primary Maternal CMV Infections */
        int activePrimaryMaternalCMV;
        /** Number of mothers who have had CMV */
        int activeSecondaryMaternalCMV;
        /** Number of Fetal CMV Infections by phenotype */
        int numFetalCMVInfections[SimContext::NUM_CMV_PHENOTYPES];
        /** Number of symptomatic CMV triggered appointments attended this week (NOT BEING OUTPUTTED CURRENTLY) */
        int numSymptomaticTriggeredAppointmentsAttended;
        /** Number of mild illness triggered appointments attended this week (NOT BEING OUTPUTTED CURRENTLY) */
        int numMildIllnessTriggeredAppointmentsAttended;
        /** Diagnosed Maternal CMV Infections this week */
        int numDiagnosedMaternalCMV;
        /** Diagnosed Maternal CMV Infections this week stratified by infection type (PI, NPI, Recent Unknown, Long Ago Unknown) */
        int numDiagnosedMaternalCMVType[SimContext::INFECTION_TYPE_MAX];
        /** Diagnosed Maternal CMV Infections total (NOT HERE) */
        /** Diagnosed Fetal CMV Infections by phenotype this week */
        int numDiagnosedFetalCMV;
        /** Diagnosed Fetal CMV Infections by phenotype total (NOT HERE) */
        /** Number of Fetuses still alive (NOT including ones already born) */
        int numAliveFetuses;
        /** Number of Fetuses/Newborns still alive (including ones already born) */
        int numAliveChildren;
        /** Total number of Live Births so far in the simulation */
        int totalNumLiveBirths;
        /** Number of Live Births this week */
        int numLiveBirths;
        /** Number of CMV+ Births by phenotype */
        int numCMVBirthsByPhenotype[SimContext::NUM_CMV_PHENOTYPES];
        /** Number of Fetal Deaths */
        int numFetalDeaths;
        /** Number of Deaths CMV+ by phenotype */
        int numCMVFetalDeathsByPhenotype[SimContext::NUM_CMV_PHENOTYPES];

        

        /* ------------------------------------------------------- */

        /** number of CMV- mothers */
        int maternalNoCMV;
        /** number of untreated CMV+ mothers */
        int maternalCMVNoTreatment;
        /** number of CMV+ mothers on treatments */
        int maternalCMVOnTreatment[SimContext::NUM_TREATMENTS];
        /** number of CMV- mothers on treatments */
        int maternalNoCMVOnTreatment[SimContext::NUM_TREATMENTS];
        /** Number of CMV- fetuses */
        int fetalNoCMV;
        /** Number of mother-child pairs that have maternal CMV but not fetal CMV */
        int maternalCMVNoFetalCMV;
        /* Treatment Outputs */
        /** Number of mother-child pairs on each treatment type  */
        int numOnTreatment[SimContext::NUM_TREATMENTS];
        int numCMVMothersOnTreatment[SimContext::NUM_TREATMENTS];
        int numCMVFetusesOnTreatment[SimContext::NUM_TREATMENTS];
        /** Number of CMV+ fetuses on each treatment */
        int fetalCMVOnTreatment[SimContext::NUM_TREATMENTS];
        /** Number of CMV- fetuses on each treatment */
        int fetalNoCMVOnTreatment[SimContext::NUM_TREATMENTS];
        /** Number of miscarriages child no cmv */
        int numMiscarriagesNoFetalCMV;
        /** Number of miscarriages when child has CMV*/
        int numMiscarriagesFetalCMV;
        /** Number of miscarriages when child has CMV but untreated */
        int numMiscarriagesFetalCMVNoTreatment;
        /** Number of miscarriages when child has CMV and treted */
        int numMiscarriagesFetalCMVTreatment[SimContext::NUM_TREATMENTS];
        /** Number of prenatal appointments scheduled */
        int numScheduledPrenatalAppointments;
        /** Number of scheduled prenatal appointments attended */
        int numAttendedPrenatalAppointments;
        /** Number of each prenatal test performed */
        int numTestPerfomed[SimContext::NUM_PRENATAL_TESTS];
        /** Number of true positives for prenatal tests */
        int numTruePositives[SimContext::NUM_PRENATAL_TESTS];
        /** Numberof True negatives for prenatal tests */
        int numTrueNegatives[SimContext::NUM_PRENATAL_TESTS];
        /** Number of False positives for prenatal tests */
        int numFalsePositives[SimContext::NUM_PRENATAL_TESTS];
        /** Number of false negatives for prenatal tests */
        int numFalseNegatives[SimContext::NUM_PRENATAL_TESTS];
        /** Number of true positives for those with Fetal CMV stratified by CMV phenotype */
        int numTruePositivesFetalCMV[SimContext::NUM_PRENATAL_TESTS][SimContext::NUM_CMV_PHENOTYPES];
        /** Number of false negatives for those with Fetal CMV stratified by CMV phenotype */
        int numFalseNegativesFetalCMV[SimContext::NUM_PRENATAL_TESTS][SimContext::NUM_CMV_PHENOTYPES];
        /** Number of new maternal CMV infections */
        int numNewMaternalCMVInfections;
        /** Number of new fetal CMV infections */
        int numNewFetalCMVInfections;
        /** Number of mothers who have ild illness this week */
        int numMaternalMildIllness;
        /** Number of new maternal mild illness this week */
        int numNewMaternalMildIllness;
        /** Number of mothers who no longer have mild illness this week (but did last week) */
        int lostMaternalMildIllness;
        /** Number of vertical transmissions that happen this week */
        int numVerticalTransmissions;
        /** Total number of vertical transmissions that have occured thus far */
        int numTotalVerticalTransmissions;
        /** CMV negative births this week */
        int birthsCMVNegative;
        /** CMV positive births this week */
        int birthsCMVPositive;
        /** Number of new fetal infections this week by phenotype*/
        int newFetalInfectionsByPhenotype[SimContext::NUM_CMV_PHENOTYPES];
        
    };
    /** MaternalTimeSummary class contains maternal weekly/monthly longitudinal stats */
	class MaternalTimeSummary {
    public:
        int numInfections;
        /** Number of new CMV infections this week */
        int newInfections;
        /** Number of patients no longer infected this week (but were last week )*/
        int lostInfections;
        /** Number of mothers who are symptomatic this week */
        int numSymptomatic;
        /** Number of new symptomatic infections this week */
        int newSymptomatic;
        /** Number of patients no longer symptomatic this week (but were last week)*/
        int lostSymptomatic;
        /** Number of mothers who have mild illness this week */
        int numMildIllness;
        /** Number of new mild illness this week */
        int newMildIllness;
        /** Number of patients who no longer have mild illness this week (but did last week)*/
        int lostMildIllness;
        /** Total nmber of miscarriages for mothers without CMV infection (up to current week)*/
        int totalNumMiscarriagesNoCMV;
        /** Number of miscarriages for mothers without CMV infection this week */
        int numMiscarriagesNoCMV;
        /** Total nmber of miscarriages for mothers with CMV infection (up to current week)*/
        int totalNumMiscarriagesCMV;
        /** Number of miscarriages for mothers wiht CMV infection this week */
        int numMiscarriagesCMV;
        /** Total number of vertical transmissions (up to current week)*/
        int totalNumVerticalTransmissions;
        /** Number of vertical transmissions this week */
        int numVerticalTransmissions;
    };

    /* ChildTimeSummary class contains maternal weekly/monthly longitudinal stats */
    class ChildTimeSummary {
    public:
        /** Total Births CMV negative */
        int totalBirthsCMVNegative;
        /** CMV negative births this month */
        int birthsCMVNegative;
        /** Total Births CMV Positive */
        int totalBirthsCMVPositive;
        /** CMV positive births this month */
        int birthsCMVPositive;
        /** Number of CMV infections by phenotype */
        int numberInfectionsByPhenotype[SimContext::NUM_CMV_PHENOTYPES];
    };

    /* Accessor functions returning const pointers to the statistics subclass objects */
    const PopulationSummary *getPopulationSummary();
    const MaternalCohortSummary *getMaternalCohortSummary();
    const ChildCohortSummary *getChildCohortSummary();
    const MaternalDeathStats *getMaternalDeathStats();
    const ChildDeathStats *getChildDeathStats();
    const PrenatalTimeSummary *getPrenatalTimeSummary(unsigned int timePeriod);
    const MaternalTimeSummary *getMaternalTimeSummary(unsigned int timePeriod);
    const ChildTimeSummary *getChildTimeSummary(unsigned int timePeriod);

    /* Functions to calculate final aggregate statistics and to write out hte stats flie */
    void initRunStats();
    void finalizeStats();
    void writeStatsFile();

    /* Functions to increment or change run statistics */
    void incrementNumCMV();

private:
    /** Pointer to the associated simulation context */
    SimContext *simContext;
    /** Stats file name */
    string statsFileName;
    /** Stats file pointer */
    FILE *statsFile;

    /** Statistics subclass object */
    PopulationSummary popSummary;
    /** Statistics subclass object */
    MaternalCohortSummary maternalSummary;
    /** Statistics subclass object */
    ChildCohortSummary childSummary;
    /** Statistics subclass object */
    MaternalDeathStats maternalDeathStats;
    /** Statistics subclass object */
    ChildDeathStats childDeathStats;
    /** vector of PatientSummary objects for all cohorts in this context */
    vector<PatientSummary> patients;
    /** Vectors of MaternalTimeSummary and ChildTimeSummary objects for each week/month time period, 
     * use pointer to object since subclass is complex and copy would be expensive
    */
   vector<PrenatalTimeSummary * > prenatalTimeSummaries;
   vector<MaternalTimeSummary * > maternalTimeSummaries;
   vector<ChildTimeSummary *> childTimeSummaries;

    /* Initialization functions for statistics objects, called by constructor */
    void initPopulationSummary();
    void initMaternalCohortSummary();
    void initChildCohortSummary();
    void initMaternalDeathStats();
    void initChildDeathStats();
    void initPrenatalTimeSummary(PrenatalTimeSummary *currTime);
    void initMaternalTimeSummary(MaternalTimeSummary* maternalCurrStats);
    void initChildTimeSummary(ChildTimeSummary* childCurrStats);

    /* Functions to finalize aggregate staticstics before printing out */
    void finalizePopulationSummary();
    void finalizeMaternalCohortSummary();
    void finalizeChildCohortSummary();
    void finalizeMaternalDeathStats();
    void finalizeChildDeathStats();
    void finalizeMaternalTimeSummary();
    void finalizeChildTimeSummary();

    /* Functions to write out each subclass object to the statistics file, called by writeStatsFile */
    void writePopulationSummary();
    void writeMaternalCohortSummary();
    void writeChildCohortSummary();
    void writeMaternalDeathStats();
    void writeChildDeathStats();
    void writePrenatalTimeSummaries();
    void writeMaternalTimeSummary();
    void writeChildTimeSummary();

};

/** \brief getPopulationSumary returns a const pointer to the PopulationSummary statistics object */
inline const RunStats::PopulationSummary *RunStats::getPopulationSummary() {
    return &popSummary;
}
/** \brief getMaternalCohortSummary returns a const pointer to the MaternalCohortSummary statistics object */
inline const RunStats::MaternalCohortSummary *RunStats::getMaternalCohortSummary() {
    return &maternalSummary;
}
/** \brief getChildCohortSummary returns a const pointer to the ChildCohortSummary statistics object */
inline const RunStats::ChildCohortSummary *RunStats::getChildCohortSummary() {
    return &childSummary;
}
/** \brief getMaternalDeathStats returns a const pointer to the MaternalDeathStats statistics object */
inline const RunStats::MaternalDeathStats *RunStats::getMaternalDeathStats() {
    return &maternalDeathStats;
}
/** \brief getChildDeathStats returns a const pointer to the ChildDeathStats statistics object */
inline const RunStats::ChildDeathStats *RunStats::getChildDeathStats() {
    return &childDeathStats;
}

/** \brief getMaternalTimeSummary returns a const pointer to the specified MaternalTimeSummary object,
	returns null if one does not exist for this time period */
inline const RunStats::MaternalTimeSummary *RunStats::getMaternalTimeSummary(unsigned int timePeriod) {
    if (timePeriod < maternalTimeSummaries.size())
        return maternalTimeSummaries[timePeriod];
    return NULL;
}
/** \brief getMaternalTimeSummary returns a const pointer to the TimeSummary statistics object */
inline const RunStats::ChildTimeSummary *RunStats::getChildTimeSummary(unsigned int timePeriod) {
    if (timePeriod < childTimeSummaries.size())
        return childTimeSummaries[timePeriod];
    return NULL;
}
