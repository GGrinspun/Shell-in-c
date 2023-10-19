#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#define MAX_LINE 80 /* 80 chars per line, per command */
#define TAM_ARRAY 20


void print_seq(void){
    
    printf("gsg seq> ");
    return;
}
void print_par(void){
    
    wait(0);
    printf("gsg par> ");
    return;
}

void terminal(int flag){
    if(flag == 0){
        print_seq();
    }
    else if(flag == 1){
        print_par();
    }
}

void tirarPrimeiroEspaco(char array[], int tamanho){     //tirar o primeiro espaço apos o ";":
    char aux_texto[tamanho];

    if(array[0] == ' '){
        for(int c1 = 0; c1<=tamanho; c1++){      
            aux_texto[c1] = array[c1+1];
        }
        strcpy(array, aux_texto);
    }
}


int main(int argc, char **argv){
    int flag_modo = 0; 
    int saida = 0;
    int g = 0;
    char Linha[MAX_LINE];
    char arg1[10];
    
    char* ptr_linha;
    char* ptr_multiplos;
    char comando_copia_linha[MAX_LINE];
    char* linha_dividida;
    char fracao_linha[MAX_LINE];
    char comando0[MAX_LINE];
    char comando1[MAX_LINE];
    char aux_texto[MAX_LINE];
    char history[MAX_LINE];

    //comando0
    char* ptr_c0;
    char* comando0_dividido;
    char *tkn_comando0;
    char *array_exec_c0[MAX_LINE/2] = {NULL};

    //comando1
    char* ptr_c1;
    char* comando1_dividido;
    char *tkn_comando1;
    char *array_exec_c1[MAX_LINE/2] = {NULL};

    //flags de comandos
    int counter = 0;
    int comandos_basicos = 0;
    int comando_duplo = 0;
    int id_pipe = 0;
    int redirect_e_d = 0; //redirect esquerda para direita
    int redirect_d_e = 0; //redirect direita para esquerda
    int background = 0;
    int pos_divisao = 0;
    int pos_background = 0;

    //variaveis para redirecionamento
    char nome_arq_redirect[30];
    int redirect_simples = 0;
    int redirec_concatenado = 0;

    //outras flags
    int count_tkns = 0;




    FILE * file;

    if(argc == 2){
        flag_modo = 0;
        
        strcpy(arg1, argv[1]);
        file = fopen(arg1, "r");

        if(file == NULL || argc > 2){                                   //caso de erro na abertura
            printf("Não foi possível abrir o arquivo ou parametros estao incorretos\n");
            return 1;
        }
    }

    while(saida == 0 || fgets(Linha, MAX_LINE, file) != NULL){

        if(argc<2){                        //scanf
            terminal(flag_modo);
            scanf("%[^\n]%*c", Linha);
            
        }
        else if(argc==2){                //fgets do arquivo
                
            fgets(Linha, MAX_LINE, file);
            terminal(flag_modo);
            printf("%s|\n",Linha);
        }

        for(int i = 0; i<strlen(Linha); i++){             //tomar cuidado com o strtok e o null terminated
            if(Linha[i] == '\n'){
                Linha[i] = '\0';
            }
        }
        
        fflush(stdout);
        
        comandos_basicos = 0;
        comando_duplo = 0;
        id_pipe = 0;
        redirect_e_d = 0;
        redirect_d_e = 0;
        background = 0;
        redirect_simples = 0;
        redirec_concatenado = 0;
        pos_divisao = 0;
        pos_background = 0;

        if (strcmp(Linha, "\n") == 0 ||strcmp(Linha, "\0") == 0 ){
            comandos_basicos = 1;
        }


        if(strcmp(Linha, "!!") == 0){ //tratamento do history
            
            if(counter == 0 || strcmp(history, "!!") == 0 || strcmp(history, "\n") == 0){
                printf("No commands\n");
                comandos_basicos = 1;

            }
            if(counter > 0){
                strcpy(Linha, history);

            }

        }else if(strcmp(Linha, "!!")!= 0 && strcmp(history, "!!") != 0 ){
            strcpy(history, Linha);
        }

       

        for (int i=0; i<MAX_LINE/2; i++){      //ajustar arrays para full NULL
            array_exec_c0[i] = NULL;
            array_exec_c1[i] = NULL;
        }
        
        /*for(int i = 0; i<strlen(Linha); i++){             //tomar cuidado com o strtok e o null terminated
            if(Linha[i] == '\n'){
                Linha[i] = '\0';
            }
        }*/
        

        if(strcmp(Linha, "exit" ) == 0){
            printf("encerrando o programa\n");
            comandos_basicos = 1;
            saida = 1;
            break;
        }
        else if(strcmp(Linha, "style parallel" ) == 0){
            flag_modo = 1;
            comandos_basicos = 1;
            
            printf("\n");
        
        }
        else if(strcmp(Linha, "style sequential") == 0){
            flag_modo = 0;
            comandos_basicos = 1;
        
            printf("\n");
        
        }
        else if(strcmp(Linha, "")== 0||strcmp(Linha, " ") == 0 || strcmp(Linha, "\n") == 0 || strcmp(Linha, "\0") == 0){
            strcpy(Linha, " ");
            printf("linha vazia\n");
            comandos_basicos = 1;
        }
        else{ //outro comando, verificar pipes redirect e background
            char copia_linha[MAX_LINE];
            strcpy(copia_linha, Linha);
           
            

            for(int L = 0; L<strlen(Linha); L++){
                if(Linha[L] == ';'){        // comando duplo, comando_duplo = 1                    
                    comando_duplo = 1;
                }

            }
                

            char *comandos_tokens = strtok_r(copia_linha, ";", &ptr_multiplos);
            while(comandos_tokens != NULL){
                strcpy(comando_copia_linha, comandos_tokens);
                comandos_basicos = 0;
                comando_duplo = 0;
                id_pipe = 0;
                redirect_e_d = 0;
                redirect_d_e = 0;
                background = 0;
                redirect_simples = 0;
                redirec_concatenado = 0;
                pos_divisao = 0;

                for(int i = 0; i<strlen(comando_copia_linha); i++){             //tomar cuidado com o strtok e o null terminated
                    if(comando_copia_linha[i] == '|'){                 // pipe detectado id_pipe =
                        id_pipe = 1;
                            
                    }
                    else if(comando_copia_linha[i] == '>'){
                        redirect_e_d = 1;
                        
                        if(comando_copia_linha[i+1] != '>'){
                            redirect_simples = 1;
                        }
                        else if(comando_copia_linha[i+1] == '>'){
                            redirec_concatenado = 1;
                        }
                    }
                    else if(comando_copia_linha[i] == '<'){
                        redirect_d_e = 1;
                    }
                }
                if(id_pipe == 1){                                                            // execucao estilo pipe

                    //dividir comando_copia_linha | para comandos
                    linha_dividida = strtok_r(comando_copia_linha, "|" , &ptr_linha);
                    count_tkns = 0;
                    
                    while(linha_dividida !=NULL){             //dividir por ;
                        strcpy(fracao_linha, linha_dividida);
                        if(count_tkns == 0){
                            strcpy(comando0, fracao_linha);
                        }
                        else if(count_tkns == 1){
                            strcpy(comando1, fracao_linha); 

                        }
                        count_tkns++;
                        linha_dividida = strtok_r(NULL, "|", &ptr_linha);
                    }

                    g = 0;
                    
                    comando0_dividido = strtok_r(comando0, " ", &ptr_c0);
                    while(comando0_dividido != NULL){
                    
                        array_exec_c0[g] = comando0_dividido;
                        g++;

                        comando0_dividido = strtok_r(NULL, " ", &ptr_c0);
                    }
                    array_exec_c0[g+1]= NULL;

                    g = 0;
                    
                    //dividir t2 " " para arrayc1
                    printf("dividindo comando:%s \n", comando1);
                    comando1_dividido = strtok_r(comando1, " ", &ptr_c1);
                    while(comando1_dividido != NULL){

                        array_exec_c1[g] = comando1_dividido;
                        printf("array:%s| ", array_exec_c1[g]);
                        g++;

                        comando1_dividido = strtok_r(NULL, " ", &ptr_c1);
                    }
                    array_exec_c1[g+1]= NULL;

                    //criar pipe
                    int fd[2]; 

                    if (pipe(fd) == -1){
                        printf("erro na criacao do pipe\n");
                        return 1;
                    }

                    pid_t pidP = fork();
                    if(pidP == -1){
                        printf("erro na invocacao do fork() pipe writer\n");
                        return 1;

                    }else if(pidP == 0){
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                        close(fd[1]);
                        execvp(array_exec_c0[0], array_exec_c0);
                    }

                    pid_t pidR = fork();
                    if (pidR == -1){
                        printf("erro no fork() pidR\n");
                        return 1;
                    }
                    else if(pidR == 0){
                        dup2(fd[0], STDIN_FILENO);
                        close(fd[0]);
                        close(fd[1]);
                        execvp(array_exec_c1[0], array_exec_c1);
                    }

                    //fechar no processo pai
                    close(fd[0]);
                    close(fd[1]);

                    waitpid(pidP, NULL, 0);
                    waitpid(pidR, NULL, 0);
                    printf("\n");
                }
                else if(redirect_e_d == 1){

                    if(redirec_concatenado == 1 && redirect_simples == 1){    //´tratar priblema do reconhecimento pois o programa identifica o segundo> como redirect simples
                        redirect_simples = 0;
                    }

                    linha_dividida = strtok_r(comando_copia_linha, ">" , &ptr_linha);
                    count_tkns = 0;
                    
                    while(linha_dividida !=NULL){             //dividir por >
                        strcpy(fracao_linha, linha_dividida);
                        if(count_tkns == 0){
                            strcpy(comando0, fracao_linha);
                        }
                        else if(count_tkns == 1){
                            strcpy(nome_arq_redirect, fracao_linha); 

                        }
                        count_tkns++;
                        linha_dividida = strtok_r(NULL, ">", &ptr_linha);
                    }

                    
                    tirarPrimeiroEspaco(nome_arq_redirect, strlen(nome_arq_redirect));

                    //dividir o comando0 para o execvp
                    g = 0;
                    comando0_dividido = strtok_r(comando0, " ", &ptr_c0);
                    while(comando0_dividido != NULL){
                    
                        array_exec_c0[g] = comando0_dividido;
                        g++;

                        comando0_dividido = strtok_r(NULL, " ", &ptr_c0);
                    }
                    array_exec_c0[g+1]= NULL;

                    //tentar abrir o arquivo no proc filho para não atrapalhar a tabua de arquivos do paie entao redirecionar output

                    int fileR;
                    if(redirect_simples == 1){
                        fileR = open(nome_arq_redirect, O_WRONLY | O_TRUNC | O_CREAT, 00700);       // trunc para >; o super redirect não tem isso
                    }
                    else if(redirec_concatenado == 1){
                        fileR = open(nome_arq_redirect, O_WRONLY | O_APPEND | O_CREAT, 00700);
                        
                    }

                    pid_t pidRED = fork();
                    if(pidRED == -1){
                        printf("erro na criacao do filho do redirecionamento\n");

                    }else if(pidRED == 0){

                        if(fileR == -1){
                        printf("erro na criacao do arquivo\n ");
                        return 1;
                        }
                        dup2(fileR, STDOUT_FILENO);                         //ver essa linha como fazer pra voltar pro stdout padrao 1
                        
                        execvp(array_exec_c0[0], array_exec_c0);
                        close(fileR);
                    }

                    waitpid(pidRED, NULL, 0);
                    close(fileR);

                }else if(redirect_d_e == 1){

                    linha_dividida = strtok_r(comando_copia_linha, "<" , &ptr_linha);
                    int count_tkns = 0;
                    
                    while(linha_dividida !=NULL){             //dividir por <
                        strcpy(fracao_linha, linha_dividida);
                        if(count_tkns == 0){
                            strcpy(comando0, fracao_linha);
                        }
                        else if(count_tkns == 1){
                            strcpy(nome_arq_redirect, fracao_linha); 
                        }
                        count_tkns++;
                        linha_dividida = strtok_r(NULL, "<", &ptr_linha);
                    }


                    tirarPrimeiroEspaco(comando0, strlen(comando0));
                    tirarPrimeiroEspaco(nome_arq_redirect, strlen(nome_arq_redirect));

                    g = 0;
                    comando0_dividido = strtok_r(comando0, " ", &ptr_c0);
                    while(comando0_dividido != NULL){
                    
                        array_exec_c0[g] = comando0_dividido;
                        g++;

                        comando0_dividido = strtok_r(NULL, " ", &ptr_c0);
                    }
                    array_exec_c0[g+1]= NULL;

                    int fileK;

                    fileK = open(nome_arq_redirect, O_RDONLY, 00700);

                    pid_t pidK = fork();

                    if(pidK == -1){
                        printf("problema no pidK do redirect\n");
                    }
                    else if(pidK == 0){
                        if(fileK == -1){
                            printf("erro na criacao do arquivo\n ");
                            return 1;
                        }
                        dup2(fileK, STDIN_FILENO);                         
                        
                        execvp(array_exec_c0[0], array_exec_c0);
                        close(fileK);

                    }
                    waitpid(pidK, NULL, 0);

                    close(fileK);


                }
                else{
                    g = 0;
                    strcpy(comando0, comando_copia_linha);
                    
                    comando0_dividido = strtok_r(comando0, " ", &ptr_c0);
                    while(comando0_dividido != NULL){
                    
                        array_exec_c0[g] = comando0_dividido;
                        g++;

                        comando0_dividido = strtok_r(NULL, " ", &ptr_c0);
                    }
                    array_exec_c0[g+1]= NULL;

                    if(flag_modo == 0){  // style sequential
                                                                
                        int id = fork();
                        if(id == -1){
                            printf("problema na func fork() \n");

                        }
                        if(id == 0){
                            execvp(array_exec_c0[0], array_exec_c0);
                            
                        }
                        waitpid(id, NULL, 0);//esperar processo filho terminar
                            
                    }else if(flag_modo == 1){//exec par de 1 comando
                                                
                        int id = fork();
                        if(id == -1){
                            printf("problema na func fork() \n");

                        }
                        if(id == 0){
                            execvp(array_exec_c0[0], array_exec_c0);
                            
                        }
                        //nao esperar processo filho terminar
                                 
                    }

                }



                comandos_tokens = strtok_r(NULL, ";", &ptr_multiplos);
            }


 
        }
        counter++;
    }
    if(argc == 2){
    fclose(file);
    }
    
    return 0;
}