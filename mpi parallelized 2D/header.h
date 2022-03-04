#pragma once

typedef struct DWMHeader {
	int x;
	int y;
	int z; //mantain compatibility with 3d rooms (z must always be 1)
	int frequency;
} Header;