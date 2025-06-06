#pragma once

#include "include.h"

/**
	SummaryStats class contains a list of the summary statistics from each of the
	input files (simulation contexts) that are executed in a given run of the model.
	It contains the functions to generate a summary from a given RunStats object, add it to
	the list of summaries, and output all the summaries to the popstats file.  Main calls these
	functions to add each new summary and append to popstats.out at the end of the run.
*/
class SummaryStats
{
public:
	/* Contstructor and Destructor */
	SummaryStats(string summariesFileName);
	~SummaryStats(void);

    /** Summary class stores the summary information that is written to the popstats file */
	class Summary {
	public:
        /** The name of the run set this summary belongs to */
        string runSetName;
        /** The name of the run corresponding to this summary */
        string runName;
        /** The number of patients in the cohort */
        int numCohorts;
        /* Proportion of mothers that had CMV Infection */
        double proportionMaternalCMV;
        /** Proportion of mothers that had Mild Illness */
        double proportionMaternalMildIllness;
        /** Proportion of mothers that had miscarriages */
        double proportionMiscarriage;
        /** Average week of birth for children born without CMV */
        double averageWeekBirthNoCMV;
        /** Average week of birth for children born with CMV */
        double averageWeekBirthWithCMV;
        /** Proportion of children that had CMV Infection */
        double proportionChildCMV;
    };

    /* addRunStats adds a new summary to the vector from a RunStats object */
	void addRunStats(RunStats *runStats);
	/* finalizeStats calculates the final cost-effectiveness ratios for each run */
	void finalizeStats();
	/* writeSummariesFile appends the summary inforation to the popstats.out file */
	void writeSummariesFile();

private:
	/*** list of run set vectors of individual run Summary objects,
		uses Summary pointers since objects are large and copy is expensive */
	// TODO: a map from strings to summary vectors would be more efficient, had problems
	//	using this in VC++
	list<vector<Summary *> > summaries;

	/** summaries file name */
	string summariesFileName;
	/** summaries file pointer */
	FILE *summariesFile;

	/* writes out popstats file header */
	void writeSummariesFileHeader();
};