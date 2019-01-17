#include <iostream>
#include <string.h>

#include "bandwidth.h"

using namespace std;

int main(int argc, char ** argv) {

	if (argc != 4) {
		cout << "Usage: test <select> <blockSizeInCL> <totalSizeInMB>" << endl;
		return 1;
	}

	char* select = argv[1];
	unsigned blockSizeInCL = stoi(argv[2]);
	unsigned totalSizeInMB = stoi(argv[3]);

	unsigned totalSizeInCL = (totalSizeInMB*1000*1000)/64;

	unsigned numReps = 1;

	double bandwidth = 0.0;

	cout << "blockSizeInCL: " << blockSizeInCL << endl;
	cout << "totalSizeInCL: " << totalSizeInCL << endl;

	if (strcmp(select, "mem") == 0) {
		cout << "Memory BW" << endl;
		for(unsigned i = 0; i < numReps; i++) {
			bandwidth += BandwidthBench::MeasureMemoryBW(blockSizeInCL, totalSizeInCL);
		}
		cout << "bandwidth: " << bandwidth/numReps << endl;
	}
	else if (strcmp(select, "write") == 0) {
		cout << "Disk Write" << endl;
		BandwidthBench::WriteFile(totalSizeInCL);
	}
	else if (strcmp(select, "disk") == 0) {
		cout << "Disk BW" << endl;
		for(unsigned i = 0; i < numReps; i++) {
			bandwidth += BandwidthBench::MeasureDiskBWNoWrite(blockSizeInCL,totalSizeInCL);
		}
		cout << "bandwidth: " << bandwidth/numReps << endl;
	}

	return 0;
}