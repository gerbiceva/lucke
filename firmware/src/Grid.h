#pragma once
#include "Config.h"
#include <vector>
#include <unordered_map>

struct Grid {
	int totalGridWidth;
	int totalGridHeigth;
	int wsize;				// subgrid width
	int hsize;				// subgrid height
	int nw, nh;				// number of squares by width and height

	std::unordered_map<int, std::vector<int>> hash;

	// Grid() : nw(width / wsize), nh(heigth / hsize) {};
	Grid(int width, int height, int wsize, int hsize) : totalGridWidth(width), totalGridHeigth(height), wsize(wsize), hsize(hsize), nh(height / hsize) {};

	const std::vector<int> &getGridIndexes(int x, int y) {
    	// TODO: Find bigger prime numbers or better hashing
		int hnum = x * 7741 + y * 7757;
		
		if(hash.find(hnum) != hash.end()) {
			return hash[hnum];
		}

		for(int yi = y * hsize; yi < (y + 1) * hsize; yi++) {
			for(int xi = x * wsize; xi < (x + 1) * wsize; xi++) {
				hash[hnum].push_back(xi + yi * totalGridWidth);
			}
		}

		return hash[hnum];
	}

	void updateSubpixelSize(int wsize, int hsize) {
		wsize = wsize;
		hsize = hsize;
		nw = (totalGridWidth / wsize);
		nh = (totalGridHeigth / wsize);

		hash.clear();
	}
};

/*
void Controller::update(){
	if(!enabled) return;

	for(uint16_t y = 0; y < grid.nh; y++) {
		for(uint16_t x = 0; x < grid.nw; x++) {
			const auto& indexes = grid.getGridIndexes(x,y);
			int dmxIndex = dmxAddrOffset + (x * lamp->numPxls) + (y * lamp->numPxls) * grid.nw;
			// printf("dmxIndex = %d\n", dmxIndex);
			for(auto index : indexes) {
				for (uint16_t k = 0; k < lamp->numPxls; k++) {
					// LOGF("led[%d] = dmx [%d]\n", (index * lamp->numPxls + k), (dmxIndex + k));
					ledBuffer[index * lamp->numPxls + k] = dmxBuffer[dmxIndex + k]; 
				}C
			}
		}
	}
}
*/
