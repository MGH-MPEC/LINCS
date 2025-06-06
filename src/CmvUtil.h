#pragma once

#include "include.h"

class CmvUtil
{
public:
    /* Empty Constructor and Destructor. Should never create an instance of this class */
    CmvUtil(void);
    ~CmvUtil(void);

    /* Constant values for CMV Version and file/directory information */
    static const char *CMV_INPUT_VERSION;
    static const char *CMV_VERSION_STRING;
	static const char *CMV_EXECUTABLE_COMPILED_DATE;
    static const char *FILE_EXTENSION_FOR_TEMP;
    static const char *FILE_EXTENSION_FOR_TRACE;
    static const char *FILE_EXTENSION_FOR_OUTPUT;
    static const char *FILE_EXTENSION_FOR_COSTS_OUTPUT;
    static const char *FILE_EXTENSION_FOR_INPUT;
    static const char *FILE_EXTENSION_INPUT_SEARCH_STR;
    static const char *FILE_NAME_SUMMARIES;

    /* Vector of the file names to be run, and the inputs and results directories path */
    static std::vector<std::string> filesToRun;
    static std::string inputsDirectory;
    static std::string resultsDirectory;

    /* Functions for handling directories and locating the input files */
	static void useCurrentDirectoryForInputs();
	static void findInputFiles();
	static void createResultsDirectory();
	static void changeDirectoryToResults();
	static void changeDirectoryToInputs();

	/* Functions for returning the current system date and time */
	static void getDateString(char *buffer, int bufsize);
	static void getTimeString(char *buffer, int bufsize);
	static int getTrimester(Patient *patient);

	/* Functions and state variables for generating uniform and gaussian random numbers */
	static void setRandomSeedType(bool useTimeSeed);
	static void setFixedSeed(Patient *patient);
	static double getRandomDouble();
	static int selectFromDist(std::vector<double>& odds, double randNum);
	static int getRandomInt(int min, int max);
	static double getRandomGaussian(double mean, double stdDev);
	static bool rollBasedOnOdds(double odds);
	static bool useRandomSeedByTime;
	static MTRand mtRand;

	/* Probability modification functions */
	static double probToRate(double prob);
	static double rateToProb(double rate);
	static double probRateMultiply(double prob, double rateMult);
	static double probToLogit(double prob);
	static double logitToProb(double logit);
	static double probLogitAdjustment(double prob, double logitAdjust);

	/* Functions for opening and closing files */
	static bool fileExists(const char *filename);
	static FILE *openFile(const char *filename, const char *mode);
	static void closeFile(FILE *file);
};

/** \brief setRandomSeedType sets up the random number generator to use seed by time (i.e. random seed) or fixed seed
 *
 * \param useTimeSeed a boolean that determines whether to use fixed or random seed: if true, use random, else use fixed
 **/
inline void CmvUtil::setRandomSeedType(bool useTimeSeed) {
	useRandomSeedByTime = useTimeSeed;
	if (useRandomSeedByTime)
		mtRand.seed((unsigned int) time(0));
	else
		mtRand.seed(8675309);
} /* end setRandomSeedType */

/** \brief setfixed seed type sets the seed using the patient number
 *
 */
inline void CmvUtil::setFixedSeed(Patient *patient){
	mtRand.seed(patient->getGeneralState()->patientNum);
}/* end setFixedSeed */

/** \brief getRandomDouble returns a random number within the range [0,1)
 *
 * \param callSiteId an integer specifying what function called the random number generator: was used for synchronized fixed seed and now no longer has a function
 * \param patient a pointer to the Patient; was used for synchronized fixed seed and now no longer has a function
 * \return a double randomly selected in the range [0,1)
 **/
inline double CmvUtil::getRandomDouble() {
	return mtRand();
} /* end getRandomDouble */



/** \brief getRandomGaussian returns a random normally distributed value with the specified mean and standard deviation
 *
 * \param mean a double representing the mean of the normal distribution
 * \param stdDev a double representing the standard deviation of the normal distribution
 * \param callSiteId an integer specifying what function called the random number generator: was used for synchronized fixed seed and now no longer has a function
 * \param patient a pointer to the Patient; was used for synchronized fixed seed and now no longer has a function
 * \return a double randomly selected from the defined distribution
 **/
inline double CmvUtil::getRandomGaussian(double mean, double stdDev) {
	// Polar form of Box-Muller transformation
    double x1, x2, w, y1, y2;
	do {
		x1 = 2.0 * mtRand() - 1.0;
		x2 = 2.0 * mtRand() - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );
	w = sqrt( (-2.0 * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;

	return (mean + (y2 * stdDev));
} /* end getRandomGaussian */

/** \brief probToRate converts a probability to a rate
 *
 * \f$ rate = -\log (1 - prob) \f$
 *
 * \param prob a double representing the probability to be converted to a rate
 * \return a double representing the equivalent rate
 **/
inline double CmvUtil::probToRate(double prob) {
	return (-1 * log(1 - prob));
} /* end probToRate */

/** \brief rateToProb converts a rate to a probability
 *
 *  \f$ prob = 1 - e^{-rate} \f$
 *
 * \param rate a double representing the rate to be converted to a probability
 * \return a double representing the equivalent probability
 **/
inline double CmvUtil::rateToProb(double rate) {
	return (1 - exp(-1 * rate));
} /* end rateToProb */

/** \brief probRateMultiply modifies a probability by a rate multiplier
 *
 * \f$ return = 1 - (1 - prob)^{rateMult} \f$
 *
 * \param prob a double representing the probability to be multiplied
 * \param rateMult a double representing the rate multiplier to apply to the probability
 * \return a double representing the adjusted probability
 **/
inline double CmvUtil::probRateMultiply(double prob, double rateMult) {
	// Formula is derived from conversion to rate, perform multiply, and convert back to prob
	if (rateMult == 0)
		return 0;
	if (rateMult == 1)
		return prob;
	return (1 - pow(1 - prob, rateMult));
} /* end probRateMultiply */

/** \brief probToLogit converts a probability to a logit
 *
 * \f$ logit = \log(\frac{prob}{1 - prob}) \f$
 *
 * \param prob a double representing the probability to be converted to a logit
 * \return a double representing the equivalent logit
 **/
inline double CmvUtil::probToLogit(double prob) {
	//Watch out for invalid values!
	return log(prob / (1 - prob));
}

/** \brief logitToProb converts a logit to a probability
 *
 * \f$ prob = \frac{1}{1 + e^{-logit}} \f$
 *
 * \param logit a double representing the logit to be converted to a probability
 * \return a double representing the equivalent probability
 **/
inline double CmvUtil::logitToProb(double logit) {
	return 1 / (1 + exp(-1 * logit));
}

/** \brief probLogitAdjustment takes a probability and adjusts it by a given logit by converting the probability to a logit, summing the logits together, and converting back to a probability
 *
 * \param prob a double representing the initial probability
 * \param logitAdjust a double representing the logit factor to add to the probability
 * \return a double representing the new adjusted probability
 **/
inline double CmvUtil::probLogitAdjustment(double prob, double logitAdjust) {
	return logitToProb(probToLogit(prob) + logitAdjust);
}

/** \brief selectFromDist takes in a vector of floats containing probabilities and a float between 0 and 1 to determine which index
 * of the vector to pick
 * 
 * \param odds a float vector containing the odds of each index being selected. The contents of the vector must sum to 1 for proper use of function. 
 * \param randNum a float that is used to pick which index is picked. Should use a randomly generated float between 0 and 1. 
*/
inline int CmvUtil::selectFromDist(std::vector<double>& odds, double randNum) {
    vector<float> sumOdds(odds.size() + 1, 0);
    sumOdds[0] = 0;
	int returnInt = -1;
    for (int i = 1; i <= odds.size(); i++) {
        for (int n = 0; n < i; n++) {
            sumOdds[i] = sumOdds[i] + odds[n];
        }
    }
    for (int i = 0; i <= odds.size(); i++) {
        if (randNum > sumOdds[i] && randNum <= sumOdds[i + 1]) {
            returnInt = i;
        }
    }
	if (returnInt < 0) {
		cout << "WARNING: selectFromDist did not properly select an outcome." <<endl;
	}
    return returnInt; // if nothing happens, return -1 to indicate error occurred
}

/** \brief getRandomInt returns an integer within a range specified by a min and max 
 * 
 * \param min the minimum in the range
 * \param max the maximum in the range
**/
inline int CmvUtil::getRandomInt(int min, int max) {
	vector<double> integerVect(max-min, 1/((double)max-(double)min));
	double randNum = mtRand();
	int randInt = selectFromDist(integerVect, randNum);
	randInt = randInt + min;
	return randInt;
}

/** \brief rollBasedOnOdds takes in a probability and rolls for true based on that probability 
 * \param odds the odds of rolling true
*/
inline bool CmvUtil::rollBasedOnOdds(double odds) {
	bool result = false;
	double randNum = mtRand();
	if (randNum < odds) {
		result = true;
	}
	return result;
}