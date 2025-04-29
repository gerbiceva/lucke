#pragma once
#include "Config.h"
#include <vector>
#include <unordered_map>

struct Grid {
	int width = GRID_WIDTH;
	int heigth = GRID_HEIGHT;
	int wsize = GRID_WSIZE;
	int hsize = GRID_HSIZE;
	int nw, nh;

	std::unordered_map<int, std::vector<int>> hash;

	Grid() : nw(width / wsize), nh(heigth / hsize) {};
	Grid(int wsize, int hsize, int width, int height) : wsize(wsize), hsize(hsize), nh(heigth / hsize) {};

	const std::vector<int> &getGridIndexes(int x, int y) {
    	// TODO: Find bigger prime numbers or better hashing
		int hnum = x * 7741 + y * 7757;
		
		if(hash.find(hnum) != hash.end()) {
			return hash[hnum];
		}

		for(int yi = y * hsize; yi < (y + 1) * hsize; yi++) {
			for(int xi = x * wsize; xi < (x + 1) * wsize; xi++) {
				hash[hnum].push_back(xi + yi * width);
			}
		}

		return hash[hnum];
	}
};