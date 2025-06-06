#pragma once

#include "include.h"

/**
	The StateUpdater class is the base class for all simulation updater classes in the model.
	All of its child classes should implement the performInitialUpdates and performMonthlyUpdates
	virtual functions to perform the patient initialization and subsequent monthly updates to
	both the patient state and statistics objects.  If these functions are omitted in a
	given child class, the parent functions defined in this class will be used instead.
	The child classes cannot directly modify the patient or statistics information, instead they
	must use the update functions provided in this parent class (which is a friend class to both
	Patient and RunStats).
*/
class StateUpdater
{
public:
	/* Constructor and Destructor */
	StateUpdater(Patient *patient);
	virtual ~StateUpdater(void);

	/**
     * performInitialUpdates performs all the state and stats updates upon patient creation
    */
    virtual void performInitialization();
	/**
	 *  performInitialUpdates perform all of the state and statistics updates upon patient creation
	 **/
	virtual void performInitialUpdates();
	/**
	 *  performWeeklyUpdates perform all of the state and statistics updates for a simulated week
	 **/
	virtual void performWeeklyUpdates();
	/**
	 * performMonthlyUpdates perform all of the state and statistics updates for a simulated month
	*/
	virtual void performMonthlyUpdates();
	/**
	 *  changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model
	 **/
	void setSimContext(SimContext *newSimContext);

protected:

	/** Pointer  to the patient state */
	Patient *patient;
	/** Pointer to the simulation context (i.e. .in file information) */
	SimContext *simContext;
	/** Pointer to the run stats object (i.e. the .out file information) */
	RunStats *runStats;
	/** Pointer to the cost stats object (i.e. the .cout file information) */
	CostStats *costStats;
	/** Pointer to the tracer (i.e. the trace file information) */
	Tracer *tracer;

	void resetMonthNum();
	void resetWeekNum();

	/* setsPatientNumber sets the patient identification number */
	void setPatientNumber(int patientNum);
	/* set tracingEnabled sets whether tracing is enabled for patient */
	void setTracingEnabled(bool tracingEnabled);
	/* setInitialMonth sets which month to start the simulation at */
	void setInitialMonth(int initialMonth);
	/* setBreastFeeding sets whether the mother is currently breastfeeding */
	void setBreastFeeding(bool breastfeeding);
	/* setPatientSubCohort sets the patient's sub cohort */
	void setPatientSubCohort(int subCohort);
	/* incrementWeek updates the pregnancy week number and patient age */
	void incrementWeek();
	/* incrementMonth updates the month number after delivery and patient age */
	void incrementMonth();
	/* Increases the total number of persons who have been initialized by one */
	void incrementCohortSize();
	/* Increases the prenatal test ID number (and by extension number of prenatal tests performed so far) by one */
	void incrementPrenatalTestID();
	/* Sets the trimester of pregnancy */
	void setPregnancyTrimester();
	/* Sets whether prenatal test has been performed during the simulation */
	void setPrenatalTestsPerformed(bool performed, int testIndex);
	/* Sets last prenatal test result */
	void setPrenatalTestsLastResult(int result, int testIndex);
	/* Sets testResult of a Prenatal Test Instance to "true" in the Pending Follow Up Tests patient array */
	void setPendingFollowUpTestPerformed(bool performed, SimContext::PrenatalTestInstance followUpInstance);
	/* Sets testResult of a Prenatal Test Instance to "true" in the Pending Confirmatory Tests patient array */
	void setPendingConfirmatoryTestPerformed(bool performed, SimContext::PrenatalTestInstance confirmatoryInstance);
	/* Sets the week that a prenatal test was last performed in the weekTestLastPerformed array (week, testIndex)*/
	void setWeekTestLastPerformed(int week, int testIndex);
	/* For a given week, sets its test performed value to true for the patient */
	void setAppointmentThisWeek(bool prenatalAppointmentThisWeek);
	/* For a given week, sets its appointment-triggered-by-symptoms to true for the patient */
	/* Sets the week of result return, the test that was performed, and the result of the test (0 for negative/none, 1 for positive/low, 2 for high - only for Avidity) */
	void setPrenatalTestResultReturn(int week, SimContext::PrenatalTestInstance prenatalTestInstance);
	/** Sets whether the patient mother child pair is on treatment */
	void setOnTreatment(bool onTreatment);
	/** Sets the treatment number that the patient mother child pair is on */
	void setMostRecentTreatmentNumber(int treatmentNumber);
	/** Sets the week that treatment was started */
	void setWeekTreatmentStarted(int week, int treatment);
	/** Sets whether patient has had treatment */
	void setHadTreatment(bool hadTreatment, int treatment);


	/* setMaternalAliveStatus sets the life status of the mother */
	void setMaternalIsAlive(bool isAlive);
	/* setPatientAge sets the maternal age*/
	void setMaternalAgeMonths(int ageMonths);

	/* setPrevalentCMV sets whether mother had active CMV at the beginning of the simulation */
	void setPrevalentCMVCase(bool isPrevalentCMV);
	/* setPrevCMVCase sets whether mother has had CMV in the past before the simulation */
	void setPrevCMVCase(bool prevCMV);
	/* setPrevCMVStratum sets the time stratum of how long ago the mother had CMV before the simulation (0 if no previous CMV at model start )*/
	void setPrevCMVStratum(int prevCMVStratum);
	/* setPrevCMVPrimaryOrSecondary sets whether the previous CMV prior to simulation start (if any) was primary or secondary or none */
	void setPrevCMVPrimaryOrSecondary(int prevCMVPrimaryOrSecondary);
	/* setHadCMV sets whether mother has ever had CMV, including during the simulation */
	void setHadCMV(bool hadCMV);
	/* setHadCMVDuringSim sets whether mother has had CMV during the simulation (since the beginning of simulation, not before )*/
	void setHadCMVDuringSim(bool hadCMVDuringSim);
	/* setWeekLastCMV sets the last month that the patient had Active CMV */
	void setWeekLastCMV(int weekLastCMV);
	/* setActiveCMV sets whether patient has active cmv */
	void setActiveCMV(bool activeCMV);
	/* setViremia sets whether patient has CMV viremia */
	void setViremia(bool viremia);
	/* setMaternalIgM sets the mother's TRUE IgM status */
	void setMaternalIgM(bool IgM);
	/* setWeekLastIgMTest sets the week of the last IgM Test */
	void setWeekLastIgMTest(int week);
	/* setMaternalIgG sets the mother's TRUE IgG status */
	void setMaternalIgG(bool IgG);
	/* setWeekLastIgGTest sets the week of the last IgG Test */
	void setWeekLastIgGTest(int week);
	/* setMaternalAvidity sets the mother's TRUE avidity level */
	void setMaternalAvidity(int avidity);
	/* setWeekLastAvidityTest sets the week of the last IgG Avidity test */
	void setWeekLastAvidityTest(int week);
	/* setWeekOfMaternalCMVInfection sets the week number that the mother got the CMV infection. Can be negative (before simulation start) */
	void setWeekOfMaternalCMVInfection(int weekOfMaternalCMVInfection);
	/* setMildIllness sets whether patient has mild illness */
	void setMildIllness(bool mildIllness);
	/* setHadMildIllness sets whether patient has had mild illness during simulation */
	void setHadMildIllness(bool hadMildIllness);
	/* setWeekOfMildIllness sets the week that the last mild illness began */
	void setWeekOfMildIllness(int week);
	/* setVerticalTransmission sets whether vertical transmission occured */
	void setVerticalTransmission(bool verticalTransmission);
	/* setPrimaryOrSecondary sets whether mother's infection was a primary or secondary infection */
	void setPrimaryOrSecondary(int primaryOrSecondary);

	/** MATERNAL MONITORING STATE UPDATERS */
	/* setExposureStatus sets the maternal CMV exposure status */
	void setExposureStatus(bool exposureStatus);
	/* setKnwownCMV sets whether CMV infection is known */
	void setKnownCMV(bool knownCMV);
	/* setKnownHadCMV sets whether previous CMV infection is known */
	void setKnownHadCMV(bool knownHadCMV);
	/* setDiagnosedMaternalCMV sets whether mother has been diagnosed */
	void setDiagnosedMaternalCMV(bool diagnosedCMV);
	/* setKnownPrimaryOrSecondary sets whether it's known that the infection is none, primary, secondary, or unknown */
	void setKnownPrimaryOrSecondary(int primaryOrSecondary);
	/* setWeekMaternalCMVDiagnosed sets the week that CMV was diagnosed for the mother*/
	void setWeekMaternalCMVDiagnosed(int week);
	/* setMonthCMVDetected sets the month that CMV was detected */
	void setMonthCMVDetected(int monthCMVDetected);
	/* setMildIllnessSymptoms sets whether patient is experiencing symptoms from mild illness */
	void setMildIllnessSymptoms(bool mildIllnessSymptoms);
	/* setMaternalViremiaStatus sets the mother's viremia status */
	void setKnownMaternalViremia(bool viremiaStatus);
	/* setMaternalIgM sets the mother's IgM status */
	void setKnownMaternalIgM(bool IgM);
	/* setMaternalIgG sets the mother's IgG status */
	void setKnownMaternalIgG(bool IgG);
	/* setMaternalIgGAvidity sets the mother's IgG avidity level */
	void setKnownAvidity(int avidity);
	/* setPCRWouldBePositive sets whether CMV PCR would be positive */
	void setPCRWouldBePositive(bool PCRWouldBePositive);
	/* setMaternalCMVSymptoms(bool CMVSymptoms)*/
	void setMaternalCMVSymptoms(bool CMVSymptoms);
	/* setNumRegularAppointments */
	void setNumRegularAppointments(int numRegularAppointments);
	/* incrementNumRegularAppointments */
	void incrementNumRegularAppointments();
	/* setNumMissedAppointments sets the number of missed appointments for the mother */
	void setNumMissedAppointments(int numMissedAppointments);
	/* incrementNumMissedAppointments increments the mother's number of missed appointments by 1 */
	void incrementNumMissedAppointments();
	/* setNumMissedAppointments sets the number of ultrasounds for the mother */
	void setNumUltrasounds(int numUltrasounds);
	/* push back Prenatal Follow Up Test to patient's follow up test vector*/
	void pushBackPrenatalFollowUp(SimContext::PrenatalTestInstance prenatalTestInstance);
	/* push back Prenatal Confirmatory Test to patient's confirmatory test vector */
	void pushBackPrenatalConfirmatory(SimContext::PrenatalTestInstance prenatalTestInstance);
	/* push back Prenatal Special Case Test to patient's special case test vector */
	void pushBackPrenatalSpecialCase(SimContext::PrenatalTestInstance prenatalTestInstance);
	/* deleteFromPendingFollowUps*/
	void markPendingFollowUpTestAsDone(int testID);
	/* deleteFromPendingConfirmatory */
	void markPendingConfirmatoryTestAsDone(int testID);
	/* clearTestsNoMoreBaseCase clears the testNoMoreBaseCase integer vector */
	void initializeAvailableBaseCaseTests();
	/* incrementNumMissedAppointments increments the mother's number of ultrasounds by 1 */
	void incrementNumUltrasounds();
	/** setSuspectedCMVStatus sets the clinically suspected CMV status */
	void setSuspectedCMVStatus(int suspectedCMVStatus);
	/** setSuspectedCMVTrimester sets the trimester that CMV was first suspected */
	void setSuspectedCMVTrimester(int trimester);
	/** set whether to do weekly IgM/IgG/Avidity Outcome check  */
	void setPerformWeeklyIgmIggAvidityOutcomeCheck(bool perform);
	/** set current delay between IgM and IgG tests */
	void setIgmToIggDelay(int weeks);
	/** set current delay between IgG and IgG Avidity tests */
	void setIggToAvidityDelay(int weeks);
	


	/* CHILD STATUSES */
	/* setChildIsAlive sets whether child is alive (whether as a fetus or postpartum) */
	void setChildIsAlive(bool isAlive);
	/* setChildAgeWeeks set child's age in weeks */
	void setChildAgeWeeks(int ageWeeks);
	/* setChildAgeMonths sets child's age in months */
	void setChildAgeMonths(int ageMonths);
	/* setChildAgeYears sets child's age in years */
	void setChildAgeYears(int ageYears);
	/* setDeliveryOccurred sets whether delivery of the child has occurred */
	void setDeliveryOccurred(bool deliveryOccurred);
	/* setBiologicalSex sets the child's biological sex */
	void setBiologicalSex(int biologicalSex);
	/* setChildInfectedCMV sets whether child is infected with CMV */
	void setChildInfectedCMV(bool infectedCMV);
	/* setChildDetectableCMV sets whether child's CMV infection is detectable */
	void setChildDetectableCMV(bool detectableCMV);
	/* setChildHadCMV sets whether child has ever had CMV */
	void setChildHadCMV(bool hadCMV);
	/* setCMVAfterBirth sets whether initial CMV infection was before or after birth */
	void setCMVAfterBirth(bool CMVAfterBirth);
	/* setPhenotypeCMV sets the phenotype of CMV in the child */
	void setPhenotypeCMV(int phenotypeCMV);
	/* setTimeOfCMVInfection sets the time of CMV infection */
	void setChildCMVInfectionTime(int infectionTime);
	/* setInfectionTimeInMonths sets whether time of infection is in weeks or months */
	void setInfectionTimeInMonths(bool infectionTimeInMonths);
	/* setChildCMVPrimaryOrSecondary sets whether child's CMV infection is from primary or secondary maternal CMV infection */
	void setChildCMVPrimaryOrSecondary(int primaryOrSecondary);

	/* Child Monitoring State Updaters */
	/* setHadUltrasound sets whether ultrasound has been taken for the fetus */
	void setHadUltrasound(bool ultrasound);
	/* setHadAmniocentesis sets whether fetus has had amniocentesis performed */
	void setHadAmniocentesis(bool amniocentesis);
	/* setChildObservedSymptomaticCMV sets whether the fetus's CMV infection is observed to be symptomatic */
	void setChildObservedSymptomaticCMV(bool observedSymptomatic);
	/* setChildMonitoringHadCMV */
	void setChildKnownCMV(bool knownCMV);

	/* TREATMENT SYMPTOM REDUCTION */
	void symptomReductionAlgorithm();


	/* RUN STATS UPDATERS */
	/** Prenatal Run Stats Statistics Updaters */
	void incrementActiveMaternalCMV(int primaryOrSecondary);
	/** Increment num children alive (NOT including the ones already born) */
	void incrementNumAliveFetuses();
	/** Setting the total number of live births so far in the simulation */
	void setTotalLiveBirthsSoFar(int totalNumLiveBirths);
	/** Increment num live births */
	void incrementNumLiveBirths();

	void incrementNumEverOnAnyTreatment();
	void incrementNumEverOnTreatmentNumber(int treatment);
	/** Increment number of mothers who had treatment at any point who birthed a cCMV POSITIVE child */
	void incrementNumMothersWithTreatmentBirthedCMVPositiveChild(int treatment);
	/** Increment number of mothers who had treatment at any point who birthed a cCMV NEGATIVE child */
	void incrementNumMothersWithTreatmentBirthedCMVNegativeChild(int treatment);
	/** Increment number of cCMV Positive children who were born to mothers who were ever on treatment */
	void incrementNumCMVPositiveChildrenBornWithTreatment();
	/** Increment number of cCMV positive children who were born to mothers who were NEVER on treatment */
	void incrementNumCMVPositiveChildrenBornNeverTreatment();
	void incrementNumCurrOnTreatment(int treatment);
	void incrementNumCMVMothersOnTreatment(int treatment);
	void incrementCMVFetusesOnTreatment(int treatment);

	/* ------------------------------------------------------------------- */

	void incrementMaternalNoCMV();
	void incrementMaternalCMVNoTreatment();
	void incrementMaternalCMVOnTreatment(int treatment);
	void incrementMaternalNoCMVOnTreatment(int treatment);
	void incrementNumFetalNoCMV();
	void incrementMaternalCMVNoFetalCMV();
	void incrementNumMiscarriagesNoFetalCMV();
	void incrementNumMiscarriagesFetalCMV();
	void incrementNumMiscarriagesNoTreatment();
	void incrementNumMiscarriagesFetalCMVOnTreatment(int treatment);
	void incrementNumAppointmentsAttended();
	void incrementNumSymptomaticAppointmentsAttended();
	void incrementNumMildIllnessAppointmentsAttended();
	void incrementNumScheduledPrenatalAppointment();
	void incrementNumTestPerformed(int testNum);
	void incrementNumTruePositives(int testNum);
	void incrementNumTrueNegatives(int testNum);
	void incrementNumFalsePositives(int testNum);
	void incrementNumFalseNegatives(int testNum);
	void incrementNumMaternalPrimaryInfectionInTrimester(int trimester);
	void incrementNumMaternalSecondaryInfectionInTrimester(int trimester);
	void incrementPrimaryVTByTrimester(int trimester);
	void incrementSecondaryVTByTrimester(int trimester);
	void incrementNumTruePositivesFetalCMV(int testNum, int phenotype);
	void incrementNumFalseNegativesFetalCMV(int testNum, int phenotype);
	void incrementNumNewMaternalCMV();
	void incrementNumNewFetalCMV();
	void incrementNumMaternalMildIllness();
	void incrementNumNewMaternalMildIllness();
	void incrementLostMaternalMildIllness();
	void incrementWeeklyNumVerticalTransmissions();
	void incrementBirthsNegativeMaternalCMV();
	void incrementBirthsPositiveMaternalCMV();
	void incrementNewFetalInfectionsByPhenotype(int phenotype);

	/** Maternal Cohort Run Stats Statistics Updaters */
    void incrementMaternalCMVinfections();
	void incrementMaternalPrimaryInfections();
	void incrementMaternalSecondaryInfections();
	void incrementMaternalPrimaryInfectionsAbleToVT();
	void incrementMaternalSecondaryInfectionsAbleToVT();
	void incrementMaternalCMVInfectionsByTrimester(int trimester);
	void incrementMaternalCMVInfectionsBeforeConception(int beforeConception);
	void incrementVerticalTransmissionsPrimaryOrSecondary(int primaryOrSecondary);
    void incrementMaternalMildIllness();
    void setMaternalCMVMildIllness(int numCMVMildIllness);
    void setMaternalCMVNoMildIllness(int numCMVNoMildIllness);
	void incrementMisacarriageNoCMV();
	void incrementMiscarriageWithCMV();
    void setMiscarriageNoCMV(int numMiscarriageNoCMV);
    void setMiscarriageWithCMV(int numMiscarriageWithCMV);
	void incrementMaternalNumDiagnosedCMV();
	void incrementMaternalNumDiagnosedCMVPrimaryOrSecondary(int primaryOrSecondary);
	void incrementMaternalNumDiagnosedTruePos();
	void incrementMaternalNumDiagnosedTruePosPrimaryOrSecondary(int primaryOrSecondary);
	void incrementMaternalNumDiagnosedFalsePos();
	void incrementMaternalNumDiagnosedFalsePosPrimaryOrSecondary(int primaryOrSecondary);
	void incrementWeeklyMaternalNumDiagnosedCMV();
	void incrementWeeklyMaternalNumDiagnosedCMVPrimaryOrSecondary(int primaryOrSecondary);

    /* Child Cohort Run Stats Statistics Updaters */
    void incrementChildCMVInfections(int trimester);
    void incrementChildMildIllness();
    void incrementNumWithCMVByType(int phenotype);
    void incrementSumWeekBirthNoCMV(int weeks);
    void incrementSumWeekBirthWithCMV(int weeks);
	void incrementNumBirthsNoCMV();
	//void incrementNumBirthsWithCMV();
	void incrementNumFetalDiagnosedCMV();
	void incrementNumFetalDeathsAtBirth();

private:
	/* getPrenatalTimeSummary returns a non-const pointer to the PrenatalTimeSummary object for the current time period, 
	creates a new one if needed. */
	RunStats::PrenatalTimeSummary *getPrenatalTimeSummaryForUpdate();

};
