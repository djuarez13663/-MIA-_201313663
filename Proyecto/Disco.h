#include <stdio.h>
#include <time.h>

typedef struct mbr{
    int tamano_mbr;
    char fecha_creacion_mbr[200];
    int disk_signature_mbr;
}MBR;

typedef struct particion{
    char part_status;
    char part_type;
    char part_fit;
    int part_start;
    int part_size;
    char name[16];
}particion;

void CreateMBR(MBR nuevo,int size){
    nuevo.tamano_mbr = size;
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char salida[128];
    strftime(salida,128,"%d/%m/%y %H:%M:%S",tlocal);
    strcpy(nuevo.fecha_creacion_mbr,salida);
    int semilla = time(NULL);
    srand(semilla);
    nuevo.disk_signature_mbr = rand()%100 + 1;

}
