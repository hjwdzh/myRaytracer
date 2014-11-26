#include "sampler.h"
#include "time.h"
#include <algorithm>
using namespace std;

float* create_sampler(int size) {
	srand((unsigned)time(0));
	float* sampler = new float[size * size * 2 + 2];
	int block = size * size;
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < size; ++j) {
			sampler[(i*size+j)<<1] = ((j+0.0)+(i+0.0)/size)/size + (rand()+0.0) / RAND_MAX / block;
			sampler[((i*size+j)<<1)+1] = ((j+0.0)/size+(i+0.0))/size + (rand()+0.0) / RAND_MAX / block;
		}
	for (int i = 0; i < 2; ++i) {
		for (int j = 1; j < block; ++j) {
			int k = rand() % (j + 1);
			float temp = sampler[(j << 1) + i];
			sampler[(j << 1) + i] = sampler[(k << 1) + i];
			sampler[(k << 1) + i] = temp;
		}
	}
	sampler[size*size*2-2] = -1;
	sampler[size*size*2-1] = -1;

	return sampler;
}
