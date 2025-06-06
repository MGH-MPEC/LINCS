#include "include.h"

/** \brief Empty constructor and destructor, should never create an instance of this class */
CmvUtil::CmvUtil(void)
{
}
CmvUtil::~CmvUtil(void)
{
}

/* Constant string values for CMVSim version and file/directory information */
/** The CMVSim Input Version to match that from the .in file */
const char *CmvUtil::CMV_INPUT_VERSION = "1";
/** CMVSim version string: the version label used */
const char *CmvUtil::CMV_VERSION_STRING = "2_8_0";
/** The compile date of the most recent release */
const char *CmvUtil::CMV_EXECUTABLE_COMPILED_DATE = "2025-05-01";
/** .tmp */
const char *CmvUtil::FILE_EXTENSION_FOR_TEMP = ".tmp";
/** .txt */
const char *CmvUtil::FILE_EXTENSION_FOR_TRACE = ".txt";
/** .out */
const char *CmvUtil::FILE_EXTENSION_FOR_OUTPUT = ".out";
/** .cout */
const char *CmvUtil::FILE_EXTENSION_FOR_COSTS_OUTPUT = ".cout";
/** .in */
const char *CmvUtil::FILE_EXTENSION_FOR_INPUT = ".in";
/** *.in */
const char *CmvUtil::FILE_EXTENSION_INPUT_SEARCH_STR = "*.in";
/** popstats.outCmvUtil*/
const char *CmvUtil::FILE_NAME_SUMMARIES = "popstats.out";

/** Vector of the file names to be run*/
std::vector<std::string> CmvUtil::filesToRun;
/** The inputs directory path */
std::string CmvUtil::inputsDirectory;
/** The output directory path */
std::string CmvUtil::resultsDirectory;
/** True if we're using random seed, false for fixed seed */
bool CmvUtil::useRandomSeedByTime;

/** \brief Random number generator class */
MTRand CmvUtil::mtRand;

/** \brief useCurrentDirectoryForInputs determines the current directory and sets as inputs directory */
void CmvUtil::useCurrentDirectoryForInputs() {
#if defined(_WIN32)
	char buffer[512];
	_getcwd(buffer, 512);
	inputsDirectory = buffer;
#elif __APPLE__
	char buffer[512];
	uint32_t size = sizeof(buffer);
	_NSGetExecutablePath(buffer, &size);
	inputsDirectory = buffer;
	inputsDirectory = inputsDirectory.substr(0, inputsDirectory.find_last_of("\\/"));
#else
	char buffer[512];
	getcwd(buffer, 512);
	inputsDirectory = buffer;
#endif
} /* end useCurrentDirectoryForInputs */

/** \brief findInputFiles locates all the .in files in the current directory and adds them
	to the filesToRun vector */
void CmvUtil::findInputFiles() {
#if defined(_WIN32)
	long hFile;
	struct _finddata_t tFileInfo;
	hFile = _findfirst( FILE_EXTENSION_INPUT_SEARCH_STR, &tFileInfo );
	int nInputFiles = 0;
	string fileName;

	//get the list of files that we have to process
	filesToRun.clear();
	do {
		fileName = (char *) tFileInfo.name;
		filesToRun.push_back(fileName);
		nInputFiles++;
	} while ( _findnext ( hFile, &tFileInfo ) == 0 );
	_findclose( hFile );
#else
	glob_t files;
	glob(FILE_EXTENSION_INPUT_SEARCH_STR, GLOB_ERR, NULL, &files);
	int nInputFiles = 0;
	string fileName;

	filesToRun.clear();
	//get the list of files that we have to process
	int i;
	for( i = 0; i < files.gl_pathc; i++) {
		fileName = (char *) files.gl_pathv[i];
		filesToRun.push_back(fileName);
		++nInputFiles;
	}
	globfree( &files);
#endif
} /* end findInputFiles */

/** \brief createResultsDirectory creates the directory "results" as a subdirectory of the inputs one */
void CmvUtil::createResultsDirectory() {
#if defined(_WIN32)
	resultsDirectory = inputsDirectory;
	resultsDirectory.append("\\");
	resultsDirectory.append("results");
	_mkdir(resultsDirectory.c_str());
#else
	resultsDirectory = inputsDirectory;
	resultsDirectory.append("/");
	resultsDirectory.append("results");
	mkdir(resultsDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
} /* end createResultsDirectory */

/** \brief changeDirectoryToResults changes the working directory to the results one */
void CmvUtil::changeDirectoryToResults() {
#if defined(_WIN32)
	_chdir(resultsDirectory.c_str());
#else
	chdir(resultsDirectory.c_str());
#endif
} /* end changeDirectoryToResults */

/** \brief changeDirectoryToInputs changes the working directory to the inputs one */
void CmvUtil::changeDirectoryToInputs() {
#if defined(_WIN32)
	_chdir(inputsDirectory.c_str());
#else
	chdir(inputsDirectory.c_str());
#endif
} /* end changeDirectoryToInputs */

/** \brief getDateString places the current date string in the specified buffer
 *  \param buffer a pointer to a char array representing the buffer to add the resulting date string to
 *  \param bufsize an integer representing the size of buffer
 **/
void CmvUtil::getDateString(char *buffer, int bufsize) {
#if defined(_WIN32)
	_strdate(buffer);
#else
	time_t currTime;
	time(&currTime);
	strftime(buffer, bufsize, "%m/%d/%y",localtime(&currTime));
#endif
} /* end getDateString */

/** \brief getTimeString places the current system time string in the specified buffer
 *  \param buffer a pointer to a char array representing the buffer to add the resulting time string to
 *  \param bufsize an integer representing the size of buffer */
void CmvUtil::getTimeString(char *buffer, int bufsize) {
#if defined(_WIN32)
	_strtime(buffer);
#else
	time_t currTime;
	time(&currTime);
	strftime(buffer, bufsize, "%H:%M:%S",localtime(&currTime));
#endif
} /* end getTimeString */

/** \brief getTrimester returns the current trimester of pregnancy given the patient's 
 * current time into the pregnancy. 
*/
int CmvUtil::getTrimester(Patient* patient) {
	int trimester;
	if (patient->getGeneralState()->weekNum < 13 && patient->getGeneralState()->weekNum >=0) {
		trimester = SimContext::TRIMESTER_1;
	}
	else if (patient->getGeneralState()->weekNum < 28) {
		trimester = SimContext::TRIMESTER_2;
	}
	else if (patient->getGeneralState()->weekNum < 42) {
		trimester = SimContext::TRIMESTER_3;
	}
	else {
		cout << "ERROR: Trying to get trimester while not in pregnancy. Check where CmvUtil::getTrimester() is being called." << endl; cin.get();
	}
	return trimester;
}


/** \brief fileExists returns true if the specified file exists, false otherwise
 * \param filename a pointer to a character array representing the name of the file
 **/
bool CmvUtil::fileExists(const char *filename) {
	FILE *file;
	//fopen_s(&file, filename, "r");
	file = fopen(filename, "r");
	if (!file)
		return false;
	fclose(file);
	return true;
} /* end fileExists */

/** \brief openFile opens the specified file in the given mode
 * \param filename a pointer to a character array representing the name of the file
 * \param mode a pointer to a character array representing the mode to open the file in: "r" for read, "w" for write, "a" for append, "r+" for reading and writing an existing file, "w+" for reading and writing an empty file, "a+" for reading and appending to a file
 **/
FILE *CmvUtil::openFile(const char *filename, const char *mode) {
	FILE *file = fopen(filename, mode);
	return file;
} /* end openFile */

/** \brief closeFile closes the specified file
 * \param filename a pointer to a character array representing the name of the file
 **/
void CmvUtil::closeFile(FILE *file) {
	fclose(file);
} /* end closeFile */

