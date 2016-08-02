#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <dir.h>
#include "Disco.h"

//************* VARIABLES ***************

//************* METODOS *****************

void IngresarCom();
void CrearDisco(char* com);
void BorrarDisco(char* com);
void ManejarParticiones(char* com);

//***************************************

int main()
{

    while(1){
        IngresarCom();
    }
    return 0;
}
//************ INGRESAMOS EL COMANDO *****************
void IngresarCom(){
    char comFinal[500];
    for(int i = 0; i < 500; i++){
        comFinal[i] = '\0';
    }
    int salir = 0;
    printf("Ingrese Comando:\n");
    while(salir == 0){
        char com[200];
        fgets(com,200,stdin);
        fflush(stdin);

        int pos = strlen(com) - 2;

        if(com[pos] != '\\'){
            salir = 1;
        }else{
            com[pos] = '\0';
        }
        com[pos+1] = '\0';
        if(com[0] != '#'){
            strcat(comFinal,com);
        }
    }
    // *************** VERIFICAR QUE COMANDO ES ********

    int i = 0;
    char comAux[500];
    strcpy(comAux,comFinal);
    while(comAux[i]){
        comAux[i] = (tolower(comAux[i]));
        i++;
    }
    // *************** CREAR DISCO ********************
    if(strstr(comAux,"mkdisk")!=NULL){
        CrearDisco(comFinal);
    }else if(strstr(comAux,"rmdisk")!=NULL){
        BorrarDisco(comFinal);
    }else if(strstr(comAux,"fdisk")!=NULL){
        ManejarParticiones(comFinal);
    }

}
// *************** CREAR DISCO **********************
void CrearDisco(char* com){
    int size = 0;
    int error = 0;
    char unit = 'm';
    char path[200];
    char auxPath[200];
    char name[100];
    char auxName[100];

    int flagsize = 0;
    int flagpath = 0;
    int flagname = 0;
    // **************** SEPARAMOS VALORES ********************
    char* parametro;
    parametro = strtok(com," ::");
    while(parametro != NULL){
        if(strcasecmp(parametro,"-size")==0){
            parametro = strtok(NULL," ::");
            size = atoi(parametro);
            if(size < 1){
                printf("ERROR: El tamaño del archivo debe ser mayor a 0\n");
                error = 1;
                break;
            }
            flagsize = 1;
        }else if(strcasecmp(parametro,"+unit")==0){
            parametro = strtok(NULL," ::");
            unit = parametro[0];
            unit = tolower(unit);
            if((unit != 'k')&&(unit != 'm')){
                printf("ERROR: Unidad \"%c\" no disponible.\n",unit);
                error = 1;
                break;
            }

        }else if(strcasecmp(parametro,"-path")==0){
            parametro = strtok(NULL," ::");
            strcpy(auxPath,parametro);

            int b = 0;

            for(int c = 0; c<200; c++){
                path[c] = '\0';
            }

            for(int a = 1;a<(strlen(auxPath)-1);a++){
                    path[b] = auxPath[a];
                    b++;
            }

            if(mkdir(path,S_IRWXU)==0){
                printf("Se creo la carpeta: %s\n",auxPath);
            }
            flagpath = 1;
        }else if(strcasecmp(parametro,"-name")==0){
            parametro = strtok(NULL, " ::");
            strcpy(auxName,parametro);
            int c = 0;
            for(int a = 0; a < 100; a++){
                name[a] = '\0';
            }
            for(int b = 1; b < (strlen(auxName)-1); b++){
                name[c] = auxName[b];
                c++;
            }
            if(strstr(name,".dsk")!=NULL){
                //strcat(path,"/");
                strcat(path,name);

            }else{
                printf("ERROR: Extension del disco debe ser .dsk\n");
                error = 1;
                break;
            }
            flagname = 1;
        }else if(strcasecmp(parametro,"mkdisk")==0){

        }else{
            printf("ERROR: Parametro \"%s\" no admitido para esta funcion.\n",parametro);
            error = 1;
            break;
        }

        parametro = strtok(NULL," ::");
    }
    // ******************* CREAMOS E INGRESAMOS MBR ******************
    if(error == 0){
        if(flagsize == 1 && flagpath == 1 && flagname == 1){
            FILE *arch = fopen(path,"rb+");
            if(arch){
                fclose(arch);
                printf("ERROR: Disco ya Existe\n");
            }else{
                FILE *arch1 = fopen(path,"wb");

                int tam = 0;

                if(unit == 'k'){
                    tam = size * 1024;
                }else{
                    tam = size * 1024 * 1024;
                }

                for(int a = 0; a < tam; a++){
                    fseek(arch1,a,SEEK_SET);
                    fwrite("\0",sizeof(char),1,arch1);
                }


                CreateMBR(arch1,tam);
                //printf("disco.disk_signature_mbr = %d\n",nuevo.disk_signature_mbr);
                //printf("disco.fecha_creacion_mbr = %s\n",nuevo.fecha_creacion_mbr);
                //printf("disco.tamano_mbr = %d\n",nuevo.tamano_mbr);
                //fseek(arch1,0,SEEK_SET);
                //fwrite(&nuevo,sizeof(MBR),1,arch1);



                fclose(arch1);
                printf("Disco Creado\n");
            }
        }else{
            printf("ERROR: Faltan parametros obligatorios.\n");
        }
    }

}

void BorrarDisco(char* com){
    int error = 0;
    char path[200];
    char auxPath[200];
    char* parametro;
    parametro = strtok(com," ::");
    // **************** SEPARAR PARAMETROS *****************
    while(parametro != NULL){
        if(strcasecmp(parametro,"-path")==0){
            parametro = strtok(NULL, " ::");
            strcpy(auxPath,parametro);
            for(int a = 0; a < 200; a++){
                path[a] = '\0';
            }
            int c = 0;
            for(int b = 1; b < (strlen(auxPath)-1); b++){
                path[c] = auxPath[b];
                c++;
            }
        }else if(strcasecmp(parametro,"rmdisk")==0){

        }else{
            printf("ERROR: Parametro \"%s\" no admitido para esta funcion.\n",parametro);
            error = 1;
            break;
        }

        parametro = strtok(NULL," ::");
    }

    if(error == 0){
        if(remove(path) == 0){
            printf("Se ha eliminado el Disco.\n");
        }else{
            printf("El Disco no Existe.\n");
        }
    }
}

void ManejarParticiones(char* com){
    // *********** VARIABLES ************
    int error = 0;
    char* parametro;

    int size = 0;
    char unit = 'k';
    char path[200];
    char type = 'p';
    char fit = 'w';
    char name[50];

    // *********** BANDERAS ************
    int flagsize = 0;
    int flagpath = 0;
    int flagname = 0;
    int flagadd = 0;
    int flagdelete = 0;

    // *********** AUXILIARES ***********
    char auxPath[200];
    char auxName[50];

    parametro = strtok(com," ::");



    while(parametro != NULL){
        if(strcasecmp(parametro,"-size")==0){
            parametro = strtok(NULL, " ::");
            size = atoi(parametro);
            if(size < 1){
                printf("ERROR: El tamaño del archivo debe ser mayor a 0\n");
                error = 1;
                break;
            }
            flagsize = 1;
        }else if(strcasecmp(parametro,"+unit")==0){
            parametro = strtok(NULL," ::");
            unit = parametro[0];
            unit = tolower(unit);
            if((unit != 'k')&&(unit != 'm')&&(unit != 'b')){
                printf("ERROR: Unidad \"%c\" no disponible.\n",unit);
                error = 1;
                break;
            }
        }else if(strcasecmp(parametro,"-path")==0){
            parametro = strtok(NULL," ::");
            strcpy(auxPath,parametro);
            for(int a = 0; a < 200; a++){
                path[a] = '\0';
            }
            int c = 0;
            for(int b = 1; b < (strlen(auxPath)-1); b++){
                path[c] = auxPath[b];
                c++;
            }

            FILE *arch = fopen(path,"rb");
            if(arch){
                fclose(arch);
            }else{
                printf("ERROR: El Disco Seleccionado No Existe");
                error = 1;
                break;
            }
            flagpath = 1;
        }else if(strcasecmp(parametro,"+type")==0){
            parametro = strtok(NULL," ::");
            type = parametro[0];
            type = tolower(type);
            if((type != 'p')&&(type != 'e')&&(type != 'l')){
                printf("ERROR: Tipo de Particion \"%c\" no Admitida.\n",type);
                error = 1;
                break;
            }
        }else if(strcasecmp(parametro,"+fit")==0){
            parametro = strtok(NULL," ::");
            fit = parametro[0];
            fit = tolower(fit);
            if((strcasecmp(parametro,"bf")!=0)&&(strcasecmp(parametro,"ff")!=0)&&(strcasecmp(parametro,"wf")!=0)){
                printf("ERROR: Fit de Particion \"%s\" no Admitido.\n",parametro);
                error = 1;
                break;
            }
        }else if(strcasecmp(parametro,"+delete")==0){
            parametro = strtok(NULL," ::");

        }else if(strcasecmp(parametro,"-name")==0){
            parametro = strtok(NULL," ::");
            strcpy(auxName,parametro);
            for(int a = 0; a < 50; a++){
                name[a] = '\0';
            }
            int c = 0;
            for(int b = 1; b < (strlen(auxName)-1); b++){
                name[c] = auxName[b];
                c++;
            }
            flagname = 1;
        }else if(strcasecmp(parametro,"+add")==0){
            parametro = strtok(NULL," ::");
        }else if(strcasecmp(parametro,"fdisk")==0){

        }else{
            printf("ERROR: Parametro \"%s\" no admitido para esta funcion.\n",parametro);
            error = 1;
            break;
        }

        parametro = strtok(NULL, " ::");
    }

    if(error == 0){
        // *********** AJUSTAR TAMANO ******
        if(unit == 'k'){
            size = size * 1024;
        }else if(unit == 'm'){
            size = size * 1024 * 1024;
        }
        // *********** ACCION **************
        if((flagadd == 0)&&(flagdelete == 1)){
            // *********** SE ELIMINA PARTICION **********
        }else if((flagadd == 1)&&(flagdelete == 0)){
            // *********** SE MODIFICA PARTICION *********
        }else if((flagadd == 0)&&(flagdelete == 0)){
            // *********** SE CREA LA PARTICION **********
            if((flagname == 1)&&(flagpath == 1)&&(flagsize == 1)){
                FILE *disco = fopen(path,"rb+");
                //MBR auxPart;
                //fseek(disco,0,SEEK_SET);
                //fread(&auxPart,sizeof(MBR),1,disco);
                CreateParticion(disco,size,fit,name,type);
                //VerParticiones(disco);
                fclose(disco);


            }else{
                printf("ERROR: Faltan Parametros Obligatorios para Crear Particion.\n");
            }
        }
    }
}

void VerParticiones(FILE* disco){
    MBR auxPart;
    fseek(disco,0,SEEK_SET);
    fread(&auxPart,sizeof(MBR),1,disco);
    printf("******************************************************\n");
    for(int a = 0; a < 4; a++){
        if(auxPart.partition_table[a].part_status != '0'){
            printf("Particion.name = %s\n",auxPart.partition_table[a].name);
            printf("Particion.part_type = %c\n",auxPart.partition_table[a].part_type);
            printf("Particion.part_size = %d\n",auxPart.partition_table[a].part_size);
            printf("Particion.part_start = %d\n",auxPart.partition_table[a].part_start);
            printf("******************************************************\n");
        }
    }
}


