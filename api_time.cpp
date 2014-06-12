#include "main.hpp"
#include "api_time.hpp"
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/* All this was commented our because i replicated a lot of it with lua */

unsigned long api_time::zone(string csv, string tz) {
	/*
	boost::local_time::tz_database db;
	db.load_from_file(csv);
	boost::local_time::time_zone_ptr tok_zone = db.time_zone_from_region(tz);
	boost::local_time::local_date_time tok_time = boost::local_sec_clock::local_time(tok_zone);
	return tok_time.total_seconds();
	*/
	return 0;
}

/*
 * // Load the timezone database
tz_database db;
// TODO: Adjust this path to your environment
db.load_from_file("./boost/libs/date_time/data/date_time_zonespec.csv"); 

// Get the Sydney timezone
time_zone_ptr sydney_zone = db.time_zone_from_region("Australia/Sydney");

// Current date/time in Sydney
local_date_time sydney_time = local_sec_clock::local_time(sydney_zone);

// Format sydney_time in desired format
std::ostringstream formatter;
formatter.imbue(std::locale(), new local_time_facet("%Y%m%d"));
formatter << sydney_time;*/

/*
 *   tz_database tz_db; 
  tz_db.load_from_file("../date_time_zonespec.csv"); 
  time_zone_ptr nyc_tz = tz_db.time_zone_from_region("America/New_York"); 
  time_zone_ptr syd_tz = tz_db.time_zone_from_region("Australia/Sydney"); 
// Convert it to NY time epoch (not sure how) 
  local_date_time in_sydney(now, syd_tz, local_date_time::NOT_DATE_TIME_ON_ERROR); 
  local_date_time in_ny = in_sydney.local_time_in(nyc_tz); 
  cout << "\nNow in New York " << in_ny << endl; 


OR 
 // exctact the date and duration since 00:00:00 from now 
  date in_date(2004,10,04);     // example 
  time_duration td(12,14,32);   // example 
  // create the local_date_time in sydney 
  local_date_time syd_time(in_date, 
                           td, 
                           syd_tz, 
                           local_date_time::NOT_DATE_TIME_ON_ERROR); 
                           * 
  // convert it to New York 
  local_date_time in_ny = in_sydney.local_time_in(nyc_tz); 
  // print time now in New York. 
  cout << "\nNow in New York " << in_ny << endl; */

/*
#include "main.hpp"
#include "api_time.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#define METRIC_YEAR 0
#define METRIC_MONTH 1
#define METRIC_WEEK 2
#define METRIC_DAY 3
#define METRIC_HOUR 4
#define METRIC_MINUTE 5
#define METRIC_SECOND 6
#define METRIC_MAX 7

struct ago_data {
	unsigned long ms;
	char name[7];
};

ago_data ag[METRIC_MAX] = {
	{31536000, "year"},
	{2678400, "month"},
	{604800, "week"},
	{86400, "day"},
	{3600, "hour"},
	{60, "minute"},
	{0, "second"}
};

api_time* g_time = new api_time;

void api_time::set_start_time(){ 
	this->start_time = std::time(0);
}

unsigned long api_time::uptime() {
	return std::time(0) - this->start_time;
}

string api_time::ago(unsigned long diff) {
	stringstream ss;
	
	for(int i = 0; i < METRIC_MAX; i++) {
		unsigned long total = ((ag[i].ms > 0) ? (unsigned long) floor(diff / ag[i].ms) : diff);
		
		if(ag[i].ms > 0) { 
			diff %= ag[i].ms;
		}
		
		if(total > 0) {
			ss << total << " " << ag[i].name << ((total > 1) ? "s " : " ");
		}
	}
	
	return (ss.str().substr(0, ss.str().length() - 1));
}

// If the final result is lower than unix timestamp 0, it errors out.
// I could make this return __int64, but since Lua only uses floats...
// Not really much can be done about that
unsigned long api_time::strtotime(string input, unsigned long use) {
	if(!input.empty()) {
		if(input == "tomorrow") {
			return use + ag[METRIC_DAY].ms;
		} else if(input == "yesterday") {
			return use - ag[METRIC_DAY].ms;
		} else if(input == "today" || input == "now") {
			return std::time(0);
		} else if(input.substr(0, 4) == "last" || input.substr(0, 4) == "next") {
			for(int i = 0; i < METRIC_MAX; i++) {
				if(input.substr(5) == ag[i].name) {
					return ((input.c_str()[0] == 'n') ? (use + ag[i].ms) : (use - ag[i].ms));
				}
			}
		} else if(input.c_str()[0] == '+' || input.c_str()[0] == '-') {
			boost::cmatch match;
			boost::regex parse("^(\\+|\\-|)(\x20|)(\\d+)\\s(\\S+)");
			
			if(boost::regex_search(input.c_str(), match, parse)) {
				string length = match[3];
				string metric = match[4];
				
				//printf("len [%s] - met [%s]\n", length.c_str(), metric.c_str());
				
				unsigned long length_time = 0;
				
				try {
					length_time = boost::lexical_cast<unsigned long>(length);
				} catch( boost::bad_lexical_cast & ) {
					printf("WARNING: lexical_cast failed on length [%s]\n", length.c_str());
				}
				
				if(length_time > 0) {
					for(int i = 0; i < METRIC_MAX; i++) {
						if(memcmp(ag[i].name, metric.c_str(), strlen(ag[i].name)) == 0) {
							return ((input.c_str()[0] == '+') ? (use + ag[i].ms * length_time) : (use - ag[i].ms * length_time));
						}
					}
				}
			}
		}
	}
	
	return 0; // This is what we'd call an error, I guess
}

api_time* api_time::singleton() {
	return g_time;
}
*/
