/***************************************************************************
 *
 * Author: "Sjors H.W. Scheres"
 * MRC Laboratory of Molecular Biology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This complete copyright notice must be included in any revised version of the
 * source code. Additional authorship citations may be added, but existing
 * author citations must be preserved.
 ***************************************************************************/

#ifndef MASK_H_
#define MASK_H_

#include "multidim_array.h"
#include "fftw.h"


namespace relion
{
	// Mask out corners outside sphere (replace by average value)
	// Apply a soft mask (raised cosine with cosine_width pixels width)
	void softMaskOutsideMap(MultidimArray<DOUBLE> &vol, DOUBLE radius = -1., DOUBLE cosine_width = 3, MultidimArray<DOUBLE> *Mnoise = NULL);

	// Apply a soft mask and set density outside the mask at the average value of those pixels in the original map
	void softMaskOutsideMap(MultidimArray<DOUBLE> &vol, MultidimArray<DOUBLE> &msk, bool invert_mask = false);

	// Make an automated mask, based on:
	// 1. initial binarization (based on ini_mask_density_threshold)
	// 2. Growing extend_ini_mask in all directions
	// 3. Putting a raised-cosine edge on the mask with width width_soft_mask_edge
	// If verb, then output description of steps and progress bars
	void autoMask(MultidimArray<DOUBLE> &img_in, MultidimArray<DOUBLE> &msk_out,
		DOUBLE  ini_mask_density_threshold, DOUBLE extend_ini_mask, DOUBLE width_soft_mask_edge, bool verb = false);

	// Fills mask with a soft-edge circular mask (soft-edge in between radius and radius_p), centred at (x, y, z)
	void raisedCosineMask(MultidimArray<DOUBLE> &mask, DOUBLE radius, DOUBLE radius_p, int x, int y, int z = 0);

}
#endif /* MASK_H_ */
