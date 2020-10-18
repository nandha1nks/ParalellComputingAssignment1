#include<stdio.h>
#include<mpi.h>
#include<time.h>
#include<stdlib.h>
#define N 100
#define start 2
#define dest 70

int main(int argc, char** argv){
	int rank, np;
	MPI_Init(&argc, &argv);
	int graph[N][N], com[N][N];
	int i,j, resumeIdx;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	int batch = N/np;
	
	if(rank == 0){
		for(i=0;i<N;i++) {
			graph[i][i] = 0;
			com[i][i] = 0;
			for(j=i+1;j<N;j++) {
				graph[i][j] = rand()%2;
				graph[j][i] = com[j][i] = com[i][j] = graph[i][j];
			}
		}
		
		clock_t stime = clock();
		int l = 2;
		for(i=1;i<np;i++) {
			MPI_Send(&graph, N*N, MPI_INT, i, 0, MPI_COMM_WORLD); 
		}
		if(graph[start][dest] != 0) {
			printf("The length of the path is 1\n");
			resumeIdx = 0;
			l = 1;
			resumeIdx = 0;
			for(i = 1;i<np;i++){
				MPI_Send(&resumeIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
			}
		}
		resumeIdx = 1;
		while(l!=1 && l<=N) {
			for(int i = 1;i<np;i++){
				MPI_Send(&resumeIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(&com[i*batch][0], batch*N, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
			
			int a[N][N], k;
			for(i = 0; i< batch;i++){
				for(j=0;j<N;j++){
					for(k=0;k<N;k++){
						a[i][j] = com[i][k] * graph[k][j];
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
			if(graph[start][dest] != 0) {
				printf("The length of the path is %d\n", l);
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
		
		if(l>N){
			resumeIdx = 0;
			for(i = 1;i<np;i++){
				MPI_Send(&resumeIdx, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
			}
		}
		
		clock_t end = clock() - stime;
		float d = ((double)end)/CLOCKS_PER_SEC;
		printf("%fsec\n", d);
		
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
			for(i = 0; i< batch;i++){
				for(j=0;j<N;j++){
					for(k=0;k<N;k++){
						a[i][j] = com[i][k] * graph[k][j];
					}
				}
			}
			MPI_Send(&a, batch*N, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}
	MPI_Finalize();
	return 0;
}
