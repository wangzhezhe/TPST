#ifndef COMMON_H__
#define COMMON_H__

#include <vector>

using namespace std;

inline int min( int a, int b ) { return a < b ? a : b; }
inline int max( int a, int b ) { return a > b ? a : b; }

//
//  saving parameters
//
const int NSTEPS = 1000;
const int SAVEFREQ = 1;

//
// particle data structure
//
typedef struct 
{
  double x;
  double y;
  double z;
  double vx;
  double vy;
  double vz;
  double ax;
  double ay;
  double az;
} particle_t;

//
//  timing routines
//
double read_timer( );

//
//  simulation routines
//
void set_size( int n );

void init_particles(int n, vector<particle_t*> & pList);
//void init_particles( int n, particle_t *p );
//void apply_force( particle_t &particle, particle_t &neighbor , double *dmin, double *davg, int *navg);
void apply_force(particle_t * particle, particle_t *neighbor, double *dmin, double *davg, int *navg);

void move(vector<particle_t *> & pList);


//
//  I/O routines
//
FILE *open_save( char *filename, int n );
void save(FILE *f, vector<particle_t *> pList);

//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );

#endif
