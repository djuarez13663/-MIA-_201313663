#include <stdio.h>
#include <time.h>

typedef struct particion{
    char part_status;
    char part_type;
    char part_fit;
    int part_start;
    int part_size;
    char name[16];
}PARTICION;

typedef struct mbr{
    int tamano_mbr;
    char fecha_creacion_mbr[128];
    int disk_signature_mbr;
    PARTICION partition_table[4];
}MBR;

typedef struct ebr{
    char part_status;
    char part_fit;
    int part_start;
    int part_size;
    int part_next;
    char part_name[16];
}EBR;

void CreateMBR(FILE *arch,int size);
void CreateParticion(FILE *arch,int _size,char fit, char* name,char tipo);
int getCountPartition(MBR disco);
int getPosNuevaPart(MBR disco);
int getInitPartition(MBR disco,int _size);
void OrderPartitions(MBR *disco);
int ExistExtended(MBR disco);
void Delete_Partition(FILE *disco,char* name,char* type_delete);
void getInitLogic(FILE *arch, int _size, EBR nuevaEBR);

void CreateMBR(FILE *arch,int size){
    MBR nuevo;

    nuevo.tamano_mbr = size;
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char salida[128];
    strftime(salida,128,"%d/%m/%y %H:%M:%S",tlocal);
    //printf("Fecha: %s\n",salida);
    strcpy(nuevo.fecha_creacion_mbr,salida);
    int semilla = time(NULL);
    srand(semilla);
    nuevo.disk_signature_mbr = rand()%1000 + 1;

    for(int a = 0; a < 4; a++){
        strcpy(nuevo.partition_table[a].name,"Vacia");
        nuevo.partition_table[a].part_fit = '\0';
        nuevo.partition_table[a].part_size = 0;
        nuevo.partition_table[a].part_start = -1;
        nuevo.partition_table[a].part_status = '0';
        nuevo.partition_table[a].part_type = '\0';
    }
    //printf("TAMANO MBR: %d\n",sizeof(MBR));
    fseek(arch,0,SEEK_SET);
    fwrite(&nuevo,sizeof(MBR),1,arch);
}

void CreateParticion(FILE *arch,int _size,char fit, char* name,char tipo){
    MBR auxPart;
    fseek(arch,0,SEEK_SET);
    fread(&auxPart,sizeof(MBR),1,arch);



    if(tipo == 'p'){
        if((auxPart.partition_table[0].part_status == '0')&&(getCountPartition(auxPart)==0)){
            // NUEVA PARTICION
            if(_size <= auxPart.tamano_mbr){
                PARTICION nueva;
                strcpy(nueva.name,name);
                nueva.part_fit = fit;
                nueva.part_size = _size;
                nueva.part_start = sizeof(MBR) + 1;
                nueva.part_status = '1';
                nueva.part_type = tipo;
                // POSICION EN EL ARREGLO
                auxPart.partition_table[0] = nueva;
                // ORDENAMOS EL ARREGLO
                OrderPartitions(&auxPart);
                //ESCRIBIMOS EL ARCHIVO
                fseek(arch,0,SEEK_SET);
                fwrite(&auxPart,sizeof(MBR),1,arch);
            }else{
                printf("ERROR: No Hay Espacio Suficiente Para Esa Particion.\n");
            }

        }else{
            int pos = getPosNuevaPart(auxPart);
            if(pos != -1){
                int start = getInitPartition(auxPart,_size);
                if(start != -1){
                    // NUEVA PARTICION
                    PARTICION nueva;
                    strcpy(nueva.name,name);
                    nueva.part_fit = fit;
                    nueva.part_size = _size;
                    nueva.part_start = start;
                    nueva.part_status = '1';
                    nueva.part_type = tipo;
                    // POSICION EN EL ARREGLO
                    auxPart.partition_table[pos] = nueva;
                    // ORDENAMOS EL ARREGLO
                    OrderPartitions(&auxPart);
                    // ESCRIBIMOS EL ARCHIVO
                    fseek(arch,0,SEEK_SET);
                    fwrite(&auxPart,sizeof(MBR),1,arch);

                }else{
                    printf("ERROR: No Hay Espacio Suficiente Para Esa Particion.\n");
                }
            }else{
                printf("ERROR: No Hay Espacio Para Mas Particiones.\n");
            }
        }
    }else if(tipo == 'e'){
        if(auxPart.partition_table[0].part_status == '0'){
            if(_size <= auxPart.tamano_mbr){
                // PARTICION NUEVA
                PARTICION nueva;
                strcpy(nueva.name,name);
                nueva.part_fit = fit;
                nueva.part_size = _size;
                nueva.part_start = sizeof(MBR) + 1;
                nueva.part_status = '1';
                nueva.part_type = tipo;
                // POSICION EN EL ARREGLO
                auxPart.partition_table[0] = nueva;
                // ORDENAMOS EL ARREGLO
                OrderPartitions(&auxPart);
                // ESCRIBIMOS EL ARCHIVO
                fseek(arch,0,SEEK_SET);
                fwrite(&auxPart,sizeof(MBR),1,arch);
                // AGREGAMOS PRIMER EBR
                EBR nuevaEBR;
                strcpy(nuevaEBR.part_name,"Inicio");
                nuevaEBR.part_fit = '\0';
                nuevaEBR.part_next = -1;
                nuevaEBR.part_size = sizeof(EBR);
                nuevaEBR.part_start = nueva.part_start;
                nuevaEBR.part_status = '0';
                fseek(arch,nueva.part_start,SEEK_SET);
                fwrite(&nuevaEBR,sizeof(EBR),1,arch);

            }else{
                printf("ERROR: No Hay Espacio Suficiente Para Esa Particion.\n");
            }

        }else{
            int pos = getPosNuevaPart(auxPart);
            if(pos != -1){
                if(ExistExtended(auxPart) == 0){
                    int start = getInitPartition(auxPart,_size);
                    if(start != -1){
                        // PARTICION NUEVA
                        PARTICION nueva;
                        strcpy(nueva.name,name);
                        nueva.part_fit = fit;
                        nueva.part_size = _size;
                        nueva.part_start = start;
                        nueva.part_status = '1';
                        nueva.part_type = tipo;
                        // POSICION EN EL ARREGLO
                        auxPart.partition_table[pos] = nueva;
                        // ORDENAMOS EL ARREGLO
                        OrderPartitions(&auxPart);
                        // ESCRIBIMOS EL ARCHIVO
                        fseek(arch,0,SEEK_SET);
                        fwrite(&auxPart,sizeof(MBR),1,arch);
                        // AGREGAMOS PRIMER EBR
                        EBR nuevaEBR;
                        strcpy(nuevaEBR.part_name,"Inicio");
                        nuevaEBR.part_fit = '\0';
                        nuevaEBR.part_next = -1;
                        nuevaEBR.part_size = sizeof(EBR);
                        nuevaEBR.part_start = nueva.part_start;
                        nuevaEBR.part_status = '0';
                        fseek(arch,nueva.part_start,SEEK_SET);
                        fwrite(&nuevaEBR,sizeof(EBR),1,arch);
                    }else{
                        printf("ERROR: No Hay Espacio Suficiente Para Esa Particion.\n");
                    }
                }else{
                    printf("ERROR: Ya Existe Una Particion Extendida En El Disco.\n");
                }
            }else{
                printf("ERROR: No Hay Espacio Para Mas Particiones.\n");
            }
        }
    }else{
        // PARTICION LOGICA
        if(ExistExtended(auxPart)==1){
            EBR nuevaEBR;
            nuevaEBR.part_fit = fit;
            nuevaEBR.part_size = _size;
            nuevaEBR.part_status = '1';
            strcpy(nuevaEBR.part_name,name);
            getInitLogic(arch,_size,nuevaEBR);
        }else{
            printf("ERROR: Debe Crear Primero Una Particion Extendida.\n");
        }
    }
}

int getCountPartition(MBR disco){
    int count = 0;

    for(int a = 0; a < 4; a++){
        if(disco.partition_table[a].part_status != '0'){
            //printf("disco.partition_table[%d].part_status = %c\n",a,disco.partition_table[a].part_status);
            count++;
        }
    }
    //printf("Count: %d\n",count);
    return count;
}

int getPosNuevaPart(MBR disco){
    int pos = -1;
    int aux = 0;
    while(aux < 4){
        if(disco.partition_table[aux].part_status == '0'){
            pos = aux;
            break;
        }
        aux++;
    }
    return pos;
}

int getInitPartition(MBR disco,int _size){
    int start = -1;
    int refe = 0;
    int _max = 0;
    int asigno = 0;

    if(disco.partition_table[0].part_start != (sizeof(MBR)+1)){
        refe = sizeof(MBR)+1;
        _max = disco.partition_table[0].part_start - 1;
        if((_max - refe)>=_size){
            start = refe;
            asigno = 1;
        }
    }

    if(asigno == 0){
        for(int cont = 0; cont < 3; cont++){
            if(disco.partition_table[cont].part_status != '0'){
                refe = disco.partition_table[cont].part_start + disco.partition_table[cont].part_size;
            }else{
                break;
            }

            if(disco.partition_table[cont+1].part_status != '0'){
                _max = disco.partition_table[cont+1].part_start - 1;
            }else{
                _max = disco.tamano_mbr;
            }
            if((_max - refe) >= _size){
                start = refe;
                break;
            }
        }
    }
    return start;
}

void OrderPartitions(MBR *disco){
    for(int a = 0; a < 4; a++){
        for(int b = 0; b < 3; b++){
            if((*disco).partition_table[b].part_status != '0'){
                if((*disco).partition_table[b+1].part_status != '0'){
                    if((*disco).partition_table[b].part_start > (*disco).partition_table[b+1].part_start){
                        PARTICION temp = (*disco).partition_table[b];
                        (*disco).partition_table[b] = (*disco).partition_table[b+1];
                        (*disco).partition_table[b+1] = temp;
                    }
                }
            }else{
                if((*disco).partition_table[b+1].part_status != '0'){
                    PARTICION temp = (*disco).partition_table[b];
                    (*disco).partition_table[b] = (*disco).partition_table[b+1];
                    (*disco).partition_table[b+1] = temp;
                }
            }
        }
    }
}

int ExistExtended(MBR disco){
    int exists = 0;
    for(int a = 0; a < 4; a++){
        if(disco.partition_table[a].part_type == 'e'){
            if(disco.partition_table[a].part_status == '1'){
                exists = 1;
            }
        }
    }
    return exists;
}

void Delete_Partition(FILE *arch,char* name,char* type_delete){
    MBR disco;
    fseek(arch,0,SEEK_SET);
    fread(&disco,sizeof(MBR),1,arch);

    for(int a = 0; a < 4; a++){
            if(strcmp(disco.partition_table[a].name,name)==0){
                disco.partition_table[a].part_status = '0';

                if(strcasecmp(type_delete,"full")==0){

                    for(int b = 0; b <= disco.partition_table[a].part_size; a++){
                        int pos = b + disco.partition_table[a].part_start;
                        fseek(arch,pos,SEEK_SET);
                        fwrite("\0",sizeof(char),1,arch);
                    }

                    strcpy(disco.partition_table[a].name,"Vacia");
                    disco.partition_table[a].part_fit = '\0';
                    disco.partition_table[a].part_size = 0;
                    disco.partition_table[a].part_start = -1;
                    disco.partition_table[a].part_type = '\0';
                }
            }
    }
    OrderPartitions(&disco);
    fseek(arch,0L,SEEK_SET);
    fwrite(&disco,sizeof(MBR),1,arch);

}

void getInitLogic(FILE *arch, int _size, EBR nuevaEBR){
    int start = -1;
    int asigno = 0;
    int refe = 0;
    int _max = 0;
    MBR disco;
    fseek(arch,0,SEEK_SET);
    fread(&disco,sizeof(MBR),1,arch);

    for(int a = 0; a < 4; a++){
        if((disco.partition_table[a].part_type == 'e')&&(disco.partition_table[a].part_status == '1')){
            EBR auxEBR;
            fseek(arch,disco.partition_table[a].part_start,SEEK_SET);
            fread(&auxEBR,sizeof(EBR),1,arch);
            if(auxEBR.part_next == -1){
                refe = disco.partition_table[a].part_start + sizeof(EBR);
                _max = disco.partition_table[a].part_start + disco.partition_table[a].part_size;
                if((_max - refe) >= _size){
                    start = refe;
                    asigno = 1;
                    auxEBR.part_next = start;
                    fseek(arch,disco.partition_table[a].part_start,SEEK_SET);
                    fwrite(&auxEBR,sizeof(EBR),1,arch);
                    //EBR nuevaEBR;
                    nuevaEBR.part_next = -1;
                    nuevaEBR.part_start = start;
                    fseek(arch,start,SEEK_SET);
                    fwrite(&nuevaEBR,sizeof(MBR),1,arch);
                }
            }else{
                while(auxEBR.part_next != -1){
                    int startAnterior = auxEBR.part_start;
                    refe = auxEBR.part_start + auxEBR.part_size;
                    fseek(arch,auxEBR.part_next,SEEK_SET);
                    fread(&auxEBR,sizeof(EBR),1,arch);
                    _max = auxEBR.part_start-1;
                    if((_max - refe) >= _size){
                        start = refe;
                        asigno = 1;
                        fseek(arch,startAnterior,SEEK_SET);
                        fread(&auxEBR,sizeof(EBR),1,arch);
                        auxEBR.part_next = start;
                        fseek(arch,startAnterior,SEEK_SET);
                        fwrite(&auxEBR,sizeof(EBR),1,arch);
                        nuevaEBR.part_next = _max +1;
                        nuevaEBR.part_start = start;
                        fseek(arch,start,SEEK_SET);
                        fwrite(&nuevaEBR,sizeof(EBR),1,arch);
                        break;
                    }
                }
                if(asigno == 0){
                    refe = auxEBR.part_start + auxEBR.part_size;
                    _max = disco.partition_table[a].part_start + disco.partition_table[a].part_size;
                    if((_max - refe) >= _size){
                        start = refe;
                        auxEBR.part_next = start;
                        fseek(arch,auxEBR.part_start,SEEK_SET);
                        fwrite(&auxEBR,sizeof(EBR),1,arch);
                        nuevaEBR.part_start = start;
                        nuevaEBR.part_next = -1;
                        fseek(arch,start,SEEK_SET);
                        fwrite(&nuevaEBR,sizeof(EBR),1,arch);
                    }
                }
            }
        }
    }

    //return start;
}
