#ifndef COWTOOLS_H
#define COWTOOLS_H

float lerp(float a, float b, float t)
{
	return a * (1-t) + b * t;
}

#endif
