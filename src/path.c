#include "path.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

AppPaths globalPaths = {0};

#ifdef _WIN32
void initPaths(){
  if(!GetModuleFileNameA(NULL,globalPaths.root,APP_PATH_MAX-1)){
    fprintf(stderr,"failed to get global paths\n");
    exit(EXIT_FAILURE);
  }
  strcpy(globalPaths.shaders,globalPaths.root);
  int root_len = strlen(globalPaths.root);
  int sep_count = 0;
  int shader_index = 0;
  for(int i=root_len-1;i>=0;i--){
    if(globalPaths.root[i]==SEP_CHAR){
      sep_count++;
    }
    if(sep_count == 1){
      shader_index = i+1;
      break;
    }
  }
  strcpy(&(globalPaths.shaders[shader_index]),"shaders");
	
}
#else

void initPaths(){
	ssize_t k = readlink("/proc/self/exe",globalPaths.root,APP_PATH_MAX);
    if (k <= 0){
		fprintf(stderr,"failed to get global paths\n");
		exit(EXIT_FAILURE);
	}
    globalPaths.root[k] = 0;            // readlink doesn't add the terminating zero
    strcpy(globalPaths.shaders,globalPaths.root);
	int root_len = strlen(globalPaths.root);
	int sep_count = 0;
	int shader_index = 0;
	for(int i=root_len-1;i>=0;i--){
		if(globalPaths.root[i]==SEP_CHAR){
			sep_count++;
		}
		if(sep_count == 1){
			shader_index = i+1;
			break;
		}
	}	
	strcpy(&(globalPaths.shaders[shader_index]),"shaders");
}

#endif



char* pathJoin(PATH_TYPE type,char* name,int name_size,int* result_size){
  char* result;
  int len;
  switch(type){
  case SHADERS:
    {
      len = strlen(globalPaths.shaders);
      result = malloc(sizeof(char)*(len+name_size+2));//NOTE: 2 is for the null terminator and the separator
      strcpy(result,globalPaths.shaders);
    }break;
  }
  *result_size = len+name_size+2;
  result[len] = SEP_CHAR;
  strcpy(&(result[len+1]),name);
  result[*result_size -1] = '\0';
  return result;
}
