#pragma once

#include "include.h"

/*
    Patient class holds all of the stats for a patient running in the simulation. 
*/

class Patient
{
public:
    friend class StateUpdater;

    Patient(SimContext *simContext, RunStats *runStats, CostStats *costStats, Tracer *tracer, int patientNum, bool trace);
    ~Patient(void);

    /** GeneralState class holds information about the overall patient class, which includes both the mother and child. */
    class GeneralState {
    public:
        /** Unique patient identifier */
        int patientNum;
        /** True if this patient is traced in the output trace file */
        bool tracingEnabled;
        /** The current discount factor */
        double discountFactor;
        /** The current quality of life multiplier */
        double QOLMultiplier;
        /** The current week number (is disabled once the pregnancy ends)*/
        int weekNum;
        /** The current trimester. Dependent on weekNum */
        int trimester;
        /** The current month number (is enabled once the pregnancy ends)*/
        int monthNum;
        /** Initial month number. Will default to 0*/
        int initialWeekNum;
        /** Breastfeeding Status */
        bool breastfeedingStatus;
        /** Prenatal Test ID number (also doubles as total number of prenatal tests performed on mother-child pair)*/
        int prenatalTestID;
        /** Prenatal tests and whether they've ever been performed */
        bool prenatalTestsPerformed[SimContext::NUM_PRENATAL_TESTS];
        /** Results of the last time each prenatal test was performed */
        int prenatalTestsLastResult[SimContext::NUM_PRENATAL_TESTS];
        /** Last week that prenatal test was performed (vector indexed by test number) */
        int weekTestLastPerformed[SimContext::NUM_PRENATAL_TESTS];
        /** Whether this week of pregnancy has an appointment for the patient */
        bool prenatalAppointmentThisWeek;
        /** Whether this week of pregnancy has an appointment triggered by symptomatic CMV or mild illness */

        /** This week's prenatal tests to perform */
        vector<SimContext::PrenatalTestInstance> prenatalTestsToPerformThisWeek;
        /** Pending Follow-Up Prenatal Tests */
        vector<SimContext::PrenatalTestInstance> pendingFollowUpTests[SimContext::MAX_WEEKS_PREGNANT];
        /** Pending Confirmatory Prenatal Tests */
        vector<SimContext::PrenatalTestInstance> pendingConfirmatoryTests[SimContext::MAX_WEEKS_PREGNANT];
        /** Pending Special Case Tests */
        vector<SimContext::PrenatalTestInstance> specialCaseTests[SimContext::MAX_WEEKS_PREGNANT];
        /** Prenatal Test Result return array -- contains information about when prenatal tests are returned */
        vector<SimContext::PrenatalTestInstance> prenatalTestResultReturn[SimContext::MAX_WEEKS_PREGNANT];
        /** Whether patient/mother pair is on treatment */
        bool onTreatment;
        /** Treatment that the mother/pair is on */
        int mostRecentTreatmentNumber;
        /** Week that treatment was started */
        int weekTreatmentStarted[SimContext::NUM_TREATMENTS];
        /** Treatments that the patient has had */
        bool hadTreatment[SimContext::NUM_TREATMENTS];
    };

    /** MaternalState class holds information about the mother's characteristics */
    class MaternalState {
    public:
        /** Life status of mother */
        bool isAlive;
        /** The patient's current age in months */
        int maternalAgeMonths;
        /** The patient's current age in exact numbers (years). E.g. 28.789 years */
        double maternalAgeYearsExact;
        /** The mother's current age in years (whole number). E.g. 28 years*/
        int maternalAgeYears;
        /** The current discounted costs accured for this patient */
        double costsDiscounted;
        double costsUndiscounted;
        /** The current discounted and undiscounted life months this patient has lived */
        double LMsDiscounted;
        double LMsUndiscounted;
        
    };

    /** MaternalDiseaseState holds information about the mother's disease state, such as infection state, clinical history, symptoms, etc. */
    class MaternalDiseaseState {
    public:
        /** True if the mother had active CMV at beginning of simulation*/
        bool isPrevalentCMVCase;
        /** True if the mother had previously had active CMV prior to the simulation but does not at the beginning of simulation*/
        bool prevCMVCase;
        /** Stratum of how long ago the previous CMV infection was (0 - No prev CMV)*/
        int prevCMVStratum;
        /** Whether previous CMV case was primary or secondary */
        int prevCMVPrimaryOrSecondary;
        /** Whether mother has ever been infected with CMV */
        bool hadCMV;
        /** Whether mother has had CMV DURING SIMULATION (doesn't include prevalent CMV or infections prior to simulation start)*/
        bool hadCMVDuringSim;
        /** Month Last had Active CMV (regardless of whether vertical transmission will occur)*/
        int weekLastCMV;
        /** Whether currently infected with CMV */
        bool activeCMV;
        /** Whether current CMV is viremic */
        bool viremia;
        /** IgM Status */
        bool IgM;
        /** IgG status */
        bool IgG;
        /** CMV Avidity */
        int avidity;
        /** Whether PCR would be positive */
        bool PCRWouldBePositive;
        /** An integer indicating the week the mother was infected and vertical transmission will occur */
        int weekOfMaternalCMVInfection;
        /** An integer indicating the week the child was infected */
        int weekOfChildCMVInfection;
        /** Mild Illness Status */
        bool mildIllness;
        /** Whether mother has had mild illness */
        bool hadMildIllness;
        /** Week of Mild Illness */
        int weekMildIllness;
        /** Vertical Transmission. Indicates if vertical transmission will occur/has occurred even if the baby has not been infected yet */
        bool verticalTransmission;
        /** Specifies whether infection is primary or secondary */
        int primaryOrSecondary;

        /* Maternal Biology Characteristics */

        /** Time to Viremia from Infection */
        int weeksToViremiaFromInfection;
        /** Length of Viremia */
        int lengthOfViremia;
        /** Time to IgM Positive From Infection */
        int weeksToIgmPositiveFromInfection;
        /** Length of IgM Positivity */
        int lengthOfIgmPositivity;
        /** Time To IgG Positive From Infection */
        int weeksToIggPositiveFromInfection;
        /** Time To Low Avidity From Infection */
        int weeksToLowAvidityFromInfection;
        /** Time To High Avidity From Low Avidity */
        int weeksToHighAvidityFromLowAvidity;
    };

    /** MaternalMonitoringState contains information for ther mother's testing, clinical visits, and observed health state */
    class MaternalMonitoringState {
    public:
        /** CMV Exposure Status*/
        bool exposureStatus;
        /** Known CMV Infection */
        bool knownCMV;
        /** Known that mother has had CMV infection before */
        bool knownHadCMV;
        /** Diagnosed CMV */
        bool diagnosedCMV;
        /** Week that CMV was diagnosed */
        
        /** Diagnosed CMV status is none, primary, secondary, recent unknown, or long-ago unknown */
        int diagnosedCMVStatus;
        /** Week that CMV was detected/diagnosed */
        int weekCMVDiagnosed;
        /** Month that CMV was detected */
        int monthCMVDetected;
        /** Mild Illness Symptoms */
        bool mildIllnessSymptoms;
        /** Viremia status*/
        bool viremiaStatus;
        /** IgM status */
        bool IgMStatus;
        int weekLastIgMTest;
        /** IgG Status */
        bool IgGStatus;
        int weekLastIgGTest;
        /** IgG Avidity - 0) none, 1) low, 2) high*/
        int avidity;
        int weekLastAvidityTest;
        /** CMV Symptoms */
        bool CMVSymptoms;
        /** Number of regular appointments */
        int numRegularAppointments;
        /** Number of missed appointments */
        int numMissedAppointments;
        /** Number of Ultrasounds */
        int numUltrasounds;
        /** Suspected CMV status (none, primary, secondary)*/
        int suspectedCMVStatus;
        /** Trimester that CMV was first suspected (0 - none, 1 - first, 2 - second, 3 - third)*/
        int suspectedCMVTrimester;
        /** Boolean that determines whether to do weekly check for IgM/IgG/Avidity test outcome (whether to perform IgMIgGAvidityOutcome function)*/
        bool performWeeklyIgmIggAvidityOutcomeCheck;
        /** Current delay between IgM and IgG test in weeks */
        int IgmToIggDelay;
        /** Current delay between IgG and IgG Avidity test in weeks */
        int IggToAvidityDelay;

        /** Stores pending test results waiting to be returned */
        //vector<SimContext::PrenatalTestState> prenatalPendingTestResults;
        /** Stores pending follow-up tests */
        //vector<SimContext::PrenatalTestState> prenatalFollowUpTests;
        /** Stores scheduled prenatal tests waiting to be done */
        //vector<SimContext::PrenatalTestState> prenatalScheduledConfirmatoryTests;
        /** Stores pending special case triggered tests */
        //vector<SimContext::PrenatalTestState> prenatalSpecialCaseTests;
        /** Stores tests that were done that are still within their minimum testing interval */
        //vector<SimContext::PrenatalTestState> prenatalTestsWithinMinInterval;
    };

    class ChildState {
    public:
        // Life Status of Child
        bool isAlive;
        // The child's age in weeks
        int ageWeeks;
        // The child's age in months
        int ageMonths;
        // The child's age in years
        int ageYears;
        // Delivery status - default is set to 0 for unborn.
        bool deliveryOcurred;
        /** The child's biological sex*/
        int biologicalSex;
    };


    class ChildDiseaseState {
    public:
        /** Whether child is infected with CMV */
        bool infectedCMV;
        /** Whether child is detectable with CMV */
        bool detectableCMV;
        /** Whether child has had CMV */
        bool hadCMV;
        /** Week of Child CMV Infection */
        /** CMV before or after birth */
        bool CMVAfterBirth;
        /** Phenotype of CMV infection */
        int phenotypeCMV;
        /** Time of CMV infection */
        int infectionTime;
        /** Time of CMV infection by trimester */
        int infectionTrimester;
        /** Specifies whether time of infection is in weeks or months */
        bool infectionTimeInMonths;
        /** child CMV infection primary or secondary */
        int primaryOrSecondary;
    };
    
    class ChildMonitoringState {
    public:
        /* PRENATAL MONITORING PARAMETERS */
        /** Whether fetus has had ultrasound taken */
        bool ultrasound;
        /** Amniocentesis */
        bool amniocentesis;

        /* */
        bool observedSymptomatic;

        /* NEONATAL MONITORING PARAMETERS */
        /* Whether fetus is known to have had CMV */
        bool knownCMV;

    };

    /** Pointer to the associated simulation context*/
    SimContext *simContext;
    /** Stats file name */
    string statsFileName;
    /** Stats file pointer*/
    FILE *statsFile;


    /* Accessor functions return const pointers to the Patient state subclass objects */
    const GeneralState *getGeneralState();
    const MaternalState *getMaternalState();
    const MaternalDiseaseState *getMaternalDiseaseState();
    const MaternalMonitoringState *getMaternalMonitoringState();
    const ChildState *getChildState();
    const ChildDiseaseState *getChildDiseaseState();
    const ChildMonitoringState *getChildMonitoringState();

    /* simulateWeek runs a single week of simulation for this patients, and updates
        its state and runStats statistics*/
    void simulateWeek();
    /* simulateMonth runs a single month of simulation for this patients, and updates
        its state and runStats statistics*/
    void simulateMonth();
    /* isAliveMother returns true if the mother is alive, false if dead */
    void isAliveMother();
    /* isAliveChild returns true if the child is alive, false if dead */
    void isAliveChild();

private:
    /** pointer to the statistics object */
    RunStats *runStats;
    /** pointer to the costs statistics object */
    CostStats *costStats;
    /** pointer to the trace object */
    Tracer *tracer;

    /** Patient state subclass object */
    GeneralState generalState;
    /** Patient state maternal object */
    MaternalState maternalState;
    /** Disease state object */
    MaternalDiseaseState maternalDiseaseState;
    /** Monitoring state object*/
    MaternalMonitoringState maternalMonitoringState;
    /** Patient state child object */
    ChildState childState;
    /** Child Disease State object*/
    ChildDiseaseState childDiseaseState;
    /** Child Monitoring State object*/
    ChildMonitoringState childMonitoringState;
    /** BeginWeekUpdater updater subclass object */
    BeginWeekUpdater beginWeekUpdater;
    /** CMV Infection Updater subclass object */
    CMVInfectionUpdater cmvInfectionUpdater;
    /** Maternal Biology Updater subclass object */
    MaternalBiologyUpdater maternalBiologyUpdater;
    /** Pregnancy Updater subclass object */
    PregnancyUpdater pregnancyUpdater;
    /** Prenatal Test Updater subclass object */
    PrenatalTestUpdater prenatalTestUpdater;
    /** End Week Updater subclass object */
    EndWeekUpdater endWeekUpdater;
}; // end Patient

/** getGeneralState returns a const pointer to the generalState object */
inline const Patient::GeneralState *Patient::getGeneralState() {
    return &generalState;
}
/** getGeneralState returns a const pointer to the maternalState object */
inline const Patient::MaternalState *Patient::getMaternalState() {
    return &maternalState;
}
/** getDiseaseState returns a const pointer to the maternalDiseaseState object */
inline const Patient::MaternalDiseaseState *Patient::getMaternalDiseaseState() {
    return &maternalDiseaseState;
}
/** getMonitoringState returns a const pointer to the maternalMonitoringState object */
inline const Patient::MaternalMonitoringState *Patient::getMaternalMonitoringState() {
    return &maternalMonitoringState;
}
/** getGeneralState returns a const pointer to the childState object */
inline const Patient::ChildState *Patient::getChildState() {
    return &childState;
}
/** getDiseaseState returns a const pointer to the childDiseaseState object */
inline const Patient::ChildDiseaseState *Patient::getChildDiseaseState() {
    return &childDiseaseState;
}
/** getChildMonitoringState returns a const pointer to the childMonitoringState object */
inline const Patient::ChildMonitoringState *Patient::getChildMonitoringState() {
    return &childMonitoringState;
}