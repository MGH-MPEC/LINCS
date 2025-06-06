#include "include.h"

/* BEGIN MONTH UPDATER FUNCTIONS */
/** \brief Constructor takes in the patient object and determines if updates can occur */
BeginWeekUpdater::BeginWeekUpdater(Patient *patient) : StateUpdater(patient) {

}

/* \brief Destructor is empty, no cleanup required */
BeginWeekUpdater::~BeginWeekUpdater(void) {
    //void
}

void BeginWeekUpdater::performInitialization() {
    StateUpdater::performInitialUpdates();
}

void BeginWeekUpdater::performWeeklyUpdates() {
    if (patient->getGeneralState()->tracingEnabled) {
        tracer->printTrace(1, "Week %d: \n", patient->getGeneralState()->weekNum);
    }
    StateUpdater::setAppointmentThisWeek(false);

}