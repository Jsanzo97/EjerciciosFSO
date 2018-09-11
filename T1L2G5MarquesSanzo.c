/*
 * Autores:
 * Jaime Marqués Castrillo
 * Jorge Sanzo Hernando
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdint.h>
#include <string.h>

									//Variables globales que comparten 1 o mas hilos
int *Buffer1;
char **Buffer2;
int tamBuffer1;
int tamBuffer2;
int k=0, j=0, p=0, num, orden=1;
									//Semaforos necesarios
sem_t HayEspacioBuffer1;
sem_t HayDatoBuffer1;
sem_t HayEspacioBuffer2;
sem_t HayDatoBuffer2;
sem_t MutexBuffer1;
sem_t MutexBuffer2;

void *creaNumeros(void *arg){					//Productor

	/* El productor espera a que haya espacio en el buffer para meter el dato .
	 * Se garantiza la exclusion mutua para escribir en el buffer el numero aleatorio.
	 * Cuando escriben el numero avisan de que hay datos en el buffer
	 */
	srand(time(NULL));
	int x=0, i=1;
	double numRand;
	int num = (int) arg;
	while (i <= num){
		numRand = (((double)rand()/(double)RAND_MAX)*99998.0)+1.0;
		sem_wait(&HayEspacioBuffer1);
		sem_wait(&MutexBuffer1);
		Buffer1[x] = (int)numRand;
		fflush(stdout);
		sem_post(&MutexBuffer1);
		sem_post(&HayDatoBuffer1);
		x=(x+1)%tamBuffer1;
		i++;
		}
	printf("Numero de datos producidos: %d\n", i-1);
	fflush(stdout);
	pthread_exit(NULL);
	return NULL;
}

void *esPrimo (void *arg){						//Consumidor

	/* Cada hilo consumidor debe esperar a que haya datos en el buffer
	 * Se garantiza la exclusion mutua para la lectura
	 * Cuando leen el dato avisan de que hay nuevo espacio en el buffer
	 * Estos hilos son a la vez productores que escriben en el buffer2
	 * Cuando escriben el dato avisan de que hay un dato en el buffer2
	 * Se garantiza exclusion mutua para la escritura
	 *
	 */
	int *id = (int*) arg;
	int i, a=0;
	char aux[10];
	while(k<num){
		sem_wait(&HayDatoBuffer1);
		sem_wait(&MutexBuffer1);
		for(i=1; i<Buffer1[j]+1; i++){
			if(Buffer1[j]%i==0){
				a++;
			}
		}
		sem_wait(&HayEspacioBuffer2);
		sem_wait(&MutexBuffer2);
		//Creamos el string que almacenara el buffer2
		if(a==2){
			strcpy(Buffer2[p],"Hilo numero: ");
			sprintf(aux,"%d", *id);
			strcat(Buffer2[p],aux);
			strcat(Buffer2[p]," analizo el numero: ");
			sprintf(aux,"%d",Buffer1[j]);
			strcat(Buffer2[p],aux);
			strcat(Buffer2[p]," y es primo. Orden de produccion: ");
			sprintf(aux,"%d", orden);
			strcat(Buffer2[p],aux);

		}else{
			strcpy(Buffer2[p],"Hilo numero: ");
			sprintf(aux,"%d", *id);
			strcat(Buffer2[p],aux);
			strcat(Buffer2[p]," analizo el numero: ");
			sprintf(aux,"%d",Buffer1[j]);
			strcat(Buffer2[p],aux);
			strcat(Buffer2[p]," y no es primo. Orden de produccion: ");
			sprintf(aux,"%d", orden);
			strcat(Buffer2[p],aux);

		}
		orden++;
		sem_post(&MutexBuffer2);
		sem_post(&HayDatoBuffer2);

		j=(j+1)%tamBuffer1;
		p=(p+1)%tamBuffer2;
		a=0;
		k++;
		sem_post(&MutexBuffer1);
		sem_post(&HayEspacioBuffer1);
	}
	printf("Numero de datos consumidos: %d\n",k);
	fflush(stdout);
	pthread_exit(NULL);
	return NULL;
}

void *escribeFichero(void *arg){				//Consumidor final

	/* Debe esperar a que haya datos en el buffer2
	 * Cuando escribe en el fichero avisa de que hay espacio en el buffer2
	 */
	FILE *fichero;
	int num = (int)arg;
	int q=0, w=0;
	fichero= fopen("C:/Users/Jorge/Desktop/fichero.txt","w");
	if(fichero == NULL){
			fichero= fopen("C:/Users/Jorge/Desktop/fichero.txt","a");
		}

	while(q<num){
		sem_wait(&HayDatoBuffer2);
		fprintf (fichero,"%s\n",Buffer2[w]);
		sem_post(&HayEspacioBuffer2);
		fflush(stdout);
		q++;
		w=(w+1)%tamBuffer2;
	}
	printf("Numero de datos en el fichero: %d\n",q);
	printf("Datos guardados en fichero.txt\n");
	fflush(stdout);
	fclose(fichero);
	printf("\nFIN");
	fflush(stdout);
	exit(-1);
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[]) {
	int cantNumeros;
	int *I;
	pthread_t HProductor;
	pthread_t HConsum [4];
	pthread_t HEscritor;

	cantNumeros = atof(argv[1]);
	num = cantNumeros;
	if (cantNumeros<2){
		printf("Argumento 1 incorrecto");
		exit(-1);
	}
	printf("Cantidad de numeros a producir = %d\n", cantNumeros );

	tamBuffer1 = atof(argv[2]);
	if(tamBuffer1>(cantNumeros/2) || tamBuffer1==0){
		printf("Argumento 2 incorrecto");
		exit(-1);
	}
	printf("TamañoBuffer1 = %d\n", tamBuffer1 );
	if ((Buffer1 = malloc(tamBuffer1*sizeof(int)))){
	}else{
		printf("Error de programa");
		exit(-1);
	}

	tamBuffer2 = atof(argv[3]);
	if(tamBuffer2>(cantNumeros/2) || tamBuffer2==0){
		printf("Argumento 3 incorrecto");
			exit(-1);
	}
	printf("TamañoBuffer2 = %d\n", tamBuffer2 );
	if ((Buffer2 = malloc(tamBuffer2*sizeof(int)))){
	}else{
		printf("Error de programa");
		exit(-1);
	}
	if((Buffer2 = (char**)malloc(tamBuffer2*sizeof(char*)))){
		int i=0;
		for (i=0; i<tamBuffer2; i++)
			if((Buffer2[i] = (char*)malloc(100*sizeof(char*)))){
			}else{
				printf("Error de programa\n");
				exit(-1);
			}
	}


	fflush(stdout);

	sem_init (&HayEspacioBuffer1, 0, tamBuffer1);
	sem_init (&HayDatoBuffer1, 0, 0);
	sem_init (&MutexBuffer1, 0, 1);
	sem_init (&HayEspacioBuffer2, 0, tamBuffer2);
	sem_init (&HayDatoBuffer2, 0, 0);
	sem_init (&MutexBuffer2, 0, 1);

	int i;
	pthread_create (&HProductor, NULL, creaNumeros, (void *)cantNumeros);
	if ((I = malloc(4*sizeof(int)))){
	}else{
		printf("Error de programa");
		exit(-1);
		}
	for(i=0; i<4; i++){
		I[i]=i+1;
	}
	for(i=0; i<4; i++){
		pthread_create (&HConsum[i], NULL, esPrimo, (void*)&I[i]);
	}
	pthread_create (&HEscritor, NULL, escribeFichero, (void *)cantNumeros);
	for(i=0; i<4; i++){
		pthread_join(HConsum[i], NULL);
	}
	return 0;
}
