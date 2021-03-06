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
int IsMounted(char* id);
void MBRReport(char* path, char* id);
void DiskReport(char* path, char* id);

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
                    if((disco.partition_table[a].part_size + newSize + 1) >= (2*1024*1024)){
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
                    if((disco.partition_table[a].part_size + newSize + 1) >= (2*1024*1024)){
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
                            if((auxEBR.part_size + newSize + 1) >= (2*1024*1024)){
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

int IsMounted(char* id){
    int ismounted = 0;

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
            ismounted = 0;
        }else{
            ismounted = 1;
        }
    }else{
        printf("ERROR: ID Invalido.\n");
        ismounted = 0;
    }

    return ismounted;
}

void MBRReport(char* path, char* id){
    FILE *rep = fopen("Reporte.dot","w");

    fprintf(rep,"digraph g{\n");
    fprintf(rep,"node[shape = box];\n");
    //*********** OBTENEMOS DIRECCION DE DISCO ******
    char auxPath[200];
    int disco = id[2] - 97;
    strcpy(auxPath,mounted[disco].path);
    //***********************************************
    //*********** OBTENEMOS MBR *********************
    FILE *arch = fopen(mounted[disco].path,"rb+");
    MBR auxMBR;
    fseek(arch,0,SEEK_SET);
    fread(&auxMBR,sizeof(MBR),1,arch);
    //***********************************************
    //*********** OBTENEMOS NOMBRE DISCO ************

    char nameDisk[50];
    char* split = strtok(auxPath,"/");
    while(split != NULL){
        strcpy(nameDisk,split);
        split = strtok(NULL,"/");
    }
    strcpy(auxPath,mounted[disco].path);
    //***********************************************

    fprintf(rep,"label = \"Reporte MBR %s\";\n",nameDisk);
    fprintf(rep,"labelloc = \"t\";\n");

    // *********** TABLA DE MBR ********************
    fprintf(rep,"\"Record\" [label = <<table border = \"1\" cellspacing = \"0\">\n");
    fprintf(rep,"<tr><td colspan = \"2\"><b>MBR %s</b></td></tr>\n",nameDisk);
    fprintf(rep,"<tr><td><b>Nombre</b></td><td><b>Valor</b></td></tr>\n");
    fprintf(rep,"<tr><td><b>mbr_size</b></td><td>%d</td></tr>\n",auxMBR.tamano_mbr);
    fprintf(rep,"<tr><td><b>mbr_fecha_creacion</b></td><td>%s</td></tr>\n",auxMBR.fecha_creacion_mbr);
    fprintf(rep,"<tr><td><b>mbr_disk_signature</b></td><td>%d</td></tr>\n",auxMBR.disk_signature_mbr);
    //
    for(int a = 0; a < 4; a++){
        if(auxMBR.partition_table[a].part_status == '1'){
            fprintf(rep,"<tr><td><b>part_status_%d</b></td><td>%c</td></tr>\n",(a+1),auxMBR.partition_table[a].part_status);
            fprintf(rep,"<tr><td><b>part_type_%d</b></td><td><b>%c</b></td></tr>\n",(a+1),(toupper(auxMBR.partition_table[a].part_type)));
            fprintf(rep,"<tr><td><b>part_fit_%d</b></td><td><b>%c</b></td></tr>\n",(a+1),(toupper(auxMBR.partition_table[a].part_fit)));
            fprintf(rep,"<tr><td><b>part_start_%d</b></td><td>%d</td></tr>\n",(a+1),auxMBR.partition_table[a].part_start);
            fprintf(rep,"<tr><td><b>part_size_%d</b></td><td>%d</td></tr>\n",(a+1),auxMBR.partition_table[a].part_size);
            fprintf(rep,"<tr><td><b>part_name_%d</b></td><td>%s</td></tr>\n",(a+1),auxMBR.partition_table[a].name);
        }
    }

    fprintf(rep,"</table>>];\n");
    // *********************************************

    // ************ TABLAS DE EBR ******************
    int cont = 1;
    EBR auxEBR;
    for(int b = 0; b < 4; b++){
        if(auxMBR.partition_table[b].part_type == 'e' && auxMBR.partition_table[b].part_status == '1'){
            fseek(arch,auxMBR.partition_table[b].part_start,SEEK_SET);
            fread(&auxEBR,sizeof(EBR),1,arch);
            while(auxEBR.part_next != -1){
                fseek(arch,auxEBR.part_next,SEEK_SET);
                fread(&auxEBR,sizeof(EBR),1,arch);
                fprintf(rep,"\"Record%d\" [label = <<table border = \"1\" cellspacing = \"0\">\n",cont);

                fprintf(rep,"<tr><td colspan = \"2\"><b>EBR_%d</b></td></tr>\n",cont);
                fprintf(rep,"<tr><td><b>Nombre</b></td><td><b>Valor</b></td></tr>\n");
                fprintf(rep,"<tr><td><b>part_status_%d</b></td><td>%c</td></tr>\n",cont,auxEBR.part_status);
                fprintf(rep,"<tr><td><b>part_fit_%d</b></td><td><b>%c</b></td></tr>\n",cont,toupper(auxEBR.part_fit));
                fprintf(rep,"<tr><td><b>part_start_%d</b></td><td>%d</td></tr>\n",cont,auxEBR.part_start);
                fprintf(rep,"<tr><td><b>part_size_%d</b></td><td>%d</td></tr>\n",cont,auxEBR.part_size);
                fprintf(rep,"<tr><td><b>part_next_%d</b></td><td>%d</td></tr>\n",cont,auxEBR.part_next);
                fprintf(rep,"<tr><td><b>part_name_%d</b></td><td>%s</td></tr>\n",cont,auxEBR.part_name);

                fprintf(rep,"</table>>];\n");
                cont++;
            }
        }
    }
    // *********************************************

    fprintf(rep,"}");
    fflush(rep);
    fclose(rep);
    fclose(arch);
    // ************* GENERAR LA GRAFICA Y ABRIRLA ********************
    char comando[500];
    strcpy(comando,"");
    strcat(comando,"dot -Tpng Reporte.dot -o \"");
    strcat(comando, path);
    strcat(comando,"\"");
    system(comando);
    strcpy(comando,"");
    strcat(comando,"viewnior \"");
    strcat(comando,path);
    strcat(comando,"\"&");
    system(comando);
}

void DiskReport(char* path, char* id){
    //*********** OBTENEMOS DIRECCION DE DISCO ******
    char auxPath[200];
    int disco = id[2] - 97;
    strcpy(auxPath,mounted[disco].path);
    //printf("%s\n",auxPath);
    //***********************************************
    // *********** OBTENEMOS MBR ****************
    FILE *arch = fopen(mounted[disco].path,"rb+");
    MBR auxMBR;
    fseek(arch,0,SEEK_SET);
    fread(&auxMBR,sizeof(MBR),1,arch);
    // ******************************************
    //*********** OBTENEMOS NOMBRE DISCO ************

    char nameDisk[50];
    char* split = strtok(auxPath,"/");
    while(split != NULL){
        strcpy(nameDisk,split);
        split = strtok(NULL,"/");
    }
    strcpy(auxPath,mounted[disco].path);
    //***********************************************
    // *********** ARCHIVO DE REPORTE ***************
    FILE *rep = fopen("Reporte.dot","w");

    fprintf(rep,"digraph g{\n");
    fprintf(rep,"node[shape = record];\n");
    fprintf(rep,"label = \"Reporte DISK %s\";\n",nameDisk);
    fprintf(rep,"labelloc = \"t\";\n");
    // ********** MBR ************
    fprintf(rep,"struct [shape = record, label = \"MBR %s &#92;n mbr_size: %d &#92;n mbr_disk_signature: %d &#92;n mbr_fecha_creacion: %s",nameDisk,auxMBR.tamano_mbr,auxMBR.disk_signature_mbr,auxMBR.fecha_creacion_mbr);
    // ********* PARTICIONES *****
    for(int a = 0; a < 4; a++){
        if(auxMBR.partition_table[a].part_status == '1'){
            // VERIFICAR ESPACIO VACIO AL INICIO
            if(a == 0){
                if((sizeof(MBR)+1) != auxMBR.partition_table[a].part_start){
                    fprintf(rep,"|Espacio Libre &#92;n %d bytes" ,(auxMBR.partition_table[a].part_start - sizeof(MBR) -1));
                }
            }else{
                if((auxMBR.partition_table[a-1].part_start + auxMBR.partition_table[a-1].part_size) != auxMBR.partition_table[a].part_start){
                    fprintf(rep,"|Espacio Libre &#92;n %d bytes",(auxMBR.partition_table[a].part_start - (auxMBR.partition_table[a-1].part_start + auxMBR.partition_table[a-1].part_size)));
                }
            }

            // EXTENDIDA CON LOGICA
            if(auxMBR.partition_table[a].part_type == 'e'){
                int inicio = 0;
                fprintf(rep,"|{part_name: %s &#92;n part_start: %d &#92;n part_size: %d &#92;n part_type: %c",auxMBR.partition_table[a].name,auxMBR.partition_table[a].part_start,auxMBR.partition_table[a].part_size,toupper(auxMBR.partition_table[a].part_type));
                //LOGICAAAAAAAAAAAS
                EBR auxEBR;
                fseek(arch,auxMBR.partition_table[a].part_start,SEEK_SET);
                fread(&auxEBR,sizeof(EBR),1,arch);
                fprintf(rep,"|{");
                if(auxEBR.part_next == -1){
                    fprintf(rep,"Espacio Libre: &#92;n %d bytes",(auxMBR.partition_table[a].part_size - sizeof(EBR)));
                }else{
                    if(auxEBR.part_next != (auxMBR.partition_table[a].part_start + sizeof(EBR))){
                        fprintf(rep,"Espacio Libre: &#92;n %d bytes",(auxEBR.part_next - (auxMBR.partition_table[a].part_start + sizeof(EBR))));
                        inicio = 1;
                    }
                    while(auxEBR.part_next != -1){
                        fseek(arch,auxEBR.part_next,SEEK_SET);
                        fread(&auxEBR,sizeof(EBR),1,arch);
                        if(inicio == 1){
                            fprintf(rep,"|");
                        }else{
                            inicio = 1;
                        }
                        fprintf(rep,"part_name: %s &#92;n part_start: %d &#92;n part_size: %d &#92;n part_next: %d",auxEBR.part_name,auxEBR.part_start,auxEBR.part_size,auxEBR.part_next);
                        if(auxEBR.part_next != -1){
                            if(auxEBR.part_next != (auxEBR.part_start + auxEBR.part_size)){
                                fprintf(rep,"|Espacio Libre: &#92;n %d bytes",(auxEBR.part_next - (auxEBR.part_start + auxEBR.part_size)));
                            }
                        }else{
                            if((auxEBR.part_start + auxEBR.part_size) != (auxMBR.partition_table[a].part_start + auxMBR.partition_table[a].part_size)){
                                fprintf(rep,"|Espacio Libre: &#92;n %d bytes",((auxMBR.partition_table[a].part_start + auxMBR.partition_table[a].part_size) - (auxEBR.part_start + auxEBR.part_size)));
                            }
                        }
                    }
                }

                fprintf(rep,"}}");
            }else{
            // PRIMARIA
                fprintf(rep,"|part_name: %s &#92;n part_start: %d &#92;n part_size: %d &#92;n part_type: %c",auxMBR.partition_table[a].name,auxMBR.partition_table[a].part_start,auxMBR.partition_table[a].part_size,toupper(auxMBR.partition_table[a].part_type));
            }
            if((a == 3)||(auxMBR.partition_table[a+1].part_status=='0')){
                if((auxMBR.partition_table[a].part_start + auxMBR.partition_table[a].part_size) != auxMBR.tamano_mbr){
                    fprintf(rep,"|Espacio Libre &#92;n %d bytes",(auxMBR.tamano_mbr - (auxMBR.partition_table[a].part_start + auxMBR.partition_table[a].part_size)));
                    break;
                }
            }
        }
    }
    // ***************************
    fprintf(rep,"\"];");
    fprintf(rep,"}");
    fflush(rep);

    fclose(rep);
    fclose(arch);
    // ************* GENERAR LA GRAFICA Y ABRIRLA ********************
    char comando[500];
    strcpy(comando,"");
    strcat(comando,"dot -Tpng Reporte.dot -o \"");
    strcat(comando, path);
    strcat(comando,"\"");
    system(comando);
    strcpy(comando,"");
    strcat(comando,"viewnior \"");
    strcat(comando,path);
    strcat(comando,"\"&");
    system(comando);
}
