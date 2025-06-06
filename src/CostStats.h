#pragma once

#include "include.h"

class CostStats
{
public:
    /** Make the StateUpdater class a friend calss so it can modify the private data */
    friend class StateUpdater;
    
    /* Constructors and Destructor */
    CostStats(string runName, SimContext *simContext);
    ~CostStats(void);

    class PopulationSummary {
    public:
        /** Total number of patients in this cohort */
		int numPatients;
        /** Total number ever detected CMV Positive */
		int numDetected;
    };

    /** class for stats for entire population can be discounted/undisc */
    class AllStats {
    public: 

    };

    /** class for stats for counts of events */
    class EventStats {
    public: 

    };

    /* Accessor functions returning const pointers to the statistics subclass objects */
    const PopulationSummary *getPopulationSummary();

private:
    /** Pointer to the associated simulation context */
    SimContext *simContext;
    /** Cost Stats file name */
    string costStatsFileName;
    /** Stats file pointer */
    FILE *costStatsFile;

    /** Statistics subclass object */
    PopulationSummary popSummary;


    /* Initialization functions for statistics objects, called by constructor */
    void initPopulationSummary();
    void initAllStats();
    void initEventStats();

    /* Functions to finalize aggregate statistics before printing out */
	void finalizePopulationSummary();
	void finalizeAllStats();
	void finalizeEventStats();

    /* Functions to write out each subclass object to the statistics file, called by writeStatsFile */
	void writePopulationSummary();
};

/** \brief getPopulationSumary returns a const pointer to the PopulationSummary statistics object */
inline const CostStats::PopulationSummary *CostStats::getPopulationSummary() {
	return &popSummary;
}