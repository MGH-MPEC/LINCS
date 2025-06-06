// console_main.cpp : Defines the main entry point for a console based application
//

/** \mainpage The CMVSim Model: Main Page
 *
 * 
 * The design of CMVSim is deliberately similar to the design of CEPAC in terms of how the modules are related to each other. 
 * This should mainly be used as a reference guide for programmers in conjunction with the flowcharts and CMVSim user guide.
 * Good places to start are the Patient object, the StateUpdater object, and the SimContext object.  As a general overview, Patient
 * contains the current patient state, SimContext primarily contains information drawn from the CMVSim .in files, RunStats
 * primarily contains information used in CMVSim .out files, SummaryStats primarily contains information for the popStats files, and Tracer
 * contains information for the trace files.  In addition, the monthly update functions from the main page of the flowchart each
 * have their own class which inherits from StateUpdater.  The main function can be found in ConsoleMain.cpp.  A number of helper 
 * functions are found in the CmvUtil class.
 *
 * Good luck!
 *
 */

#include "include.h"

string TRUE_FALSE[2] = {"False", "True"};

/** \brief Main function for a console based application */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        CmvUtil::inputsDirectory = argv[1];
        CmvUtil::changeDirectoryToInputs();
    }
    else {
        CmvUtil::useCurrentDirectoryForInputs();
        #if __APPLE__
            CmvUtil::changeDirectoryToInputs();
        #endif
    }
    /** Create the results directory */
    CmvUtil::createResultsDirectory();
    string summariesFileName = CmvUtil::FILE_NAME_SUMMARIES;
    SummaryStats *summaryStats = new SummaryStats(summariesFileName);

    /** Determine and loop over list of input files in the working directory */
    CmvUtil::findInputFiles();

    for (vector<string>::iterator inputFileIter = CmvUtil::filesToRun.begin(); 
        inputFileIter != CmvUtil::filesToRun.end(); inputFileIter++) {
        
        /** Display the file name being run */
        printf("Running File: %s\n", (*inputFileIter).c_str());
        string inputFileName = *inputFileIter;
        string runName = inputFileName.substr(0, inputFileName.find(CmvUtil::FILE_EXTENSION_FOR_INPUT));

        /** Create the results sub directory for the run (same name as the .in file) */
        /*
        #if defined(_WIN32)
            string resultsSubDirectory = CmvUtil::resultsDirectory;
            resultsSubDirectory.append("\\");
            size_t lastindex = inputFileName.find_last_of(".");
            string rawFileName = inputFileName.substr(0, lastindex);
            resultsSubDirectory.append(rawFileName);
            _mkdir(resultsSubDirectory.c_str());
        #else
            string resultsSubDirectory = CmvUtil::resultsDirectory;
			resultsSubDirectory.append("/");
			size_t lastindex = inputFileName.find_last_of("."); 
			string rawFileName = inputFileName.substr(0, lastindex); 
			resultsSubDirectory.append(rawFileName);
			mkdir(resultsSubDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        #endif
        */

        /** Create the simulation context and read in the input file/ */
        SimContext* simContext = new SimContext(runName);
        
        try {
            simContext->readInputs();
            
        }
        
        catch (string &errorString) {
            printf("%s\n", errorString.c_str());
            continue;
        }

        /** Initializes the random number generator with either a random or fixed seed */
        CmvUtil::setRandomSeedType(false); // use fixed seed only for now.

        /* Create a new run statistics object for this simulation context */
        RunStats *runStats = new RunStats(runName, simContext);
        CostStats *costStats = new CostStats(runName, simContext);
        Tracer *tracer = new Tracer(runName, simContext, 1);

        tracer->openTraceFile();
        tracer->printTraceHeader();

        /** Load the number of cohorts and settings for stopping simulation */
        int numCohortsLimit = simContext->getRunSpecsInputs()->numCohorts;
        int numRun = 0;

        CmvUtil::changeDirectoryToResults();
  

        /* Patient statistics counting vectors */
        vector<int> NUM_IN_EACH_SUBCOHORT(5, 0);
        vector<float> PROPORTION_IN_EACH_SUBCOHORT(5, 0);
        double sumMaternalAge = 0;
        double sumSquaredMaternalAge = 0;
        double meanMaternalAge;
        double stdDevMaternalAge;
        int numPrevCMV = 0;
        
        while (numRun < numCohortsLimit ) {
            bool trace = false;
            if (numRun < simContext->getRunSpecsInputs()->numberOfPatientsToTrace) {
                trace = true;
            }
            /* Create Patient object */
            Patient *patient = new Patient(simContext, runStats, costStats, tracer, numRun, trace);

            /* Setting up the state updaters for the patient */
            StateUpdater* stateUpdater = new StateUpdater(patient);

            /** Printing initial patient stats to trace */
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "Patient #%d \n", numRun);
                tracer->printTrace(1, "    Tracing enabled: %s, Maternal Age (years): %d\n", TRUE_FALSE[patient->getGeneralState()->tracingEnabled].c_str(),
                        patient->getMaternalState()->maternalAgeYears);
                tracer->printTrace(1, "    Prevalent CMV: %s, Previous CMV: %s, ", TRUE_FALSE[patient->getMaternalDiseaseState()->isPrevalentCMVCase].c_str(),
                    TRUE_FALSE[patient->getMaternalDiseaseState()->prevCMVCase].c_str());
                if (patient->getMaternalDiseaseState()->weekLastCMV < -5000) {
                    tracer->printTrace(1, "Week Last CMV: NEVER\n");
                }
                else {
                    tracer->printTrace(1, "Week Last CMV: %d\n", patient->getMaternalDiseaseState()->weekLastCMV);
                }
                tracer->printTrace(1, "BEGIN WEEKLY SIM\n");
            }
            
            /** While delivery has not occurred and child is alive, loop over the pregnancy of the patient and simulate weeks */

            while (!patient->getChildState()->deliveryOcurred && patient->getChildState()->isAlive) {
                patient->simulateWeek();             
            }
            /* End of pregnancy */
            
            /** Simulate life months of child until death */
            // THIS WILL BE CODE FOR SIMULATING THE CHILD'S LIFE. WILL NOT BE USED FOR FIRST PART OF MODEL DEVELOPMENT

            /* End of life */
            if (patient->getGeneralState()->tracingEnabled) {
                tracer->printTrace(1, "\n");
            }

            delete patient;
            delete stateUpdater;
            numRun++;
        }
        /** Write out the stats file for this simulation context and add to the summary stats */
		runStats->finalizeStats();
        try {
            runStats->writeStatsFile();
        }
        catch (string &errorString) {
            printf("%s\n", errorString.c_str());
        }
        
        /** Add the individual run stats to the summary stats object */
        summaryStats->addRunStats(runStats);

        tracer->closeTraceFile();
        delete tracer;

        /** destroy the runStats, costStats, and simContext objects */
        delete runStats;
        delete costStats;
        delete simContext;

    };
    /** 
     * Four parts to main function: Pregnancy/Prenatal, Postpartum (week 40 - 44), Infant (Month 1 - 24), Adult (Month 24+).
     * Important to make distinct parts because they have different timesteps and relevant update functions. 
    */
    //summaryStats->finalizeStats();
    try {
        summaryStats->writeSummariesFile();
    }
    catch (string &errorString) {
        printf("%s\n", errorString.c_str());
    }
    delete summaryStats;
    std::cout << "Simulation Ended Successfully. Press ENTER key to exit." << endl; std::cin.get();

    return 0;
} /* end main */