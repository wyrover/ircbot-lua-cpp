#pragma once

class api_time {
public:
	unsigned long zone(string csv, string tz);

	static api_time* singleton();

private:
	unsigned long start_time;
};
