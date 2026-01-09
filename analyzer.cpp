#include "analyzer.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

// We keep data here because we don't want to edit analyzer.h
static unordered_map<string, long long> countTable;
static unordered_map<string, array<long long, 24>> hourCountTable;
/*tried to merge it with split function also
bool isNumber(const string& numbr) {
    if (numbr.empty()) 
        return false;
    for (char c : numbr) {
        if (!isdigit((unsigned char)c)) 
            return false;
    }
    return true;
}
*/
/* old
bool splitto6parts(const string& line, string parts[6]) {
    int index = 0;
    string current = "";

    for (char c : line) {
        if (c == ',') {
            if (index >= 5) 
                return false;
            parts[index++] = current;
            current.clear();
        } else {
            current += c;
        }
    }
    if (index != 5) //it must have 6 parts(or 5 commas)
        return false;
    parts[5] = current;
    return true;
}
*/
bool compZone(const ZoneCount& a, const ZoneCount& b) {
    if (a.count != b.count) {
        return a.count > b.count;
    }
    return a.zone < b.zone;
}

bool compSlot(const SlotCount& a, const SlotCount& b) {
    if (a.count != b.count)
        return a.count > b.count;
    if (a.zone != b.zone)
        return a.zone < b.zone;
    return a.hour < b.hour;
}

bool splittoparts(const string& line, string& pickUpZone, int& hour) {
    int index = 0;
    string current = "";
    int datelength = 0;
    char c1 = ' ';
    char c2 = ' ';
    bool isIdEmpty = true;
    pickUpZone.clear();

    for (char c : line) {
        if (c == ',') {
            if (index == 0 && isIdEmpty)
                return false;
            if (index >= 5)
                return false;
			//if (index == 0) 
            //    parts[0] = current;
            //if (index == 1)
            //    pickUpZone = current;
            else if (index == 3) {
                if (datelength < 16) {
                    return false;
                }
                if (!isdigit((unsigned char)c1) || !isdigit((unsigned char)c2))
                    return false;

                hour = (c1 - '0') * 10 + (c2 - '0');
                if (hour < 0 || hour > 23)
                    return false;
            }

            index++;
        } else {
            if (index == 1)
                pickUpZone += c;
            else if (index == 0) {
//                if (!isdigit((unsigned char)c))
//                    return false;
                //if (!isdigit((unsigned char)c)) {
				//	return false;
				//}
                isIdEmpty = false;
            }
            else if (index == 3) {
                if (datelength == 11)
                    c1 = c;
                if (datelength == 12)
                    c2 = c;
                datelength++;
            }
        }
    }
    if (index != 5) //it must have 6 parts(or 5 commas)
        return false;
    return true;
}
/* tried to merge it with split function
 bool getHour(const string& date, int& hour) {
    if (date.size() < 16) 
        return false;
    char c1 = date[11];
    char c2 = date[12];

    if (!isdigit((unsigned char)c1) || !isdigit((unsigned char)c2))
        return false;

    hour = (c1 - '0') * 10 + (c2 - '0');

    if (hour < 0 || hour > 23) 
        return false;

    return true;
}
*/
void TripAnalyzer::ingestFile(const string& csvPath) {
    countTable.clear();
    hourCountTable.clear();

    ifstream file(csvPath);
    if (!file) return;

    string line;
    string pickUpZone;
    pickUpZone.reserve(50);

    while (getline(file, line)) {
        int hour;
        if (!splittoparts(line, pickUpZone, hour))
            continue;
        //if (!isNumber(parts[0])) 
        //    continue;
        if (pickUpZone.empty())
            continue;
        //if(!getHour(parts[2],hour))
        //    continue;

        countTable[pickUpZone]++;
        hourCountTable[pickUpZone][hour]++;
    }
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> vec;
    for (auto it = countTable.begin(); it != countTable.end(); ++it) {
        ZoneCount z;
        z.zone = it->first;
        z.count = it->second;
        vec.push_back(z);
    }

    size_t limit = std::min((size_t)k, vec.size());

    partial_sort(vec.begin(), vec.begin()+ limit, vec.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) 
            return a.count > b.count;

        return a.zone < b.zone;
    });

    if ((int)vec.size() > k) 
        vec.resize(k);
    return vec;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> vec;
    for (auto it = hourCountTable.begin(); it != hourCountTable.end(); ++it) {
        string zone = it->first;

        for (int h = 0; h < 24; h++) {
            if (it->second[h] > 0) {
                SlotCount s;
                s.zone = zone;
                s.hour = h;
                s.count = it->second[h];
                vec.push_back(s);
            }
        }
    }

    size_t limit = std::min((size_t)k, vec.size());

    partial_sort(vec.begin(), vec.begin() + limit, vec.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) 
            return a.count > b.count;
        if (a.zone != b.zone) 
            return a.zone < b.zone;
        return a.hour < b.hour;
});
    partial_sort(vec.begin(), vec.begin() + limit, vec.end(), compSlot);


    if((int)vec.size() > k)
    if ((int)vec.size() > k)
        vec.resize(k);
        
    return vec;
    
}
