#pragma once

#include "include.h"
//CORRECT ONE

class SimContext{
public:
    SimContext(string runFile);
    ~SimContext(void);

    /**
     * performInitialUpdates performs all the state and stats updates upon patient creation
    */

    int counter;

    /** The specific genders available*/
    enum BIOLOGICAL_SEX {MALE, FEMALE};
    /** Negative or Positive for CMV */
    enum CMV_NEGATIVE_OR_POSITIVE {CMV_NEGATIVE, CMV_POSITIVE};
    /** Last Got CMV Strata */
    enum LAST_GOT_CMV_STRATA {VERY_RECENT_CMV, MEDIUM_RECENT_CMV, LESS_RECENT_CMV, LONG_TIME_AGO_CMV, NEVER_CMV};
    /** Trimester enumeration */
    enum TRIMESTERS {PRECONCEPTION, TRIMESTER_1, TRIMESTER_2, TRIMESTER_3};
    /** Follow Up Tests */
    enum FOLLOW_UP_TESTS {FOLLOWUP_1, FOLLOWUP_2, FOLLOWUP_3};
    /** Primary or secondary infection biological truth */
    enum TRUE_PRIMARY_OR_SECONDARY {TRUE_NO_INFECTION, TRUE_PRIMARY, TRUE_SECONDARY};
    /** Primary or Secondary */
    enum PRIMARY_OR_SECONDARY {NONE, PRIMARY, SECONDARY, RECENT_UNKNOWN, LONG_AGO_UNKNOWN, INFECTION_TYPE_MAX = LONG_AGO_UNKNOWN + 1};
    /** Tests */
    enum PRENATAL_TESTS {ROUTINE_US, DETAILED_US, AMNIOCENTESIS, MATERNAL_PCR, IGM, IGG, IGG_AVIDITY};
    /** Treatment Policies */
    enum PRENATAL_TREATMENT_POLICIES {NO_CMV_DIAGNOSES, MATERNAL_CMV_DIAGNOSIS, FETAL_CMV_DIAGNOSIS};
    /** Treatment Numbers */
    enum TREATMENTS {TREATMENT_1, TREATMENT_2, TREATMENT_3};
    /** Asymptomatic/Symptomatic */
    enum ASYMPTOMATIC_OR_SYMPTOMATIC {ASYMPTOMATIC, SYMPTOMATIC};
    /** Prenatal Test Names */
    static const char *PRENATAL_TEST_NAMES[];
    /** Prenatal Test Type Names */
    static const char *PRENATAL_TEST_TYPE_NAMES[];
    /** Test Outcome (Negative/Positive) */
    static const char *TEST_OUTCOME[];
    /** IgG Avidity Test Outcome (None/Low/High) */
    static const char *AVIDITY_OUTCOME[];
    /** Primary or Secondary Strings */
    static const char *PRIMARY_OR_SECONDARY_CHAR[];
    /** Last got CMV strata char */
    static const char *LAST_GOT_CMV_STRATA_CHAR[];
    /** Test type: whether it's a normal test, confirmatory test, follow-up test, or special case test */
    enum PRENATAL_TEST_TYPE {PN_TEST_BASE, PN_TEST_CONF, PN_TEST_FOLLOWUP, PN_TEST_SPECIAL};
    /** Test follow up: inital test, first follow-up, or second follow-up */
    enum PRENATAL_TEST_FOLLOWUP {PN_NOT_FU, PN_FU_1, PN_FU_2, PN_FU_3, PN_FU_4};
    /** Confirmatory test number */
    enum PRENATAL_TEST_CONFIRMATORY {PN_NOT_CONF, PN_CONF_1, PN_CONF_2, PN_CONF_3, PN_CONF_4};
    /** Avidity levels */
    enum AVIDITY_LEVELS {HIGH_AVIDITY, LOW_AVIDITY, NO_AVIDITY};
    /** Test Results */
    enum TEST_RESULT {NORMAL, ABNORMAL};
    
    /* Prenatal Test Instance captures information about a particular instance of a prenatal test being administered.
     To initialize, you can do PrenatalTestInstance prenatalTestInstance = {testID, testAssay, testType, confirmatoryNumber, 
     testFollowingUpFrom, weekScheduled, weekToTest, testPerformed, result, weekToReturn, returnToPatient}.*/
    struct PrenatalTestInstance {
        // The ID number of the specific test instance struct created
        int testID;
        // The index of the test being used 
        int testAssay;
        /** Test type */
        int testType;
        /** Confirmatory Number (0 if base case or follow-up)*/
        int confirmatoryNumber;
        /** Test that this test is following up from (if any) */
        int testFollowingUpFrom;
        /** Week that the test was scheduled */
        int weekScheduled;
        /** Week that test is/will be conducted (relevant for follow-up tests)*/
        int weekToTest;
        /** Whether test has been performed */
        bool testPerformed;
        /** result of the test */
        int result;
        /** week to return the test */
        int weekToReturn;
        // Whether the result will be returned to the patient
        bool returnToPatient;
    };

    /* Prenatal Test class that captures information about a particular prenatal test being administered */
    class PrenatalTestState {
    public:
        PrenatalTestState(int testID);
        virtual ~PrenatalTestState(void);
        // The ID number of the specific test state object created
        int testID;
        
        int testAssay;
        /** Test type */
        int testType;
        /** Follow-Up number (0 if base case or confirmatory )*/
        int followUpNumber;
        /** Confirmatory Number (0 if base case or follow-up)*/
        int confirmatoryNumber;
        /** Test that this test is following up from (if any) */
        int testFollowingUpFrom;
        /** Week that the test was scheduled */
        int weekScheduled;
        /** Week that test is/will be conducted (relevant for follow-up tests)*/
        int weekToTest;
        /** result of the test */
        bool result;
        /** secondary result of the test (currently only applicable for maternal antibody (IgM/igG) test )*/
        bool secondaryResult;
        int weekToReturn;
        // Whether the result will be returned to the patient
        bool returnToPatient;
    };

    /** Number of possible ages in years that the patient could be */
    static const int AGE_YEARS = 101;
    /** Max number of years that could be lived by patient */
    static const int AGE_MAXIMUM = 100;
    /** Number of maternal age strata */
    static const int NUM_AGE_STRATA = 6;
    /** Number of maternal age strata bounds */
    static const int NUM_AGE_STRATA_BOUNDS = NUM_AGE_STRATA + 1;
    /** Youngest patient age possible (for life tables) */
    static const int AGE_STARTING = 0;
    /** Maximum number of background tests available within a week */
    static const int MAX_WEEKLY_BACKGROUND_TESTS = 5;
    /** Number of "Last Active CMV strata bounds" */
    static const int NUM_LAST_CMV_STRATA = 4;
    /** Number of time buckets for clinical visits during pregnancy */
    static const int NUM_PRENATAL_VISIT_TIME_BUCKETS = 3;
    /** Number of CMV Phenotypes */
    static const int NUM_CMV_PHENOTYPES = 5;
    /** Maximum number of months of pregnancy */
    static const int MAX_WEEKS_PREGNANT = 42;
    /** Number of trimesters -- Preconception - 0, First Trimester - 1, Second Trimeter - 2, Third Trimester - 3 */
    static const int NUM_TRIMESTERS = 4;
    /** Number of strata for custom age distribution */
    static const int NUM_CUSTOM_AGE_STRATA = 30;
    /** Number of Prenatal Tests */
    static const int NUM_PRENATAL_TESTS = 7;
    /** Number of indices for no none/true primary/true secondary infection */
    static const int NUM_INDX_TRUE_PRIMARY_SECONDARY_NONE = 3;
    /** Number of patients to be traced in the tracefile */
    static int numPatientsToTrace;
    /** Number of treatments available to use */
    static const int NUM_TREATMENTS = 3;
    /** Number of treatment policies */
    static const int NUM_TREATMENT_POLICIES = 3;

    class RunSpecsInputs {
    public:
        /* Shortcuts and Misc inputs */
        string runSetName;
        string runName;
        /** RunSpecs PUT LOC HERE */
        int numCohorts;
        
        /** RunSpecs D3 */
        double annualDiscountFactor;
        /** RunSpecs E3*/
        double weeklyDiscountFactor;
        /** RunSpecs F3*/
        double monthlyDiscountFactor;
        /** RunSpecs E6 */
        bool randomSeedByTime;
        /** RunSpecs C12:C15 */
        int lastActiveCMVStrataBounds[NUM_LAST_CMV_STRATA+1];
        /** RunSpecs K4 */
        int numberOfPatientsToTrace;
        /** RunSpecs PUT LOC HERE */
        string userProgramLocale;
        /** RunSpecs PUT LOC HERE */
        string inputVersion;
        /** RunSpecs PUT LOC HERE */
        string modelVersion;
        /** Week at which to switch to monthly timestep */
        int weekSwitchToMonth; // after the specified week, timestep switches to month. (eg if weekSwitchToMonth = 22, timestep switches to month after week 22 is over.)
    };

    class CohortInputs {
    public:
        /** CohortInputs C5 */
        double ageMonthsMean;
        /** CohortInputs C6 */
        double ageMonthsStdDev;
        /** Minimum Age Months */
        int minimumAgeMonths;
        /** Maximum Age Months */
        int maximumAgeMonths;
        /** Maternal Age Strata Bounds */
        int maternalAgeStrataBounds[NUM_AGE_STRATA_BOUNDS];
        /** CohortInputs C14:G18 */
        double lastGotActiveCMVByStrata[NUM_LAST_CMV_STRATA+1];
        /** CohortInputs C23:G25*/
        double weeklyProbBirth[AGE_YEARS][MAX_WEEKS_PREGNANT];
        /** CohortInputs Weekly Probability of Birth for those CMV positive [AGE][WEEKS PREGNANT] */
        double CMVPositiveWeeklyProbBirth[AGE_YEARS][MAX_WEEKS_PREGNANT];
        /** Prob Previous Maternal CMV Infection was primary */
        double probPrevCMVWasPrimary[NUM_LAST_CMV_STRATA];
        /** CohortInputs One-Time Probability of Vertical Transmission for mothers who had CMV infection that began before model start */
        double oneTimeVerticalTransmissionProb[NUM_INDX_TRUE_PRIMARY_SECONDARY_NONE][NUM_LAST_CMV_STRATA];
        /** CohortInputs One-Time Vertical Transmission Week of Risk */
        int oneTimeVerticalTansmissionWeek[NUM_INDX_TRUE_PRIMARY_SECONDARY_NONE][NUM_LAST_CMV_STRATA];
        /** CohortInputs D31:H31*/
        double probSymptomsWithPrimaryCMV[NUM_TRIMESTERS];
        /** CohortInputs D32:H32*/
        double probSymptomsWithSecondaryCMV[NUM_TRIMESTERS];
        /** CohortInputs D31:H32 -- probability of each CMV phenotype upon vertical transmission [PRIMARY/SECONDARY INFECTION][TRIMESTER][PHENOTYPE]*/
        double probPhenotypesUponVT[NUM_INDX_TRUE_PRIMARY_SECONDARY_NONE][NUM_TRIMESTERS][NUM_CMV_PHENOTYPES]; 
        /** CohortInputs probability male */
        double probMale;
        /** CohortInputs Enable Custom Age Distribution */
        bool enableCustomAgeDist;
        /** CohortInputs Custom Age Distribution strata month ranges */
        int customAgeDistMonthStrata[NUM_CUSTOM_AGE_STRATA];
        /** CohortInputs Custom Age Distribution probabilities by strata */
        double customAgeDistStrataProbs[NUM_CUSTOM_AGE_STRATA+1];
        /** CohortInputs Low Avidity duration mean (weeks) */
        double lowAvidityDurationMean;
        /** CohortInputs Low Avidity duration std dev (weeks)*/
        double lowAvidityDurationStdDev;

    };

    class PrenatalNatHistInputs {
    public:
        /* Weekly Probabilities*/
        double weeklyPrimaryCMV;
        double weeklySecondaryCMV;
        double weeklyProbMildIllness;
        double weeklyProbKnownInfection;

        /* Maternal Biology Biomarkers Timing */
        double weeksToViremiaFromInfectionMean;
        double weeksToViremiaFromInfectionStdDev;
        double lengthOfViremiaMean;
        double lengthOfViremiaStdDev;
        double weeksToIgmPositiveFromInfectionMean;
        double weeksToIgmPositiveFromInfectionStdDev;
        double lengthOfIgmPositivityMean;
        double lengthOfIgmPositivityStdDev;
        double weeksToIggPositiveFromInfectionMean;
        double weeksToIggPositiveFromInfectionStdDev;
        double weeksToLowAvidityFromInfectionMean;
        double weeksToLowAvidityFromInfectionStdDev;
        double weeksToHighAvidityFromLowAvidityMean;
        double weeksToHighAvidityFromLowAvidityStdDev;

        /* Prenatal Weekly Probabilities (currently for both primary and secondary maternal infections, but this can be changed in the future)*/
        int maternalInfectionToVerticalTransmissionDelay;
        int verticalTransmissionToDetectableDelay;

        /* Vertical Transmission Probabilities */
        double primaryVerticalTransmission[NUM_TRIMESTERS];
        double secondaryVerticalTransmission[NUM_TRIMESTERS];
        /** Child Weekly Primary CMV Probability */
        //double childWeeklyPrimaryCMV[MAX_WEEKS_PREGNANT];
        /** Child Weekly Secondary CMV Probability */
        //double childWeeklySecondaryCMV[MAX_WEEKS_PREGNANT];
        /** Background weekly probability of spontaneous abortion/miscarriage/IUFD/Stillbirth */
        double backgroundMiscarriage[AGE_YEARS][MAX_WEEKS_PREGNANT];
        /** CMV Positive weekly probability of spontaneous abortion/miscarriage/IUFD/Stillbirth */
        double CMVPositiveMiscarriage[AGE_YEARS][MAX_WEEKS_PREGNANT];

        /** Probability of fetal death during birth */
        double fetalDeathRiskDuringBirth[MAX_WEEKS_PREGNANT][2];
    };

    class BackgroundScreeningInputs {
    public:

        /* Probability of Seeking Healthcare */
        double seekingHealthcareSymptomatic;
        double seekingHealthcareMildIllness;
        double seekingHealthcareKnownInfection;

        /* Weeks with regularly scheduled appointments */
        double weeksWithScheduledAppointments[MAX_WEEKS_PREGNANT];
        /* Probability of attending regularly scheduled appointments */
        double probAttendingRegularAppointmentsByWeek[MAX_WEEKS_PREGNANT];

        /* Likelihood of reporting symptoms during prenatal appointments */
        double probReportingSymptoms;

        /* CMV prematurity multipliers */
        double prematurityMultipliers[MAX_WEEKS_PREGNANT][NUM_CMV_PHENOTYPES];

        /* Background Testing Administration backgroundTesting[weeks pregnant][background test]*/
        bool backgroundTesting[MAX_WEEKS_PREGNANT][NUM_PRENATAL_TESTS];
        /* Testing upon Suspected Maternal CMV */
        /** for Primary, Nonprimary, Recent Unknown, and Not Recent Unknown maternal CMV (unknown = not known whether primary or secondary infection)*/
        /*** Weeks with scheduled visit [WEEK][NONE(0), PRIMARY(1), NONPRIMARY(2), RECENT UNKNOWN(3), NOT RECENT UNKNOWN (4)] */
        bool weeksWithVisitsUponMaternalDiagnosis[MAX_WEEKS_PREGNANT][5];
        /*** Weekly probability of attending visit (if scheduled) [WEEK][NONE(0), PRIMARY(1), NONPRIMARY(2), RECENT UNKNOWN(3), NOT RECENT UNKNOWN (4)] */
        double probAttendUponMaternalDiagnosis[MAX_WEEKS_PREGNANT][5];
        /*** Tests by week upon Suspected Maternal CMV [WEEK][PRENATAL TEST][NONE(0), PRIMARY(1), NONPRIMARY(2), RECENT UNKNOWN(3), NOT RECENT UNKNOWN (4)] */
        bool testingUponMaternalDiagnosis[MAX_WEEKS_PREGNANT][NUM_PRENATAL_TESTS][5];

        /** Activated Tests upon Suspected Maternal CMV [PRENATAL TEST][NONE(0), PRIMARY(1), NONPRIMARY(2), RECENT UNKNOWN(3), NOT RECENT UNKNOWN (4)] */
        //bool activatedTestsDiagnosedMaternalCMV[NUM_PRENATAL_TESTS][4];
        /** Probability of tests upon Suspected Maternal CMV [PRENATAL TEST][NONE(0), PRIMARY(1), NONPRIMARY(2), UNKNOWN(3)] */
        //double probTestDiagnosedMaternalCMV[NUM_PRENATAL_TESTS][4];
        /** Perform test every N visits, stratified by the week in which the visit occurs [WEEK][TEST NUM][NONE(0), PRIMARY(1), NONPRIMARY(2), UNKNOWN(3)] */
        //bool weeksPerformTestsDiagnosedMaternalCMV[MAX_WEEKS_PREGNANT][NUM_PRENATAL_TESTS][4];

        /* Testing upon Suspected Child CMV */
        /** Weeks with scheduled visit [WEEK][ASYMPTOMATIC/SYMPTOMATIC] */
        bool weeksWithVisitsUponChildDiagnosis[MAX_WEEKS_PREGNANT][2];
        /** Weekly probability of attending visit (if scheduled) [WEEK][ASYMPTOMATIC/SYMPTOMATIC] */
        double probAttendUponChildDiagnosis[MAX_WEEKS_PREGNANT][2];
        /** Tests by week upon Suspected Child CMV [WEEK][PRENATAL TEST][ASYMPTOMATIC/SYMPTOMATIC] */
        bool testingUponChildDiagnosis[MAX_WEEKS_PREGNANT][NUM_PRENATAL_TESTS][2];

        /** Activated Tests upon Suspected Child CMV [WEEK][PRENATAL TEST][ASYMPTOMATIC/SYMPTOMATIC] */
        //bool activatedTestsSuspectedChildCMV[NUM_PRENATAL_TESTS][2];
        /** Probability of tests upon Suspected CHILD CMMV [PRENATAL TEST][ASYMPTOMATIC/SYMPTOMATIC]*/
        //double probTestSuspectedChildCMV[NUM_PRENATAL_TESTS][2];
        /** Perform test every N visits, stratified by the week in which the visit occurs [WEEK][TEST NUM][ASYMPTOMATIC/SYMPTOMATIC]*/
        //bool weeksPerformTestsDiagnosedChildCMV[MAX_WEEKS_PREGNANT][NUM_PRENATAL_TESTS][2];
    };

    class PrenatalTestingInputs {
    public:
        class PrenatalTest{
        public:
            bool enableTest;
            string testName;
            int testArray;

            /* Special case triggering */
            double mildIllnessTrigger;
            double knownInfectionTrigger;
            double symptomaticCMVTrigger;
            int previousTestTrigger;

            /* Offer and accepatance of prenatal test */
            double probOfferedAndAccept;
            
            /* Cost of prenatal test */
            double costOfTest;

            /* Prenatal test characteristics */
            double probResultReturn;
            int weeksToResultReturn;
            int weeksToRepeatTestIfNoReturn;
            /** Minimum test interval in weeks between tests */
            int minimumTestInterval;
            /** Probability of pregnancy termination upon abnormal result */
            double probTerminationUponAbnormal;
            /** The week that the test becomes available to be administered (before this week, even if the test is scheduled or would otherwise be given, it will be ignored */
            int availableStartingWeek;
            /** The week that the test is no longer available to be administered (starting this week, even if the test is scheduled or would otherwise be given, it will not be performed.) */
            int notAvailableAfter;
            
            /* Test Sensitivity and Specificity */
            double testSensitivity[NUM_TRIMESTERS][NUM_CMV_PHENOTYPES];
            double testSpecificity[NUM_TRIMESTERS];

            /* Confirmatory Testing */
            int numRepeatedConfirmatory;
            double probSchedulingConfirmatory;
            int weeksBetweenConfirmatoryTests;
            double additionalCostConfirmatoryTest;

            /* Follow-up testing if the test is a base test */
            bool followUpTests[NUM_PRENATAL_TESTS][2];
            double followUpTestsProbabilities[NUM_PRENATAL_TESTS][2];
            int followUpTestDelays[NUM_PRENATAL_TESTS][2];

            /* Follow-up testing if the test was triggered by a previous test */
            /** followUpTests array is stratified by [PRENATAL TEST THIS IS FOLLOWING UP FROM][FOLLOW UP TESTS TO THIS TEST][IS THIS TEST RESULT NORMAL OR ABNORMAL]*/
            bool FUTriggeredByTest[NUM_PRENATAL_TESTS][NUM_PRENATAL_TESTS][2];
            double FUTriggeredByTestProbabilities[NUM_PRENATAL_TESTS][NUM_PRENATAL_TESTS][2];
             /** FUTriggeredByTestDelays array is stratified by [PRENATAL TEST THIS IS FOLLOWING UP FROM][FOLLOW UP TESTS TO THIS TEST][IS THIS TEST RESULT NORMAL OR ABNORMAL]*/
            int FUTriggeredByTestDelays[NUM_PRENATAL_TESTS][NUM_PRENATAL_TESTS][2];
        }; /* End prenatal test class */

        
        PrenatalTest routineUltrasound;
        PrenatalTest detailedUltrasound;
        PrenatalTest amniocentesis;
        PrenatalTest maternalPCR;
        PrenatalTest igmTest;
        PrenatalTest iggTest;
        PrenatalTest avidityTest;
        
        vector<PrenatalTest> PrenatalTestsVector;
    };

    class PrenatalTreatmentInputs {
    public:
        /** Prenatal Treatment Policies */
        bool treatmentBasedOnPolicy[NUM_TREATMENT_POLICIES];
        int treatmentNumberBasedOnPolicy[NUM_TREATMENT_POLICIES];
        int treatmentDurationBasedOnPolicy[NUM_TREATMENT_POLICIES];

        class PrenatalTreatment {
        public:
            double costPerWeek;
            double reductionOfWeeklyPrimaryCMV;
            double reductionOfWeeklyPrimaryProportionSymptomatic;
            double reductionOfWeeklySecondaryCMV;
            double reductionOfWeeklySecondaryProportionSymptomatic;

            /** Reduction of Vertical Transmission From Previous Primary CMV */
            double reductionOfPreviousVTFromPrimaryCMV[NUM_LAST_CMV_STRATA];
            /** Reduction of Vertical Transmission From Previous Secondary CMV */
            double reductionOfPreviousVTFromSecondaryCMV[NUM_LAST_CMV_STRATA];
            /** Reduction of Vertical Transmission From Primary CMV [week] */
            double reductionOfVerticalTransmissionFromPrimaryCMV[MAX_WEEKS_PREGNANT];
            /** Reduction of Vertical Transmission From Secondary CMV [week] */
            double reductionOfVerticalTransmissionFromSecondaryCMV[MAX_WEEKS_PREGNANT];

            /** Reduction of Symptomms */
            double reductionOfSymptoms[NUM_CMV_PHENOTYPES][NUM_CMV_PHENOTYPES];
        };

        PrenatalTreatment prenatalTreatment1;
        PrenatalTreatment prenatalTreatment2;
        PrenatalTreatment prenatalTreatment3;
        vector<PrenatalTreatment> PrenatalTreatmentsVector;
    };

    class PostpartumInputs {
    public:

    };
    class InfantInputs {
    public:

    };
    class AdultInputs {
    public:
    
    };

    /* readInputs function reads in all the inputs from the given input file, 
    throws exception if there is an error */
    void readInputs();

    const RunSpecsInputs *getRunSpecsInputs();
    const CohortInputs *getCohortInputs();
    const PrenatalNatHistInputs *getPrenatalNatHistInputs();
    const BackgroundScreeningInputs *getBackgroundScreeningInputs();
    const PrenatalTestingInputs *getPrenatalTestingInputs();
    const PrenatalTreatmentInputs *getPrenatalTreatmentInputs();
    const PostpartumInputs *getPostpartumInputs();
    const InfantInputs *getInfantInputs();
    const AdultInputs *getAdultInputs();

private:
    /* Input file name and file pointer*/
    string inputFileName;
    FILE *inputFile;

    /* Objects for storing the input data */
    RunSpecsInputs runSpecsInputs;
    CohortInputs cohortInputs;
    PrenatalNatHistInputs prenatalNatHistInputs;
    BackgroundScreeningInputs backgroundScreeningInputs;
    PrenatalTestingInputs prenatalTestingInputs;
    PrenatalTreatmentInputs prenatalTreatmentInputs;
    PostpartumInputs postpartumInputs;
    InfantInputs infantInputs;
    AdultInputs adultInputs;

    /* Private Functions for reading in the inputs, called by readInputs */
    void readRunSpecsInputs();
    void readCohortInputs();
    void readPrenatalNatHistInputs();
    void readHealthcareInteractionsScheduleInputs();
    void readPrenatalTestSpecs();
    void readPrenatalTreatmentInputs();
    void readPostPartumInputs();
    void readInfantInputs();
    void readAdultInputs();
    bool readAndSkipPast(const char* searchStr, FILE* file);
    bool readAndSkipPast2(const char* searchStr, const char* searchStr2, FILE* file);

};

/* getRunSpecsInputs returns a const pointer to the RunSpecsInput data object */\
inline const SimContext::RunSpecsInputs *SimContext::getRunSpecsInputs() {
    return &runSpecsInputs;
}

/* getRunSpecsInputs returns a const pointer to the RunSpecsInput data object */\
inline const SimContext::CohortInputs *SimContext::getCohortInputs() {
    return &cohortInputs;
}

/* getRunMaternalCohortInputs returns a const pointer to the MaternalCohortInputs data object */\
inline const SimContext::PrenatalNatHistInputs *SimContext::getPrenatalNatHistInputs() {
    return &prenatalNatHistInputs;
}

/* getBackgroundScreeningInputs returns a const pointer to the BackgroundScreeningInputs data object */
inline const SimContext::BackgroundScreeningInputs *SimContext::getBackgroundScreeningInputs() {
    return &backgroundScreeningInputs;
}

/* getRunSpecsInputs returns a const pointer to the RunSpecsInput data object */\
inline const SimContext::PrenatalTestingInputs *SimContext::getPrenatalTestingInputs() {
    return &prenatalTestingInputs;
}

/* getPreantalTreatmentInputs returns a const pointer to the PrenatalTreatmentInputs data object */
inline const SimContext::PrenatalTreatmentInputs *SimContext::getPrenatalTreatmentInputs() {
    return &prenatalTreatmentInputs;
}

/* getRunSpecsInputs returns a const pointer to the RunSpecsInput data object */\
inline const SimContext::PostpartumInputs *SimContext::getPostpartumInputs() {
    return &postpartumInputs;
}
/* getRunSpecsInputs returns a const pointer to the RunSpecsInput data object */\
inline const SimContext::InfantInputs *SimContext::getInfantInputs() {
    return &infantInputs;
}
/* getRunSpecsInputs returns a const pointer to the RunSpecsInput data object */\
inline const SimContext::AdultInputs *SimContext::getAdultInputs() {
    return &adultInputs;
}