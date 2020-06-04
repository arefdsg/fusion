#include "timer.h"
#include <string>
#include <vector>
#include <fstream>
#ifdef __APPLE__
#include <sys/time.h>
#else
#include <windows.h>
#endif

#ifdef __APPLE__
vector<timeval> holders;
#else
vector<LARGE_INTEGER> holders;
LARGE_INTEGER freq;
#endif

extern bool safe_getline(ifstream &f, string& s);

timer::timer() {
	string line;
	speedup = 1.0;
	ifstream f((string("../speedup.txt")));
	if (f.is_open()) {
		safe_getline(f, line);
		speedup = stof(line);
	}
	f.close();
#ifdef __APPLE__

#else
	QueryPerformanceFrequency(&freq);
#endif
}

unsigned timer::create() {
    unsigned ret = holders.size();
#ifdef __APPLE__
    timeval temp;
    gettimeofday(&temp, NULL);
#else
	LARGE_INTEGER temp;
	QueryPerformanceCounter(&temp);
#endif
	holders.push_back(temp);
	return ret;
}

void timer::update(unsigned index) {
#ifdef __APPLE__
    gettimeofday(&(holders[index]), NULL);
#else
	QueryPerformanceCounter(&(holders[index]));
#endif
}

double timer::delta(unsigned index) {
#ifdef __APPLE__
    timeval temp;
    gettimeofday(&temp, NULL);
    double f = (temp.tv_sec - holders[index].tv_sec);
    f += (temp.tv_usec - holders[index].tv_usec)/1000000.0;
    return f * speedup;
#else
	LARGE_INTEGER temp;
	QueryPerformanceCounter(&temp);
	double f = 1.0 / freq.QuadPart;
	f = (temp.QuadPart - holders[index].QuadPart) * f;
	return f * speedup;
#endif
}

unsigned timer::get_current_time() {
#ifdef __APPLE__
	timeval temp;
	gettimeofday(&temp, NULL);
	double f = temp.tv_sec;
	f += temp.tv_usec / 1000000.0;
#else
	LARGE_INTEGER temp;
	QueryPerformanceCounter(&temp);
	double f = 1.0 / freq.QuadPart;
	f = (temp.QuadPart) * f;
#endif
	return unsigned(f*1000000.0);
}