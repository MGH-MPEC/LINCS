#include "include.h"

/** \brief Constructor takes run name and associated simulation context pointer as parameters
 *
 * \param runName a string representing the run name associated with this costStats object
 * \param simContext a pointer to the SimContext representing the inputs associated with this CostSTats object*/

CostStats::CostStats(string runName, SimContext *simContext) {
	costStatsFileName = runName;
	costStatsFileName.append(CmvUtil::FILE_EXTENSION_FOR_COSTS_OUTPUT);
	this->simContext = simContext;
}

/** \brief Destructor */
CostStats::~CostStats(void) {
} /* end Destructor */
