#include "include.h"
#include <string>

/* Constructor takes run name as parameter */
SimContext::SimContext(string runName) {
	counter=0;
	inputFileName = runName;
	inputFileName.append(".in");
	runSpecsInputs.runName = runName;
}
/* Destructor cleans up the allocated memory*/
SimContext::~SimContext(void) {
    // clean up allocated memory here
}


/* Initialize the number of patients to trace to 50 */
int SimContext::numPatientsToTrace = 50;

/* Initialize the constant character strings */
const char *SimContext::PRENATAL_TEST_NAMES[] = {
	"Routine_US", "Detailed_US", "Amniocentesis", "Maternal_PCR", "IgM", "IgG", "Avidity"
};
const char *SimContext::PRENATAL_TEST_TYPE_NAMES[] = {
	"Base Test", "Confirmatory Test", "Follow-Up Test", "Special Case Triggered Test"
};
const char *SimContext::TEST_OUTCOME[] = {
	"Negative", "Positive"
};
const char *SimContext::AVIDITY_OUTCOME[] = {
	"No Avidity", "Low", "High"
};
const char *SimContext::PRIMARY_OR_SECONDARY_CHAR[] = {
	"No Infection", "Primary", "Secondary", "Unknown"
};
const char *SimContext::LAST_GOT_CMV_STRATA_CHAR[] = {
	"VeryRecentCMV", "MediumRecentCMV", "LessRecentCMV", "LongTimeAgoCMV", "NeverCMV"
};

// Default constructor for PrenatalTestState
SimContext::PrenatalTestState::PrenatalTestState(int testID) {
    this->testID = testID;
}

SimContext::PrenatalTestState::~PrenatalTestState(){

}

void SimContext::readInputs() {
	CmvUtil::changeDirectoryToInputs();
	inputFile = CmvUtil::openFile(inputFileName.c_str(), "r");
	if (inputFile == NULL) {
		string errorString = "	ERROR - Could not open input file ";
		errorString.append(inputFileName);
		throw errorString;
	}

	/* Read all the input data from the file*/
	readRunSpecsInputs();
	readCohortInputs();
	readPrenatalNatHistInputs();
	readHealthcareInteractionsScheduleInputs();
	readPrenatalTestSpecs();
	readPrenatalTreatmentInputs();
	readPostPartumInputs();
	readInfantInputs();
	readAdultInputs();

	/* Close the input file */
	CmvUtil::closeFile(inputFile);
}

/* readRunSpecsInputs reads data from the RunSpecs tab of the input sheet */
void SimContext::readRunSpecsInputs() {
	char buffer[256];
	// read in name of set this run belongs to 
	readAndSkipPast("Runset", inputFile);
	fscanf(inputFile, "%299s", buffer);
	runSpecsInputs.runSetName = buffer;
	// read in cohort size
	readAndSkipPast("CohortSize", inputFile);
	fscanf(inputFile, "%d", &runSpecsInputs.numCohorts);
	// read in annual discount factor
	readAndSkipPast("AnnualDiscountFactor", inputFile);
	fscanf(inputFile, "%lf", &runSpecsInputs.annualDiscountFactor);
	// random initial seed by time
	readAndSkipPast("RandomInitialSeedByTime", inputFile);
	fscanf(inputFile, "%d", &runSpecsInputs.randomSeedByTime);
	// last active CMV Strata
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[0], inputFile); // "VERY RECENT" (on input sheet)
	fscanf(inputFile, "%d", &runSpecsInputs.lastActiveCMVStrataBounds[0]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[1], inputFile); // "MEDIUM RECENT" (on input sheet)
	fscanf(inputFile, "%d", &runSpecsInputs.lastActiveCMVStrataBounds[1]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[2], inputFile); // "LESS RECENT" (on input sheet)
	fscanf(inputFile, "%d", &runSpecsInputs.lastActiveCMVStrataBounds[2]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[3], inputFile); // "LONG TIME AGO" (on input sheet)
	fscanf(inputFile, "%d", &runSpecsInputs.lastActiveCMVStrataBounds[3]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[4], inputFile); // "NEVER CMV" (on input sheet)
	fscanf(inputFile, "%d", &runSpecsInputs.lastActiveCMVStrataBounds[4]);


	// read number of patients to trace
	readAndSkipPast("NumPatientsTrace", inputFile);
	fscanf(inputFile, "%d", &runSpecsInputs.numberOfPatientsToTrace);
	}

/* readCohortInputs reads data from the Cohort tab of the input sheet */
void SimContext::readCohortInputs() {
	char buffer[256];
	int i, j, strata, ageYears, week, tempBool, tempInt;
	// read Age Mean
	readAndSkipPast("AgeMean", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.ageMonthsMean);
	// read Age Std Dev
	readAndSkipPast("AgeStdDev", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.ageMonthsStdDev);
	// read Minimum Age
	readAndSkipPast("AgeMinimum", inputFile);
	fscanf(inputFile, "%d", &cohortInputs.minimumAgeMonths);
	// read Maximum Age
	readAndSkipPast("AgeMaximum", inputFile);
	fscanf(inputFile, "%d", &cohortInputs.maximumAgeMonths);
	/* read maternal age strata bounds */
	readAndSkipPast("MaternalAgeStrataBounds", inputFile);
	for (i = 0; i < NUM_AGE_STRATA_BOUNDS; i++) {
		fscanf(inputFile, "%d", &cohortInputs.maternalAgeStrataBounds[i]);
	}

	// read last got CMV statistics
	readAndSkipPast("LastGotActiveCMV", inputFile);
	for (i = 0; i < NUM_LAST_CMV_STRATA; i++) {
		readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[i], inputFile);
		fscanf(inputFile, "%lf", &cohortInputs.lastGotActiveCMVByStrata[i]);
	}
	readAndSkipPast("NeverCMV", inputFile);

	fscanf(inputFile, "%lf", &cohortInputs.lastGotActiveCMVByStrata[NUM_LAST_CMV_STRATA]);
	

	/* setting probabilities to 1 from last age bound to age 100 (guaranteed miscarriage after a certain age)*/

	for (ageYears = cohortInputs.maternalAgeStrataBounds[-1]+1; ageYears < AGE_YEARS; ageYears++) {
		for (week = 0; week < 41; week++) {
			cohortInputs.weeklyProbBirth[ageYears][week] = 1; 
		}
	}
	
	/** Probability that previous maternal CMV (from before model star) was a primary infection */
	readAndSkipPast("LastActiveCMVPrimaryOrSecondary", inputFile);
	readAndSkipPast("VeryRecentPrimaryOrSecondary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probPrevCMVWasPrimary[VERY_RECENT_CMV]);
	readAndSkipPast("MediumRecentPrimaryOrSecondary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probPrevCMVWasPrimary[MEDIUM_RECENT_CMV]);
	readAndSkipPast("LessRecentPrimaryOrSecondary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probPrevCMVWasPrimary[LESS_RECENT_CMV]);
	readAndSkipPast("LongTimeAgoPrimaryOrSecondary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probPrevCMVWasPrimary[LONG_TIME_AGO_CMV]);


	/** One-Time CMV Vertical Transmission for mothers who had CMV infection that began before model start */
	/** Probability of One-Time Vertical Transmission for Primary Maternal Infection*/
	readAndSkipPast("PrevPrimaryOneTimeVerticalTransmission", inputFile);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[VERY_RECENT_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_PRIMARY][VERY_RECENT_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_PRIMARY][VERY_RECENT_CMV]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[MEDIUM_RECENT_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_PRIMARY][MEDIUM_RECENT_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_PRIMARY][MEDIUM_RECENT_CMV]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[LESS_RECENT_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_PRIMARY][LESS_RECENT_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_PRIMARY][LESS_RECENT_CMV]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[LONG_TIME_AGO_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_PRIMARY][LONG_TIME_AGO_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_PRIMARY][LONG_TIME_AGO_CMV]);

	/** Probability of One-Time Vertical Transmission for Secondary Maternal Infection*/
	readAndSkipPast("PrevSecondaryOneTimeVerticalTransmission", inputFile);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[VERY_RECENT_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_SECONDARY][VERY_RECENT_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_SECONDARY][VERY_RECENT_CMV]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[MEDIUM_RECENT_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_SECONDARY][MEDIUM_RECENT_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_SECONDARY][MEDIUM_RECENT_CMV]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[LESS_RECENT_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_SECONDARY][LESS_RECENT_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_SECONDARY][LESS_RECENT_CMV]);
	readAndSkipPast(SimContext::LAST_GOT_CMV_STRATA_CHAR[LONG_TIME_AGO_CMV], inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.oneTimeVerticalTransmissionProb[TRUE_SECONDARY][LONG_TIME_AGO_CMV]);
	fscanf(inputFile, "%d", &cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_SECONDARY][LONG_TIME_AGO_CMV]);

	// initializing One-Time Vertical Transmission from previous infection probabilities and week numbers to 0 if mother has never had infection.
	/** Initializing to 0 since these probabilities will never be called, they are just here to preserve the ability for the array to be properly
	 * indexed by enumerated infection status. If there is no infection (enumerated 0), there is NEVER ANY probability of vertical transmission.
	*/
	cohortInputs.oneTimeVerticalTransmissionProb[TRUE_NO_INFECTION][VERY_RECENT_CMV] = 0;
	cohortInputs.oneTimeVerticalTransmissionProb[TRUE_NO_INFECTION][MEDIUM_RECENT_CMV] = 0;
	cohortInputs.oneTimeVerticalTransmissionProb[TRUE_NO_INFECTION][LESS_RECENT_CMV] = 0;
	cohortInputs.oneTimeVerticalTransmissionProb[TRUE_NO_INFECTION][LONG_TIME_AGO_CMV] = 0;

	cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_NO_INFECTION][VERY_RECENT_CMV] = 0;
	cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_NO_INFECTION][MEDIUM_RECENT_CMV] = 0;
	cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_NO_INFECTION][LESS_RECENT_CMV] = 0;
	cohortInputs.oneTimeVerticalTansmissionWeek[TRUE_NO_INFECTION][LONG_TIME_AGO_CMV] = 0;

	/** Probability of symptomatic maternal CMV infection */
	readAndSkipPast("ProbSymptomaticMaternalInfection", inputFile);
	cohortInputs.probSymptomsWithPrimaryCMV[PRECONCEPTION] = 0;
	cohortInputs.probSymptomsWithSecondaryCMV[PRECONCEPTION] = 0;
	readAndSkipPast("Trimester1Primary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probSymptomsWithPrimaryCMV[TRIMESTER_1]);
	readAndSkipPast("Trimester1Secondary", inputFile); 
	fscanf(inputFile, "%lf", &cohortInputs.probSymptomsWithSecondaryCMV[TRIMESTER_1]);
	readAndSkipPast("Trimester2Primary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probSymptomsWithPrimaryCMV[TRIMESTER_2]);
	readAndSkipPast("Trimester2Secondary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probSymptomsWithSecondaryCMV[TRIMESTER_2]);
	readAndSkipPast("Trimester3Primary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probSymptomsWithPrimaryCMV[TRIMESTER_3]);
	readAndSkipPast("Trimester3Secondary", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probSymptomsWithSecondaryCMV[TRIMESTER_3]);
	// child cohort characteristics

	readAndSkipPast("PeriConception", inputFile);
	readAndSkipPast("VTFromPrimaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_PRIMARY][PRECONCEPTION][i]);
		cohortInputs.probPhenotypesUponVT[TRUE_NO_INFECTION][PRECONCEPTION][i] = 0; // initialized to 0 to preserve ability for array to be properly indexed by enumerated infection status
	}
	readAndSkipPast("VTFromSecondaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_SECONDARY][PRECONCEPTION][i]);
	}
		
	readAndSkipPast("Trimester1", inputFile);
	readAndSkipPast("VTFromPrimaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_PRIMARY][TRIMESTER_1][i]);
		cohortInputs.probPhenotypesUponVT[TRUE_NO_INFECTION][TRIMESTER_1][i] = 0; // initialized to 0 to preserve ability for array to be properly indexed by enumerated infection status
	}
	readAndSkipPast("VTFromSecondaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_SECONDARY][TRIMESTER_1][i]);
	}
	readAndSkipPast("Trimester2", inputFile);
	readAndSkipPast("VTFromPrimaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_PRIMARY][TRIMESTER_2][i]);
		cohortInputs.probPhenotypesUponVT[TRUE_NO_INFECTION][TRIMESTER_2][i] = 0; // initialized to 0 to preserve ability for array to be properly indexed by enumerated infection status
	}
	readAndSkipPast("VTFromSecondaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_SECONDARY][TRIMESTER_2][i]);
	}
	readAndSkipPast("Trimester3", inputFile);
	readAndSkipPast("VTFromPrimaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_PRIMARY][TRIMESTER_3][i]);
		cohortInputs.probPhenotypesUponVT[TRUE_NO_INFECTION][TRIMESTER_3][i] = 0; // initialized to 0 to preserve ability for array to be properly indexed by enumerated infection status
	}
	readAndSkipPast("VTFromSecondaryInfection", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &cohortInputs.probPhenotypesUponVT[TRUE_SECONDARY][TRIMESTER_3][i]);
	}
	readAndSkipPast("ProbMale", inputFile);
	fscanf(inputFile, "%lf", &cohortInputs.probMale);
	readAndSkipPast("EnableCustomAgeDist", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.enableCustomAgeDist = tempBool;
	// custom age distribution
	if (cohortInputs.enableCustomAgeDist) {
		for (i = 0; i < NUM_CUSTOM_AGE_STRATA; i++) {
			string searchString = "Stratum" + to_string(i);
			readAndSkipPast(searchString.c_str(), inputFile);
			fscanf(inputFile, "%d", &cohortInputs.customAgeDistMonthStrata[i]);
			if (i == NUM_CUSTOM_AGE_STRATA - 1) { // filling in the maximum month for the maximum stratum for the entire distribution
				fscanf(inputFile, "%d", &cohortInputs.customAgeDistStrataProbs[NUM_CUSTOM_AGE_STRATA]);
			}
			else { // if not at the last stratum, ignore the upper bound, since it can be determined by the subsequent lower bound. 
				fscanf(inputFile, "%d", &tempInt);
			}
			// Read in the probability for the particular stratum. 
			fscanf(inputFile, "%lf", &cohortInputs.customAgeDistStrataProbs[i]);
		}
	}
	// low avidty duration
	cohortInputs.lowAvidityDurationMean = 13;
	cohortInputs.lowAvidityDurationStdDev = (double)(52/12)*0.5;
	/** Baseline Probabilities of Premature Birth */
	readAndSkipPast("WeeklyProbabilityBirth", inputFile);
	// looping over all age ranges

	/** Weekly Probabilities of Birth (no CMV)*/
	
	for (week = 0; week < MAX_WEEKS_PREGNANT; week++) { 
		string weekName = "Week" + to_string(week);
		readAndSkipPast(weekName.c_str(), inputFile);
		for (strata = 0; strata < NUM_AGE_STRATA; strata++) {
			double prob;
			fscanf(inputFile, "%lf", &prob);
			for (ageYears = cohortInputs.maternalAgeStrataBounds[strata]; ageYears < cohortInputs.maternalAgeStrataBounds[strata+1]; ageYears++) {
				cohortInputs.weeklyProbBirth[ageYears][week] = prob;
			}
			
		}
	}
	/** CMV Positive Probabilities of Premature Birth*/
	readAndSkipPast("CMVPositiveWeeklyProbabilityBirth", inputFile);
	// looping over all age ranges 
	
	for (week = 2; week < MAX_WEEKS_PREGNANT; week++) { 
		string weekName = "Week" + to_string(week);
		readAndSkipPast(weekName.c_str(), inputFile);
		for (strata = 0; strata < NUM_AGE_STRATA; strata++) {
			double prob;
			fscanf(inputFile, "%lf", &prob);
			for (ageYears = cohortInputs.maternalAgeStrataBounds[strata]; ageYears < cohortInputs.maternalAgeStrataBounds[strata+1]; ageYears++) {
				cohortInputs.CMVPositiveWeeklyProbBirth[ageYears][week] = prob;
			}
			
		}
	}
}

/* readMaternalCohortInputs reads data from the MaternalCohort tab of the input sheet */
void SimContext::readPrenatalNatHistInputs() {
	int i, j, week, strata, ageYears;
	readAndSkipPast("PrenatalNatHist", inputFile);
	readAndSkipPast("WeeklyPrimaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeklyPrimaryCMV);
	readAndSkipPast("WeeklySecondaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeklySecondaryCMV);
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeklyProbMildIllness);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeklyProbKnownInfection);

	/** Maternal Biological Markers Timing */
	readAndSkipPast("TimeToViremiaFromInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToViremiaFromInfectionMean);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToViremiaFromInfectionStdDev);
	readAndSkipPast("LengthOfViremia", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.lengthOfViremiaMean);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.lengthOfViremiaStdDev);
	readAndSkipPast("TimeToIgMPositiveFromInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToIgmPositiveFromInfectionMean);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToIgmPositiveFromInfectionStdDev);
	readAndSkipPast("LengthOfIgMPositivity", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.lengthOfIgmPositivityMean);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.lengthOfIgmPositivityStdDev);
	readAndSkipPast("TimeToIgGPositiveFromInfection", inputFile);///
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToIggPositiveFromInfectionMean);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToIggPositiveFromInfectionStdDev);
	readAndSkipPast("TimeToLowAvidityFromInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToLowAvidityFromInfectionMean);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToLowAvidityFromInfectionStdDev);
	readAndSkipPast("TimeToHighAvidityFromLowAvidity", inputFile);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToHighAvidityFromLowAvidityMean);
	fscanf(inputFile, "%lf", &prenatalNatHistInputs.weeksToHighAvidityFromLowAvidityStdDev);
	readAndSkipPast("MaternalInfectionToVerticalTransmissionDelay", inputFile);
	fscanf(inputFile, "%d", &prenatalNatHistInputs.maternalInfectionToVerticalTransmissionDelay);
	readAndSkipPast("VerticalTransmissionToDetectableDelay", inputFile);
	fscanf(inputFile, "%d", &prenatalNatHistInputs.verticalTransmissionToDetectableDelay);
	
	//readAndSkipPast("PrimaryMaternalVerticalTransmission", inputFile);
	string weekstr = "week";
	string weekNum;
	string weekNumName;
	const char* weekNumNameChar; 
	/* looping over child weekly primary CMV probabilities to add to array */
	/*
	for (i = 0; i < MAX_WEEKS_PREGNANT; i++) {
		weekNum = std::to_string(i);
		weekNumName = weekstr + weekNum;
		weekNumNameChar = weekNumName.c_str();
		readAndSkipPast(weekNumNameChar, inputFile);

		fscanf(inputFile, "%lf", &prenatalNatHistInputs.childWeeklyPrimaryCMV[i]);
		
	}
	readAndSkipPast("SecondaryMaternalVerticalTransmission", inputFile);
	*/
	/* looping over child weekly secondary CMV probabilities to add to array */
	/*
	for (i = 2; i < MAX_WEEKS_PREGNANT; i++) {
		weekNum = std::to_string(i);
		weekNumName = weekstr + weekNum;
		weekNumNameChar = weekNumName.c_str();
		readAndSkipPast(weekNumNameChar, inputFile);

		fscanf(inputFile, "%lf", &prenatalNatHistInputs.childWeeklySecondaryCMV[i]);
		
	}
	*/
	/* reading probabilities of vertical transmission stratified by trimester and primary/secondary infection */
	readAndSkipPast("PrimaryMaternalVerticalTransmissionByTrimester", inputFile);
	prenatalNatHistInputs.primaryVerticalTransmission[0] = 0;
	for (i = 1; i < NUM_TRIMESTERS; i++) {
		fscanf(inputFile, "%lf", &prenatalNatHistInputs.primaryVerticalTransmission[i]);
		cout << prenatalNatHistInputs.primaryVerticalTransmission[i] << endl;
	}

	readAndSkipPast("SecondaryMaternalVerticalTransmissionByTrimester", inputFile);
	prenatalNatHistInputs.secondaryVerticalTransmission[0] = 0;
	for (i = 1; i < NUM_TRIMESTERS; i++) {
		fscanf(inputFile, "%lf", &prenatalNatHistInputs.secondaryVerticalTransmission[i]);
	}

	/* Background Weekly Probability of Miscarriage */
	// looping over age ranges
	readAndSkipPast("BackgroundWeeklyProbabilityMiscarriage", inputFile);
	for (strata = 0; strata < NUM_AGE_STRATA; strata++) {
		string ageRange = "AgeRange" + to_string(strata+1);
		readAndSkipPast(ageRange.c_str(), inputFile);
		for (week = 2; week < MAX_WEEKS_PREGNANT; week++) { 
			weekNum = std::to_string(week);
			weekNumName = "week" + weekNum;
			weekNumNameChar = weekNumName.c_str();
			readAndSkipPast(weekNumNameChar, inputFile);

			double prob;
			fscanf(inputFile, "%lf", &prob);
			for (ageYears = cohortInputs.maternalAgeStrataBounds[strata]; ageYears < cohortInputs.maternalAgeStrataBounds[strata+1]; ageYears++) {
				prenatalNatHistInputs.backgroundMiscarriage[ageYears][week] = prob;
			}
			
		}
	}
	readAndSkipPast("CMVPositiveWeeklyProbabilityMiscarriage", inputFile);
	/* CMV Positive Weekly Probability of Miscarriage */
	// looping over age ranges
	for (strata = 0; strata < NUM_AGE_STRATA; strata++) {
		string ageRange = "AgeRange" + to_string(strata+1);
		readAndSkipPast(ageRange.c_str(), inputFile);
		for (week = 2; week < MAX_WEEKS_PREGNANT; week++) { 
			string weekName = "week" + to_string(week);
			readAndSkipPast(weekName.c_str(), inputFile);

			double prob;
			fscanf(inputFile, "%lf", &prob);
			for (ageYears = cohortInputs.maternalAgeStrataBounds[strata]; ageYears < cohortInputs.maternalAgeStrataBounds[strata+1]; ageYears++) {
				prenatalNatHistInputs.CMVPositiveMiscarriage[ageYears][week] = prob;
			}
			
		}
	}

	readAndSkipPast("FetalDeathRiskDuringBirth", inputFile);
	for (week = 0; week < MAX_WEEKS_PREGNANT; week++) {
		string weekName = "week" + to_string(week);
		readAndSkipPast(weekName.c_str(), inputFile);
		fscanf(inputFile, "%lf", &prenatalNatHistInputs.fetalDeathRiskDuringBirth[week][CMV_NEGATIVE]);
		fscanf(inputFile, "%lf", &prenatalNatHistInputs.fetalDeathRiskDuringBirth[week][CMV_POSITIVE]);
	}
}

/* readBackgroundScreeningInputs reads data from the BackgroundScreening tab of the input sheet */
void SimContext::readHealthcareInteractionsScheduleInputs() {
	int i, j, numAppointments, test, tempBool;
	double probAttendance;
	int everyNWeeks;
	int weeksInWeekRange;

	char tmpBuf[256];
	string week = "week";
	string weekNum, weekNumName;
	const char* weekNumNameChar;
	readAndSkipPast("HealthcareInteractionSchedule", inputFile);

	readAndSkipPast("ProbSeekingHealthcare", inputFile);
	readAndSkipPast("SymptomaticCMV", inputFile);

	fscanf(inputFile, "%lf", &backgroundScreeningInputs.seekingHealthcareSymptomatic);

	readAndSkipPast("MildIllness", inputFile);

	fscanf(inputFile, "%lf", &backgroundScreeningInputs.seekingHealthcareMildIllness);
	
	readAndSkipPast("KnownInfection", inputFile);

	fscanf(inputFile, "%lf", &backgroundScreeningInputs.seekingHealthcareKnownInfection);
	
	readAndSkipPast("LikelihoodReportingSymptoms", inputFile);

	fscanf(inputFile, "%d", &backgroundScreeningInputs.probReportingSymptoms);

	/* Regular Background Screening by Week (no CMV diagnoses) */

	/** week 0 values are all 0*/
	backgroundScreeningInputs.weeksWithScheduledAppointments[0] = false;
	backgroundScreeningInputs.probAttendingRegularAppointmentsByWeek[0] = 0;
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		backgroundScreeningInputs.backgroundTesting[0][i] = 0;
	}
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		sprintf(tmpBuf, "week%d", i);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf(inputFile, "%d", &tempBool);
		backgroundScreeningInputs.weeksWithScheduledAppointments[i] = tempBool;
		fscanf(inputFile, "%lf", &backgroundScreeningInputs.probAttendingRegularAppointmentsByWeek[i]);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			fscanf(inputFile, "%d", &tempBool);
			backgroundScreeningInputs.backgroundTesting[i][j] = tempBool;
		}
	}

	/* Screening Upon Maternal CMV */
	readAndSkipPast("ScreeningUponMaternalCMV", inputFile);

	// For week 0, set all inputs to 0, since nothing can happen during week 0.
	for (i = 0; i < 5; i++) {
		backgroundScreeningInputs.weeksWithVisitsUponMaternalDiagnosis[0][i] = false;
		backgroundScreeningInputs.probAttendUponMaternalDiagnosis[0][i] = 0;
		for (int j = 0; j < NUM_PRENATAL_TESTS; j++) {
			backgroundScreeningInputs.testingUponMaternalDiagnosis[0][j][i] = false;
		}
	}

	// In the case where there is no CMV diagnosis for mother, set all test values to 0. These values will never be called upon anyways. 
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		backgroundScreeningInputs.weeksWithVisitsUponMaternalDiagnosis[i][NONE] = false;
		backgroundScreeningInputs.probAttendUponMaternalDiagnosis[i][NONE] = 0;
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			backgroundScreeningInputs.testingUponMaternalDiagnosis[i][j][NONE] = false;
		}
	}

	readAndSkipPast("PrimaryMaternalCMV", inputFile);
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		sprintf(tmpBuf, "week%d", i);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf(inputFile, "%d", &tempBool);
		backgroundScreeningInputs.weeksWithVisitsUponMaternalDiagnosis[i][PRIMARY] = tempBool;
		fscanf(inputFile, "%lf", &backgroundScreeningInputs.probAttendUponMaternalDiagnosis[i][PRIMARY]);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			fscanf(inputFile, "%d", &tempBool);
			backgroundScreeningInputs.testingUponMaternalDiagnosis[i][j][PRIMARY] = tempBool;
		}
	}


	readAndSkipPast("SecondaryMaternalCMV", inputFile);
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		sprintf(tmpBuf, "week%d", i);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf(inputFile, "%d", &tempBool);
		backgroundScreeningInputs.weeksWithVisitsUponMaternalDiagnosis[i][SECONDARY] = tempBool;
		fscanf(inputFile, "%lf", &backgroundScreeningInputs.probAttendUponMaternalDiagnosis[i][SECONDARY]);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			fscanf(inputFile, "%d", &tempBool);
			backgroundScreeningInputs.testingUponMaternalDiagnosis[i][j][SECONDARY] = tempBool;
		}
	}

	readAndSkipPast("RecentUnknownMaternalCMV", inputFile);
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		sprintf(tmpBuf, "week%d", i);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf(inputFile, "%d", &tempBool);
		backgroundScreeningInputs.weeksWithVisitsUponMaternalDiagnosis[i][RECENT_UNKNOWN] = tempBool;
		fscanf(inputFile, "%lf", &backgroundScreeningInputs.probAttendUponMaternalDiagnosis[i][RECENT_UNKNOWN]);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			fscanf(inputFile, "%d", &tempBool);
			backgroundScreeningInputs.testingUponMaternalDiagnosis[i][j][RECENT_UNKNOWN] = tempBool;
		}
	}

	readAndSkipPast("LongAgoUnknownMaternalCMV", inputFile);
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		sprintf(tmpBuf, "week%d", i);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf(inputFile, "%d", &tempBool);
		backgroundScreeningInputs.weeksWithVisitsUponMaternalDiagnosis[i][LONG_AGO_UNKNOWN] = tempBool;
		fscanf(inputFile, "%lf", &backgroundScreeningInputs.probAttendUponMaternalDiagnosis[i][LONG_AGO_UNKNOWN]);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			fscanf(inputFile, "%d", &tempBool);
			backgroundScreeningInputs.testingUponMaternalDiagnosis[i][j][LONG_AGO_UNKNOWN] = tempBool;
		}
	}

	/* Prenatal Screening Upon Fetus/Child Diagnosed With CMV */
	readAndSkipPast("ScreeningUponChildCMV", inputFile);

	// For week 0, set all inputs to 0, since nothing can happen during week 0.
	backgroundScreeningInputs.weeksWithVisitsUponChildDiagnosis[0][ASYMPTOMATIC] = false;
	backgroundScreeningInputs.weeksWithVisitsUponChildDiagnosis[0][SYMPTOMATIC] = false;
	backgroundScreeningInputs.probAttendUponChildDiagnosis[0][ASYMPTOMATIC] = 0;
	backgroundScreeningInputs.probAttendUponChildDiagnosis[0][SYMPTOMATIC] = 0;
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		backgroundScreeningInputs.testingUponChildDiagnosis[0][i][ASYMPTOMATIC] = false;
		backgroundScreeningInputs.testingUponChildDiagnosis[0][i][SYMPTOMATIC] = false;
	}


	readAndSkipPast("AsymptomaticFetalInfection", inputFile);
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		sprintf(tmpBuf, "week%d", i);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf(inputFile, "%d", &tempBool);
		backgroundScreeningInputs.weeksWithVisitsUponChildDiagnosis[i][ASYMPTOMATIC] = tempBool;
		fscanf(inputFile, "%lf", &backgroundScreeningInputs.probAttendUponChildDiagnosis[i][ASYMPTOMATIC]);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			fscanf(inputFile, "%d", &tempBool);
			backgroundScreeningInputs.testingUponChildDiagnosis[i][j][ASYMPTOMATIC] = tempBool;
		}
	}

	readAndSkipPast("SymptomaticFetalInfection", inputFile);
	for (i = 1; i < MAX_WEEKS_PREGNANT; i++) {
		sprintf(tmpBuf, "week%d", i);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf(inputFile, "%d", &tempBool);
		backgroundScreeningInputs.weeksWithVisitsUponChildDiagnosis[i][SYMPTOMATIC] = tempBool;
		fscanf(inputFile, "%lf", &backgroundScreeningInputs.probAttendUponChildDiagnosis[i][SYMPTOMATIC]);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
			fscanf(inputFile, "%d", &tempBool);
			backgroundScreeningInputs.testingUponChildDiagnosis[i][j][SYMPTOMATIC] = tempBool;
		}
	}
	/* OUTPUT VERIFYING THAT THE BACKGROUND SCREENING INPUTS ARE READ CORRECTLY 
	for (i = 0; i < MAX_WEEKS_PREGNANT; i++) {
		for (test = 0; test < NUM_PRENATAL_TESTS; test++) {
			cout << backgroundScreeningInputs.weeksPerformTestsDiagnosedChildCMV[i][test][0] << "   ";
		}
		cout << endl;
	}
	cin.get();
	*/
}

/* readChildCohortInputs reads data from the ChildCohort tab of the input sheet */
void SimContext::readPrenatalTestSpecs() {
	int i, j, test, tempBool;
	string tempString;
	string week = "week";
	string weekNum;
	string weekNumName;
	const char* weekNumNameChar;
	readAndSkipPast("PrenatalTestSpecs", inputFile);
	/* DEFINING INDIVIDUAL PRENATAL TESTS */

	/* Prenatal Test 1 */
	test = ROUTINE_US;

	readAndSkipPast("PrenatalTest1", inputFile);
	prenatalTestingInputs.routineUltrasound.testArray = ROUTINE_US;
	/** Special case triggering*/
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.mildIllnessTrigger);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.knownInfectionTrigger);
	readAndSkipPast("SymptomaticCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.symptomaticCMVTrigger);
	//readAndSkipPast("PreviousTest", inputFile);
	//fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.previousTestTrigger);
	/** Acceptance of prenatal test*/
	readAndSkipPast("OfferAndAcceptanceOfPrenatalTest", inputFile);
	readAndSkipPast("BaselineProbOfferedTest", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.probOfferedAndAccept);
	
	readAndSkipPast("PrenatalTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.costOfTest);
	
	/** Prenatal Test Characteristics */
	readAndSkipPast("ProbResultReturnToPatient", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.probResultReturn);
	readAndSkipPast("WeeksToResultReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.weeksToResultReturn);
	readAndSkipPast("WeeksToRepeatTestIfNoReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.weeksToRepeatTestIfNoReturn);
	
	readAndSkipPast("MinimumTestInterval", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.minimumTestInterval);
	readAndSkipPast("ProbTerminationUponAbnormalResult", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.probTerminationUponAbnormal);
	readAndSkipPast("AvailableStartingWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.availableStartingWeek);
	readAndSkipPast("NotAvailableAfterWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.notAvailableAfter);
	readAndSkipPast("SensitivityTrimester1", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.testSensitivity[TRIMESTER_1][i]);
	}
	readAndSkipPast("SensitivityTrimester2", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.testSensitivity[TRIMESTER_2][i]);
	}
	readAndSkipPast("SensitivityTrimester3", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.testSensitivity[TRIMESTER_3][i]);
	}
	readAndSkipPast("SpecificityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.testSpecificity[TRIMESTER_1]);
	readAndSkipPast("SpecificityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.testSpecificity[TRIMESTER_2]);
	readAndSkipPast("SpecificityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.testSpecificity[TRIMESTER_3]);
	/** Confirmatory testing */
	
	readAndSkipPast("ConfirmatoryTesting", inputFile);
	readAndSkipPast("NumRepeatTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.numRepeatedConfirmatory);
	readAndSkipPast("ProbSchedulingConfirmatoryTests", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.probSchedulingConfirmatory);
	readAndSkipPast("WeeksBetweenConfirmatoryTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.weeksBetweenConfirmatoryTests);
	readAndSkipPast("AdditionalConfirmatoryTestCost", inputFile);

	fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.additionalCostConfirmatoryTest);
	
	
	/** Follow Up Testing */
	readAndSkipPast("FollowUpTestingBaseTest", inputFile);
	readAndSkipPast("AbnormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.routineUltrasound.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.routineUltrasound.followUpTests[i][ABNORMAL] = tempBool;
		}
	}
	
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.routineUltrasound.followUpTestsProbabilities[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.followUpTestsProbabilities[i][ABNORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.routineUltrasound.followUpTestDelays[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.followUpTestDelays[i][ABNORMAL]);
		}
	}
	readAndSkipPast("NormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.routineUltrasound.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.routineUltrasound.followUpTests[i][NORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.routineUltrasound.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.routineUltrasound.followUpTestsProbabilities[i][NORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.routineUltrasound.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.routineUltrasound.followUpTestDelays[i][NORMAL]);
		}
	}
	
	
	/* Prenatal Test 2 */
	test = DETAILED_US;
	readAndSkipPast("PrenatalTest2", inputFile);
	prenatalTestingInputs.detailedUltrasound.testArray = DETAILED_US;

	/** Special case triggering*/
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.mildIllnessTrigger);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.knownInfectionTrigger);
	readAndSkipPast("SymptomaticCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.symptomaticCMVTrigger);
	//readAndSkipPast("PreviousTest", inputFile);
	//fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.previousTestTrigger);
	/** Acceptance of prenatal test*/
	readAndSkipPast("OfferAndAcceptanceOfPrenatalTest", inputFile);
	readAndSkipPast("BaselineProbOfferedTest", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.probOfferedAndAccept);
	
	readAndSkipPast("PrenatalTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.costOfTest);
	
	/** Prenatal Test Characteristics */
	readAndSkipPast("ProbResultReturnToPatient", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.probResultReturn);
	readAndSkipPast("WeeksToResultReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.weeksToResultReturn);
	readAndSkipPast("WeeksToRepeatTestIfNoReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.weeksToRepeatTestIfNoReturn);
	
	readAndSkipPast("MinimumTestInterval", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.minimumTestInterval);
	readAndSkipPast("ProbTerminationUponAbnormalResult", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.probTerminationUponAbnormal);
	readAndSkipPast("AvailableStartingWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.availableStartingWeek);
	readAndSkipPast("NotAvailableAfterWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.notAvailableAfter);
	readAndSkipPast("SensitivityTrimester1", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.testSensitivity[TRIMESTER_1][i]);
	}
	readAndSkipPast("SensitivityTrimester2", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.testSensitivity[TRIMESTER_2][i]);
	}
	readAndSkipPast("SensitivityTrimester3", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.testSensitivity[TRIMESTER_3][i]);
	}
	readAndSkipPast("SpecificityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.testSpecificity[TRIMESTER_1]);
	readAndSkipPast("SpecificityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.testSpecificity[TRIMESTER_2]);
	readAndSkipPast("SpecificityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.testSpecificity[TRIMESTER_3]);
	/** Confirmatory testing */
	
	readAndSkipPast("ConfirmatoryTesting", inputFile);
	readAndSkipPast("NumRepeatTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.numRepeatedConfirmatory);
	readAndSkipPast("ProbSchedulingConfirmatoryTests", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.probSchedulingConfirmatory);
	readAndSkipPast("WeeksBetweenConfirmatoryTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.weeksBetweenConfirmatoryTests);
	readAndSkipPast("AdditionalConfirmatoryTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.additionalCostConfirmatoryTest);
	
	
	/** Follow Up Testing */
	readAndSkipPast("FollowUpTestingBaseTest", inputFile);
	readAndSkipPast("AbnormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.detailedUltrasound.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.detailedUltrasound.followUpTests[i][ABNORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.detailedUltrasound.followUpTestsProbabilities[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.followUpTestsProbabilities[i][ABNORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.detailedUltrasound.followUpTestDelays[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.followUpTestDelays[i][ABNORMAL]);
		}
	}
	readAndSkipPast("NormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.detailedUltrasound.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.detailedUltrasound.followUpTests[i][NORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.detailedUltrasound.followUpTestsProbabilities[i][NORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.followUpTestsProbabilities[i][NORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.detailedUltrasound.followUpTestDelays[i][NORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.followUpTestDelays[i][NORMAL]);
		}
	}


	/* Follow Ups for this test if this test was following up from another test */
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			// Follow-ups from the same test are defined as confirmatory tests, so they are not relevant. Zero out all of them in the vector. 
			for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
				prenatalTestingInputs.detailedUltrasound.FUTriggeredByTest[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.detailedUltrasound.FUTriggeredByTest[i][j][NORMAL] = 0;
				prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
			}
		}
		else {
			string testName = "Test";
			testName.append(std::to_string(i+1));
			readAndSkipPast(testName.c_str(), inputFile);
			readAndSkipPast("AbnormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTest[i][j][ABNORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTest[i][j][ABNORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestProbabilities[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestDelays[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("NormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTest[i][j][NORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTest[i][j][NORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestProbabilities[i][j][NORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.detailedUltrasound.FUTriggeredByTestDelays[i][j][NORMAL]);
				}
			}

		}
	}
	
	/* Prenatal Test 3 */
	test = AMNIOCENTESIS;
	readAndSkipPast("PrenatalTest3", inputFile);
	prenatalTestingInputs.amniocentesis.testArray = AMNIOCENTESIS;
	/** Special case triggering*/
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.mildIllnessTrigger);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.knownInfectionTrigger);
	readAndSkipPast("SymptomaticCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.symptomaticCMVTrigger);
	//readAndSkipPast("PreviousTest", inputFile);
	//fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.previousTestTrigger);
	/** Acceptance of prenatal test*/
	readAndSkipPast("OfferAndAcceptanceOfPrenatalTest", inputFile);
	readAndSkipPast("BaselineProbOfferedTest", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.probOfferedAndAccept);
	
	readAndSkipPast("PrenatalTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.costOfTest);
	
	/** Prenatal Test Characteristics */
	readAndSkipPast("ProbResultReturnToPatient", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.probResultReturn);
	
	readAndSkipPast("WeeksToResultReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.weeksToResultReturn);

	readAndSkipPast("WeeksToRepeatTestIfNoReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.weeksToRepeatTestIfNoReturn);
	
	readAndSkipPast("MinimumTestInterval", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.minimumTestInterval);
	readAndSkipPast("ProbTerminationUponAbnormalResult", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.probTerminationUponAbnormal);
	readAndSkipPast("AvailableStartingWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.availableStartingWeek);
	readAndSkipPast("NotAvailableAfterWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.notAvailableAfter);
	readAndSkipPast("SensitivityTrimester1", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.testSensitivity[TRIMESTER_1][i]);
	}
	readAndSkipPast("SensitivityTrimester2", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.testSensitivity[TRIMESTER_2][i]);
	}
	readAndSkipPast("SensitivityTrimester3", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.testSensitivity[TRIMESTER_3][i]);
	}
	
	readAndSkipPast("SpecificityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.testSpecificity[TRIMESTER_1]);
	readAndSkipPast("SpecificityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.testSpecificity[TRIMESTER_2]);
	readAndSkipPast("SpecificityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.testSpecificity[TRIMESTER_3]);

	/** Confirmatory testing */
	
	readAndSkipPast("ConfirmatoryTesting", inputFile);
	readAndSkipPast("NumRepeatTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.numRepeatedConfirmatory);
	readAndSkipPast("ProbSchedulingConfirmatoryTests", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.probSchedulingConfirmatory);
	readAndSkipPast("WeeksBetweenConfirmatoryTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.weeksBetweenConfirmatoryTests);
	readAndSkipPast("AdditionalConfirmatoryTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.additionalCostConfirmatoryTest);
	
	
	/** Follow Up Testing */
	readAndSkipPast("FollowUpTestingBaseTest", inputFile);
	readAndSkipPast("AbnormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.amniocentesis.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.amniocentesis.followUpTests[i][ABNORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.amniocentesis.followUpTestsProbabilities[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.followUpTestsProbabilities[i][ABNORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.amniocentesis.followUpTestDelays[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.followUpTestDelays[i][ABNORMAL]);
		}
	}
	readAndSkipPast("NormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.amniocentesis.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.amniocentesis.followUpTests[i][NORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.amniocentesis.followUpTestsProbabilities[i][NORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.followUpTestsProbabilities[i][NORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.amniocentesis.followUpTestDelays[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.followUpTestDelays[i][NORMAL]);
		}
	}


	/* Follow Ups for this test if this test was following up from another test */
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			// Follow-ups from the same test are defined as confirmatory tests, so they are not relevant. Zero out all of them in the vector. 
			for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
				prenatalTestingInputs.amniocentesis.FUTriggeredByTest[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.amniocentesis.FUTriggeredByTest[i][j][NORMAL] = 0;
				prenatalTestingInputs.amniocentesis.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.amniocentesis.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				prenatalTestingInputs.amniocentesis.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.amniocentesis.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
			}
		}
		else {
			string testName = "Test";
			testName.append(std::to_string(i+1));
			readAndSkipPast(testName.c_str(), inputFile);
			readAndSkipPast("AbnormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.amniocentesis.FUTriggeredByTest[i][j][ABNORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.amniocentesis.FUTriggeredByTest[i][j][ABNORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.amniocentesis.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.FUTriggeredByTestProbabilities[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.amniocentesis.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.FUTriggeredByTestDelays[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("NormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.amniocentesis.FUTriggeredByTest[i][j][NORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.amniocentesis.FUTriggeredByTest[i][j][NORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.amniocentesis.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.amniocentesis.FUTriggeredByTestProbabilities[i][j][NORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.amniocentesis.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.amniocentesis.FUTriggeredByTestDelays[i][j][NORMAL]);
				}
			}

		}
	}

	/* Prenatal Test 4 */
	test = MATERNAL_PCR;
	readAndSkipPast("PrenatalTest4", inputFile);
	prenatalTestingInputs.maternalPCR.testArray = MATERNAL_PCR;

	/** Special case triggering*/
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.mildIllnessTrigger);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.knownInfectionTrigger);
	readAndSkipPast("SymptomaticCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.symptomaticCMVTrigger);
	//readAndSkipPast("PreviousTest", inputFile);
	//fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.previousTestTrigger);
	/** Acceptance of prenatal test*/
	readAndSkipPast("OfferAndAcceptanceOfPrenatalTest", inputFile);
	readAndSkipPast("BaselineProbOfferedTest", inputFile);

	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.probOfferedAndAccept);
	
	readAndSkipPast("PrenatalTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.costOfTest);
	
	/** Prenatal Test Characteristics */
	readAndSkipPast("ProbResultReturnToPatient", inputFile);

	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.probResultReturn);
	
	readAndSkipPast("WeeksToResultReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.weeksToResultReturn);
	
	readAndSkipPast("WeeksToRepeatTestIfNoReturn", inputFile);

	fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.weeksToRepeatTestIfNoReturn);
	
	readAndSkipPast("MinimumTestInterval", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.minimumTestInterval);
	readAndSkipPast("ProbTerminationUponAbnormalResult", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.probTerminationUponAbnormal);
	readAndSkipPast("AvailableStartingWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.availableStartingWeek);
	readAndSkipPast("NotAvailableAfterWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.notAvailableAfter);
	readAndSkipPast("SensitivityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.testSensitivity[TRIMESTER_1][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.maternalPCR.testSensitivity[TRIMESTER_1][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.testSensitivity[TRIMESTER_2][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.maternalPCR.testSensitivity[TRIMESTER_2][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.testSensitivity[TRIMESTER_3][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.maternalPCR.testSensitivity[TRIMESTER_3][i] = 0;
	}
	
	readAndSkipPast("SpecificityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.testSpecificity[TRIMESTER_1]);
	readAndSkipPast("SpecificityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.testSpecificity[TRIMESTER_2]);
	readAndSkipPast("SpecificityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.testSpecificity[TRIMESTER_3]);


	/** Confirmatory testing */
	
	readAndSkipPast("ConfirmatoryTesting", inputFile);
	readAndSkipPast("NumRepeatTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.numRepeatedConfirmatory);
	readAndSkipPast("ProbSchedulingConfirmatoryTests", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.probSchedulingConfirmatory);
	readAndSkipPast("WeeksBetweenConfirmatoryTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.weeksBetweenConfirmatoryTests);
	readAndSkipPast("AdditionalConfirmatoryTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.additionalCostConfirmatoryTest);
	
	
	/** Follow Up Testing */
	readAndSkipPast("FollowUpTestingBaseTest", inputFile);
	readAndSkipPast("AbnormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.maternalPCR.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.maternalPCR.followUpTests[i][ABNORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.maternalPCR.followUpTestsProbabilities[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.followUpTestsProbabilities[i][ABNORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.maternalPCR.followUpTestDelays[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.followUpTestDelays[i][ABNORMAL]);
		}
	}
	readAndSkipPast("NormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.maternalPCR.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.maternalPCR.followUpTests[i][NORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.maternalPCR.followUpTestsProbabilities[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.followUpTestsProbabilities[i][NORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.maternalPCR.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.followUpTestDelays[i][NORMAL]);
		}
	}


	/* Follow Ups for this test if this test was following up from another test */
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			// Follow-ups from the same test are defined as confirmatory tests, so they are not relevant. Zero out all of them in the vector. 
			for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
				prenatalTestingInputs.maternalPCR.FUTriggeredByTest[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.maternalPCR.FUTriggeredByTest[i][j][NORMAL] = 0;
				prenatalTestingInputs.maternalPCR.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.maternalPCR.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				prenatalTestingInputs.maternalPCR.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.maternalPCR.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
			}
		}
		else {
			string testName = "Test";
			testName.append(std::to_string(i+1));
			readAndSkipPast(testName.c_str(), inputFile);
			readAndSkipPast("AbnormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.maternalPCR.FUTriggeredByTest[i][j][ABNORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.maternalPCR.FUTriggeredByTest[i][j][ABNORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.maternalPCR.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.FUTriggeredByTestProbabilities[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.maternalPCR.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.FUTriggeredByTestDelays[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("NormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.maternalPCR.FUTriggeredByTest[i][j][NORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.maternalPCR.FUTriggeredByTest[i][j][NORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.maternalPCR.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.maternalPCR.FUTriggeredByTestProbabilities[i][j][NORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.maternalPCR.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.maternalPCR.FUTriggeredByTestDelays[i][j][NORMAL]);
				}
			}

		}
	}
	

	/* Prenatal Test 5 */
	test = IGM;
	readAndSkipPast("PrenatalTest5", inputFile);
	prenatalTestingInputs.igmTest.testArray = IGM;

	/** Special case triggering*/
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.mildIllnessTrigger);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.knownInfectionTrigger);
	readAndSkipPast("SymptomaticCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.symptomaticCMVTrigger);
	//readAndSkipPast("PreviousTest", inputFile);
	//fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.previousTestTrigger);
	/** Acceptance of prenatal test*/
	readAndSkipPast("OfferAndAcceptanceOfPrenatalTest", inputFile);
	readAndSkipPast("BaselineProbOfferedTest", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.probOfferedAndAccept);
	
	readAndSkipPast("PrenatalTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.costOfTest);
	
	/** Prenatal Test Characteristics */
	readAndSkipPast("ProbResultReturnToPatient", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.probResultReturn);
	
	readAndSkipPast("WeeksToResultReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.weeksToResultReturn);
	
	readAndSkipPast("WeeksToRepeatTestIfNoReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.weeksToRepeatTestIfNoReturn);
	
	readAndSkipPast("MinimumTestInterval", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.minimumTestInterval);
	readAndSkipPast("ProbTerminationUponAbnormalResult", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.probTerminationUponAbnormal);
	readAndSkipPast("AvailableStartingWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.availableStartingWeek);
	readAndSkipPast("NotAvailableAfterWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.notAvailableAfter);

	readAndSkipPast("TestSensitivity", inputFile);
	readAndSkipPast("SensitivityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.testSensitivity[TRIMESTER_1][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.igmTest.testSensitivity[TRIMESTER_1][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.testSensitivity[TRIMESTER_2][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.igmTest.testSensitivity[TRIMESTER_2][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.testSensitivity[TRIMESTER_3][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.igmTest.testSensitivity[TRIMESTER_3][i] = 0;
	}
	readAndSkipPast("TestSpecificity", inputFile);
	readAndSkipPast("SpecificityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.testSpecificity[TRIMESTER_1]);
	readAndSkipPast("SpecificityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.testSpecificity[TRIMESTER_2]);
	readAndSkipPast("SpecificityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.testSpecificity[TRIMESTER_3]);


	/** Confirmatory testing */
	
	readAndSkipPast("ConfirmatoryTesting", inputFile);
	readAndSkipPast("NumRepeatTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.numRepeatedConfirmatory);
	readAndSkipPast("ProbSchedulingConfirmatoryTests", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.probSchedulingConfirmatory);
	readAndSkipPast("WeeksBetweenConfirmatoryTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.weeksBetweenConfirmatoryTests);
	readAndSkipPast("AdditionalConfirmatoryTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.additionalCostConfirmatoryTest);
	
	
	/** Follow Up Testing */
	readAndSkipPast("FollowUpTestingBaseTest", inputFile);
	readAndSkipPast("AbnormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.igmTest.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.igmTest.followUpTests[i][ABNORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.igmTest.followUpTestsProbabilities[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.followUpTestsProbabilities[i][ABNORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.igmTest.followUpTestDelays[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.followUpTestDelays[i][ABNORMAL]);
		}
	}
	readAndSkipPast("NormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.igmTest.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.igmTest.followUpTests[i][NORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.igmTest.followUpTestsProbabilities[i][NORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.followUpTestsProbabilities[i][NORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.igmTest.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.followUpTestDelays[i][NORMAL]);
		}
	}


	/* Follow Ups for this test if this test was following up from another test */
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			// Follow-ups from the same test are defined as confirmatory tests, so they are not relevant. Zero out all of them in the vector. 
			for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
				prenatalTestingInputs.igmTest.FUTriggeredByTest[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.igmTest.FUTriggeredByTest[i][j][NORMAL] = 0;
				prenatalTestingInputs.igmTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.igmTest.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				prenatalTestingInputs.igmTest.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.igmTest.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
			}
		}
		else {
			string testName = "Test";
			testName.append(std::to_string(i+1));
			readAndSkipPast(testName.c_str(), inputFile);
			readAndSkipPast("AbnormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.igmTest.FUTriggeredByTest[i][j][ABNORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.igmTest.FUTriggeredByTest[i][j][ABNORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.igmTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.igmTest.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.FUTriggeredByTestDelays[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("NormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.igmTest.FUTriggeredByTest[i][j][NORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.igmTest.FUTriggeredByTest[i][j][NORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.igmTest.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.igmTest.FUTriggeredByTestProbabilities[i][j][NORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.igmTest.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.igmTest.FUTriggeredByTestDelays[i][j][NORMAL]);
				}
			}

		}
	}
	
	/* Prenatal Test 6 */
	test = IGG;
	readAndSkipPast("PrenatalTest6", inputFile);
	prenatalTestingInputs.avidityTest.testArray = IGG;

	/** Special case triggering*/
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.mildIllnessTrigger);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.knownInfectionTrigger);
	readAndSkipPast("SymptomaticCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.symptomaticCMVTrigger);
	//readAndSkipPast("PreviousTest", inputFile);
	//fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.previousTestTrigger);
	/** Acceptance of prenatal test*/
	readAndSkipPast("OfferAndAcceptanceOfPrenatalTest", inputFile);
	readAndSkipPast("BaselineProbOfferedTest", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.probOfferedAndAccept);
	
	readAndSkipPast("PrenatalTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.costOfTest);
	
	/** Prenatal Test Characteristics */
	readAndSkipPast("ProbResultReturnToPatient", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.probResultReturn);
	
	readAndSkipPast("WeeksToResultReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.weeksToResultReturn);
	
	readAndSkipPast("WeeksToRepeatTestIfNoReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.weeksToRepeatTestIfNoReturn);
	
	readAndSkipPast("MinimumTestInterval", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.minimumTestInterval);
	readAndSkipPast("ProbTerminationUponAbnormalResult", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.probTerminationUponAbnormal);
	readAndSkipPast("AvailableStartingWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.availableStartingWeek);
	readAndSkipPast("NotAvailableAfterWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.notAvailableAfter);
	readAndSkipPast("SensitivityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.testSensitivity[TRIMESTER_1][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.iggTest.testSensitivity[TRIMESTER_1][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.testSensitivity[TRIMESTER_2][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.iggTest.testSensitivity[TRIMESTER_2][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.testSensitivity[TRIMESTER_3][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.iggTest.testSensitivity[TRIMESTER_3][i] = 0;
	}
	
	readAndSkipPast("SpecificityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.testSpecificity[TRIMESTER_1]);
	readAndSkipPast("SpecificityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.testSpecificity[TRIMESTER_2]);
	readAndSkipPast("SpecificityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.testSpecificity[TRIMESTER_3]);


	/** Confirmatory testing */
	
	readAndSkipPast("ConfirmatoryTesting", inputFile);
	readAndSkipPast("NumRepeatTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.numRepeatedConfirmatory);
	readAndSkipPast("ProbSchedulingConfirmatoryTests", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.probSchedulingConfirmatory);
	readAndSkipPast("WeeksBetweenConfirmatoryTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.weeksBetweenConfirmatoryTests);
	readAndSkipPast("AdditionalConfirmatoryTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.additionalCostConfirmatoryTest);
	
	
	/** Follow Up Testing */
	readAndSkipPast("FollowUpTestingBaseTest", inputFile);
	readAndSkipPast("AbnormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.iggTest.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.iggTest.followUpTests[i][ABNORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.iggTest.followUpTestsProbabilities[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.followUpTestsProbabilities[i][ABNORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.iggTest.followUpTestDelays[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.followUpTestDelays[i][ABNORMAL]);
		}
	}
	readAndSkipPast("NormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.iggTest.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.iggTest.followUpTests[i][NORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.iggTest.followUpTestsProbabilities[i][NORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.followUpTestsProbabilities[i][NORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.iggTest.followUpTestDelays[i][NORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.followUpTestDelays[i][NORMAL]);
		}
	}


	/* Follow Ups for this test if this test was following up from another test */
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		string testName = "Test";
		if (i != IGM && i != (IGM + 1)){
			testName.append(std::to_string(i+1));
		}
		else if (i == IGM) { // if following up from positive IgM result 
			testName.append("5Positive");
		}
		else { // if following up from negative IgM result
			testName.append("5Negative");
		}

		readAndSkipPast(testName.c_str(), inputFile);
		readAndSkipPast("AbnormalResult", inputFile);
		readAndSkipPast("Activated", inputFile);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++){
			if (j == (test)) {
				prenatalTestingInputs.iggTest.FUTriggeredByTest[i][j][ABNORMAL] = false;
			}
			else {
				fscanf(inputFile, "%d", &tempBool);
				prenatalTestingInputs.iggTest.FUTriggeredByTest[i][j][ABNORMAL] = tempBool;
			}
		}
		readAndSkipPast("ProbFollowUp", inputFile);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++){
			if (j == (test)) {
				prenatalTestingInputs.iggTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
			}
			else {
				fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL]);
			}
		}
		readAndSkipPast("SchedulingDelay", inputFile);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++){
			if (j == (test)) {
				prenatalTestingInputs.iggTest.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
			}
			else {
				fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.FUTriggeredByTestDelays[i][j][ABNORMAL]);
			}
		}
		readAndSkipPast("NormalResult", inputFile);
		readAndSkipPast("Activated", inputFile);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++){
			if (j == (test)) {
				prenatalTestingInputs.iggTest.FUTriggeredByTest[i][j][NORMAL] = false;
			}
			else {
				fscanf(inputFile, "%d", &tempBool);
				prenatalTestingInputs.iggTest.FUTriggeredByTest[i][j][NORMAL] = tempBool;
			}
		}
		readAndSkipPast("ProbFollowUp", inputFile);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++){
			if (j == (test)) {
				prenatalTestingInputs.iggTest.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
			}
			else {
				fscanf(inputFile, "%lf", &prenatalTestingInputs.iggTest.FUTriggeredByTestProbabilities[i][j][NORMAL]);
			}
		}
		readAndSkipPast("SchedulingDelay", inputFile);
		for (j = 0; j < NUM_PRENATAL_TESTS; j++){
			if (j == (test)) {
				prenatalTestingInputs.iggTest.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
			}
			else {
				fscanf(inputFile, "%d", &prenatalTestingInputs.iggTest.FUTriggeredByTestDelays[i][j][NORMAL]);
			}
		}
		

	}
	

	/* Prenatal Test 7 */
	test = IGG_AVIDITY;
	readAndSkipPast("PrenatalTest7", inputFile);
	prenatalTestingInputs.avidityTest.testArray = IGG_AVIDITY;

	/** Special case triggering*/
	readAndSkipPast("MildIllness", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.mildIllnessTrigger);
	readAndSkipPast("KnownInfection", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.knownInfectionTrigger);
	readAndSkipPast("SymptomaticCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.symptomaticCMVTrigger);
	//readAndSkipPast("PreviousTest", inputFile);
	//fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.previousTestTrigger);
	/** Acceptance of prenatal test*/
	readAndSkipPast("OfferAndAcceptanceOfPrenatalTest", inputFile);
	readAndSkipPast("BaselineProbOfferedTest", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.probOfferedAndAccept);

	readAndSkipPast("PrenatalTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.costOfTest);
	
	/** Prenatal Test Characteristics */
	readAndSkipPast("ProbResultReturnToPatient", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.probResultReturn);

	readAndSkipPast("WeeksToResultReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.weeksToResultReturn);

	readAndSkipPast("WeeksToRepeatTestIfNoReturn", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.weeksToRepeatTestIfNoReturn);
	
	readAndSkipPast("MinimumTestInterval", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.minimumTestInterval);
	readAndSkipPast("ProbTerminationUponAbnormalResult", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.probTerminationUponAbnormal);
	readAndSkipPast("AvailableStartingWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.availableStartingWeek);
	readAndSkipPast("NotAvailableAfterWeek", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.notAvailableAfter);
	readAndSkipPast("SensitivityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.testSensitivity[TRIMESTER_1][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.avidityTest.testSensitivity[TRIMESTER_1][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.testSensitivity[TRIMESTER_2][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.avidityTest.testSensitivity[TRIMESTER_2][i] = 0;
	}
	readAndSkipPast("SensitivityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.testSensitivity[TRIMESTER_3][0]);
	for (i = 1; i < NUM_CMV_PHENOTYPES; i++) {
		prenatalTestingInputs.avidityTest.testSensitivity[TRIMESTER_3][i] = 0;
	}
	
	readAndSkipPast("SpecificityTrimester1", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.testSpecificity[TRIMESTER_1]);
	readAndSkipPast("SpecificityTrimester2", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.testSpecificity[TRIMESTER_2]);
	readAndSkipPast("SpecificityTrimester3", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.testSpecificity[TRIMESTER_3]);


	/** Confirmatory testing */
	
	readAndSkipPast("ConfirmatoryTesting", inputFile);
	readAndSkipPast("NumRepeatTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.numRepeatedConfirmatory);
	readAndSkipPast("ProbSchedulingConfirmatoryTests", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.probSchedulingConfirmatory);
	readAndSkipPast("WeeksBetweenConfirmatoryTests", inputFile);
	fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.weeksBetweenConfirmatoryTests);
	readAndSkipPast("AdditionalConfirmatoryTestCost", inputFile);
	fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.additionalCostConfirmatoryTest);
	
	/** Follow Up Testing */
	readAndSkipPast("FollowUpTestingBaseTest", inputFile);
	readAndSkipPast("AbnormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.avidityTest.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.avidityTest.followUpTests[i][ABNORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.avidityTest.followUpTests[i][ABNORMAL] = false;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.followUpTestsProbabilities[i][ABNORMAL] );
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.avidityTest.followUpTestDelays[i][ABNORMAL] = 0;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.followUpTestDelays[i][ABNORMAL]);
		}
	}
	readAndSkipPast("NormalResult", inputFile);
	readAndSkipPast("Activated", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.avidityTest.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &tempBool);
			prenatalTestingInputs.avidityTest.followUpTests[i][NORMAL] = tempBool;
		}
	}
	readAndSkipPast("ProbFollowUp", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.avidityTest.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.followUpTestsProbabilities[i][NORMAL]);
		}
	}
	readAndSkipPast("SchedulingDelay", inputFile);
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			prenatalTestingInputs.avidityTest.followUpTests[i][NORMAL] = false;
		}
		else {
			fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.followUpTestDelays[i][NORMAL]);
		}
	}


	/* Follow Ups for this test if this test was following up from another test */
	for (i = 0; i < NUM_PRENATAL_TESTS; i++) {
		if (i == (test)) {
			// Follow-ups from the same test are defined as confirmatory tests, so they are not relevant. Zero out all of them in the vector. 
			for (j = 0; j < NUM_PRENATAL_TESTS; j++) {
				prenatalTestingInputs.avidityTest.FUTriggeredByTest[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.avidityTest.FUTriggeredByTest[i][j][NORMAL] = 0;
				prenatalTestingInputs.avidityTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.avidityTest.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				prenatalTestingInputs.avidityTest.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				prenatalTestingInputs.avidityTest.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
			}
		}
		else {
			string testName = "Test";
			testName.append(std::to_string(i+1));
			readAndSkipPast(testName.c_str(), inputFile);
			readAndSkipPast("AbnormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.avidityTest.FUTriggeredByTest[i][j][ABNORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.avidityTest.FUTriggeredByTest[i][j][ABNORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.avidityTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.FUTriggeredByTestProbabilities[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.avidityTest.FUTriggeredByTestDelays[i][j][ABNORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.FUTriggeredByTestDelays[i][j][ABNORMAL]);
				}
			}
			readAndSkipPast("NormalResult", inputFile);
			readAndSkipPast("Activated", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.avidityTest.FUTriggeredByTest[i][j][NORMAL] = false;
				}
				else {
					fscanf(inputFile, "%d", &tempBool);
					prenatalTestingInputs.avidityTest.FUTriggeredByTest[i][j][NORMAL] = tempBool;
				}
			}
			readAndSkipPast("ProbFollowUp", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.avidityTest.FUTriggeredByTestProbabilities[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%lf", &prenatalTestingInputs.avidityTest.FUTriggeredByTestProbabilities[i][j][NORMAL]);
				}
			}
			readAndSkipPast("SchedulingDelay", inputFile);
			for (j = 0; j < NUM_PRENATAL_TESTS; j++){
				if (j == (test)) {
					prenatalTestingInputs.avidityTest.FUTriggeredByTestDelays[i][j][NORMAL] = 0;
				}
				else {
					fscanf(inputFile, "%d", &prenatalTestingInputs.avidityTest.FUTriggeredByTestDelays[i][j][NORMAL]);
				}
			}

		}
	}


	

	// Pushing the prenatal tests to the PrenatalTestsVector vector containing all specified prenatal test parameters
	prenatalTestingInputs.PrenatalTestsVector.push_back(prenatalTestingInputs.routineUltrasound);
	prenatalTestingInputs.PrenatalTestsVector.push_back(prenatalTestingInputs.detailedUltrasound);
	prenatalTestingInputs.PrenatalTestsVector.push_back(prenatalTestingInputs.amniocentesis);
	prenatalTestingInputs.PrenatalTestsVector.push_back(prenatalTestingInputs.maternalPCR);
	prenatalTestingInputs.PrenatalTestsVector.push_back(prenatalTestingInputs.igmTest);
	prenatalTestingInputs.PrenatalTestsVector.push_back(prenatalTestingInputs.iggTest);
	prenatalTestingInputs.PrenatalTestsVector.push_back(prenatalTestingInputs.avidityTest);
}	

/* readPreantalTreatmentInputs reads data from the Prenatal Treatment tab of the input sheet */

void SimContext::readPrenatalTreatmentInputs() {
	int i, j, tempBool;
	double tempDouble;
	/** Reading in treatment policies */
	readAndSkipPast("PrenatalTreatment", inputFile);
	readAndSkipPast("NoMaternalOrFetalCMV", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	prenatalTreatmentInputs.treatmentBasedOnPolicy[NO_CMV_DIAGNOSES] = tempBool;
	fscanf(inputFile, "%d", &prenatalTreatmentInputs.treatmentNumberBasedOnPolicy[NO_CMV_DIAGNOSES]);
	fscanf(inputFile, "%d", &prenatalTreatmentInputs.treatmentDurationBasedOnPolicy[NO_CMV_DIAGNOSES]);
	readAndSkipPast("MaternalCMVDiagnosis", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	prenatalTreatmentInputs.treatmentBasedOnPolicy[MATERNAL_CMV_DIAGNOSIS] = tempBool;
	fscanf(inputFile, "%d", &prenatalTreatmentInputs.treatmentNumberBasedOnPolicy[MATERNAL_CMV_DIAGNOSIS]);
	fscanf(inputFile, "%d", &prenatalTreatmentInputs.treatmentDurationBasedOnPolicy[MATERNAL_CMV_DIAGNOSIS]);
	readAndSkipPast("FetalCMVDiagnosis", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	prenatalTreatmentInputs.treatmentBasedOnPolicy[FETAL_CMV_DIAGNOSIS] = tempBool;
	fscanf(inputFile, "%d", &prenatalTreatmentInputs.treatmentNumberBasedOnPolicy[FETAL_CMV_DIAGNOSIS]);
	fscanf(inputFile, "%d", &prenatalTreatmentInputs.treatmentDurationBasedOnPolicy[FETAL_CMV_DIAGNOSIS]);


	/** Reading in inputs for individual Prenatal Treatment */
	readAndSkipPast("PrenatalTreatment1", inputFile);
	readAndSkipPast("CostPerWeek", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.costPerWeek);
	readAndSkipPast("WeeklyPrimaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfWeeklyPrimaryCMV);
	
	readAndSkipPast("PercentSymptomatic", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfWeeklyPrimaryProportionSymptomatic);
	
	readAndSkipPast("WeeklySecondaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfWeeklySecondaryCMV);

	readAndSkipPast("PercentSymptomatic", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfWeeklySecondaryProportionSymptomatic);
	
	readAndSkipPast("ReductionOfVTFromPrevPrimaryMaternalInfection", inputFile);
	readAndSkipPast("VeryRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromPrimaryCMV[VERY_RECENT_CMV]);
	readAndSkipPast("MediumRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromPrimaryCMV[MEDIUM_RECENT_CMV]);
	readAndSkipPast("LessRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromPrimaryCMV[LESS_RECENT_CMV]);
	readAndSkipPast("LongTimeAgo", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromPrimaryCMV[LONG_TIME_AGO_CMV]);

	readAndSkipPast("ReductionOfVTFromPrevSecondaryMaternalInfection", inputFile);
	readAndSkipPast("VeryRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromSecondaryCMV[VERY_RECENT_CMV]);
	readAndSkipPast("MediumRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromSecondaryCMV[MEDIUM_RECENT_CMV]);
	readAndSkipPast("LessRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromSecondaryCMV[LESS_RECENT_CMV]);
	readAndSkipPast("LongTimeAgo", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfPreviousVTFromSecondaryCMV[LONG_TIME_AGO_CMV]);

	readAndSkipPast("ReductionOfVerticalTransmissionFromPrimaryCMV", inputFile);
	// zeroing out reduction for weeks 0 and 1 because no events occur. 
	for (i = 0; i < MAX_WEEKS_PREGNANT; i++) {
		string weekStr = "Week";
		weekStr.append(std::to_string(i));
		readAndSkipPast(weekStr.c_str(), inputFile);
		fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfVerticalTransmissionFromPrimaryCMV[i]);
		
	}
	readAndSkipPast("ReductionOfVerticalTransmissionFromSecondaryCMV", inputFile);

	for (i = 0; i < MAX_WEEKS_PREGNANT; i++) {
		string weekStr = "Week";
		weekStr.append(std::to_string(i));
		readAndSkipPast(weekStr.c_str(), inputFile);
		fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfVerticalTransmissionFromSecondaryCMV[i]);
		
	}
	/** Reduction of prenatalTreatment1 symptoms/phenotypes*/
	readAndSkipPast("ProbSwitching", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		string toType = "ToType";
		toType.append(std::to_string(i+1));
		readAndSkipPast(toType.c_str(), inputFile);
		readAndSkipPast("-", inputFile);
		prenatalTreatmentInputs.prenatalTreatment1.reductionOfSymptoms[0][i] = 0;
		for (j = 1; j < NUM_CMV_PHENOTYPES; j++) {
			fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment1.reductionOfSymptoms[j][i]);
		}
	}

	/** Prenatal Treatment 2 */
	readAndSkipPast("PrenatalTreatment2", inputFile);
	readAndSkipPast("CostPerWeek", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.costPerWeek);
	readAndSkipPast("WeeklyPrimaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfWeeklyPrimaryCMV);
	
	readAndSkipPast("PercentSymptomatic", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfWeeklyPrimaryProportionSymptomatic);
	
	readAndSkipPast("WeeklySecondaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfWeeklySecondaryCMV);
	
	readAndSkipPast("PercentSymptomatic", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfWeeklySecondaryProportionSymptomatic);
	
	readAndSkipPast("ReductionOfVTFromPrevPrimaryMaternalInfection", inputFile);
	readAndSkipPast("VeryRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromPrimaryCMV[VERY_RECENT_CMV]);
	readAndSkipPast("MediumRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromPrimaryCMV[MEDIUM_RECENT_CMV]);
	readAndSkipPast("LessRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromPrimaryCMV[LESS_RECENT_CMV]);
	readAndSkipPast("LongTimeAgo", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromPrimaryCMV[LONG_TIME_AGO_CMV]);

	readAndSkipPast("ReductionOfVTFromPrevSecondaryMaternalInfection", inputFile);
	readAndSkipPast("VeryRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromSecondaryCMV[VERY_RECENT_CMV]);
	readAndSkipPast("MediumRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromSecondaryCMV[MEDIUM_RECENT_CMV]);
	readAndSkipPast("LessRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromSecondaryCMV[LESS_RECENT_CMV]);
	readAndSkipPast("LongTimeAgo", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfPreviousVTFromSecondaryCMV[LONG_TIME_AGO_CMV]);

	readAndSkipPast("ReductionOfVerticalTransmissionFromPrimaryCMV", inputFile);

	for (i = 0; i < MAX_WEEKS_PREGNANT; i++) {
		string weekStr = "Week";
		weekStr.append(std::to_string(i));
		readAndSkipPast(weekStr.c_str(), inputFile);
		fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfVerticalTransmissionFromPrimaryCMV[i]);
		
	}
	readAndSkipPast("ReductionOfVerticalTransmissionFromSecondaryCMV", inputFile);

	for (i = 2; i < MAX_WEEKS_PREGNANT; i++) {
		string weekStr = "Week";
		weekStr.append(std::to_string(i));
		readAndSkipPast(weekStr.c_str(), inputFile);
		fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfVerticalTransmissionFromSecondaryCMV[i]);
		
	}
	/** Reduction of prenatalTreatment2 symptoms/phenotypes*/
	readAndSkipPast("ProbSwitching", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		string toType = "ToType";
		toType.append(std::to_string(i+1));
		readAndSkipPast(toType.c_str(), inputFile);
		readAndSkipPast("-", inputFile);
		prenatalTreatmentInputs.prenatalTreatment2.reductionOfSymptoms[0][i] = 0;
		for (j = 1; j < NUM_CMV_PHENOTYPES; j++) {
			fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment2.reductionOfSymptoms[j][i]);
		}
	}

	/** Prenatal Treatment 3 */
	readAndSkipPast("PrenatalTreatment3", inputFile);
	readAndSkipPast("CostPerWeek", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.costPerWeek);
	readAndSkipPast("WeeklyPrimaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfWeeklyPrimaryCMV);
	
	readAndSkipPast("PercentSymptomatic", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfWeeklyPrimaryProportionSymptomatic);
	
	readAndSkipPast("WeeklySecondaryCMV", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfWeeklySecondaryCMV);

	readAndSkipPast("PercentSymptomatic", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfWeeklySecondaryProportionSymptomatic);
	
	readAndSkipPast("ReductionOfVTFromPrevPrimaryMaternalInfection", inputFile);
	readAndSkipPast("VeryRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromPrimaryCMV[VERY_RECENT_CMV]);
	readAndSkipPast("MediumRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromPrimaryCMV[MEDIUM_RECENT_CMV]);
	readAndSkipPast("LessRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromPrimaryCMV[LESS_RECENT_CMV]);
	readAndSkipPast("LongTimeAgo", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromPrimaryCMV[LONG_TIME_AGO_CMV]);

	readAndSkipPast("ReductionOfVTFromPrevSecondaryMaternalInfection", inputFile);
	readAndSkipPast("VeryRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromSecondaryCMV[VERY_RECENT_CMV]);
	readAndSkipPast("MediumRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromSecondaryCMV[MEDIUM_RECENT_CMV]);
	readAndSkipPast("LessRecent", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromSecondaryCMV[LESS_RECENT_CMV]);
	readAndSkipPast("LongTimeAgo", inputFile);
	fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfPreviousVTFromSecondaryCMV[LONG_TIME_AGO_CMV]);

	readAndSkipPast("ReductionOfVerticalTransmissionFromPrimaryCMV", inputFile);

	for (i = 0; i < MAX_WEEKS_PREGNANT; i++) {
		string weekStr = "Week";
		weekStr.append(std::to_string(i));
		readAndSkipPast(weekStr.c_str(), inputFile);
		fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfVerticalTransmissionFromPrimaryCMV[i]);

	}
	readAndSkipPast("ReductionOfVerticalTransmissionFromSecondaryCMV", inputFile);

	for (i = 0; i < MAX_WEEKS_PREGNANT; i++) {
		string weekStr = "Week";
		weekStr.append(std::to_string(i));
		readAndSkipPast(weekStr.c_str(), inputFile);
		fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfVerticalTransmissionFromSecondaryCMV[i]);
		
	}
	/** Reduction of prenatalTreatment3 symptoms/phenotypes*/
	readAndSkipPast("ProbSwitching", inputFile);
	for (i = 0; i < NUM_CMV_PHENOTYPES; i++) {
		string toType = "ToType";
		toType.append(std::to_string(i+1));
		readAndSkipPast(toType.c_str(), inputFile);
		readAndSkipPast("-", inputFile);
		prenatalTreatmentInputs.prenatalTreatment3.reductionOfSymptoms[0][i] = 0;
		for (j = 1; j < NUM_CMV_PHENOTYPES; j++) {
			fscanf(inputFile, "%lf", &prenatalTreatmentInputs.prenatalTreatment3.reductionOfSymptoms[j][i]);
		}
	}

	prenatalTreatmentInputs.PrenatalTreatmentsVector.push_back(prenatalTreatmentInputs.prenatalTreatment1);
	prenatalTreatmentInputs.PrenatalTreatmentsVector.push_back(prenatalTreatmentInputs.prenatalTreatment2);
	prenatalTreatmentInputs.PrenatalTreatmentsVector.push_back(prenatalTreatmentInputs.prenatalTreatment3);
	

}

/* readPregnancyInputs reads data from the Pregnancy tab of the input sheet */
void SimContext::readPostPartumInputs() {

}

/* readInfantInputs reads data from the Infant tab of the input sheet (NOT IN USE)*/
void SimContext::readInfantInputs() {

}

/* readPostPartumInputs reads data from the Adult tab of the input sheet (NOT IN USE)*/
void SimContext::readAdultInputs() {

}

bool SimContext::readAndSkipPast(const char* searchStr, FILE* file) {
	char temp[513];
	fscanf(file, "%512s", temp);
	while ( strcmp(temp, searchStr) != 0 ) {

		fscanf(file, "%512s", temp);

		if ( feof(file) ) {
			if(counter<=100){
			printf("\nWARNING: unexpected end of input file. Looking for %s \n",searchStr);
			counter++;
			}

			return false;
		}

	}
	return true;
}  // readAndSkipPast

bool SimContext::readAndSkipPast2( const char* searchStr1, const char* searchStr2, FILE* file ) {
	bool ret = readAndSkipPast(searchStr1, file);
	if (ret == true)
		ret = readAndSkipPast(searchStr2, file);
	return ret;
}  // readAndSkipPast2

