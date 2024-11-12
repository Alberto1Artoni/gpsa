#include <unordered_map>
#include <omp.h>
#include <algorithm>
#include "helpers.hpp"

unsigned long SequenceInfo::gpsa_sequential(float** S) {
    unsigned long visited = 0;

    // Boundary
    for (unsigned int i = 1; i < rows; i++) {
        S[i][0] = i * gap_penalty;
        visited++;
    }

    for (unsigned int j = 0; j < cols; j++) {
        S[0][j] = j * gap_penalty;
        visited++;
    }
    
    // Main part
    for (unsigned int i = 1; i < rows; i++) {
        for (unsigned int j = 1; j < cols; j++) {
            float match = S[i - 1][j - 1] + (X[i - 1] == Y[j - 1] ? match_score : mismatch_score);
            float del = S[i - 1][j] + gap_penalty;
            float insert = S[i][j - 1] + gap_penalty;
            S[i][j] = std::max({match, del, insert});
          
            visited++;
        }
    }

    return visited;
}

unsigned long SequenceInfo::gpsa_taskloop(float** S, int grain_size=1) {

    unsigned long visited = 0;

    // Boundary
#pragma omp parallel
  {
  #pragma omp single
    {
    #pragma omp taskloop reduction(+:visited)
    for (unsigned int i = 1; i < rows; i++) {
        S[i][0] = i * gap_penalty;
        visited++;
      }
    }
  }

    // Boundary
#pragma omp parallel
  {
  #pragma omp single
    {
    #pragma omp taskloop reduction(+:visited)
    for (unsigned int j = 0; j < cols; j++) {
        S[0][j] = j * gap_penalty;
        visited++;
      }
    }
  }

    // add granularity of each block 
    int ndiags = rows + cols - 1;

    // loop over anti-diagonals
    for (int wave = 0; wave * grain_size < ndiags; ++wave) {

      int startIdx = (wave < cols ? 0: (wave - cols + 1) * grain_size);
      int nblocks  = (wave < rows ? wave + 1: rows + cols -1 - wave) * grain_size + startIdx;

     // begin parallel region
#pragma omp parallel 
     // the loop is executed by a single master thread
#pragma omp single
     // taskloop will schedule tasks that are then associated to threads
#pragma omp taskloop reduction(+:visited) 

     // loop over blocks of the current antidiagonal
      for (int bi = startIdx; bi < nblocks ; bi += grain_size) {

        int bj = wave * grain_size - bi;

        if (bj > cols) continue; // Ensure that it is within bounds
       
        int i_end = std::min(bi + grain_size, rows);
        int j_end = std::min(bj + grain_size, cols);

        if ( wave == 0) {bi = 1; bj = 1;}

     // Process each cell within the current block
        for (int i = bi; i < i_end; ++i) {
          for (int j = bj; j < j_end; ++j) {
 
          if (i == 0 || j == 0) continue;
 
          float match = S[i - 1][j - 1] + (X[i - 1] == Y[j - 1] ? match_score : mismatch_score);
          float del = S[i - 1][j] + gap_penalty;
          float insert = S[i][j - 1] + gap_penalty;
          S[i][j] = std::max({match, del, insert});
          visited++;
 
          }
        }

      }
    }

    return visited;
}


unsigned long SequenceInfo::gpsa_tasks(float** S, int grain_size=1) {

    unsigned long visited = 0;

    // Boundary
  #pragma omp parallel
    {
    #pragma omp single
      {
      #pragma omp taskloop reduction(+:visited)
        for (unsigned int i = 1; i < rows; i++) {
            S[i][0] = i * gap_penalty;
            visited++;
      }
    }
  }

#pragma omp parallel
  {
  #pragma omp single
    {
    #pragma omp taskloop reduction(+:visited)
    for (unsigned int j = 0; j < cols; j++) {
        S[0][j] = j * gap_penalty;
        visited++;
    }
  }
}

   // add granularity of each block 
    int ndiags = rows + cols - 1;

    // loop over anti-diagonals
    for (int wave = 0; wave * grain_size < ndiags; ++wave) {


      int startIdx = (wave < cols ? 0: (wave - cols + 1) * grain_size);
      int nblocks  = (wave < rows ? wave + 1: rows + cols -1 - wave) * grain_size + startIdx;

     // begin parallel region
#pragma omp parallel 
     {
     // the loop is executed by a single master thread
#pragma omp single  
       {
     // taskgroup groups the tasks for the reduction on visited
#pragma omp taskgroup task_reduction(+:visited)
        {
      for (int bi = startIdx; bi < nblocks ; bi += grain_size) {

        int bj = wave * grain_size - bi;

        if (bj > cols) continue; // Ensure within bounds
                                
     // spawn a task for each block
#pragma omp task in_reduction(+: visited)
       {
         
        int i_end = std::min(bi + grain_size, rows);
        int j_end = std::min(bj + grain_size, cols);

        if ( wave == 0) {bi = 1; bj = 1;}

     // Process each cell within the current block
        for (int i = bi; i < i_end; ++i) {
          for (int j = bj; j < j_end; ++j) {

          if (i == 0 || j == 0) continue;

          float match = S[i - 1][j - 1] + (X[i - 1] == Y[j - 1] ? match_score : mismatch_score);
          float del = S[i - 1][j] + gap_penalty;
          float insert = S[i][j - 1] + gap_penalty;
          S[i][j] = std::max({match, del, insert});
          visited++;

          }
        }
      } 
    }
          }
        }  
      } 
    }     

    return visited;
}
