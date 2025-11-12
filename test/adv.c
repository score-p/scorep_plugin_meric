/*
 * SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
/*
 * Solve the 1D scalar linear advection equation
 * dt(u) + a dx(u) = 0
 * with a finite volume method and upwind fluxes.
 *
 * The analytic solution is u(t,x) = u(0, x-a*t) = u0(x - a*t),
 * whith initial condition u0(x).
 *
 * With CFL = 1, the method degenerates to a shift of the cell averages
 * to the right(if a>0) or left(if a<0) neighboring cell in each time step.
 * This means that the transport problem with piece-wise constant initial
 * condition is solved exactly.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_MPI
#include <mpi.h>
#endif

#ifdef USE_OMP
#include <omp.h>
#endif

#ifndef USE_MPI
#ifndef USE_OMP
#include <time.h>
#endif
#endif

#ifdef __cplusplus
#include <algorithm>
#include <cmath>
#define MYMAX( a, b ) ( std::max( a, b ) )
#define MYMIN( a, b ) ( std::min( a, b ) )
#define MYFLOOR( a ) ( std::floor( a ) )
#define MYSIN( a ) ( std::sin( a ) )
#define MYSQRT( a ) ( std::sqrt( a ) )
#include <limits>
#define MYINT_MAX std::numeric_limits < int > ::max()
#else
#include <math.h>
#define MYMAX( a, b ) ( ( a >= b ) ? ( a ) : ( b ) )
#define MYMIN( a, b ) ( ( a < b ) ? ( a ) : ( b ) )
#define MYFLOOR( a ) ( floor( a ) )
#define MYSIN( a ) ( sin( a ) )
#define MYSQRT( a ) ( sqrt( a ) )
#include <limits.h>
#define MYINT_MAX INT_MAX
#endif

void
print_paradigm_info( void );

/*----------------------------------------------------------------------------*/
/* Parameters                                                                 */
/*----------------------------------------------------------------------------*/
struct Parameters
{
    /* Work */
    int n_global;
    int n_time_steps;

    /* Domain */
    double L;
    double T;

    /* Discretization */
    double dx;
    double dt;
    double CFL;

    /* Physics */
    double a;
};

typedef struct Parameters Parameters;
Parameters
make_parameters( int n_global,
                 int n_time_steps );
void
print_parameters( Parameters params );

/*----------------------------------------------------------------------------*/
/* Command line                                                               */
/*----------------------------------------------------------------------------*/
void
parse_command_line( int   argc,
                    char* argv[],
                    int*  n_global,
                    int*  n_time_steps );
int
get_int_option( int         argc,
                char*       argv[],
                const char* option_name,
                int*        argi,
                int*        value );
int
parse_long( const char* str,
            long*       result );

/*----------------------------------------------------------------------------*/
/* Solution fields                                                            */
/*----------------------------------------------------------------------------*/
void
initialize_solution( double* u,
                     int     first_cell,
                     int     n_local,
                     double  dx,
                     double  L );
void
compute_cell_update( double* u,
                     double* unew,
                     int     first_cell,
                     int     n_local,
                     double  dt,
                     double  dx,
                     double  a );
void
swap( double** u,
      double** unew );
double
compute_l2err2( double* u,
                int     first_cell,
                int     n_local,
                double  t,
                double  dx,
                double  L,
                double  a );

double
analytic_solution( double x,
                   double t,
                   double L,
                   double a );

inline double
initial_condition( double x, double L )
{
    /* Respect the periodic boundary conditions */
    x -= MYFLOOR( x / L ) * L;
    return MYSIN( x );
}

inline double
global_pos( int li, int first_cell, double dx )
{
    return dx * ( ( li - 1 + first_cell ) + 0.5 );
}

/*----------------------------------------------------------------------------*/
/* MPP                                                                        */
/*----------------------------------------------------------------------------*/
void
init_mpp( int*   argc,
          char** argv[],
          int*   wsize,
          int*   wrank );
void
finalize_mpp( void );
void
abort_program( const char* msg );
void
broadcast_inputs( int  wsize,
                  int  wrank,
                  int* n_global,
                  int* n_time_steps );
void
distribute_work( int  n_global,
                 int  wsize,
                 int  wrank,
                 int* first_cell,
                 int* n_inner_cells );
void
communicate_boundaries( double* u,
                        int     n_local,
                        int     wsize,
                        int     wrank );
double
sum_to_root( double local );


double
get_time( void );

/*----------------------------------------------------------------------------*/
/* main                                                                       */
/*----------------------------------------------------------------------------*/
int
main( int argc, char* argv[] )
{
    int wsize, wrank;
    init_mpp( &argc, &argv, &wsize, &wrank );

    int n_global     = 1000000;
    int n_time_steps = 10;
    parse_command_line( argc, argv, &n_global, &n_time_steps );
    broadcast_inputs( wsize, wrank, &n_global, &n_time_steps );
    const Parameters params = make_parameters( n_global, n_time_steps );

    if ( wrank == 0 )
    {
        print_paradigm_info();
        print_parameters( params );
    }

    double starttime = get_time();

    int first_cell, n_inner_cells;
    distribute_work( params.n_global, wsize, wrank, &first_cell, &n_inner_cells );

    /* Including two ghost cells for the boundaries */
    const int n_local = n_inner_cells + 2;
    double*   u, * unew, * tmp;
    u    = ( double* )malloc( sizeof( double ) * n_local );
    unew = ( double* )malloc( sizeof( double ) * n_local );

    int    step = 0;
    double t    = 0.;
    initialize_solution( u, first_cell, n_local, params.dx, params.L );

    while ( step < params.n_time_steps )
    {
        ++step;
        t += params.dt;
        communicate_boundaries( u, n_local, wsize, wrank );
        compute_cell_update( u, unew, first_cell, n_local, params.dt, params.dx, params.a );
        swap( &u, &unew );
    }

    /* check against analytic solution */
    const double tol        = 1e-16;
    const double l2err      = MYSQRT( sum_to_root( compute_l2err2( u, first_cell, n_local, t, params.dx, params.L, params.a ) ) );
    int          error_code = 0;
    if ( wrank == 0 )
    {
        int correct = ( l2err < tol );
        printf( "Time = %.6e, L2 error of solution = %.6e\nVerification: %s\n", t, l2err, ( ( correct ) ? "SUCCESS" : "FAILURE" ) );
        if ( !correct )
        {
            error_code = 1;
            fprintf( stderr, "L2 error of solution exceeds tolerance: %.6e > %.6e\n", l2err, tol );
        }
    }

    free( u );
    free( unew );

    double runtime = get_time() - starttime;
    if ( wrank == 0 )
    {
       printf("Computation took %f seconds\n", runtime);
    }

    finalize_mpp();
    return error_code;
}


void
print_paradigm_info( void )
{
    printf( "Paradigms:" );
#ifdef USE_MPI
    int wsize;
    MPI_Comm_size( MPI_COMM_WORLD, &wsize );
    printf( " mpi(%d ranks)", wsize );
#endif
#ifdef USE_OMP
    printf( " omp(%d threads)", omp_get_max_threads() );
#endif
#ifndef USE_MPI
#ifndef USE_OMP
    printf( " serial" );
#endif
#endif
    printf( "\n" );
}

/*----------------------------------------------------------------------------*/
/* Parameters                                                                 */
/*----------------------------------------------------------------------------*/
Parameters
make_parameters( int n_global, int n_time_steps )
{
    Parameters params = {
        .n_global     = n_global,
        .n_time_steps = n_time_steps,
        .L            = 1.,
        .T            = 1.,
        .CFL          = 1.
    };
    params.dx = params.L / ( double )params.n_global,
    params.dt = params.T / ( double )params.n_time_steps,
    params.a  = params.CFL * params.dx / params.dt;
    return params;
}

void
print_parameters( Parameters p )
{
    printf( ""
            "Parameters:\n"
            "-----------\n"
            "Work:\n"
            "n_global     = %d\n"
            "n_time_steps = %d\n"
            "\n"
            "Domain:\n"
            "L   = %.6e\n"
            "T   = %.6e\n"
            "\n"
            "Discretization:\n"
            "dx  = %.6e\n"
            "dt  = %.6e\n"
            "CFL = %.6e\n"
            "\n"
            "Physics:\n"
            "a   = %.6e\n"
            "\n"
            "-----------\n"
            "\n",
            p.n_global, p.n_time_steps, p.L, p.T, p.dx, p.dt, p.CFL, p.a );
}

/*----------------------------------------------------------------------------*/
/* Command line                                                               */
/*----------------------------------------------------------------------------*/
void
parse_command_line( int argc, char* argv[], int* n_global, int* n_time_steps )
{
    int argi = 1;
    while ( argi < argc )
    {
        int matched = 0;
        matched |= get_int_option( argc, argv, "--Nx", &argi, n_global );
        matched |= get_int_option( argc, argv, "--Nt", &argi, n_time_steps );
        if ( !matched )
        {
            printf( "argi = %d, argv[i] = %s\n", argi, argv[ argi ] );
            abort_program( "Unknown argument" );
        }
    }
}

int
get_int_option( int argc, char* argv[], const char* option_name, int* argi, int* value )
{
    if ( *argi >= argc )
    {
	return 0;
    }
    if ( strncmp( argv[ *argi ], option_name, strlen( option_name ) ) == 0 )
    {
        ++( *argi );
        if ( *argi >= argc )
        {
            abort_program( "Argument needs value" );
        }
        long v   = 0;
        int  err = parse_long( argv[ *argi ], &v );
        if ( err || v <= 0 || v > MYINT_MAX )
        {
            abort_program( "Invalid argument value" );
        }
        *value = ( int )v;
        ++( *argi );
        return 1;
    }
    else
    {
        return 0;
    }
}

int
parse_long( const char* str, long* result )
{
    errno = 0;
    char* end;

    *result = strtol( str, &end, 10 );
    return ( errno != 0 ) || ( *end != '\0' );
}


/*----------------------------------------------------------------------------*/
/* Solution fields                                                            */
/*----------------------------------------------------------------------------*/
void
initialize_solution( double* u, int first_cell, int n_local, double dx, double L )
{
#ifdef USE_OMP
#pragma omp parallel for
#endif
    for ( int li = 1; li < n_local - 1; ++li )
    {
        const double x = global_pos( li, first_cell, dx );
        u[ li ] = initial_condition( x, L );
    }
}

void
compute_cell_update( double* u, double* unew, int first_cell, int n_local, double dt, double dx, double a )
{
    /* compute new cell averages */
#ifdef USE_OMP
#pragma omp parallel for
#endif
    for ( int li = 1; li < n_local - 1; ++li )
    {
        const double factor     = dt / dx;
        const double flux_left  = ( MYMAX( a, 0. ) * u[ li - 1 ] + MYMIN( a, 0. ) * u[ li ] );
        const double flux_right = ( MYMAX( a, 0. ) * u[ li ] + MYMIN( a, 0. ) * u[ li + 1 ] );
        unew[ li ] = u[ li ] + factor * flux_left - factor * flux_right;
    }
}

void
swap( double** u, double** unew )
{
    double* tmp = *u;
    *u    = *unew;
    *unew = tmp;
}

double
compute_l2err2( double* u, int first_cell, int n_local, double t, double dx, double L, double a )
{
    double l2err2 = 0.;
#ifdef USE_OMP
#pragma omp parallel for reduction(+:l2err2)
#endif
    for ( int li = 1; li < n_local - 1; ++li )
    {
        const double x    = global_pos( li, first_cell, dx );
        const double diff = u[ li ] - analytic_solution( x, t, L, a );
        l2err2 += dx * dx * diff * diff;
    }
    return l2err2;
}

double
analytic_solution( double x, double t, double L, double a )
{
    return initial_condition( x - t * a, L );
}


/*----------------------------------------------------------------------------*/
/* MPP                                                                        */
/*----------------------------------------------------------------------------*/
#if defined( USE_MPI )
void
init_mpp( int* argc, char** argv[], int* wsize, int* wrank )
{
    MPI_Init( argc, argv );
    MPI_Comm_size( MPI_COMM_WORLD, wsize );
    MPI_Comm_rank( MPI_COMM_WORLD, wrank );
}

void
finalize_mpp( void )
{
    MPI_Finalize();
}

void
abort_program( const char* msg )
{
    printf( "Error: %s\n", msg );
    MPI_Abort( MPI_COMM_WORLD, 1 );
}

void
broadcast_inputs( int wsize, int wrank, int* n_global, int* n_time_steps )
{
    MPI_Bcast( n_global, 1, MPI_INT,  0, MPI_COMM_WORLD );
    MPI_Bcast( n_time_steps, 1, MPI_INT,  0, MPI_COMM_WORLD );
}

void
distribute_work( int n_global, int wsize, int wrank, int* first_cell, int* n_inner_cells )
{
    const int cells_est = n_global / wsize;
    const int missing   = n_global - cells_est * wsize;

    *n_inner_cells = cells_est;
    if ( wrank < missing )
    {
        *n_inner_cells += 1;
    }
    *first_cell = 0;
    MPI_Exscan( n_inner_cells, first_cell, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if ( *n_inner_cells == 0 )
    {
        abort_program( "Specified less cells than MPI ranks" );
    }
}

void
communicate_boundaries( double* u, int n_local, int wsize, int wrank )
{
    const int left_neighbor  = ( wrank > 0 ) ? wrank - 1 : wsize - 1;
    const int right_neighbor = ( wrank < wsize - 1 ) ? wrank + 1 : 0;

    /* Send to left */
    MPI_Sendrecv( &u[ 1 ], 1, MPI_DOUBLE, left_neighbor, 0,
                  &u[ n_local - 1 ], 1, MPI_DOUBLE, right_neighbor, 0,
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE );

    /* Send to right */
    MPI_Sendrecv( &u[ n_local - 2 ], 1, MPI_DOUBLE, right_neighbor, 0,
                  &u[ 0 ], 1, MPI_DOUBLE, left_neighbor, 0,
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE );
}

double
sum_to_root( double local )
{
    double global = 0.;
    MPI_Reduce( &local, &global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );
    return global;
}

double
get_time( void )
{
    return MPI_Wtime();
}

#else

void
init_mpp( int* argc, char** argv[], int* wsize, int* wrank )
{
    *wsize = 1;
    *wrank = 0;
}

void
finalize_mpp( void )
{
}

void
abort_program( const char* msg )
{
    printf( "Error: %s\n", msg );
    exit( 1 );
}

void
broadcast_inputs( int wsize, int wrank, int* n_global, int* n_time_steps )
{
}

void
distribute_work( int n_global, int wsize, int wrank, int* first_cell, int* n_inner_cells )
{
    *first_cell    = 0;
    *n_inner_cells = n_global;
}


void
communicate_boundaries( double* u, int n_local, int wsize, int wrank )
{
    u[ 0 ]           = u[ n_local - 2 ];
    u[ n_local - 1 ] = u[ 1 ];
}

double
sum_to_root( double local )
{
    return local;
}


double
get_time( void )
{
#ifdef USE_OMP
    return omp_get_wtime();
#else
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return tv.tv_sec + tv.tv_nsec * 1.0e-9;
#endif
}
#endif

