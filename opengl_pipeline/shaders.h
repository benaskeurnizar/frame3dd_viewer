#pragma once


void LinkVShader(unsigned int*VertexShader,const char* VertexCodeAdr);

void LinkFrShader(unsigned int*FragmentShader,const char* FragmentCodeAdr);

void LinkPShader(unsigned int* VertexShader,unsigned int* FragmentShader,unsigned int* ProgramShader);
