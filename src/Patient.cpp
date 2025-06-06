#include "include.h"

// Disable warning for unsafe use of this pointer in initialization,
//	state updater constructor only copies the pointer and does not access any of its fields so it is safe
#pragma warning(disable:4355)

/** Constructor takes in the patient number, simulation context, run stats object, and tracing object
	Initializes all subclass state values */

Patient::Patient(SimContext *simContext, RunStats *runStats, CostStats *costStats, Tracer *tracer, int patientNum, bool trace) :
        simContext(simContext),
        runStats(runStats),
        costStats(costStats),
        tracer(tracer),
        cmvInfectionUpdater(this), 
        maternalBiologyUpdater(this),
        pregnancyUpdater(this),
        prenatalTestUpdater(this),
        beginWeekUpdater(this),
        endWeekUpdater(this)
{
    double randNum;
    this->generalState.patientNum = patientNum;
    //Reset seed for patient if using fixed seed
    CmvUtil::setFixedSeed(this);
    this->generalState.tracingEnabled = false;
    if (patientNum < simContext->getRunSpecsInputs()->numberOfPatientsToTrace) {
        this->generalState.tracingEnabled = true;
    }
    this->generalState.weekNum = 0; // initial week number is set to 0
    this->generalState.discountFactor = 1.0;
    this->generalState.prenatalTestID = 0;

    if (patientNum < simContext->getRunSpecsInputs()->numberOfPatientsToTrace) {
        this->generalState.tracingEnabled = true;
    }
    this->generalState.monthNum = 0;
    this->generalState.weekNum = 0; 

    /* FINISH setting discount factor */
    

    /* FINISH setting QOL Multiplier */

    /* setting current week Number */
    
    /* set breastfeeding status */
    this->generalState.breastfeedingStatus = false; // false to start out, since no breastfeeding during pregnancy

    /* setting initial prenatal test ID number */
    this->generalState.prenatalTestID = 0;
    /* setting initial week last performed test for prenatal tests */
    for (int i = 0; i < SimContext::NUM_PRENATAL_TESTS; i++) {
        this->generalState.prenatalTestsPerformed[i] = 0; // initialize all tests performed to false
        this->generalState.prenatalTestsLastResult[i] = 0; // initialize all last test results to 0 (negative or none)
        this->generalState.weekTestLastPerformed[i] = -1000; // initialize week that test was last performed to -1000 so that it's effectively never occurred before
    }
    this->generalState.prenatalAppointmentThisWeek = false;

    /* set maternal life status */
    this->maternalState.isAlive = true;

    /** Determine the maternal patient's age */
    double ageMean;
    double ageStdDev;
    int ageMinimum;
    int ageMaximum;
    ageMean = simContext->getCohortInputs()->ageMonthsMean;
    ageStdDev = simContext->getCohortInputs()->ageMonthsStdDev;
    ageMinimum = simContext->getCohortInputs()->minimumAgeMonths;
    ageMaximum = simContext->getCohortInputs()->maximumAgeMonths;
    double ageMonthsDouble = CmvUtil::getRandomGaussian(ageMean, ageStdDev);
    int ageMonths = (int) (ageMonthsDouble + 0.5);
    if (ageMonths < ageMinimum) {
        ageMonths = ageMinimum; }
    else if (ageMonths > ageMaximum) {
        ageMonths = ageMaximum; }
    this->maternalState.maternalAgeMonths = ageMonths;
    this->maternalState.maternalAgeYearsExact = (double)ageMonths/12;
    this->maternalState.maternalAgeYears = (int) (this->maternalState.maternalAgeYearsExact + 0.5);
    if (simContext->getCohortInputs()->enableCustomAgeDist) {
        // code for custom age distribution
    }
    this->generalState.onTreatment = false; 
    this->generalState.mostRecentTreatmentNumber = -1;
    //setting treatment number to -1 at first since it's not applicable until treatment begins to be administered. 
    
    /* No patients are on treatment at model start.
     setting week treatment started to 100 for all treatments. This is a nonsensically large number greater than the number of weeks of pregnancy
     so that if treatments are accidentally activated when they're not supposed to it's easier to debug.
    */ 
    this->generalState.weekTreatmentStarted[SimContext::TREATMENT_1] = 100;
    this->generalState.weekTreatmentStarted[SimContext::TREATMENT_2] = 100;
    this->generalState.weekTreatmentStarted[SimContext::TREATMENT_3] = 100;
    // setting had treatment to false for all treatments */
    for (int i = 0; i < SimContext::NUM_TREATMENTS; i++) {
        this->generalState.hadTreatment[i] = false;
    }

    /* SETTING MATERNAL DISEASE STATES */

    /** Drawing for Maternal Biology Timing Characteristics */
    // Drawing for Time to Viremia from Infection */
    this->maternalDiseaseState.weeksToViremiaFromInfection = -1;
    int weeksToViremiaFromInfectionMean = simContext->getPrenatalNatHistInputs()->weeksToViremiaFromInfectionMean;
    int weeksToViremiaFromInfectionStdDev = simContext->getPrenatalNatHistInputs()->weeksToViremiaFromInfectionStdDev;
    while(this->maternalDiseaseState.weeksToViremiaFromInfection < 0) {
        this->maternalDiseaseState.weeksToViremiaFromInfection = (int)(CmvUtil::getRandomGaussian(weeksToViremiaFromInfectionMean, weeksToViremiaFromInfectionStdDev) + 0.5);
    }
    // Drawing for Length of Viremia
    this->maternalDiseaseState.lengthOfViremia = -1;
    int lengthOfViremiaMean = simContext->getPrenatalNatHistInputs()->lengthOfViremiaMean;
    int lengthOfViremiaStdDev = simContext->getPrenatalNatHistInputs()->lengthOfViremiaStdDev;
    while(this->maternalDiseaseState.lengthOfViremia < 0) {
        this->maternalDiseaseState.lengthOfViremia = (int)(CmvUtil::getRandomGaussian(lengthOfViremiaMean, lengthOfViremiaStdDev) + 0.5);
    }
    // Drawing for Time to IgM Positive From Infection
    this->maternalDiseaseState.weeksToIgmPositiveFromInfection = -1;
    int weeksToIgmPositiveFromInfectionMean = simContext->getPrenatalNatHistInputs()->weeksToIgmPositiveFromInfectionMean;
    int weeksToIgmPositiveFromInfectionStdDev = simContext->getPrenatalNatHistInputs()->weeksToIgmPositiveFromInfectionStdDev;
    while(this->maternalDiseaseState.weeksToIgmPositiveFromInfection < 0) {
        this->maternalDiseaseState.weeksToIgmPositiveFromInfection = (int)(CmvUtil::getRandomGaussian(weeksToIgmPositiveFromInfectionMean, weeksToIgmPositiveFromInfectionStdDev) + 0.5);
    }
    // Drawing for Length of IgM positivity
    this->maternalDiseaseState.lengthOfIgmPositivity = -1;
    int lengthOfIgmPositivityMean = simContext->getPrenatalNatHistInputs()->lengthOfIgmPositivityMean;
    int lengthOfIgmPositivityStdDev = simContext->getPrenatalNatHistInputs()->lengthOfIgmPositivityStdDev;
    while(this->maternalDiseaseState.lengthOfIgmPositivity < 0) {
        this->maternalDiseaseState.lengthOfIgmPositivity = (int)(CmvUtil::getRandomGaussian(lengthOfIgmPositivityMean, lengthOfIgmPositivityStdDev) + 0.5);
    }
    // Drawing for Time To IgG Positive From Infection 
    this->maternalDiseaseState.weeksToIggPositiveFromInfection = -1;
    int weeksToIggPositiveFromInfectionMean = simContext->getPrenatalNatHistInputs()->weeksToIggPositiveFromInfectionMean;
    int weeksToIggPositiveFromInfectionStdDev = simContext->getPrenatalNatHistInputs()->weeksToIggPositiveFromInfectionStdDev;
    while(this->maternalDiseaseState.weeksToIggPositiveFromInfection < 0) {
        this->maternalDiseaseState.weeksToIggPositiveFromInfection = (int)(CmvUtil::getRandomGaussian(weeksToIggPositiveFromInfectionMean, weeksToIggPositiveFromInfectionStdDev) + 0.5);
    }
    // Drawing for Time To Low Avidity From Infection
    this->maternalDiseaseState.weeksToLowAvidityFromInfection = -1;
    int weeksToLowAvidityFromInfectionMean = simContext->getPrenatalNatHistInputs()->weeksToLowAvidityFromInfectionMean;
    int weeksToLowAvidityFromInfectionStdDev = simContext->getPrenatalNatHistInputs()->weeksToLowAvidityFromInfectionStdDev;
    while(this->maternalDiseaseState.weeksToLowAvidityFromInfection < 0) {
        this->maternalDiseaseState.weeksToLowAvidityFromInfection = (int)(CmvUtil::getRandomGaussian(weeksToLowAvidityFromInfectionMean, weeksToLowAvidityFromInfectionStdDev) + 0.5);
    }
    // Drawing for Time To High Avidity From Low Avidity
    this->maternalDiseaseState.weeksToHighAvidityFromLowAvidity = -1;
    int weeksToHighAvidityFromLowAvidityMean = simContext->getPrenatalNatHistInputs()->weeksToHighAvidityFromLowAvidityMean;
    int weeksToHighAvidityFromLowAvidityStdDev = simContext->getPrenatalNatHistInputs()->weeksToHighAvidityFromLowAvidityStdDev;
    while(this->maternalDiseaseState.weeksToHighAvidityFromLowAvidity < 0) {
        this->maternalDiseaseState.weeksToHighAvidityFromLowAvidity = (int)(CmvUtil::getRandomGaussian(weeksToHighAvidityFromLowAvidityMean, weeksToHighAvidityFromLowAvidityStdDev) + 0.5);
    }

    // Setting CMV active status and timing of last active infection (if any)
    this->maternalDiseaseState.activeCMV = false;
    double probPrevCMVCase = 1-simContext->getCohortInputs()->lastGotActiveCMVByStrata[SimContext::NEVER_CMV];
    vector<double> lastActiveCMV(SimContext::NUM_LAST_CMV_STRATA + 1, 0);

    for (int j = 0; j < SimContext::NUM_LAST_CMV_STRATA + 1; j++) {
        lastActiveCMV[j] = simContext->getCohortInputs()->lastGotActiveCMVByStrata[j];
    }

    // setting maternal biology stats. If previous CMV, some of these will be changed in the next code block
    this->maternalDiseaseState.hadCMVDuringSim = false;
    this->maternalDiseaseState.activeCMV = false;
    this->maternalDiseaseState.viremia = false;
    this->maternalDiseaseState.IgG = false; // need to roll for initial positive?
    this->maternalDiseaseState.IgM = false; // need to roll for initial positive?
    this->maternalDiseaseState.avidity = SimContext::NO_AVIDITY;
    this->maternalDiseaseState.prevCMVCase = false;
    this->maternalDiseaseState.hadCMV = false;
    this->maternalDiseaseState.weekLastCMV = -10000; // set it to absurdly low number to indicate that they never had CMV
    this->maternalDiseaseState.weekOfMaternalCMVInfection = -10000;
    this->maternalDiseaseState.prevCMVPrimaryOrSecondary = SimContext::TRUE_NO_INFECTION;
    this->maternalDiseaseState.primaryOrSecondary = SimContext::TRUE_NO_INFECTION;
    
    /** Rolling for previous CMV infection (pre simulation) and setting maternal biology characteristics for patient at model start. */
    randNum = CmvUtil::getRandomDouble();
    int prevCMVStratum = CmvUtil::selectFromDist(lastActiveCMV, randNum);

    this->maternalDiseaseState.prevCMVStratum = prevCMVStratum;
    if (prevCMVStratum != simContext->NEVER_CMV) { // If any previous CMV before, set the proper patient disease state characteristics
        this->maternalDiseaseState.prevCMVCase = true;
        this->maternalDiseaseState.hadCMV = true;
        
        
        int lastGotCMV;
        int weeksFromPrevInfectionAtStart; // At model start, the number of weeks since the previous CMV infection
        if (prevCMVStratum == simContext->VERY_RECENT_CMV) {
            lastGotCMV = CmvUtil::getRandomInt(simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[0], simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[1]);
        }
        else if (prevCMVStratum == simContext->MEDIUM_RECENT_CMV) {
            lastGotCMV = CmvUtil::getRandomInt(simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[1], simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[2]);
        }
        else if (prevCMVStratum == simContext->LESS_RECENT_CMV) {
            lastGotCMV = CmvUtil::getRandomInt(simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[2], simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[3]);
        }
        else if (prevCMVStratum == simContext->LONG_TIME_AGO_CMV) {
            lastGotCMV = CmvUtil::getRandomInt(simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[3], simContext->getRunSpecsInputs()->lastActiveCMVStrataBounds[4]);
        }
        lastGotCMV = -1 * lastGotCMV; // turn into negative
        weeksFromPrevInfectionAtStart = this->generalState.weekNum - lastGotCMV;
        this->maternalDiseaseState.weekLastCMV = lastGotCMV;
        this->maternalDiseaseState.weekOfMaternalCMVInfection = lastGotCMV;

        /* Setting initial maternal biology for Viremia and IgM */
        // Setting Viremia at start of patient simulation (if viremic)
        if (weeksFromPrevInfectionAtStart >= this->maternalDiseaseState.weeksToViremiaFromInfection && weeksFromPrevInfectionAtStart < this->maternalDiseaseState.lengthOfViremia) {
            this->maternalDiseaseState.viremia = true;
        }
        // Setting IgM at start of patient simulation (if IgM is positive)
        if (weeksFromPrevInfectionAtStart >= this->maternalDiseaseState.weeksToIgmPositiveFromInfection && weeksFromPrevInfectionAtStart < this->maternalDiseaseState.lengthOfIgmPositivity) {
            this->maternalDiseaseState.IgM = true;
        }
        /* rolling for whether previous infection prior to simulation start was primary or secondary */
        bool primary = false;
        randNum = CmvUtil::getRandomDouble();
        if (randNum < simContext->getCohortInputs()->probPrevCMVWasPrimary[prevCMVStratum]) {
            primary = true;
        }
        
        if (primary) {
            this->maternalDiseaseState.prevCMVPrimaryOrSecondary = SimContext::TRUE_PRIMARY;
            this->maternalDiseaseState.primaryOrSecondary = SimContext::TRUE_PRIMARY;

            // Set initial maternal biology for previous PRIMARY infection only.

            // Setting IgG at start of patient simulation (if IgG is positive)
            if (weeksFromPrevInfectionAtStart >= this->maternalDiseaseState.weeksToIggPositiveFromInfection) {
                this->maternalDiseaseState.IgG = true;
            }
            // Setting Avidity at start of patient simulation (if Avidity is either high or low)
            //  Set as low avidity if in that timeframe
            if (weeksFromPrevInfectionAtStart >= this->maternalDiseaseState.weeksToLowAvidityFromInfection && 
            weeksFromPrevInfectionAtStart < (this->maternalDiseaseState.weeksToHighAvidityFromLowAvidity + this->maternalDiseaseState.weeksToLowAvidityFromInfection)) {
                this->maternalDiseaseState.avidity = SimContext::LOW_AVIDITY;
            }
            //  Set as high avidity if in that timeframe
            if (weeksFromPrevInfectionAtStart >= (this->maternalDiseaseState.weeksToHighAvidityFromLowAvidity + this->maternalDiseaseState.weeksToLowAvidityFromInfection)) {
                this->maternalDiseaseState.avidity = SimContext::HIGH_AVIDITY;
            }
        }
        else { 
            // if not primary infection or no infection, it's a secondary infection
            this->maternalDiseaseState.prevCMVPrimaryOrSecondary = SimContext::TRUE_SECONDARY;
            this->maternalDiseaseState.primaryOrSecondary = SimContext::TRUE_SECONDARY;

            // Set initial maternal biology for previous NON-PRIMARY infection.
            // Setting IgG at start of patient simulation as true, since if the last CMV infection was non-primary (secondary), IgG must already be positive from the previous primary infection prior to this last infection
            this->maternalDiseaseState.IgG = true;
            // Setting avidity at start of patient simulation as high, since it's presumably been a long time since the initial primary infection that cused the IgG avidity to increase in the first place. 
            this->maternalDiseaseState.avidity = SimContext::HIGH_AVIDITY;
        }
        /* Setting the proper maternal biology states*/
        // At model start, the number of weeks since the previous CMV infection
        
        
    }

    this->maternalDiseaseState.isPrevalentCMVCase = false;

    if ((this->getGeneralState()->weekNum - this->getMaternalDiseaseState()->weekLastCMV) < 3 ) {
        this->maternalDiseaseState.activeCMV = true;
        this->maternalDiseaseState.isPrevalentCMVCase = true;
    }

    this->maternalDiseaseState.mildIllness = false;

    this->maternalDiseaseState.hadMildIllness = false; // all mothers start with no mild illness
    this->maternalDiseaseState.weekMildIllness = -10000;
    this->maternalDiseaseState.verticalTransmission = false;
    this->maternalDiseaseState.primaryOrSecondary = 0; // primary or secondary transmission starts at 0 for all simulated patients. Will be 1 for primary and 2 for secondary. 


    /* SETTING MATERNAL MONITORING STATES */
    this->maternalMonitoringState.exposureStatus = false;
    this->maternalMonitoringState.knownCMV = false;
    this->maternalMonitoringState.knownHadCMV = false;
    this->maternalMonitoringState.diagnosedCMV = false;
    this->maternalMonitoringState.diagnosedCMVStatus = SimContext::NONE;
    this->maternalMonitoringState.weekCMVDiagnosed = -1; //set to -1 initially for not applicable
    this->maternalMonitoringState.monthCMVDetected = -1; //set to -1 initially for not applicable
    this->maternalMonitoringState.mildIllnessSymptoms = false;
    this->maternalMonitoringState.IgMStatus = false;
    this->maternalMonitoringState.IgGStatus = false;
    this->maternalMonitoringState.CMVSymptoms = false;
    this->maternalMonitoringState.numRegularAppointments = 0; // initializing number of appointments to 0
    this->maternalMonitoringState.numMissedAppointments = 0; // initializing number of missed appointments to 0
    this->maternalMonitoringState.numUltrasounds = 0; // initializing number of ultrasounds to 0
    this->maternalMonitoringState.suspectedCMVStatus = SimContext::NONE;
    this->maternalMonitoringState.suspectedCMVTrimester = -1; //set to -1 initially for not applicable
    this->maternalMonitoringState.performWeeklyIgmIggAvidityOutcomeCheck = true;
    this->maternalMonitoringState.IgmToIggDelay = 0; // initialized to 0. Once IgM test occurs, will use simContext value
    this->maternalMonitoringState.IggToAvidityDelay = 0; // initialized to 0. Once IgG test occurs, will use simContext value



    /* INITIALIZING CHILD STATES */
    this->childState.isAlive = true;
    this->childState.ageWeeks = 0;
    this->childState.ageMonths = 0;
    this->childState.ageYears = 0;
    this->childState.deliveryOcurred = false;
    /** Set biological sex*/
    this->childState.biologicalSex = SimContext::FEMALE;
    randNum = CmvUtil::getRandomDouble();
    if (randNum < simContext->getCohortInputs()->probMale) {
        this->childState.biologicalSex = SimContext::MALE;
    }

    /* INITIALIZING CHILD DISEASE STATES */
    this->childDiseaseState.infectedCMV = false;
    this->childDiseaseState.detectableCMV = false;
    this->childDiseaseState.hadCMV = false;
    this->childDiseaseState.CMVAfterBirth = false;
    this->childDiseaseState.phenotypeCMV = -1;
    this->childDiseaseState.infectionTime = -1; //set to -1 for not in use
    this->childDiseaseState.infectionTrimester = -1;
    this->childDiseaseState.infectionTimeInMonths = -1;
    this->childDiseaseState.primaryOrSecondary = SimContext::TRUE_NO_INFECTION;

    /* INITIALIZING CHILD MONITORING STATES (all to false) */
    this->childMonitoringState.ultrasound = false;
    this->childMonitoringState.amniocentesis = false;
    this->childMonitoringState.observedSymptomatic = false;
    this->childMonitoringState.knownCMV = false;

    
    cmvInfectionUpdater.performInitialization();
    maternalBiologyUpdater.performInitialization();
    pregnancyUpdater.performInitialization();
    prenatalTestUpdater.performInitialization();
    beginWeekUpdater.performInitialization();
    endWeekUpdater.performInitialization();
    
}

Patient::~Patient(void) {
}

/** simulateWeek runs a single week of simulation for this patient, and updates
 * its state and runStats statistics.
*/
void Patient::simulateWeek() {
    /* Disease and General Health updaters called */
    beginWeekUpdater.performWeeklyUpdates();
    cmvInfectionUpdater.performWeeklyUpdates();
    maternalBiologyUpdater.performWeeklyUpdates();
    pregnancyUpdater.performWeeklyUpdates();
    /* Making sure that birth takes place after 41 weeks at maximum */
    if (generalState.weekNum == 41) {
        childState.deliveryOcurred = true;
    }
    // only continue with the other weekly updaters if the child is not born yet and is still alive this week (after rolling for miscarriage/birth/continuation)
    if (this->childState.isAlive && !this->childState.deliveryOcurred) {
        prenatalTestUpdater.performWeeklyUpdates();
        endWeekUpdater.performWeeklyUpdates();
    }
    else {
        endWeekUpdater.performEndPatientUpdates();
    }
}