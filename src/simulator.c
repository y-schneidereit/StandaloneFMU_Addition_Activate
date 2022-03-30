#include <stdio.h>
#include <Windows.h>

#include "fmi2Functions.h"

// model specific constants
#define GUID "{49ca57ed-9c93-416f-6bc9-ee0ef5a44226}"
#define RESOURCE_LOCATION "file:///C:/Users/schyan01/github/standalonefmu_addition_binaries" // absolut path to the unziped fmu

// callback functions
static void cb_logMessage(fmi2ComponentEnvironment componentEnvironment, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...) {
	printf("%s\n", message);
}

static void* cb_allocateMemory(size_t nobj, size_t size) {
	return calloc(nobj, size);
}

static void cb_freeMemory(void* obj) {
	free(obj);
}

#define CHECK_STATUS(S) { status = S; if (status != fmi2OK) goto TERMINATE; }

int main(int argc, char *argv[]) {
	HMODULE libraryHandle = LoadLibraryA("C:\\Users\\schyan01\\github\\StandaloneFMU_Addition_binaries\\model\\binaries\\win64\\Addition_binaries.dll");

	if (!libraryHandle)
	{
		return EXIT_FAILURE;
	}
	
	fmi2InstantiateTYPE* InstantiatePtr = NULL;
	fmi2FreeInstanceTYPE* FreeInstancePtr = NULL;
	fmi2SetupExperimentTYPE* SetupExperimentPtr = NULL;
	fmi2EnterInitializationModeTYPE* EnterInitializationModePtr = NULL;
	fmi2ExitInitializationModeTYPE* ExitInitializationModePtr = NULL;
	fmi2SetRealTYPE* SetRealPtr = NULL;
	fmi2GetRealTYPE* GetRealPtr = NULL;
	fmi2DoStepTYPE* DoStepPtr = NULL;
	fmi2TerminateTYPE* TerminatePtr = NULL;

	InstantiatePtr = GetProcAddress(libraryHandle, "fmi2Instantiate");
	FreeInstancePtr = GetProcAddress(libraryHandle, "fmi2FreeInstance");
	SetupExperimentPtr = GetProcAddress(libraryHandle, "fmi2SetupExperiment");
	EnterInitializationModePtr = GetProcAddress(libraryHandle, "fmi2EnterInitializationMode");
	ExitInitializationModePtr = GetProcAddress(libraryHandle, "fmi2ExitInitializationMode");
	SetRealPtr = GetProcAddress(libraryHandle, "fmi2SetReal");
	GetRealPtr = GetProcAddress(libraryHandle, "fmi2GetReal");
	DoStepPtr = GetProcAddress(libraryHandle, "fmi2DoStep");
	TerminatePtr = GetProcAddress(libraryHandle, "fmi2Terminate");

	if (NULL == InstantiatePtr || NULL == FreeInstancePtr || NULL == SetupExperimentPtr || NULL == EnterInitializationModePtr || NULL == ExitInitializationModePtr
		|| NULL == SetRealPtr || NULL == GetRealPtr || NULL == DoStepPtr || NULL == TerminatePtr)
	{
		return EXIT_FAILURE;
	}

	fmi2Status status = fmi2OK;

	fmi2CallbackFunctions callbacks = {cb_logMessage, cb_allocateMemory, cb_freeMemory, NULL, NULL};

	fmi2Component c = InstantiatePtr("instance1", fmi2CoSimulation, GUID, RESOURCE_LOCATION, &callbacks, fmi2False, fmi2False);
	
	if (!c) return 1;
	
	fmi2Real Time = 0;
	fmi2Real stepSize = 1;
	fmi2Real stopTime = 10;

	// Informs the FMU to setup the experiment. Must be called after fmi2Instantiate and befor fmi2EnterInitializationMode
	CHECK_STATUS(SetupExperimentPtr(c, fmi2False, 0, Time, fmi2False, 0));
	
	// Informs the FMU to enter Initialization Mode.
	CHECK_STATUS(EnterInitializationModePtr(c));
	
	fmi2ValueReference x_ref = 0;
	fmi2Real x = 0;

	fmi2ValueReference y_ref = 1;
	fmi2Real y = 0;

	fmi2ValueReference Ergebnis_ref = 2;
	fmi2Real Ergebnis;

	CHECK_STATUS(SetRealPtr(c, &x_ref, 1, &x));
	CHECK_STATUS(SetRealPtr(c, &y_ref, 1, &y));
	
	CHECK_STATUS(ExitInitializationModePtr(c));
	
	printf("time, x, y, Ergenbis\n");

	for (int nSteps = 0; nSteps <= 10; nSteps++) {

		Time = nSteps * stepSize;

		// set an input
		CHECK_STATUS(SetRealPtr(c, &x_ref, 1, &x));
		CHECK_STATUS(SetRealPtr(c, &y_ref, 1, &y));
		
		// perform a simulation step
		CHECK_STATUS(DoStepPtr(c, Time, stepSize, fmi2True));	//The computation of a time step is started.
		
		// get an output
		CHECK_STATUS(GetRealPtr(c, &Ergebnis_ref, 1, &Ergebnis));

		printf("%.2f, %.0f, %.0f, %.0f\n", Time, x, y, Ergebnis);
		
		x++;
		y+=2;
	}
	
TERMINATE:
	TerminatePtr(c);
	
	// clean up
	if (status < fmi2Fatal) {
		FreeInstancePtr(c);
	}

	FreeLibrary(libraryHandle);
	
	return status;
}
