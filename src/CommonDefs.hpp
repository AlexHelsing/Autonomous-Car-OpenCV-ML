#ifndef COMMON_DEFS_HPP
#define COMMON_DEFS_HPP

#include "HsvColorSeparator.hpp"
#include "NoiseRemover.hpp"
#include "ContourFinder.hpp"

extern HsvColorSeparator colorSeparator;
extern NoiseRemover noiseRemover;
extern ContourFinder contourFinder;

extern int minContourArea;
extern int maxContourArea;

#endif // COMMON_DEFS_HPP