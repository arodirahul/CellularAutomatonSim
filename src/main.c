//
//  main.c
//  CSE-6010 Assignment1
//
//  Created by Karl Gemayel on 8/17/17.
//  Copyright © 2017 Karl Gemayel. All rights reserved.
//
//  Student name: Rahul Arodi Ramachandra
//  Student username: rar31

#include <stdio.h>
#include <stdlib.h>     // for rand, srand
#include <time.h>       // for time, clock
#include <string.h>     // for strncmp

// Use these variables for simplicity of code
#define RED 'R'
#define GREEN 'G'
#define VACANT ' '

// Global variables for probability ratio of each cell state
#define RED_PROB  0.40
#define GREEN_PROB 0.40
#define VACANT_PROB 0.2
#define SUM_OF_WEIGHTS 1

// Color macros for heat maps
#define ANSI_COLOR_RED   "\x1b[41m"
#define ANSI_COLOR_GREEN "\x1b[42m"
#define ANSI_COLOR_WHITE "\x1b[47m"
#define ANSI_COLOR_RESET "\x1b[0m"

//MACROS for enable/disable debug prints
#if 0
	#define DEBUG(a) printf a
#else
	#define DEBUG(a) (void)0
#endif

#if 1
	#define INFO(a) printf a
#else
	#define INFO(a) (void)0
#endif

int**  stat_matrix;	// Satisfaction status matrix. For cells which are satisfied, it holds a '1' in the corresponding coordinates else a '0'. For vacant cells, it holds a '2'

// Function Prototypes: generic
void usage(const char *programName);

// Function Prototypes: matrix specific
char**      malloc_matrix   (int n1, int n2);
void        free_matrix     (int n1, int n2, char **a);
void        init_matrix     (int n1, int n2, char **a);
int         simulate        (int n1, int n2, char **a, double f);
void        print_matrix    (int n1, int n2, char **a);

// Function Prototypes: stat matrix specific
int         scan_matrix(int n1, int n2, double f, char **mat, int **a);			// Scan to update the satisfaction status matrix - returns 1 if all satisfied else 0
char	    assign_cell_state(); 							// returns randomly "R", "G" and " " 40%, 40% and 20% respectively of the times
int**       malloc_stat_matrix   (int n1, int n2); 			// malloc for satisfaction status matrix
void        free_stat_matrix     (int n1, int n2, int **a); // free for satisfaction status matrix
void        init_stat_matrix     (int n1, int n2, int **a); // init for satisfaction status matrix
void 	    print_stat_matrix(int n1, int n2, int **a);		// print for satisfaction status matrix
void	    check_adjacent_cells(int n1, int n2, char** a, double f, int r_itr, int c_itr, double *ratios);	//Check the adjacent cells of [r_itr, c_itr] for 'R', 'G' or ' '

// Main function
int main(int argc, const char * argv[]) {

    // Start by reading the input from the command line prompt.
    // Note: when no additional arguments are passed through the
    // the command prompt, argc equals 1, and argv[0] is the
    // name of our executable file.

    // In this case, the program takes in 3 addition arguments:
    // (1) number of rows, (2) number of columns, (3) number of
    // times to run 'simulate', and (4) value of f

	srand(time(NULL)); 											// Called only once

    if (argc != 5) {
        usage(argv[0]);
        return 1;
    }

    int n1 = (int) strtol(argv[1], NULL, 10);                   // number of rows
    int n2 = (int) strtol(argv[2], NULL, 10);                   // number of columns
    int numSimulations = (int) strtol(argv[3], NULL, 10);       // number of times to run 'simulate'
    double f = strtod(argv[4], NULL);                           // the satisfiability fraction

    if((f > 1) && (f < 0)){					// Check for invalid 'f' value. No need to check this value elsewhere in the program
    	printf("ERROR: Invalid value for Segregation factor. Simulation Aborted!\n\n");
    	exit(1);
    }

    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
        printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
        exit(1);
    }

    if(numSimulations < 1){					// Check for invalid numSimulations value
    	printf("ERROR: Enter a number greater than 1 for Max Simulations\n\n");
    	exit(1);
    }


    // TODO: Your code here.
    // This part should actually run the simulate function over and over
    // and print the final result

    char** city_matrix;
    int sim_itr = 0;
    int citizens_moved = 0;

    city_matrix = malloc_matrix(n1, n2);	//Memory Alloc for city matrix
    stat_matrix = malloc_stat_matrix(n1, n2);	//memory alloc for stat matrix

    init_matrix(n1, n2, city_matrix);				//populate city matrix with residents
    init_stat_matrix(n1, n2, stat_matrix);			//update the stat matrix
    scan_matrix(n1, n2, f, city_matrix, stat_matrix);		// Initial scan to populate the status matrix

    // Print initial conditions
    INFO(("Initial state of the city: \n"));
    print_matrix(n1, n2, city_matrix);
    print_stat_matrix(n1, n2, stat_matrix);

    // Start simulation
    DEBUG(("Simulation Begins...\n\n"));
    if(!((n1 == 1) && (n2 == 1)) || (f != 0)){							// If matrix is 1X1 or f = 0, then return as is
	for (sim_itr = 0; sim_itr < numSimulations; sim_itr++){			        // Set the limit for  calling the simulate function to numSimulations
	    if(scan_matrix(n1, n2, f, city_matrix, stat_matrix)){
	    	INFO(("Simulation successful! All citizens satisfied \n"));	// If all citizens are happy then no need to simulate any further
	    	break;
            }
	citizens_moved += simulate(n1, n2, city_matrix, f);			// Total number of citizens moved over the course of the all simulations
	}
    }


    if (sim_itr > numSimulations){
    	INFO(("Maximum simulations done! \n"));
    }

    INFO(("Total citizens moved around or left the city = %d in %d simulations \n", citizens_moved, sim_itr));

    // Print final results
    print_matrix(n1, n2, city_matrix);
    print_stat_matrix(n1, n2, stat_matrix);

    // Free up the space occupied by the 2 matrices
    free_matrix(n1, n2, city_matrix);
    free_stat_matrix(n1, n2, stat_matrix);

    return 0;
}

/**
 * Print out the usage message.
 *
 * @param programName the name of the executable file.
 */
void usage(const char *programName) {
    printf("usage: %s  n1 n2 s f\n", programName);
    printf("where:\n");
    printf("    n1      : number of rows\n");
    printf("    n2      : number of columns\n");
    printf("    s       : number of simulation iterations\n");
    printf("    f       : minimum fraction for satisfiability\n");
    printf("\nExample: %s 10 10 100 0.3\n", programName);
}

/****** Matrix Specific Functions ******\
 *                                     *
 * These functions define what matrix  *
 * operations can be performed.        *
 *                                     *
\***************************************/


/**
 * Allocate memory for a 2D matrix
 * @param n1 the number of rows
 * @param n2 the number of columns
 * @return a pointer to the matrix
 */
char **malloc_matrix(int n1, int n2) {

    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
	printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
    }

    char **mat = NULL;       // pointer to the matrix
    int r_itr;		// row and column iterator

    // TODO: Your code here. Don't forget to:
    // - check for invalid inputs!
    // - update the 'mat' variable

    mat = (char **)malloc(n1 * sizeof(char *));			//initialization using "array of pointers" method
    for(r_itr = 0; r_itr < n1; r_itr++){
    	mat[r_itr] = (char *)malloc(n2 * sizeof(char));
    	if (!mat[r_itr]){
    		printf("ERROR: Insufficient Memory!");			// If malloc returns NULL, operation failed due to insufficient memory
    		exit(1);
    	}
    }

    return mat;
}


/**
 * Free the space allocated for the matrix.
 * @param n1 the number of rows
 * @param n2 the number of columns
 * @param a the pointer to the matrix
 */
void free_matrix(int n1, int n2, char **a) {

    // TODO: Your code here. Don't forget to:
    // - check for invalid inputs!
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
	printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
	exit(1);
    }

    int r_itr;
    for(r_itr = 0; r_itr < n1; r_itr++){
        free(a[r_itr]);
    }
    free(a);
    printf("INFO: Memory Freed\n");
}


/**
 * Initialize the matrix.
 * @param n1 the number of rows
 * @param n2 the number of columns
 * @param a the pointer to the matrix
 */
void init_matrix(int n1, int n2, char **a) {

    // TODO: Your code here. Don't forget to:
    // - check for invalid inputs!
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
	printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
	exit(1);
    }

    int r_itr, c_itr;		// row and column iterator

    for(r_itr = 0; r_itr < n1; r_itr++){
    	for(c_itr = 0; c_itr < n2; c_itr++){
    	    a[r_itr][c_itr] = assign_cell_state();	// Assign either 'R', 'G' or ' ' in a probability of 0.4, 0.4 and 0.2 respectively
        }
    }
}

/**
 * Print the matrix to standard output
 * @param n1 the number of rows
 * @param n2 the number of columns
 * @param a the pointer to the matrix
 */
void print_matrix(int n1, int n2, char **a) {

    // TODO: Your code here. Don't forget to:
    // - check for invalid inputs!
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
	printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
	exit(1);
    }

    int r_itr, c_itr;		// row and column iterator
    for(r_itr = 0; r_itr < n1; r_itr++){	// Check for invalid city matrix
	for(c_itr = 0; c_itr < n2; c_itr++){
	    if(!a[r_itr][c_itr]){
	       	printf("ERROR: Matrix has NULL values! \n");
	       	exit(1);
	    }
	    else{
	        //printf("%c ", (*(*(a+r_itr) + c_itr)));

	        if(a[r_itr][c_itr] == 'R')					// To color the cells accordingly
	    	    printf(ANSI_COLOR_RED "%c " ANSI_COLOR_RESET, (*(*(a+r_itr) + c_itr)));
	        if(a[r_itr][c_itr] == 'G')
	        	printf(ANSI_COLOR_GREEN "%c " ANSI_COLOR_RESET, (*(*(a+r_itr) + c_itr)));
	        if(a[r_itr][c_itr] == ' ')
	        	printf(ANSI_COLOR_WHITE "%c " ANSI_COLOR_RESET, (*(*(a+r_itr) + c_itr)));
	     }
	 }
	 printf("\n");
    }
    printf("\n");
}


/**
 * Simulate one time-step using matrix 'a', where f determines
 * the minimum number of desired neighbors similar to the occupant.
 * @param n1 the number of rows
 * @param n2 the number of columns
 * @param a the pointer to the matrix
 * @param f the fraction indicating the minimum number of desired similar neighbours
 * @return the number of citizens that moved or left the game in this time-step; -1 if
 * function failed.
 */
int simulate(int n1, int n2, char **a, double f) {

    // TODO: Your code here. Don't forget to:
    // - check for invalid inputs
    // - update the return value based on success/failure
    if((f > 1) || (f < 0)){
        printf("ERROR: Invalid value for Segregation Factor! Simulation Aborted!\n\n");
	return -1;
    }
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
        printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
        return -1;
    }

    int flag = 0;
    int r_itr, c_itr, i = 0;
    int citizens_moved = 0, dissatisfied_citizens = 0, citizens_moved_out = 0;
	
    int rand_r_itr = rand() % n1;	// Randomly select a cell from the city irrespective of its state
    int rand_c_itr = rand() % n2;

    DEBUG(("Randomly chosen coordinates = [%d, %d] \n", rand_r_itr, rand_c_itr));

    double r_ratio = 0.0, g_ratio = 0.0;
    double* ratios = (double *)malloc(2 * sizeof(double));

    // One pass to find the number of dissatisfied citizens
    for(r_itr = 0; r_itr < n1; r_itr++){
        for(c_itr = 0; c_itr < n2; c_itr++){
	    if(stat_matrix[r_itr][c_itr] == 0){
	        dissatisfied_citizens++;
	    }
	}
    }

    DEBUG(("Dissatisfied citizens = %d \n", dissatisfied_citizens));

    int r_dis[dissatisfied_citizens], c_dis[dissatisfied_citizens]; // arrays to store the coordinates of dissatisfied citizen

    // One more pass to store the coordinates of dissatisfied citizens
    for(r_itr = 0; r_itr < n1; r_itr++){
        for(c_itr = 0; c_itr < n2; c_itr++){
            if(stat_matrix[r_itr][c_itr] == 0){
	        r_dis[i] = r_itr;
		c_dis[i] = c_itr;
		i++;
	    }
	}
    }

    r_itr = 0; c_itr = 0;

    for(i = 0; i < dissatisfied_citizens; i++){				// For every dissatisfied citizen, do this computation
        flag = 0;
	for(r_itr = rand_r_itr; r_itr < n1; r_itr++){		// Search for a satisfiable vacant spot starts from the random coordinates to the end of the city matrix
	    for(c_itr = rand_c_itr; c_itr < n2; c_itr++){
	        if(!a[r_itr][c_itr]){
		    printf("ERROR: Matrix has NULL values! Simulation Aborted!\n\n");
		    exit(1);
		}
		if(a[r_itr][c_itr] == ' '){					// if the plot is vacant then search its surroundings
		    DEBUG(("Vacant spot found at [%d, %d]\n", r_itr, c_itr));

		    check_adjacent_cells(n1, n2, a, f, r_itr, c_itr, ratios);	// Adjacent elements inmate check
		    r_ratio = ratios[0];
		    g_ratio = ratios[1];
		    DEBUG(("R ratio and G ratio = %f, %f\n", r_ratio, g_ratio));

		    if((a[r_dis[i]][c_dis[i]] == 'R') && (r_ratio >= f)){	// Move the cell from its current position to vacant cell which can satisfy it and vacate the current position making it ' '
		        a[r_itr][c_itr] = a[r_dis[i]][c_dis[i]];
			a[r_dis[i]][c_dis[i]] = ' '; 						// Make current spot vacant
			DEBUG(("Citizen moved from cell [%d, %d] to cell [%d, %d] \n", r_dis[i], c_dis[i], r_itr, c_itr));
			citizens_moved++;
			flag = 1;
			break;
                     }
		     if((a[r_dis[i]][c_dis[i]] == 'G') && (g_ratio >= f)){
                         a[r_itr][c_itr] = a[r_dis[i]][c_dis[i]];
			 a[r_dis[i]][c_dis[i]] = ' ';
			 DEBUG(("Citizen moved from cell [%d, %d] to cell [%d, %d] \n", r_dis[i], c_dis[i], r_itr, c_itr));
			 citizens_moved++;
			 flag = 1;
			 break;
		     }
		}
	    }

            if(flag == 1){		// Break to the outermost for loop
	        break;
	    }
	}

	if(flag == 1){			// Continue to the outermost for loop
            continue;
	}

        for(r_itr = 0; r_itr < rand_r_itr; r_itr++){		// Search for a satisfiable vacant spot warps around to the beginning of the city matrix till the random coordinate selected in the beginning
	    for(c_itr = 0; c_itr < rand_c_itr; c_itr++){
                if(a[r_itr][c_itr] == ' '){
		    DEBUG(("Vacant spot found at [%d, %d]\n", r_itr, c_itr));

		    check_adjacent_cells(n1, n2, a, f, r_itr, c_itr, ratios);	// Adjacent elements inmate check
		    r_ratio = ratios[0];
		    g_ratio = ratios[1];
		    DEBUG(("R ratio and G ratio = %f, %f\n", r_ratio, g_ratio));

		    if((a[r_dis[i]][c_dis[i]] == 'R') && (r_ratio >= f)){
		        a[r_itr][c_itr] = a[r_dis[i]][c_dis[i]];
		        a[r_dis[i]][c_dis[i]] = ' ';
		        DEBUG(("Citizen moved from cell [%d, %d] to cell [%d, %d] \n", r_dis[i], c_dis[i], r_itr, c_itr));
		        citizens_moved++;
		        flag = 1;
		        break;
		    }
		    if((a[r_dis[i]][c_dis[i]] == 'G') && (g_ratio >= f)){
		        a[r_itr][c_itr] = a[r_dis[i]][c_dis[i]];
		        a[r_dis[i]][c_dis[i]] = ' ';
		        DEBUG(("Citizen moved from cell [%d, %d] to cell [%d, %d] \n", r_dis[i], c_dis[i], r_itr, c_itr));
		        citizens_moved++;
		        flag = 1;
		        break;
		    }
		}
            }

	    if(flag == 1){
		break;
	    }
	}

	if(flag == 1){
	    continue;
	}

	//Even after searching, if a satisfiable spot was not found then the dissatisfied citizen has to leave the city marking the current occupied space as vacant
	if((r_itr == rand_r_itr) && (c_itr == rand_c_itr)){ // This condition shows that the whole matrix was traversed with not finding a good spot
	    //printf("No Satisfaction for [%d, %d]\n", r_dis[i], c_dis[i]);
	    a[r_dis[i]][c_dis[i]] = ' ';
	    stat_matrix[r_dis[i]][c_dis[i]] = 2; // 2 stands for vacant spot in the status matrix
	    citizens_moved_out++;
	}

    }

    INFO(("Simulated once! %d citizens shuffled around and %d left the city! \n", citizens_moved, citizens_moved_out));
    free(ratios); // A new array is created every time this function is called. Delete as the array is going out of scope

    return citizens_moved + citizens_moved_out; // Return the total number of citizens moved or left the city
}

char assign_cell_state() { // returns randomly "R", "G" and " " 40%, 40% and 20% of the times
    char cell_state;
    int weight = rand() % 5;

    if(weight == 0){			// One of five cases = 20% probabilty
        cell_state = ' ';
    }
    if((weight == 1) || (weight == 2)){	// Two of five cases = 20% probabilty
        cell_state = 'G';
    }
    if((weight == 3) || (weight == 4)){	// Two of five cases = 40% probability
        cell_state = 'R';
    }

    return cell_state;
}

int **malloc_stat_matrix   (int n1, int n2){   // malloc for satisfaction status matrix
    int **mat = NULL;       // pointer to the matrix
    int r_itr;		// row and column iterator

    // TODO: Your code here. Don't forget to:
    // - check for invalid inputs!
    // - update the 'mat' variable

    if((n1 < 1) || (n2 < 1)){
        printf("ERROR: Invalid Matrix dimensions! Simulation Aborted!\n\n");
	exit(1);
    }

    mat = (int **)malloc(n1 * sizeof(int *));			//initialization using "array of pointers" method
    for(r_itr = 0; r_itr < n1; r_itr++){
        mat[r_itr] = (int *)malloc(n2 * sizeof(int));
	if (!mat[r_itr]){
	    printf("ERROR: Insufficient Memory!");			// Operation failed due to insufficient memory
	    exit(1);
	}
    }

    return mat;
}

void free_stat_matrix(int n1, int n2, int **a){ // free for satisfaction status matrix
    int r_itr;
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
        printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
        exit(1);
    }
    for(r_itr = 0; r_itr < n1; r_itr++){
	free(a[r_itr]);
    }
    free(a);
}

void init_stat_matrix(int n1, int n2, int **a){ // init for satisfaction status matrix
    int r_itr, c_itr;		// row and column iterator
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
        printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
        exit(1);
    }
    for(r_itr = 0; r_itr < n1; r_itr++){
	for(c_itr = 0; c_itr < n2; c_itr++){
            a[r_itr][c_itr] = 2;		// 2 means vacant, actual status of the city matrix will be updated after initial scan
	}
    }
}

void print_stat_matrix(int n1, int n2, int **a) {	// To print the status matrix

    // TODO: Your code here. Don't forget to:
    // - check for invalid inputs!
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
        printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
        exit(1);
    }
    DEBUG(("Printing Status Matrix...\n\n"));
    int r_itr, c_itr;		// row and column iterator
    for(r_itr = 0; r_itr < n1; r_itr++){
        for(c_itr = 0; c_itr < n2; c_itr++){
	    if((a[r_itr][c_itr] != 0) && (a[r_itr][c_itr] != 1) && (a[r_itr][c_itr] != 2)){
	        printf("ERROR: Stat Matrix has invalid values! \n");
	    	exit(1);
	    }
	    else{
	        DEBUG(("%d ", (*(*(a+r_itr) + c_itr))));
	    }
        }
	DEBUG(("\n"));
    }
    DEBUG(("\n"));
}

int scan_matrix(int n1, int n2, double f, char **city_mat, int **stat_mat){		// Checking which cells are satisfied and which are not
    if((f > 1) || (f < 0)){
        printf("ERROR: Invalid value for Segregation Factor! Simulation Aborted!\n\n");
	exit(1);
    }
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
        printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
        exit(1);
    }

    int r_itr, c_itr;
    double r_ratio = 0.0, g_ratio = 0.0;
    double* ratios = (double *)malloc(2 * sizeof(double));

    DEBUG(("Scanning matrix...\n\n"));

    for(r_itr = 0; r_itr < n1; r_itr++){
        for(c_itr = 0; c_itr < n2; c_itr++){
	    check_adjacent_cells(n1, n2, city_mat, f, r_itr, c_itr, ratios);	// Adjacent elements inmate check. 1 means the current cell is satisfied, 0 if dissatisfied
	    r_ratio = ratios[0];
            g_ratio = ratios[1];

            if((r_ratio == 0.0) && (g_ratio == 0.0)){
                stat_mat[r_itr][c_itr] = 1;
                DEBUG(("Hermit cell found at [%d, %d]! \n", r_itr, c_itr)); // Hermit cell is satisfied. Who needs neighbors?!
            }else if((city_mat[r_itr][c_itr] == 'R') && (r_ratio >= f)){
                stat_mat[r_itr][c_itr] = 1;
		//DEBUG(("[%d, %d], r_ratio = %f \n", r_itr, c_itr, r_ratio));
	    }else if((city_mat[r_itr][c_itr] == 'R') && (r_ratio < f)){
		//DEBUG(("[%d, %d], r_ratio = %f \n", r_itr, c_itr, r_ratio));
		stat_mat[r_itr][c_itr] = 0;
	    }else if((city_mat[r_itr][c_itr] == 'G') && (g_ratio >= f)){
		//DEBUG(("[%d, %d], g_ratio = %f \n", r_itr, c_itr, g_ratio));
		stat_mat[r_itr][c_itr] = 1;
	    }else if((city_mat[r_itr][c_itr] == 'G') && (g_ratio < f)){
		//DEBUG(("[%d, %d], g_ratio = %f \n", r_itr, c_itr, g_ratio));
		stat_mat[r_itr][c_itr] = 0;
	    }else if(city_mat[r_itr][c_itr] == ' '){
		stat_mat[r_itr][c_itr] = 2;
	    }
	}
   }

   DEBUG(("\n"));

	// -------------  Check of satisfaction of all citizens starts ----------------
    for(r_itr = 0; r_itr < n1; r_itr++){
        for(c_itr = 0; c_itr < n2; c_itr++){
	    if(stat_mat[r_itr][c_itr] == 0){
                return 0;
            }
        }
    }
    // -------------  Check of satisfaction of all citizens ends -------------------

    return 1;	// If control comes here then it means that everyone is satisfied
}


void check_adjacent_cells(int n1, int n2, char** a, double f, int r_itr, int c_itr, double* ratios){	// Checking neighbors of a particular cell/citizen and calculating the red/green to total ratio to compare with 'f'
    if((f > 1) || (f < 0)){		// Check for invalid Segregation Factor value
        printf("ERROR: Invalid value for Segregation Factor! Simulation Aborted!\n\n");
	exit(1);
    }
    if((n1 < 1) || (n2 < 1)){					// Check for invalid matrix dimensions
        printf("ERROR: Invalid matrix dimensions. Simulation Aborted!\n\n");
        exit(1);
    }
    if(!a[r_itr][c_itr]){
        printf("ERROR: NULL value in city matrix! Simulation Aborted!\n\n");
    }
    if((ratios[0] < 0) || (ratios[1] < 0 || (ratios[0] > 1) || (ratios[1] > 1))){	// Check for invalid ratios values
        printf("ERROR: Invalid value for red to green or green to red ratio! Simulation Aborted!\n\n");
        exit(1);
    }

    int r_count = 0, g_count = 0, total = 0;
    double r_ratio = 0.0, g_ratio = 0.0;

    if((r_itr > 0) && (c_itr > 0) && (a[r_itr - 1][c_itr - 1]) == 'R'){
        r_count++;
    }
    if((r_itr > 0) && (c_itr > 0) && (a[r_itr - 1][c_itr - 1]) == 'G'){
        g_count++;
    }

    if((r_itr > 0) && (a[r_itr - 1][c_itr]) == 'R'){
        r_count++;
    }
    if((r_itr > 0) && (a[r_itr - 1][c_itr]) == 'G'){
        g_count++;
    }

    if((r_itr > 0) && (c_itr < n2 - 1) && (a[r_itr - 1][c_itr + 1]) == 'R'){
        r_count++;
    }
    if((r_itr > 0) && (c_itr < n2 - 1) && (a[r_itr - 1][c_itr + 1]) == 'G'){
        g_count++;
    }

    if((c_itr > 0) && (a[r_itr][c_itr - 1]) == 'R'){
        r_count++;
    }
    if((c_itr > 0) && (a[r_itr][c_itr - 1]) == 'G'){
        g_count++;
    }

    if((c_itr < n2 - 1) && (a[r_itr][c_itr + 1]) == 'R'){
        r_count++;
    }
    if((c_itr < n2 - 1) && (a[r_itr][c_itr + 1]) == 'G'){
        g_count++;
    }

    if((r_itr < n1 - 1) && (c_itr > 0) && (a[r_itr + 1][c_itr - 1]) == 'R'){
        r_count++;
    }
    if((r_itr < n1 - 1) && (c_itr > 0) && (a[r_itr + 1][c_itr - 1]) == 'G'){
        g_count++;
    }

    if((r_itr < n1 - 1) && (a[r_itr + 1][c_itr]) == 'R'){
        r_count++;
    }
    if((r_itr < n1 - 1) && (a[r_itr + 1][c_itr]) == 'G'){
        g_count++;
    }

    if((r_itr < n1 - 1) && (c_itr < n2 - 1) && (a[r_itr + 1][c_itr + 1]) == 'R'){
        r_count++;
    }
    if((r_itr < n1 - 1) && (c_itr < n2 - 1) && (a[r_itr + 1][c_itr + 1]) == 'G'){
        g_count++;
    }
    // ---------- Adjacent elements inmate check ends ---------------------

    total = r_count + g_count;
    if(total > 0){			// To prevent a divide by zero exception
        r_ratio = (double)r_count / total;
	g_ratio = (double)g_count / total;
	ratios[0] = r_ratio;
	ratios[1] = g_ratio;
    }
    else{				// If a hermit cell is found with no neighbors
	ratios[0] = 0.0;
	ratios[1] = 0.0;
    }
}











