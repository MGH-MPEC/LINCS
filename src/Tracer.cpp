#include "include.h"

/** \brief Constructor takes in the run name, simulation context, and desired tracing level 
 * \param runName a string used to identify the run correspondong to this Tracer; this->traceFileName will be the runName appended to CmvUtil::FILE_EXTENSION_FOR_TRACE
 * \param *simContext this->simContext
 * \param traceLevel this->traceLevel
*/

Tracer::Tracer(string runName, SimContext *simContext, int traceLevel) {
    traceFileName = runName;
    traceFileName.append(CmvUtil::FILE_EXTENSION_FOR_TRACE);
    
    this->traceLevel = traceLevel;
    this->simContext = simContext;
} /* end Constructor*/

/** \brief Destructor is empty, no cleanup required */
Tracer::~Tracer(void) {

} /* end Destructor*/

void Tracer::openTraceFile() {
    CmvUtil::changeDirectoryToResults();
    traceFile = CmvUtil::openFile(traceFileName.c_str(), "w");
} /* end openTraceFile */

void Tracer::closeTraceFile() {
    if (traceFile == NULL) {
        std::cout << "INVALID TRACE FILE ADDRESS";
        return;
    }
    CmvUtil::closeFile(traceFile);
} /* end closeTraceFile */

void Tracer::printTraceHeader() {
    if (traceFile == NULL) {
        std::cout << "INVALID TRACE FILE ADDRESS";
        return;
    }
    const SimContext::RunSpecsInputs *runSpecsInputs = simContext->getRunSpecsInputs();
    printTrace(1, "==========================\n");
    printTrace(1, "BEGIN SCENARIO %s [$d]\n", runSpecsInputs->runName.c_str(), traceLevel);
}

void Tracer::printTrace(int level, const char *format, ...) {
    // return if trace file is not valid
    if (traceFile == NULL) {
        std::cout << "INVALID TRACE FILE ADDRESS";
        return;
    }
    if (level > traceLevel) {
        return;
    }
    va_list args;
    va_start (args, format);
    vfprintf(traceFile, format, args);
    va_end(args);
} /* end printTrace */

