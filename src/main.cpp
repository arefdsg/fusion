#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "core.h"
using namespace std;

engine e;
void main() {
	srand((unsigned)time(NULL));
	e.init_types();
	e.init_special();
	e.init_status();
	e.init_moves();
	e.init_mon();
	mon m1, m2, m3, m4;
	e.make_mon(string("1"), 1, m1);
	e.make_mon(string("1"), 50, m2);
	e.make_mon(string("1"), 100, m3);
	e.make_mon(string("130"), 100, m4);
	e.damage(m4, m1, string("FIRE BLAST"));
	int a = 0;
}