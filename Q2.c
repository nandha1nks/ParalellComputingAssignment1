#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<math.h>
#define N 16
#define s 1
#define d 8

int main(int argc, char** argv){
	MPI_Comm gridWorld;
	MPI_Status status;
	int np, rank;
	int matSize;
	int dims[2] = {0,0}, periods[2] = {1,1};
	int left, right, up, down;
	int **A, **B, **C;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Dims_create(np, 2, dims);
	if(dims[0] != dims[1]){
		if(rank==0) printf("The number of processors must be a square.\n");
		MPI_Finalize();
		return 0;
	}
	matSize = N/dims[0];
	int i,j,k,l,len;
	len = matSize*sizeof(int*) + matSize*matSize*sizeof(int);
	A = (int**)malloc(len);
	B = (int**)malloc(len);
	C = (int**)malloc(len);
	int **temp = (int**) malloc(len);
	for(int i=0;i<matSize;i++){
		A[i] = ((int*)(A+matSize) + matSize*i);
		B[i] = ((int*)(B+matSize) + matSize*i);
		C[i] = ((int*)(C+matSize) + matSize*i);
		temp[i] = ((int*)(temp+matSize) + matSize*i);
	}
	FILE *file;
	file = fopen("16Mat.txt", "r");
	int it = 0;
	int prR,prC, pr, c;
	prC = rank%dims[0];
	prR = rank/dims[0];
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			fscanf(file, "%d", &it);
			if(i >= prR*matSize && i < (prR+1)*matSize && j >= prC*matSize && j < (prC+1)*matSize) {
				A[i-prR*matSize][j-prC*matSize] = it;
				B[i-prR*matSize][j-prC*matSize] = it;
				C[i-prR*matSize][j-prC*matSize] = 0;
			}
		}
	}
	fclose(file);
	
	MPI_Cart_create(MPI_COMM_WORLD,2,dims,periods,1,&gridWorld);
	MPI_Cart_shift(gridWorld,1,1,&left,&right);
	MPI_Cart_shift(gridWorld,0,1,&up,&down);
	
	len = 1;
	prC = rank%dims[0];
	for(l = 0; l<prC; l++){
		MPI_Sendrecv(B+matSize,matSize*matSize,MPI_INT,up,2,temp+matSize,matSize*matSize, MPI_INT,down,2,gridWorld,&status);
		for(int i=0;i<matSize;i++)
			for(int j=0;j<matSize;j++)
				B[i][j] = temp[i][j];
	}
	while(len<N) {
		if(len>1) {
			prR = rank/dims[0];
			for(l=0;l<prR;l++){
				MPI_Sendrecv(A+matSize,matSize*matSize,MPI_INT,left,1,temp+matSize,matSize*matSize, MPI_INT,right,1,gridWorld,&status);
				for(int i=0;i<matSize;i++)
					for(int j=0;j<matSize;j++)
						A[i][j] = temp[i][j];
			}
			for(l = 0; l<dims[0]; l++) {
				for(i = 0; i<matSize; i++)
					for(k = 0;k<matSize; k++)
						for(j=0;j<matSize;j++)
							C[i][j] += A[i][k] * B[k][j];
				MPI_Sendrecv(A+matSize,matSize*matSize,MPI_INT,left,1,temp+matSize,matSize*matSize, MPI_INT,right,1,gridWorld,&status);
				for(int i=0;i<matSize;i++)
					for(int j=0;j<matSize;j++)
						A[i][j] = temp[i][j];
				MPI_Sendrecv(B+matSize,matSize*matSize,MPI_INT,up,2,temp+matSize,matSize*matSize, MPI_INT,down,2,gridWorld,&status);
				for(int i=0;i<matSize;i++)
					for(int j=0;j<matSize;j++)
						B[i][j] = temp[i][j];
			}
			for(int i=0;i<matSize;i++){
				for(int j=0;j<matSize;j++){
					A[i][j] = C[i][j];
					C[i][j] = 0;
					}
				}
		} 
		
		c = 0;
		prR = (s / matSize);
		prC = (d / matSize);
		pr = prR * dims[0] + prC;
		if(rank == pr) {
			c = A[s - prR * matSize][d - prC * matSize];
		}
		MPI_Bcast(&c, 1, MPI_INT, pr, gridWorld);
		if(c != 0){
			break;
		}
		len++;
	}
	
	if(rank == 0){
		if(len<N)
	 		printf("Length is %d\n", len);
	 	else
	 		printf("There is no path between %d and %d\n", s, d);
	}
	MPI_Finalize();
	free(A);free(B);free(C);free(temp);
	return 0;
}
