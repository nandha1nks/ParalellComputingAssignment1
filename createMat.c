#include<stdio.h>
#include<stdlib.h>

int main() {
	int N;
	printf("No. of vertices in the graph: ");
	scanf("%d", &N);
	int *a = (int*)malloc(N*N * sizeof(int));
	int i,j, temp;
	for(i=0;i<N;i++){
		a[i*N + i] = 0;
		for(j=i+1;j<N;j++){
			temp = rand()%4;
			if(temp == 1)
				temp = 1;
			else
				temp = 0;
			a[i*N + j] = temp; 
			a[j*N + i] = a[i*N + j];
		}
	}
	FILE *file;
	file = fopen("Mat.txt", "w+");
	for(i=0;i<N;i++){
		for(j=0;j<N;j++){
			fprintf(file, "%d ", a[i*N+j]);
		}
		fprintf(file, "\n");
	}
	fclose(file);
	return 0;
}
