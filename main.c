#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#define D 1000 // Dimension of A matrix (DxD) and V and Vr vectors (D)

// Contains the necessary information for a thread to do its calculation
struct Thread_Args
{
  int iterations; // Number of iterations to perform
  int from;       // Operate on vector starting from this index
  int to;         // Operate on vector up to this index
  float **Vr;     // Vector to contain the final result
  float ***A;     // Iteration matrix
  float **V;      // Vector to operate upon
  int method;     // Calculation method to use, where a line is a row (1) or a column (2)
};

// Initialize iteration matrix A as desired
void init_A_matrix(float ***A)
{
  // Reserve memory space
  *A = malloc(sizeof(float *) * D);
  if (*A == NULL)
  {
    fprintf(stderr, "error occured in first malloc of init_A_matrix\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < D; i++)
  {
    (*A)[i] = malloc(sizeof(float) * D);
    if ((*A)[i] == NULL)
    {
      fprintf(stderr, "error occured in i-th malloc of init_A_matrix\n");
      exit(EXIT_FAILURE);
    }
  }

  // Initialize values
  for (int i = 0; i < D; i++)
  {
    for (int j = 0; j < D; j++)
    {
      // Initialize as an identity matrix * 0.999
      if (i == j)
      {
        (*A)[i][j] = 0.999;
      }
    }
  }
}

// Initialize vector V that will be subject to iteration
void init_V_vector(float **V)
{
  // Reserve memory space
  *V = malloc(sizeof(float) * D);
  if (*V == NULL)
  {
    fprintf(stderr, "error occured in malloc of init_V_vector\n");
    exit(EXIT_FAILURE);
  }

  // Initialize values
  for (int i = 0; i < D; i++)
  {
    // Series from 1 to D (1, 2, 3, ..., D-1, D)
    (*V)[i] = (float)i + 1.0;
  }
}

// Initialize vector Vr that will contain the final result after iteration
void init_Vr_vector(float **Vr)
{
  // Reserve memory space
  *Vr = malloc(sizeof(float) * D);
  if (*Vr == NULL)
  {
    fprintf(stderr, "error occured in malloc of init_Vr_vector\n");
    exit(EXIT_FAILURE);
  }

  // Initialize values
  for (int i = 0; i < D; i++)
  {
    (*Vr)[i] = 0.0;
  }
}

// Initialize the thread arguments for each thread
void init_thread_args(struct Thread_Args thread_args[], int nb_threads, int iterations, float **Vr, float ***A, float **V, int method)
{
  int lines_per_thread = D / nb_threads;
  int from = 0;
  int to = from + lines_per_thread;

  for (int i = 0; i < nb_threads; i++)
  {
    thread_args[i].iterations = iterations;
    thread_args[i].from = from;
    thread_args[i].to = to;
    thread_args[i].Vr = Vr;
    thread_args[i].A = A;
    thread_args[i].V = V;
    thread_args[i].method = method;

    if (i == nb_threads - 1)
    {
      // Last thread also calculates the excess lines if nb_threads does not evenly divide the total number of lines
      thread_args[i].to = D;
    }

    from += lines_per_thread;
    to += lines_per_thread;
  }
}

// Calculate an iteration of a line of V(n+1) using A * V(n) as specified by the method
void iterate_line(int line, float **Vr, float ***A, float **V, int method)
{
  // Reset given line before each iteration
  (*Vr)[line] = 0;

  if (method == 1)
  {
    for (int j = 0; j < D; j++)
    {
      (*Vr)[line] += (*A)[line][j] * (*V)[j];
    }
  }
  if (method == 2)
  {
    for (int j = 0; j < D; j++)
    {
      (*Vr)[line] += (*A)[j][line] * (*V)[j];
    }
  }
}

// Calculate an iteration on the range of lines [from:to] of V(n+1)
void iterate_lines(int from, int to, float **Vr, float ***A, float **V, int method)
{
  for (int i = from; i < to; i++)
  {
    iterate_line(i, Vr, A, V, method);
  }
}

// Calculate n iterations on the range of lines [from:to] of V(n+1)
// Returns a pointer to the vector that contains the final result
float **iterate(int n, int from, int to, float **Vr, float ***A, float **V, int method)
{
  float **temp;

  for (int i = 0; i < n; i++)
  {
    iterate_lines(from, to, Vr, A, V, method);

    /**
     * V(n+1) = A * V(n) <=> Vr = A * V
     * Vr becomes V for the next iteration.
     * Since we no longer care about V after a given iteration, it becomes Vr for the next iteration.
     * Swap V and Vr to maintain Vr = A * V logic on each iteration
     */
    temp = V;
    V = Vr;
    Vr = temp;
  }

  // Reverse final swap
  temp = V;
  V = Vr;
  Vr = temp;

  return Vr;
}

// Starts iteration operation for a thread
// Returns a pointer to the vector that contains the final result (passed through from iterate(...))
void *thread_exec(void *args)
{
  struct Thread_Args *args_typed = (struct Thread_Args *)args;
  float **res = iterate(
      args_typed->iterations,
      args_typed->from,
      args_typed->to,
      args_typed->Vr,
      args_typed->A,
      args_typed->V,
      args_typed->method);

  pthread_exit(res);
}

int main(int argc, char *argv[])
{
  int method = atoi(argv[1]);     // Use method 1 or 2
  int threads = atoi(argv[2]);    // Number of threads to use
  int iterations = atoi(argv[3]); // Number of iterations to perform

  // Global error container
  int error = 0;

  float **A;
  float *V;
  float *Vr;
  init_A_matrix(&A);
  init_V_vector(&V);
  init_Vr_vector(&Vr);

  // Print A
  // for (int i = 0; i < D; i++)
  // {
  //   for (int j = 0; j < D; j++)
  //   {
  //     printf("%f ", A[i][j]);
  //   }
  //   printf("\n");
  // }

  // Print V
  // for (int i = 0; i < D; i++)
  // {
  //   printf("%f\n", V[i]);
  // }

  // Print Vr before
  // printf("Vr before\n");
  // for (int i = 0; i < D; i++)
  // {
  //   printf("%f\n", Vr[i]);
  // }

  // Pointer to the memory space that contains the final result vector
  // The final result may be in V or Vr depending on the even or odd number of iterations
  float **res;

  struct Thread_Args thread_args[threads];
  init_thread_args(thread_args, threads, iterations, &Vr, &A, &V, method);

  struct timeval t_start;
  struct timeval t_end;
  error = gettimeofday(&t_start, NULL); // Start performance profiling
  if (error == -1)
  {
    fprintf(stderr, "error occured in initializing gettimeofday: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  pthread_t tid[threads];
  for (int i = 0; i < threads; i++)
  {
    error = pthread_create(&tid[i], NULL, thread_exec, &(thread_args[i]));
    if (error != 0)
    {
      fprintf(stderr, "error occured in pthread_create: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    error = 0;
  }

  for (int i = 0; i < threads; i++)
  {
    // Wait for all threads to finish
    error = pthread_join(tid[i], (void *)&res);
    if (error != 0)
    {
      fprintf(stderr, "error occured in pthread_join: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    error = 0;
  }

  error = gettimeofday(&t_end, NULL); // End performance profiling once all threads are finished
  if (error == -1)
  {
    fprintf(stderr, "error occured in terminating gettimeofday: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // Print result
  printf("Result\n");
  for (int i = 0; i < D; i++)
  {
    printf("%f\n", (*res)[i]);
  }

  // Print profiling data
  printf("\n");
  printf("Method: %d\n", method);
  printf("Threads: %d\n", threads);
  printf("Iterations: %d\n", iterations);
  printf("Time: %ld microsec\n", ((t_end.tv_sec - t_start.tv_sec) * 1000000) + t_end.tv_usec - t_start.tv_usec);
  printf("Time: %f sec\n", (((t_end.tv_sec - t_start.tv_sec) * 1000000) + t_end.tv_usec - t_start.tv_usec) / 1000000.0);
}
