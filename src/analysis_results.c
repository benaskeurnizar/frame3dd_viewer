#include "analysis_results.h"


#include <string.h>
#include <stdio.h>
#include <math.h>

int loadResults(structureModel* model,Results* results,const char* address){
	FILE* f = fopen(address,"r");
	if(!f){
		fprintf(stderr,"ERROR in loadResults : failed to open results file %s \n",address);
		return 0;
	}
	results->nCases = model->caseCount;
	results->nNodes = model->nodeCount;
	results->gdisp = (double(*)[6])calloc(model->nodeCount*model->caseCount,sizeof(*results->gdisp));
	if(!results->gdisp){ fclose(f); return 0; }
	int current_case = 0;
	char buf[256];
	double maxDisp = 1e-9;
	while(current_case<results->nCases && fgets(buf, sizeof(buf), f)){
		if(!strstr(buf,"N O D E   D I S P L A C E M E N T S"))continue;
		fgets(buf,sizeof(buf),f);//skip the comment line
		while(fgets(buf,sizeof(buf),f)){
			int n;
			double v[6];
			if(sscanf(buf,"%d %lf %lf %lf %lf %lf %lf",&n,&v[0],&v[1],&v[2],&v[3],&v[4],&v[5]) != 7 ){
				break;
			}
			if(n < 1 || n > results->nNodes){
				fprintf(stderr,"ERROR in loadResults : bad node row in case %d , node : %d, max nodes : %d \n", current_case+1,n,results->nNodes);
				free(results->gdisp); results->gdisp = NULL;
				fclose(f);
				return 0;
			}
			n -= 1;
			double d = sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
			if(d>maxDisp){
				maxDisp = d;
			}
			memcpy(results->gdisp[current_case*results->nNodes+n],v,sizeof(v));
		}
		model->cases[current_case].maxDisp = maxDisp;
		maxDisp = 1e-9;
		current_case++;
	}
	fclose(f);
	if(current_case < results->nCases){
		fprintf(stderr,"ERROR in loadResults : found %d of %d load cases \n", current_case, results->nCases);
		free(results->gdisp); results->gdisp = NULL;
		return 0;
	}
	return 1;
}

int loadModes(structureModel* model,const char* address){
	if(model->mode_count==0){
		return 0;
	}
	FILE* f = fopen(address,"r");
	if(!f){
		fprintf(stderr,"ERROR in loadModes : failed to open results file %s \n",address);
		return 0;
	}
	model->modes = (Mode*)calloc(model->mode_count,sizeof(Mode));
	char buf[256];
	while(fgets(buf,sizeof(buf),f)){
		int m;
		double fq;
		long pos = ftell(f);
		if(sscanf(buf," MODE %d: f= %lf ",&m,&fq)!=2)continue;
		if(m<1 || m>model->mode_count){
			fprintf(stderr,"ERROR in loadModes : bad mode row , mode : %d, max modes : %d \n", m,model->mode_count);
			free(model->modes);
			fclose(f);
			return 0;
		}
		m-=1;
		model->modes[m].id = m;
		model->modes[m].frequency = fq;
		model->modes[m].disp = (double(*)[6])calloc(model->nodeCount,sizeof(*(model->modes[m].disp)));
		for(int i=0;i<4;i++){
			fgets(buf,sizeof(buf),f);
		}
		while(fgets(buf, sizeof buf, f)){                 // 'fp' is the FILE*
			int n; double v[6];
			if(sscanf(buf, "%d %lf %lf %lf %lf %lf %lf", &n,&v[0],&v[1],&v[2],&v[3],&v[4],&v[5]) != 7) break;
			if(n < 1 || n > model->nodeCount) { 
				fprintf(stderr,"ERROR in loadModes : bad node row , node : %d, max modes : %d \n", n,model->nodeCount);
				fclose(f);
				return 0;
			}
			memcpy(model->modes[m].disp[n-1], v, sizeof v);
		}
		fseek(f,pos,SEEK_SET);
	}
	fclose(f);
	return 1;
}

/*
 if(current_case>=results->nCases-1){
			fprintf(stderr,"ERROR in loadResults : case number %d out of bounds \n",current_case);
			return 0;
		}
 * */
