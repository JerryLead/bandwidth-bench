#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

typedef struct {
	uint64_t item[8];
} item_t;

class BandwidthBench {
private:
	static double get_time() {
		struct timeval t;
		gettimeofday(&t, NULL);
		return t.tv_sec + t.tv_usec*1e-6;
	}

	static void ShuffleRange(unsigned* base, unsigned count) {
		for (unsigned i = 0; i < count; i++) {
			base[i] = i;
		}

		srand(3);
		for (unsigned i = 0; i < count; i++) {
			unsigned index = rand() % count;
			unsigned temp = base[i];
			base[i] = base[index];
			base[index] = temp;
		}
	}

public:
	static double MeasureMemoryBW (unsigned blockSizeInCL, unsigned totalSizeInCL) {
		if (totalSizeInCL%blockSizeInCL != 0) {
			cout << "totalSizeInCL not divisible by blockSizeInCL" << endl;
			exit(1);
		}

		unsigned numBlocks = totalSizeInCL/blockSizeInCL;

		cout << "numBlocks: " << numBlocks << endl;

		item_t* base = (item_t*)aligned_alloc(64, totalSizeInCL*sizeof(item_t));
		for (unsigned i = 0; i < totalSizeInCL; i++) {
			base[i].item[0] = 2;
		}

		unsigned* indexes = (unsigned*)malloc(numBlocks*sizeof(unsigned));
		ShuffleRange(indexes, numBlocks);

		uint64_t accumulation = 0;

		double start = get_time();
		for (unsigned b = 0; b < numBlocks; b++) {

			item_t* blockBase = base + indexes[b]*blockSizeInCL;
			for (unsigned i = 0; i < blockSizeInCL; i++) {
				__builtin_prefetch(blockBase + 1);
				accumulation += blockBase[i].item[0];
			}

		}
		double end = get_time();

		uint64_t check = totalSizeInCL*2;
		if (accumulation != check) {
			cout << "Error, accumulation: " << accumulation << ", check: " << check << endl;
		}
		else {
			cout << "Pass!" << endl;
		}

		double GBperSecond = (totalSizeInCL*64)/(end-start)/1000000000;

		free(base);
		free(indexes);

		return GBperSecond;
	}


	static double MeasureDiskBW (unsigned blockSizeInCL, unsigned totalSizeInCL) {
		if (totalSizeInCL%blockSizeInCL != 0) {
			cout << "totalSizeInCL not divisible by blockSizeInCL" << endl;
			exit(1);
		}

		const int dir_err = mkdir("temp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (-1 == dir_err) {
			cout << "Error creating directory temp" << endl;
			exit(1);
		}

		unsigned numBlocks = totalSizeInCL/blockSizeInCL;

		cout << "numBlocks: " << numBlocks << endl;

		FILE* file;
		item_t* base = (item_t*)aligned_alloc(64, blockSizeInCL*sizeof(item_t));
		for (unsigned i = 0; i < blockSizeInCL; i++) {
			base[i].item[0] = 2;
		}

		string name = "temp/file";
		for (unsigned b = 0; b < numBlocks; b++) {
			string tempName = name + to_string(b);
			file = fopen(tempName.c_str(), "wb");
			fwrite (base, sizeof(item_t), blockSizeInCL, file);
			fclose(file);
		}

		unsigned* indexes = (unsigned*)malloc(numBlocks*sizeof(unsigned));
		ShuffleRange(indexes, numBlocks);

		uint64_t accumulation = 0;

		system("echo drop caches!");
		system("echo 3 | sudo tee /proc/sys/vm/drop_caches");

		item_t* base2 = (item_t*)aligned_alloc(64, blockSizeInCL*sizeof(item_t));

		double start = get_time();
		for (unsigned b = 0; b < numBlocks; b++) {
			string tempName = name + to_string(indexes[b]);
			file = fopen(tempName.c_str(), "rb");
			size_t tempSize = fread(base2, sizeof(item_t), blockSizeInCL, file);
			fclose(file);
			for (unsigned i = 0; i < blockSizeInCL; i++) {
				__builtin_prefetch(base2 + i);
				accumulation += base2[i].item[0];
			}
		}
		double end = get_time();

		uint64_t check = blockSizeInCL*numBlocks*2;
		if (accumulation != check) {
			cout << "Error, accumulation: " << accumulation << ", check: " << check << endl;
		}
		else {
			cout << "Pass!" << endl;
		}

		double GBperSecond = (totalSizeInCL*64)/(end-start)/1000000000;

		for (unsigned b = 0; b < numBlocks; b++) {
			string tempName = name + to_string(b);
			remove(tempName.c_str());
		}
		rmdir("temp");

		free(base);
		free(base2);
		free(indexes);

		return GBperSecond;
	}

	static double MeasureDiskBWNoWrite (unsigned blockSizeInCL, unsigned totalSizeInCL) {
		if (totalSizeInCL%blockSizeInCL != 0) {
			cout << "totalSizeInCL not divisible by blockSizeInCL" << endl;
			exit(1);
		}

		unsigned numBlocks = totalSizeInCL/blockSizeInCL;

		cout << "numBlocks: " << numBlocks << endl;

		unsigned* indexes = (unsigned*)malloc(numBlocks*sizeof(unsigned));
		ShuffleRange(indexes, numBlocks);

		uint64_t accumulation = 0;

		item_t* base = (item_t*)aligned_alloc(64, blockSizeInCL*sizeof(item_t));

		FILE* file;
		file = fopen("tempFile", "rb");

		double start = get_time();
		for (unsigned b = 0; b < numBlocks; b++) {
			fseek(file, indexes[b]*blockSizeInCL*sizeof(item_t), SEEK_SET);
			size_t tempSize = fread(base, sizeof(item_t), blockSizeInCL, file);
			for (unsigned i = 0; i < blockSizeInCL; i++) {
				__builtin_prefetch(base + i);
				accumulation += base[i].item[0];
			}
		}
		double end = get_time();

		fclose(file);

		uint64_t check = blockSizeInCL*numBlocks*2;
		if (accumulation != check) {
			cout << "Error, accumulation: " << accumulation << ", check: " << check << endl;
		}
		else {
			cout << "Pass!" << endl;
		}

		double GBperSecond = (totalSizeInCL*64)/(end-start)/1000000000;

		free(base);
		free(indexes);

		return GBperSecond;
	}

	static void WriteFile(unsigned totalSizeInCL) {
		FILE* file;
		item_t* base = (item_t*)aligned_alloc(64, totalSizeInCL*sizeof(item_t));
		for (unsigned i = 0; i < totalSizeInCL; i++) {
			base[i].item[0] = 2;
		}

		file = fopen("tempFile", "wb");
		fwrite (base, sizeof(item_t), totalSizeInCL, file);
		fclose(file);
		
		free(base);
	}
};