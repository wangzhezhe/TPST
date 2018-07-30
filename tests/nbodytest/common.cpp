#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"
#include <vector>

using namespace std;

double size;

//
//  tuned constants
//
#define density 0.0005
#define mass 0.01
#define cutoff 0.01
#define min_r (cutoff / 100)
#define dt 0.00005

//
//  timer
//
double read_timer()
{
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if (!initialized)
    {
        gettimeofday(&start, NULL);
        initialized = true;
    }
    gettimeofday(&end, NULL);
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

//
//  keep density constant
//
void set_size(int n)
{
    size = sqrt(density * n);
    printf("set size %lf\n", size);
}

//
//  Initialize the particle positions and velocities
//
void init_particles(int n, vector<particle_t *> &pList)
{
    srand48(time(NULL));

    int i = 0;
    for (i = 0; i < n; i++)
    {
        particle_t *p = new (particle_t);
        p->ax=dt;
        p->ay=dt;
        p->az=dt;
        pList.push_back(p);
    }

    int sx = (int)ceil(sqrt((double)n));
    int sy = (n + sx - 1) / sx;
    int sz = (n + sy - 1) / sy;

    int *shuffle = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
        shuffle[i] = i;

    for (int i = 0; i < n; i++)
    {
        //
        //  make sure particles are not spatially sorted
        //
        int j = lrand48() % (n - i);
        int k = shuffle[j];
        shuffle[j] = shuffle[n - i - 1];

        //
        //  distribute particles evenly to ensure proper spacing
        //
        pList[i]->x = size * (1. + (k % sx)) / (1 + sx);
        pList[i]->y = size * (1. + (k / sx)) / (1 + sy);
        pList[i]->z = size * (1. + (k / sx)) / (1 + sz);
        //
        //  assign random velocities within a bound
        //
        pList[i]->vx = drand48() * 2 - 1;
        pList[i]->vy = drand48() * 2 - 1;
        pList[i]->vz = drand48() * 2 - 1;
    }
    free(shuffle);
}

//
//  interact two particles
//
void apply_force(particle_t *particle, particle_t *neighbor, double *dmin, double *davg, int *navg)
{

    double dx = neighbor->x - particle->x;
    double dy = neighbor->y - particle->y;
    double dz = neighbor->z - particle->z;
    double r2 = dx * dx + dy * dy + dz * dz;
    if (r2 > cutoff * cutoff)
        return;
    if (r2 != 0)
    {
        if (r2 / (cutoff * cutoff) < *dmin * (*dmin))
        {
            *dmin = sqrt(r2) / cutoff;
        }

        (*davg) += sqrt(r2) / cutoff;
        (*navg)++;
    }

    r2 = fmax(r2, min_r * min_r);
    double r = sqrt(r2);

    //
    //  very simple short-range repulsive force
    //
    double coef = (1 - cutoff / r) / r2 / mass;
    particle->ax += coef * dx;
    particle->ay += coef * dy;
    particle->az += coef * dz;
}

//
//  integrate the ODE
//
void move(vector<particle_t *> &pList)
{
    int n = pList.size();
    //printf("debug size %d\n", n);
    int i = 0;
    for (i = 0; i < n; i++)
    {
        particle_t *p = pList[i];

        //
        //  slightly simplified Velocity Verlet integration
        //  conserves energy better than explicit Euler method
        //
        //printf("debug p update before move %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", p->vx, p->vy, p->vz, p->x, p->y, p->z, p->ax, p->ay, p->az);

        p->vx += p->ax * dt;
        p->vy += p->ay * dt;
        p->vz += p->az * dt;
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->z += p->vz * dt;

        //
        //  bounce from walls
        //
        //jump out from the hole

        //printf("debug p update %lf %lf %lf %lf %lf %lf \n", p->vx, p->vy, p->vz, p->x, p->y, p->z);
        //if ((size * (1.0 / 3) < p->x) && (p->x < size * 2 / 3) && (size * 1 / 3 < p->y) && (p->y < size * 2 / 3) && (p->z > size))
        //{
        //fly out
        //if (p->z > 1.5 * size)
        //{
        //    pList.erase(pList.begin() + i);
        //}
        //}

        //else
        //{
        if (p->x < 0 || p->x > size)
        {
            p->x = p->x < 0 ? -p->x : 2 * size - p->x;
            p->vx = -p->vx;
        }
        if (p->y < 0 || p->y > size)
        {
            p->y = p->y < 0 ? -p->y : 2 * size - p->y;
            p->vy = -p->vy;
        }

        if (p->z < 0 || p->z > size)
        {
            p->z = p->z < 0 ? -p->z : 2 * size - p->z;
            p->vz = -p->vz;
        }
        //}
    }
}

//
//  I/O routines
//
void save(FILE *f, vector<particle_t *> pList)
{

    int n = pList.size();
    for (int i = 0; i < n; i++)
    {
        fprintf(f, "%g,%g,%g\n", pList[i]->x, pList[i]->y, pList[i]->z);
    }
}

//
//  command line option processing
//
int find_option(int argc, char **argv, const char *option)
{
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], option) == 0)
            return i;
    return -1;
}

int read_int(int argc, char **argv, const char *option, int default_value)
{
    int iplace = find_option(argc, argv, option);
    if (iplace >= 0 && iplace < argc - 1)
        return atoi(argv[iplace + 1]);
    return default_value;
}

char *read_string(int argc, char **argv, const char *option, char *default_value)
{
    int iplace = find_option(argc, argv, option);
    if (iplace >= 0 && iplace < argc - 1)
        return argv[iplace + 1];
    return default_value;
}
