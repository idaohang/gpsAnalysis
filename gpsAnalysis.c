/*
 ============================================================================
 Name        : GPSAnalyse.c
 Author      : Aaron Kelly and Padraig Cunningham
 Version     : 2.0
  ============================================================================
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define d2r (M_PI / 180.0)
//#define DEBUG				// Turn on/off debugging
#define MAX_COLUMN_WIDTH 65		// Used for formatting my split table

// The node structure for storing paths
struct node {
	double lat;
	double lon;
	double elev;			// I extended Padraig's struct to contain elevation
	char timeString[22];
	struct node *next;
};

// The structure for storing times containing hr, min and sec fields
struct timeStr {
	int hr;
	int min;
	int sec;
};

// Here is my own struct, which I use to help me calculate the split times
struct splitTracker {
	double pathLen;			// Record the current distance travelled
	double elev1;			// First comparison value for calculating elevation change 
	double elev2;			// Second comparison value for calculating elevation change
	char *timeString1;		// First comparison value for calculating the duration of a split	
	char *timeString2;		// Second comparison value for tracking the duration of a split
};

double haversine_m(double lat1, double long1, double lat2, double long2);
int openFileAndLoadData();
char *readStringAfterToken(char *txtstr, char *tkn, char *res, int len, int steps);
double readDoubleAfterToken(char *txtstr, char *tkn, int steps);
double calculate_tot_dist(struct node *lh);
struct node* create_list(double lat, double lon, double elev, char *timeStr);
struct node* add_to_list(double lat, double lon, double elev, char *timeStr);
struct timeStr *timeStrFromString(char *tstring, struct timeStr *tstruct);
int timeDiffV2(struct timeStr *t1, struct timeStr *t2);
void draw_split_title();		// My function prototype
void check_path_length();		// My function prototype

struct node *head = NULL;
struct node *curr = NULL;
struct splitTracker tracker;		// Create a new splitTracker struct

char startTimeStr[25];		// Hold the start and end time strings as global vars.
char finishTimeStr[25];
int splitNumber = 0;		// Keep track of current split as a global variable


// const char *GPX_FILE_PATH = "./inputFiles/Howth-Cross.gpx"; // location of GPX file
const char *GPX_FILE_PATH = "./inputFiles/Run4.9k.gpx"; // location of GPX file
// const char *GPX_FILE_PATH = "./inputFiles/Zell75k.gpx"; // location of GPX file

int main(void) {

	openFileAndLoadData();
	draw_split_title();		// Draw the column headings for the splits
	calculate_tot_dist(head);
	printf("\n");
	return EXIT_SUCCESS;
}

// This function is called every time calculate_tot_distance() adds a new
// gps entry into the node struct. It checks to see what the current distance 
// travelled is, and when it reaches 1km, it runs some calculations and
// outputs a split time and the relevant data. 
// Then, the newest value is used as the new starting point, and the function
// starts checking all over again until it reaches the next 1km split. 
void check_path_length() {
	// Define some variables to hold the results of some calculations
	double averagePace = 0;
	double speed = 0;
	double elevationChange = 0;

	#ifdef DEBUG 
	printf("current length is %lf\n", tracker.pathLen);
	#endif

	// This if statement will trigger when the distance travelled is
	// greater than 1000 metres. 
	if(tracker.pathLen >= 1000) {
		struct timeStr tm1, tm2;	// Create new timeStr structs
		tm1 = *timeStrFromString(tracker.timeString1, &tm1); // New timeStr struct for the start time of the current split
		tm2 = *timeStrFromString(tracker.timeString2, &tm2); // New timeStr struct for the end time of the current split
		// Calculate the average pace and store it in a variable
		averagePace = timeDiffV2(&tm1, &tm2)/tracker.pathLen * 1000.0 / 60.0;
		// Calculate the average speed using the below formula to turn 
		// m/s into km/h
		speed = tracker.pathLen / timeDiffV2(&tm1, &tm2) * 3.6;
		// Calculate the elevation change from the last split
		elevationChange = tracker.elev1 - tracker.elev2;
		// Increment the split number
		splitNumber++;
	
		#ifdef DEBUG
		printf("Elapsed Time: %d sec\n", timeDiffV2(&tm1, &tm2));
		printf("The average pace is %4.2f m/km\n", averagePace);
		printf("Elevation change is %4.2f m\n", elevationChange);
		printf("Start elevation is %lf\n", tracker.elev1);
		printf("Current elevation is %lf\n", tracker.elev2);
		#endif

		printf("%d\t\t  %lf\t   %lf\t   %lf\n", splitNumber, averagePace, speed, elevationChange);

		// Reset the distance travelled to zero, copy the newest
		// split time to be the new start time to measure from
		// Do the same with elevation
		tracker.pathLen = 0;
		tracker.timeString1 = tracker.timeString2;
		tracker.elev1 = tracker.elev2;
	}
}

// A simple function to draw the column headers
void draw_split_title() {
	int i = 0;
	printf("--- Splits statistics ---\n\n");
	for(i = 0; i < MAX_COLUMN_WIDTH; i++) {
		printf("-");
	}
	printf("\n");
	printf("Split No.\t| Pace km/min\t | Speed km/h\t | Elevation m");
	printf("\n");
	for(i = 0; i < MAX_COLUMN_WIDTH; i++) {
		printf("-");
	}
	printf("\n");
}
		
// Calculate distance between two points expressed as lat and long using
// Haversine formula.
double haversine_m(double lat1, double long1, double lat2, double long2) {
	double dlong = (long2 - long1) * d2r;
	double dlat = (lat2 - lat1) * d2r;
	double a = pow(sin(dlat / 2.0), 2)
			+ cos(lat1 * d2r) * cos(lat2 * d2r) * pow(sin(dlong / 2.0), 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	double d = 6367137 * c;

	return d;
}

//Function that is called once at the start to read in the character names.
int openFileAndLoadData() {
	int lineNum = 0;
	char theLine[120];
	char endOfHdr[10] = "<trkseg>";
	char endOfData[10] = "</trkseg>";
	char *timeStrPtr;
	char tempTimeStr[30];

	FILE *fpn = fopen(GPX_FILE_PATH, "r+"); /* open for reading */
	if (fpn == NULL) { /* check does file exist etc */
		printf("Cannot open %s for reading. \n", GPX_FILE_PATH);
		printf("Set up GPX file at %s and restart. \n", GPX_FILE_PATH);
		perror("Error opening GPX file. \n");
	} else {
		// First work through the header in the GPX file.
		// Assume header finishes at "</trkseg>" stored in endOfHdr.
		while (fgets(theLine, sizeof(theLine), fpn)
				&& ((strncmp(theLine, endOfHdr,7) != 0)))
				{
					//Skip through the file until "<trkseg>" is reached (endOfHdr).
					lineNum++;
				}

		// Read the text data and store the lat, lon and time data in a linked list.
		// This operation has also been extended to read the elevation
		while (fgets(theLine, sizeof(theLine), fpn)
				&& ((strncmp(theLine, endOfData,7) != 0))) //finish when "</trkseg>" reached (endOfData)
		{
			timeStrPtr = readStringAfterToken(theLine, "<time>",tempTimeStr, 20, 6);

			add_to_list(
					readDoubleAfterToken(theLine, "lat=",5),
					readDoubleAfterToken(theLine, "lon=",5),
					readDoubleAfterToken(theLine, "<ele>",5),
					timeStrPtr);

			lineNum++;
		}
	}
	fclose(fpn);
return lineNum;
}


// a function that will calculate the total length of the track.

double calculate_tot_dist(struct node *lh){
	double lat1 = 0, lon1 = 0;
	double pathLen = 0;
	struct node *ptr = lh;
	double averagePace = 0;

	struct timeStr tm1,tm2;

	strcpy(startTimeStr, lh->timeString);

    while(ptr != NULL){
    	if (lat1 == 0){
    		// First node
    		lat1 = ptr->lat;
    		lon1 = ptr->lon;
    		ptr = ptr->next;
		tracker.timeString1 = startTimeStr;		// I've started to inject my code here
		tracker.elev1 = ptr->elev;
    	} else {
    		pathLen += haversine_m(lat1, lon1, ptr->lat, ptr->lon);
    		tracker.pathLen += haversine_m(lat1, lon1, ptr->lat, ptr->lon); //
		tracker.elev2 = ptr->elev;

		tracker.timeString2 = ptr->timeString;
		check_path_length();			//

		lat1 = ptr->lat;
    		lon1 = ptr->lon;
    		ptr = ptr->next;

	}
    }

	printf("\n--- Overall statistics ---\n\n");
	printf("Path Length: %5.0f m \n",pathLen);

	strcpy(finishTimeStr, curr->timeString);

    	tm1 = * timeStrFromString(startTimeStr, &tm1);
    	tm2 = * timeStrFromString(finishTimeStr, &tm2);

    	printf("Elapsed Time: %d sec", timeDiffV2(&tm1, &tm2));

    	averagePace = timeDiffV2(&tm1, &tm2)/pathLen*1000.0/60.0;
    	printf("\nAverage Pace: %4.2f km/min", averagePace);

    return pathLen;
}



// This function will return a  substring from string txtstr.
// The string will be searched for the first occurrence of tkn
// and then a substring on length len will be returned starting steps spaces from the
// start of tkn.
// If the token is not found in the string it returns a null pointer.

char *readStringAfterToken(char *txtstr, char *tkn, char *res, int len, int steps){
	//target string, token, length of substring and steps beyond start of token
	char *tmpstr;
	char *ret =NULL;

	tmpstr = strstr(txtstr, tkn);

	if (tmpstr ) // Checking to make sure the pointer is not NULL, i.e. strstr returned something.
	 ret = strncpy(res, (tmpstr+steps), len);

	ret[len] = '\0'; //Terminate the string before returning.
	 return ret;
}


// This function will return a double from string txtstr. The string will be searched
// for the first occurrence of tkn and the reading of the double will start steps
// places after the start of tkn.
// If the token is not found in the string it returns a value of -1.
// If there is no number after the token it returns 0.

double readDoubleAfterToken(char *txtstr, char *tkn, int steps){ //target string, token and steps beyond start of token
	char *tmpstr;
	char *rem;
	double res = -1;

	tmpstr = strstr(txtstr, tkn);

	 if (tmpstr ) // Checking to make sure the pointer is not NULL, i.e. strstr returned something.
	 res = strtod((tmpstr +steps) ,&rem);

	 return res;
}

// Create the list to be used to store the data
// This function has been extended to process elevation data
struct node* create_list(double lat, double lon, double elev, char *timeStr) {
	char ts[35];
	strcpy(ts,timeStr);
	struct node *ptr = (struct node*) malloc(sizeof(struct node));
	if (NULL == ptr) {
		printf("\n Node creation failed \n");
		return NULL;
	}
	ptr->lat = lat;
	ptr->lon = lon;
	ptr->elev = elev;
	strcpy(ptr->timeString,timeStr);

	ptr->next = NULL;

	head = curr = ptr;
	return ptr;
}

// Add nodes to the main data list
// This function has been extended to process elevation data
struct node* add_to_list(double lat, double lon, double elev, char *timeStr) {
	//char ts[35];
	//	strcpy(ts,timeStr);
		//printf("\n adding node to list with time as %s\n", timeStr);

	if (NULL == head) {
		return (create_list(lat, lon, elev, timeStr));
	}

	struct node *ptr = (struct node*) malloc(sizeof(struct node));
	if (NULL == ptr) {
		printf("\n Node creation failed \n");
		return NULL;
	}
	ptr->lat = lat;
	ptr->lon = lon;
	ptr->elev = elev;
	strcpy(ptr->timeString,timeStr);

	ptr->next = NULL;

	curr->next = ptr;
	curr = ptr;

	return ptr;
}

// A function to populate a timeStr time structure from an GPX time string.
// e.g. "2013-09-12T15:59:18Z"

struct timeStr *timeStrFromString(char *tstring, struct timeStr *tstruct){

		sscanf(tstring+11, "%d", &tstruct->hr);
		sscanf(tstring+14, "%d", &tstruct->min);
		sscanf(tstring+17, "%d", &tstruct->sec);

	return tstruct;
}

// A function to return the time difference in seconds between
// two timeStr time structures.
// It assumes the two times are from the same day.

int timeDiffV2(struct timeStr *t1, struct timeStr *t2){
	int ret = 0, s1 = 0, s2 = 0;
	s1 = t1->hr * 3600 + t1->min * 60 + t1->sec;
	s2 = t2->hr * 3600 + t2->min * 60 + t2->sec;
	ret = s2 - s1;
	return ret;
}
