/*
 * io.h
 *
 *  Created on: Oct 24, 2008
 *      Author: rasmussn
 */

#ifndef IO_H_
#define IO_H_

#include "../layers/PVLayer.h"

#ifdef PV_USE_MPI
#  include <mpi.h>
#else
#  include "../include/mpi_stubs.h"
#endif

#define MIN_BIN_PARAMS  6
#define MAX_BIN_PARAMS (18 + sizeof(double)/sizeof(int))

#define NUM_PAR_BYTE_PARAMS (MAX_BIN_PARAMS)

#define PV_BYTE_TYPE       1
#define PV_FLOAT_TYPE      2

#define PVP_FILE_TYPE      1
#define KERNEL_FILE_TYPE  6996

#define INDEX_HEADER_SIZE  0
#define INDEX_NUM_PARAMS   1
#define INDEX_FILE_TYPE    2
#define INDEX_NX           3
#define INDEX_NY           4
#define INDEX_NF           (MIN_BIN_PARAMS - 1)
#define INDEX_NUM_RECORDS  6
#define INDEX_RECORD_SIZE  7
#define INDEX_DATA_SIZE    8
#define INDEX_DATA_TYPE    9
#define INDEX_NX_PROCS    10
#define INDEX_NY_PROCS    11
#define INDEX_NX_GLOBAL   12
#define INDEX_NY_GLOBAL   13
#define INDEX_KX0         14
#define INDEX_KY0         15
#define INDEX_NPAD        16
#define INDEX_NBANDS      17
#define INDEX_TIME        18

#ifdef __cplusplus
extern "C"
{
#endif

int readFile(const char * filename, float * buf, int * nx, int * ny);
int scatterReadBuf(PVLayer * l, float * globalBuf, float * localBuf, MPI_Comm comm);
int scatterReadFile(const char * filename, PVLayer * l, float * buf, MPI_Comm comm);
int gatherWriteFile(const char * filename, PVLayer * l, float * ibuf, MPI_Comm comm);

int pv_text_write_patch(FILE * fd, PVPatch * patch);
int pv_tiff_write_patch(FILE * fd, PVPatch * patch);
int pv_tiff_write_cube(const char * filename, PVLayerCube * cube, int nx, int ny, int nf);
int pv_center_image(float * V, int nx0, int ny0, int nx, int ny);

int log_parameters(int n_time_steps, char * input_filename);

int printStats(pvdata_t * buf, int nItems, char * msg);

int pv_dump(const char * filename, int append, pvdata_t * I, int nx, int ny, int nf);
int pv_dump_sparse(const char * filename, int append, pvdata_t * I, int nx, int ny, int nf);
int pv_write_patches(const char * filename, int append,
                     int nx, int ny, int nf, float minVal, float maxVal,
                     int numPatches, PVPatch ** patches);
int pv_read_patches(FILE *fp, int nf, float minVal, float maxVal,
                    int numPatches, PVPatch ** patches);

FILE * pv_open_binary(const char * filename, int * numParams, int * type, int * nx, int * ny, int * nf);
int    pv_read_binary_params(FILE * fp, int numParams, int params[]);
int    pv_close_binary(FILE * fd);
size_t pv_read_binary_record(FILE * fd, pvdata_t * buf, int nItems);

int parse_options(int argc, char * argv[], char ** input_file,
                  char ** param_file, int * n_time_steps, int *shmem_threads);

#ifdef __cplusplus
}
#endif

#endif /* IO_H_ */
