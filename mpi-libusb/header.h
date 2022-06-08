#pragma once

/** @file */

/** @struct DWMHeader
 *  @brief Contains x, y, z axis size and frequency associated with a room.
 *  @var DWMHeader::x
 * 	The size of the x axis.
 *  @var DWMHeader::y
 *  The size of the y axis.
 *  @var DWMHeader::z
 *  The size of the z axis.
 *  @var DWMHeader::frequency
 *  The frequency. Used to determine number of algorithm iterations by runtime in seconds * DWMHeader::frequency.
 */
typedef struct DWMHeader {
	int x;
	int y;
	int z;
	int frequency;
} Header;