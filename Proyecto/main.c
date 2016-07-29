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
                printf("Error: El tamaño del archivo debe ser mayor a 0\n");
                error = 1;
                break;
            }
            flagsize = 1;
        }else if(strcasecmp(parametro,"+unit")==0){
            parametro = strtok(NULL," ::");
            unit = parametro[0];
            unit = tolower(unit);
            if((unit != 'k')&&(unit != 'm')){
                printf("Error: Unidad \"%c\" no disponible.\n",unit);
                error = 1;
                break;
            }

        }else if(strcasecmp(parametro,"-path")==0){
            parametro = strtok(NULL," ::");
            strcpy(auxPath,parametro);

            int b = 0;

            for(int c = 0; c<199; c++){
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
            for(int a = 0; a < 99; a++){
                name[a] = '\0';
            }
            for(int b = 1; b < (strlen(auxName)-1); b++){
                name[c] = auxName[b];
                c++;
            }
            if(strstr(name,".dsk")!=NULL){
                strcat(path,"/");
                strcat(path,name);

            }else{
                printf("Error: Extension del disco debe ser .dsk\n");
                error = 1;
                break;
            }
            flagname = 1;
        }else if(strcasecmp(parametro,"mkdisk")==0){

        }else{
            printf("Error: Parametro \"%s\" no admitido para esta funcion.\n",parametro);
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
                printf("Error: Disco ya Existe\n");
            }else{
                FILE *arch1 = fopen(path,"wb");

                int tam = 0;

                if(unit == 'k'){
                    tam = size * 1000;
                }else{
                    tam = size * 1000 * 1000;
                }

                for(int a = 0; a < tam; a++){
                    fseek(arch1,a,SEEK_SET);
                    fwrite("\0",sizeof(char),1,arch1);
                }

                MBR nuevo;
                CreateMBR(nuevo,tam);

                fseek(arch1,0,SEEK_SET);
                fwrite(&nuevo,sizeof(MBR),1,arch1);

                fclose(arch1);
                printf("Disco Creado\n");
            }
        }else{
            printf("Error: Faltan parametros obligatorios.\n");
        }
    }

}











