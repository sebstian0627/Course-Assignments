//Shell By Shreyansh Choudhary for MTL458 Assignment 1.
//importing Header files.
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

//PATH_MAX from limits.h. To avoid memory issues.
#define MAX_PATH_SIZE (sizeof(char)*PATH_MAX)

//Function declarations.
void display_prompt(void);
void slice_str(const char * str, char * buffer, size_t start, size_t end);
char *shell_reader(void);
char **shell_parser(char *line,char *copy);
void shell_execute(char **tokens,char *copy);

//variable declarations for various commands.
// static int total_commands;
static char *commandkeeper[5];
static char currentDirectory[MAX_PATH_SIZE];
static char rootDirectory[MAX_PATH_SIZE];

//main function loop.
void shell_loop(){
  char* input;
  char** tokens;
  int status;
  status=1;
  //saving the rootDirectory for prompts.
  if (getcwd(rootDirectory,sizeof(rootDirectory)) == NULL)
      perror("getcwd() error");

  while(status){
    display_prompt();
    input=shell_reader();
    char* copy=strdup(input);
    char* copy2=strdup(input);

    tokens=shell_parser(input,copy2);
    shell_execute(tokens,copy);

  }
}
//this function executes all the commands.
void shell_execute(char **tokens,char *copy){
  if(tokens[0]==NULL){
    return;
  }
  else{
    //saves all the lines passed in the shell for history.
    char* temp=commandkeeper[0];
    commandkeeper[0]=strdup(copy);
    commandkeeper[4]=commandkeeper[3];
    commandkeeper[3]=commandkeeper[2];
    commandkeeper[2]=commandkeeper[1];
    commandkeeper[1]=temp;
    // total_commands++;
   if(strcmp(tokens[0],"cd")==0){
    if(tokens[1]==NULL){
      chdir(rootDirectory);

    }
    else if(strcmp(tokens[1],"~")==0){
      chdir(rootDirectory);
    }
    else{
      if(chdir(tokens[1])<0){
        printf("Could not change directory: No Such File or directory Found.\n" );
      }
    }

  }else if(strcmp(tokens[0],"history")==0){
    for(int i=4;i>=0;i--){
      if(commandkeeper[i]!=NULL){
        printf("%s\n",commandkeeper[i] );
      }
    }
  }else{
    //child process is created and it executes the following commands with the help of execvp while the parent waits for child to finish.
    pid_t pid=fork();
    int status;
    if(pid<0){
      printf("Fork Failed. Please try again.\n" );
    }else if(pid==0){
      if(execvp(tokens[0],tokens)==-1){
        perror("command is not valid");
      }
    }else{
      while(wait(&status)!=pid);
    }
  }
}
}

void display_prompt(void){
  if (getcwd(currentDirectory,sizeof(currentDirectory)) == NULL)
      perror("getcwd() error");
  int issubs=1;
  if(strlen(currentDirectory)>=strlen(rootDirectory)){
  //checks whether the pwd has rootDirectory in its beginning or not.
  for( int i=0;i<strlen(rootDirectory);i++){
    if(rootDirectory[i]!=currentDirectory[i]){
      issubs=0;
    }
  }
  if(issubs){
    char buffer[strlen(currentDirectory) + 1];
    slice_str(currentDirectory,buffer,strlen(rootDirectory),strlen(currentDirectory)-1);
    printf("MTL458:~%s$", buffer);
  }
  else{
    printf("MTL458:%s$ ",currentDirectory );
  }
}else{
  printf("MTL458:%s$ ",currentDirectory );
}

}


//slices the string from starting indec to end index both inclusive.
void slice_str(const char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

//reads the input in the shell by the user.
char *shell_reader(void)
{
  char *line = malloc(sizeof(char)*MAX_PATH_SIZE);
  int character=0;
  int iter=0;
  int maxsize=MAX_PATH_SIZE;
  do{

    character=getchar();
    if(character!='\n' ){
      line[iter++]=character;

    }
    //reallocating memory when required.
    if (maxsize==iter){
      maxsize+=20;
      line= realloc(line,maxsize);
    }
  }while(character!='\n');
  line[iter]='\0';
  return line;
}


char **shell_parser(char *line,char *copy2){
  char **ans=malloc(sizeof(char*)*MAX_PATH_SIZE);
  char* token;

  token=strtok(line," \"\n");
  if(token==NULL){
    ans[0]=NULL;
    return ans;
  }
  if(strcmp(token,"cd")==0){
    // processing the string for cd, removing "" .
    char* token2;
    char* bufferrrr;
    bufferrrr=malloc(sizeof(char*)*MAX_PATH_SIZE);
    int character;
    int iter1=0;
    int jj=0;
    do{
      character=copy2[jj++];
      if(character!='\"'){
        bufferrrr[iter1++]=character;
      }
    }while(iter1<strlen(copy2));
    token2=strtok(bufferrrr," \n");
    ans[0]=token2;
    int iter=1;
    int maxsize=MAX_PATH_SIZE;
    while(token2!=NULL){
      token2=strtok(NULL,"\n");
      ans[iter++]=token2;
      if(iter==maxsize){
        maxsize+=20;
        ans=realloc(ans, maxsize);
      }

    }

    return ans;
  }else{
    char *token2;

    // parsing the inputs for execvp execution handling spaces and double quotes.
    int isInComma=0;
    for(int k=0;k<strlen(copy2);k++){
      if(copy2[k]=='\"'){
        isInComma=1;
        copy2[k]='\n';
      }
      if(copy2[k]=='\"' && !isInComma){
        isInComma=0;
      }
      if(!isInComma){
        if(copy2[k]==' '){
          copy2[k]='\n';
        }
      }
    }
    token2=strtok(copy2,"\n");
    ans[0]=token2;
    int iter=1;
    int maxsize=MAX_PATH_SIZE;
    while(token2!=NULL){
      // printf("%s Hi Hi\n",token2);
      token2=strtok(NULL,"\n");
      // if(strcmp(token2," ")==0){
      ans[iter++]=token2;
      if(iter==maxsize){
        maxsize+=20;
        ans=realloc(ans, maxsize);
      }

    }
    return ans;
  }

}



int main(int argc, char **argv){

  shell_loop();
}
