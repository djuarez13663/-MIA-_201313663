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

typedef struct MountPart{
    char name[100];
}MOUNTPART;

typedef struct Discos{
    char path[150];
    MOUNTPART MPART[20];
}DISCOS;


DISCOS mounted[26];

void CreateMBR(FILE *arch,int size);
void CreateParticion(FILE *arch,int _size,char fit, char* name,char tipo);
int getCountPartition(MBR disco);
int getPosNuevaPart(MBR disco);
int getInitPartition(MBR disco,int _size);
void OrderPartitions(MBR *disco);
int ExistExtended(MBR disco);
void Delete_Partition(FILE *disco,char* name,char* type_delete);
void getInitLogic(FILE *arch, int _size, EBR nuevaEBR);
void ModifySize(FILE *arch,char* name,int newSize);
char getTypePart(FILE *arch, char* name);
void MountDisk(char* path, char* name);
void ViewMounted();
void uMountDisk(char* id);

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

    _size--;

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
                _max = disco.tamano_mbr - 1;
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
            if((strcmp(disco.partition_table[a].name,name)==0)&&(disco.partition_table[a].part_status == '1')){
                disco.partition_table[a].part_status = '0';
                if(strcasecmp(type_delete,"full")==0){

                    for(int b = 0; b < disco.partition_table[a].part_size; b++){
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
                break;
            }
            if((disco.partition_table[a].part_type == 'e')&&(disco.partition_table[a].part_status == '1')){
                EBR auxEBR;
                fseek(arch,disco.partition_table[a].part_start,SEEK_SET);
                fread(&auxEBR,sizeof(EBR),1,arch);
                if(auxEBR.part_next != -1){
                    while(auxEBR.part_next != -1){
                        int anterior = auxEBR.part_start;
                        fseek(arch,auxEBR.part_next,SEEK_SET);
                        fread(&auxEBR,sizeof(EBR),1,arch);
                        int siguiente = auxEBR.part_next;
                        if((strcmp(auxEBR.part_name,name)==0)&&(auxEBR.part_status == '1')){
                            auxEBR.part_status = '0';
                            if(strcasecmp(type_delete,"full")==0){
                                for(int c = 0; c < auxEBR.part_size; c++){
                                    int pos = c + auxEBR.part_start;
                                    fseek(arch,pos,SEEK_SET);
                                    fwrite("\0",sizeof(char),1,arch);
                                }
                            }
                            fseek(arch,anterior,SEEK_SET);
                            fread(&auxEBR,sizeof(EBR),1,arch);
                            auxEBR.part_next = siguiente;
                            fseek(arch,anterior,SEEK_SET);
                            fwrite(&auxEBR,sizeof(EBR),1,arch);
                            break;
                        }
                    }
                }
            }
    }
    OrderPartitions(&disco);
    fseek(arch,0,SEEK_SET);
    fwrite(&disco,sizeof(MBR),1,arch);

}

void getInitLogic(FILE *arch, int _size, EBR nuevaEBR){
    int start = -1;
    int asigno = 0;
    int refe = 0;
    int _max = 0;
    _size--;
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
                _max = disco.partition_table[a].part_start + disco.partition_table[a].part_size -1;
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
                    _max = disco.partition_table[a].part_start + disco.partition_table[a].part_size - 1;
                    if((_max - refe) >= _size){
                        start = refe;
                        asigno = 1;
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

    if(asigno == 0){
        printf("ERROR: No Hay Espacio Suficiente Para Esa Particion.\n");
    }
    //return start;
}

void ModifySize(FILE *arch, char* name, int newSize){
    char type = getTypePart(arch, name);
    MBR disco;
    fseek(arch,0,SEEK_SET);
    fread(&disco,sizeof(MBR),1,arch);
    int inicio = 0;
    int fin = 0;
    newSize--;
    if(type == 'p'){
        for(int a = 0; a < 4; a++){
            if((strcmp(disco.partition_table[a].name,name)==0)&&(disco.partition_table[a].part_status == '1')){
                inicio = disco.partition_table[a].part_start + disco.partition_table[a].part_size;
                if(newSize > 0){
                    if((a == 3) || (disco.partition_table[a+1].part_status == '0')){
                        fin = disco.tamano_mbr - 1;
                    }else{
                        fin = disco.partition_table[a+1].part_start - 1;
                    }
                    if((fin - inicio) >= newSize){
                        disco.partition_table[a].part_size += (newSize + 1);
                        fseek(arch,0,SEEK_SET);
                        fwrite(&disco,sizeof(MBR),1,arch);
                        break;
                    }else{
                        printf("ERROR: No Hay Espacio Suficiente En El Disco Para Agregar Esa Cantidad A Esa Particion.\n");
                        break;
                    }
                }else if(newSize < 0){
                    // ACA CAMBIAR EL 100 PARA VALIDAR TAMANO MINIMO DE PARTICION
                    if((disco.partition_table[a].part_size + newSize + 1) >= 100){
                        disco.partition_table[a].part_size += (newSize + 1);
                        fseek(arch,0,SEEK_SET);
                        fwrite(&disco,sizeof(MBR),1,arch);
                        break;
                    }else{
                        printf("ERROR: Espacio Minimo De Particion Es De 2Mb\n");
                        break;
                    }
                }
            }
        }
    }else if(type == 'e'){
        for(int a = 0; a < 4; a++){
            if((strcmp(disco.partition_table[a].name,name)==0)&&(disco.partition_table[a].part_status == '1')){
                inicio = disco.partition_table[a].part_start + disco.partition_table[a].part_size;
                if(newSize > 0){
                    if((a == 3) || (disco.partition_table[a+1].part_status == '0')){
                        fin = disco.tamano_mbr - 1;
                    }else{
                        fin = disco.partition_table[a+1].part_start - 1;
                    }
                    if((fin - inicio) >= newSize){
                        disco.partition_table[a].part_size += (newSize + 1);
                        fseek(arch,0,SEEK_SET);
                        fwrite(&disco,sizeof(MBR),1,arch);
                        break;
                    }else{
                        printf("ERROR: No Hay Espacio Suficiente En El Disco Para Agregar Esa Cantidad A Esa Particion.\n");
                        break;
                    }
                }else if(newSize < 0){
                    //int lastLogic = 0;
                    // ACA CAMBIAR EL 100 PARA VALIDAR TAMANO MINIMO DE PARTICION
                    if((disco.partition_table[a].part_size + newSize + 1) >= 100){
                        EBR auxEBR;
                        fseek(arch,disco.partition_table[a].part_start,SEEK_SET);
                        fread(&auxEBR,sizeof(EBR),1,arch);
                        if(auxEBR.part_next != -1){
                            while(auxEBR.part_next != -1){
                                fin = auxEBR.part_start + auxEBR.part_size;
                                fseek(arch,auxEBR.part_next,SEEK_SET);
                                fread(&auxEBR,sizeof(EBR),1,arch);
                            }
                            if((inicio - fin) >= (newSize * -1)){
                                disco.partition_table[a].part_size += (newSize + 1);
                                fseek(arch,0,SEEK_SET);
                                fwrite(&disco,sizeof(MBR),1,arch);
                                break;
                            }else{
                                printf("ERROR: No Hay Espacio Suficiente En El Disco Para Reducir Esa Cantidad A Esa Particion.\n");
                            }
                        }else{
                            disco.partition_table[a].part_size += (newSize + 1);
                            fseek(arch,0,SEEK_SET);
                            fwrite(&disco,sizeof(MBR),1,arch);
                            break;
                        }
                    }else{
                        printf("ERROR: Espacio Minimo De Particion Es De 2Mb\n");
                        break;
                    }
                }
            }
        }
    }else if(type == 'l'){
        for(int a = 0; a < 4; a++){
            if((disco.partition_table[a].part_type == 'e')&&(disco.partition_table[a].part_status == '1')){
                EBR auxEBR;
                fseek(arch,disco.partition_table[a].part_start,SEEK_SET);
                fread(&auxEBR,sizeof(EBR),1,arch);
                while(auxEBR.part_next != -1){
                    fseek(arch,auxEBR.part_next,SEEK_SET);
                    fread(&auxEBR,sizeof(EBR),1,arch);
                    if(strcmp(auxEBR.part_name,name)==0){
                        inicio = auxEBR.part_start + auxEBR.part_size;
                        if(newSize > 0){
                            if(auxEBR.part_next == -1){
                                fin = disco.partition_table[a].part_start + disco.partition_table[a].part_size - 1;
                            }else{
                                fin = auxEBR.part_next - 1;
                            }
                            if((fin - inicio) >= newSize){
                                auxEBR.part_size += (newSize + 1);
                                fseek(arch,auxEBR.part_start,SEEK_SET);
                                fwrite(&auxEBR,sizeof(EBR),1,arch);
                                break;
                            }else{
                                printf("ERROR: No Hay Espacio Suficiente En El Disco Para Agregar Esa Cantidad A Esa Particion.\n");
                            }
                        }else if(newSize < 0){
                            // CAMBIAR EL 100 PARA VALIDAR TAMANO MINIMO DE PARTICIOIN
                            if((auxEBR.part_size + newSize + 1) >= 100){
                                auxEBR.part_size += (newSize + 1);
                                fseek(arch,auxEBR.part_start,SEEK_SET);
                                fwrite(&auxEBR,sizeof(EBR),1,arch);
                                break;
                            }else{
                                printf("ERROR: Espacio Minimo De Particion Es De 2Mb\n");
                            }
                        }
                    }

                }

            }
        }
    }
}

char getTypePart(FILE *arch, char* name){
    MBR auxPart;
    fseek(arch,0,SEEK_SET);
    fread(&auxPart,sizeof(MBR),1,arch);

    char exists = '0';

    for(int a = 0; a < 4; a++){
        if((strcmp(auxPart.partition_table[a].name,name)==0)){
            if(auxPart.partition_table[a].part_status == '1'){
                exists = auxPart.partition_table[a].part_type;
            }
        }
        if((auxPart.partition_table[a].part_type == 'e')&&(auxPart.partition_table[a].part_status == '1')){
            EBR auxEBR;
            fseek(arch,auxPart.partition_table[a].part_start,SEEK_SET);
            fread(&auxEBR,sizeof(EBR),1,arch);
            if(auxEBR.part_next != -1){
                fseek(arch,auxEBR.part_next,SEEK_SET);
                fread(&auxEBR,sizeof(EBR),1,arch);
                while(auxEBR.part_next != -1){
                    if((strcmp(auxEBR.part_name,name)==0)&&(auxEBR.part_status == '1')){
                        exists = 'l';
                        //break;
                    }
                    fseek(arch,auxEBR.part_next,SEEK_SET);
                    fread(&auxEBR,sizeof(EBR),1,arch);
                }
                if((strcmp(auxEBR.part_name,name)==0)&&(auxEBR.part_status == '1')){
                    exists = 'l';
                    //break;
                }
            }
        }
    }

    return exists;
}

void MountDisk(char* path,char* name){
    int disco = 0;
    int existeDisco = 0;
    int existePart = 0;
    // VER SI YA EXISTE ESE DISCO EN EL VECTOR DE PARTICIONES MONTADAS
    for(int a = 0; a < 26; a++){
        if(strcmp(path,mounted[a].path)==0){
            disco = a;
            existeDisco = 1;
            break;
        }
    }
    // SI NO EXISTE SE CREA
    if(existeDisco == 0){
        for(int b = 0; b < 26; b++){
            if(strcmp(mounted[b].path,"")==0){
                strcpy(mounted[b].path,path);
                disco = b;
                break;
            }
        }
    }
    // VEMOS SI LA PARTICION YA FUE MONTADA
    for(int c = 0; c < 20; c++){
        if(strcmp(mounted[disco].MPART[c].name,name)==0){
            existePart = 1;
            break;
        }
    }
    if(existePart == 0){
        for(int d = 0; d < 20; d++){
            if(strcmp(mounted[disco].MPART[d].name,"")==0){
                strcpy(mounted[disco].MPART[d].name,name);
                break;
            }
        }
    }else{
        printf("ERROR: La Particion Seleccionada Ya Esta Montada En El Sistema.\n");
    }
}

void ViewMounted(){
    printf("******************MOUNTED**********************\n");
    for(int a = 0; a < 26; a++){
        for(int b = 0; b < 20; b++){
            if(strcmp(mounted[a].MPART[b].name,"")!=0){
                char dev = (a + 97);
                printf("id::vd%c%d -path::\"%s\" -name::\"%s\"\n",dev,(b+1),mounted[a].path,mounted[a].MPART[b].name);
            }
        }
    }
    printf("**********************************************\n");
}

void uMountDisk(char* id){
    if((id[0] == 'v')&&(id[1] == 'd')){
        int disco = id[2] - 97;
        char aux[5];
        int b = 0;
        for(int a = 3; a < strlen(id); a++){
            aux[b] = id[a];
            b++;
        }
        //printf("%s\n",aux);
        int part = atoi(aux);
        part--;
        if(strcmp(mounted[disco].MPART[part].name,"")==0){
            printf("ERROR: La Particion \"%s\" No Ha Sido Montada.\n",id);
        }else{
            strcpy(mounted[disco].MPART[part].name,"");
        }
        for(int a = 0; a < 26; a++){
            int cont = 0;
            for(int b = 0; b < 20; b++){
                if(strcmp(mounted[a].MPART[b].name,"")!=0){
                    cont++;
                }
            }
            if(cont == 0){
                strcpy(mounted[a].path,"");
            }
        }
    }else{
        printf("ERROR: ID Invalido.\n");
    }
}





