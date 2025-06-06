#include "include.h"

/** \brief Constructor takes summariesFileName as input, clears summaries vector
 *
 *	/param runName a string identifying the file name of the summaries file (most likely 'popstats.out')
 */
SummaryStats::SummaryStats(string summariesFileName) {
	this->summariesFileName = summariesFileName;
    summaries.clear();
} /* end Constructor */

/** \brief Destructor frees allocated Summary objects and clears summaries vector */
SummaryStats::~SummaryStats(void)
{
} /* end Destructor */

/** \brief addRunStats adds a new summary to the vector from a RunStats object
 *
 * \param runStats a pointer to the RunStats object that the new Summary object will get its information from
 **/
void SummaryStats::addRunStats(RunStats *runStats) {
	/** Create a new summary object */
	Summary *summary = new Summary();
    /** Copy the population summary stats from runStats */
	const RunStats::PopulationSummary *popSummary = runStats->getPopulationSummary();
    const RunStats::MaternalCohortSummary *maternalSummary = runStats->getMaternalCohortSummary();
    const RunStats::ChildCohortSummary *childSummary = runStats->getChildCohortSummary();
    const RunStats::ChildDeathStats *childDeaths = runStats->getChildDeathStats();

    summary->runSetName = popSummary->runSetName;
	summary->runName = popSummary->runName;
    summary->numCohorts = popSummary->numCohorts;
        
    summary->proportionMaternalCMV = (float)maternalSummary->numCMVInfections/popSummary->numCohorts;
    summary->proportionMaternalMildIllness = (float)maternalSummary->numMildIllness/popSummary->numCohorts;
    summary->proportionMiscarriage = (float)(maternalSummary->numMiscarriageNoCMV + maternalSummary->numMiscarriageWithCMV)/popSummary->numCohorts;
    summary->averageWeekBirthNoCMV = (float)childSummary->averageWeekBirthNoCMV;
    summary->averageWeekBirthWithCMV = (float)childSummary->averageWeekBirthCMV;
    summary->proportionChildCMV = (float)childSummary->numCMVInfections/popSummary->numCohorts;
        

    /** Add the new summary to the appropriate vector, create a new vector if this
		is the first run of a run set */
	for (list<vector<Summary *> >::iterator i = summaries.begin(); i != summaries.end(); i++) {
		vector<Summary *> &runSetVector = *i;
		if (summary->runSetName.compare(runSetVector[0]->runSetName) == 0) {
			runSetVector.push_back(summary);
			return;
		}
	}
	vector<Summary *> runSetVector;
	runSetVector.push_back(summary);
	summaries.push_back(runSetVector);
}

void SummaryStats::finalizeStats() {

}

/** \brief writeSummariesFile appends the summary information to the popstats.out file */
void SummaryStats::writeSummariesFile() {
    /** Open the popstats file and write header if needed by calling SummaryStats::writeSummariesFileHeader() */
	CmvUtil::changeDirectoryToResults();
    if (CmvUtil::fileExists(summariesFileName.c_str())) {
		summariesFile = CmvUtil::openFile(summariesFileName.c_str(), "a");
		if (summariesFile == NULL) {
			summariesFileName.append("-tmp");
			summariesFile = CmvUtil::openFile(summariesFileName.c_str(), "w");
			if (summariesFile == NULL) {
				string errorString = "   ERROR - Could not write popstats or temporary popstats file";
				throw errorString;
			}
			writeSummariesFileHeader();
		}
	}
	else {
		summariesFile = CmvUtil::openFile(summariesFileName.c_str(), "w");
		if (summariesFile == NULL) {
			summariesFileName.append("-tmp");
			summariesFile = CmvUtil::openFile(summariesFileName.c_str(), "w");
			if (summariesFile == NULL) {
				string errorString = "   ERROR - Could not write popstats or temporary popstats file";
				throw errorString;
			}
		}
		writeSummariesFileHeader();
	}

    /** Loop over the run set vectors */
    for (list<vector<Summary *> >::iterator i = summaries.begin(); i != summaries.end(); i++) {
		vector<Summary *> &runSetVector = *i;

        /** Loop over the individual run summaries of the run set vector */
		int j;
		for (vector<Summary *>::iterator i = runSetVector.begin(); i != runSetVector.end(); i++) {
			Summary *summary = *i;
            fprintf(summariesFile, "%s\t", summary->runSetName.c_str());
            fprintf(summariesFile, "%s\t", summary->runName.c_str());
            fprintf(summariesFile, "%d\t", summary->numCohorts);
            /** Writing proportion of maternal CMV statistics */

            fprintf(summariesFile, "%lf\t", summary->proportionMaternalCMV);
            
            /** Writing proportion of maternal Mild Illness statistics */

            fprintf(summariesFile, "%lf\t", summary->proportionMaternalMildIllness);
            
            /** Writing proportion of Miscarriages statistics */

            fprintf(summariesFile, "%lf\t", summary->proportionMiscarriage);
 
            fprintf(summariesFile, "%lf\t", summary->averageWeekBirthNoCMV);

            fprintf(summariesFile, "%lf\t", summary->averageWeekBirthWithCMV);
       
            
            fprintf(summariesFile, "%lf\t", summary->proportionChildCMV);
            
        }
    }
}

/** \brief writes out summaries file header */
void SummaryStats::writeSummariesFileHeader() {
    int i;
    fprintf(summariesFile, "\t\t\tMATERNAL CMV\t\t\t\t\tMILD ILLNESS\t\t\t\t\tMISCARRIAGE\t\t\t\t\tAVERAGE WEEK BIRTH CMV-\t\t\t\t\tAVERAGE WEEK BIRTH CMV+\t\t\t\t\tCHILD CMV\t\t\t\t\t\n");
    fprintf(summariesFile, "RUN SET\tRUN NAME\tNUM COHORTS\t");
    fprintf(summariesFile, "SBCRT1\tSBCRT2\tSBCRT3\tSBCRT4\tSBCRT5\t");
    fprintf(summariesFile, "SBCRT1\tSBCRT2\tSBCRT3\tSBCRT4\tSBCRT5\t");
    fprintf(summariesFile, "SBCRT1\tSBCRT2\tSBCRT3\tSBCRT4\tSBCRT5\t");
    fprintf(summariesFile, "SBCRT1\tSBCRT2\tSBCRT3\tSBCRT4\tSBCRT5\t");
    fprintf(summariesFile, "SBCRT1\tSBCRT2\tSBCRT3\tSBCRT4\tSBCRT5\t");
    fprintf(summariesFile, "SBCRT1\tSBCRT2\tSBCRT3\tSBCRT4\tSBCRT5\t");
    fprintf(summariesFile, "\n");
    fprintf(summariesFile, "=========\t=========\t=========\t");
    fprintf(summariesFile, "=========\t=========\t=========\t=========\t=========\t");
    fprintf(summariesFile, "=========\t=========\t=========\t=========\t=========\t");
    fprintf(summariesFile, "=========\t=========\t=========\t=========\t=========\t");
    fprintf(summariesFile, "=========\t=========\t=========\t=========\t=========\t");
    fprintf(summariesFile, "=========\t=========\t=========\t=========\t=========\t");
    fprintf(summariesFile, "=========\t=========\t=========\t=========\t=========\t");
    fprintf(summariesFile, "\n");

}