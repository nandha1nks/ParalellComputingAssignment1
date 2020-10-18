#include<stdio.h>
#include<mpi.h>
#include<time.h>
#include<stdlib.h>
#define N 16
#define start 0
#define dest 8

int main(int argc, char** argv){
	int rank, np;
	MPI_Init(&argc, &argv);
	int graph[N][N], com[N][N];
	int i,j,k, resumeIdx;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	int batch = N/np;
	
	if(rank == 0){
		FILE *file;
		file = fopen("Mat.txt", "r");
		for(i=0;i<N;i++) {
			for(j=0;j<N;j++) {
				fscanf(file, "%d", &k);
				graph[i][j] = k;
				com[i][j] = k;
			}
		}
		fclose(file);
		clock_t stime = clock();
		int l = 2;
		for(i=1;i<np;i++) {
			MPI_Send(&graph, N*N, MPI_INT, i, 0, MPI_COMM_WORLD); 
		}
		if(com[start][dest] != 0) {
			resumeIdx = 0;
			l = 1;
			resumeIdx = 0;
			for(i = 1;i<np;i++){
				MPI_Send(&resumeIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
			}
		}
		resumeIdx = 1;
		while(l!=1 && l<N) {
			for(i = 1;i<np;i++){
				MPI_Send(&resumeIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(&com[i*batch][0], batch*N, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
			
			int a[N][N] = {0};
			for(i = 0;i<N;i++)
				for(j=0;j<N;j++)
					a[i][j] = 0;
			for(i = 0; i< batch;i++){
				for(k=0;k<N;k++){
					for(j=0;j<N;j++){
						a[i][j] += com[i][k] * graph[k][j];
					}
				}
			}
			for(i = 0; i<batch;i++){
				for(j=0;j<N;j++){
					com[i][j] = a[i][j];
				}
			}
			
			for(i=1;i<np;i++) {
				MPI_Recv(&com[i*batch][0], batch*N, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			if(com[start][dest] != 0) {
				resumeIdx = 0;
				for(i = 1;i<np;i++){
					MPI_Send(&resumeIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
				}
				break;
			} else {
				resumeIdx = 1;
				l++;
			}
		}
		
		if(l>=N){
			resumeIdx = 0;
			for(i = 1;i<np;i++){
				MPI_Send(&resumeIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
			}
			
		}
		
		clock_t end = clock() - stime;
		float d = ((double)end)/CLOCKS_PER_SEC;
		printf("%fsec\n", d);
		if(l<N)
			printf("Length is %d\n", l);
		else
			printf("There is no path between %d and %d\n", start, dest);
	} 
	
	
	else {
			MPI_Recv(&graph, N*N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		while(1){
			int a[N][N], k;
			MPI_Recv(&resumeIdx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(resumeIdx == 0) {
				break;
			}
			MPI_Recv(&com[0][0], N*N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for(i = 0;i<N;i++)
				for(j=0;j<N;j++)
					a[i][j] = 0;
			for(i = 0; i< batch;i++){
				for(k=0;k<N;k++){
					for(j=0;j<N;j++){
						a[i][j] += com[i][k] * graph[k][j];
					}
				}
			}
			MPI_Send(&a, batch*N, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}
	MPI_Finalize();
	return 0;
}
