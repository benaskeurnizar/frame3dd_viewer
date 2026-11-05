#include "my_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void getline_no_comment (
	FILE *fp,   /**< pointer to the file from which to read */
	char *s,    /**< pointer to the string to which to write */
	int lim    /**< the longest anticipated line length  */
){
	int     c=0, i=0;

	while (--lim > 0 && (c=getc(fp)) != EOF && 
		c != '\n' && c != '%' && c != '#' && c != '?' ) {
		if (c != ',' && c != '"' && c != ';')
			s[i++] = c;
		else
			s[i++] = ' ';
	/*      if (c == '\n')  s[i++] = c;     */
	}
	s[i] = '\0';
	if (c != '\n')
		while (--lim > 0 && (c=getc(fp)) != EOF && c != '\n')
		/* read the rest of the line, otherwise do nothing */ ;

	if ( c == EOF ) s[0] = EOF;

	return;
}

void loadCase(FILE* f,staticLoadCase* scase,structureModel* model){
	// gravity
	fscanf(f,"%lf %lf %lf",&(scase->gx),&(scase->gy),&(scase->gz));
	
	// point loads
	fscanf(f,"%d",&(scase->pointLoadCount));
	double maxPoint = 1e-9;
	double maxMoment = 1e-9;
	scase->ploads = (pointLoad*)malloc(sizeof(pointLoad)*scase->pointLoadCount);
	for(int i=0;i<scase->pointLoadCount;i++){
		int n;
		fscanf(f,"%d",&n);
		if(n<1 || n>model->nodeCount){
			fprintf(stderr, "ERROR: node load , node number %d is <= 0 or > %d\n",n, model->nodeCount);
			exit(41);
		}
		double fx,fy,fz,mx,my,mz;
		fscanf(f,"%lf %lf %lf %lf %lf %lf",&fx,&fy,&fz,&mx,&my,&mz);
		scase->ploads[i].node = n-1;
		scase->ploads[i].fx = fx;scase->ploads[i].fy = fy;scase->ploads[i].fz = fz;
		scase->ploads[i].mx = mx;scase->ploads[i].my = my;scase->ploads[i].mz = mz;
		double p = sqrtf(fx*fx+fy*fy+fz*fz);
		double m = sqrtf(mx*mx+my*my+mz*mz);
		if(p>maxPoint){
			maxPoint = p;
		}
		if(m>maxMoment){
			maxMoment = m;
		}
	}
	scase->maxPoint = maxPoint;scase->maxMoment = maxMoment;
	
	// uniform loads
	fscanf(f,"%d",&(scase->uniformLoadCount));
	double maxUniform = 1e-9;
	scase->uloads = (UniformLoad*)malloc(sizeof(UniformLoad)*scase->uniformLoadCount);
	for(int i=0;i<scase->uniformLoadCount;i++){
		int e;
		fscanf(f,"%d",&e);
		if(e<1 || e>model->elementCount){
			fprintf(stderr, "ERROR: uniform load , element number %d is <= 0 or > %d\n",e, model->elementCount);
			exit(41);
		}
		double ux,uy,uz;
		fscanf(f,"%lf %lf %lf",&ux,&uy,&uz);
		scase->uloads[i].element = e-1;
		scase->uloads[i].ux = ux;scase->uloads[i].uy = uy;scase->uloads[i].uz = uz;
		double u = sqrtf(ux*ux+uy*uy+uz*uz);
		if(u>maxUniform){
			maxUniform = u;
		}
	}
	scase->maxUniform = maxUniform;
	
	// trapezoidal loads
	fscanf(f,"%d",&(scase->trapLoadCount));
	double maxTrap = 1e-9;
	scase->tloads = (TrapizoidalLoad*)malloc(sizeof(TrapizoidalLoad)*scase->trapLoadCount);
	for(int i=0;i<scase->trapLoadCount;i++){
		int e;
		fscanf(f,"%d",&e);
		if(e<1 || e>model->elementCount){
			fprintf(stderr, "ERROR: trapezoidal load, element number %d is <= 0 or > %d\n",e, model->elementCount);
			exit(41);
		}
		scase->tloads[i].element = e-1;
		for(int d=0;d<3;d++){
			fscanf(f,"%lf %lf %lf %lf",
				&scase->tloads[i].x1[d],&scase->tloads[i].x2[d],
				&scase->tloads[i].w1[d],&scase->tloads[i].w2[d]);
		}
		double u1 = sqrt(
			scase->tloads[i].w1[0] * scase->tloads[i].w1[0] +
			scase->tloads[i].w1[1] * scase->tloads[i].w1[1] +
			scase->tloads[i].w1[2] * scase->tloads[i].w1[2]
		);

		double u2 = sqrt(
			scase->tloads[i].w2[0] * scase->tloads[i].w2[0] +
			scase->tloads[i].w2[1] * scase->tloads[i].w2[1] +
			scase->tloads[i].w2[2] * scase->tloads[i].w2[2]
		);

		double u = fmax(u1, u2);

		if (u > maxTrap){
			maxTrap = u;
		}
	}
	scase->maxTrap = maxTrap;

	// internal concentrated loads
	fscanf(f,"%d",&(scase->internalLoadCount));
	double maxInternal = 1e-9;
	scase->iloads = (InternalLoad*)malloc(sizeof(InternalLoad)*scase->internalLoadCount);
	for(int i=0;i<scase->internalLoadCount;i++){
		int e;
		fscanf(f,"%d",&e);
		if(e<1 || e>model->elementCount){
			fprintf(stderr, "ERROR: internal load, element number %d is <= 0 or > %d\n",e, model->elementCount);
			exit(41);
		}
		scase->iloads[i].element = e-1;
		fscanf(f,"%lf %lf %lf %lf",&scase->iloads[i].px,&scase->iloads[i].py,
			&scase->iloads[i].pz,&scase->iloads[i].x);
		double u = sqrtf(scase->iloads[i].px*scase->iloads[i].px+scase->iloads[i].py*scase->iloads[i].py+scase->iloads[i].pz*scase->iloads[i].pz);
		if(u>maxInternal){
			maxInternal = u;
		}
	}
	scase->maxInternal = maxInternal;
	// temperature loads
	fscanf(f,"%d",&(scase->tempLoadCount));
	scase->temp_loads = (TempLoad*)malloc(sizeof(TempLoad)*scase->tempLoadCount);
	for(int i=0;i<scase->tempLoadCount;i++){
		int e;
		fscanf(f,"%d",&e);
		if(e<1 || e>model->elementCount){
			fprintf(stderr, "ERROR: temperature load, element number %d is <= 0 or > %d\n",e, model->elementCount);
			exit(41);
		}
		TempLoad* t = &scase->temp_loads[i];
		t->element = e-1;
		fscanf(f,"%lf %lf %lf %lf %lf %lf %lf",&t->a,&t->hy,&t->hz,&t->typ,&t->tym,&t->tzp,&t->tzm);
	}

	// prescribed displacements
	fscanf(f,"%d",&(scase->preDisCount));
	scase->predis = (PreDisplacement*)malloc(sizeof(PreDisplacement)*scase->preDisCount);
	for(int i=0;i<scase->preDisCount;i++){
		int n;
		fscanf(f,"%d",&n);
		if(n<1 || n>model->nodeCount){
			fprintf(stderr, "ERROR: prescribed displacement, node number %d is <= 0 or > %d\n",n, model->nodeCount);
			exit(41);
		}
		PreDisplacement* p = &scase->predis[i];
		p->node = n-1;
		fscanf(f,"%lf %lf %lf %lf %lf %lf",&p->dx,&p->dy,&p->dz,&p->dxx,&p->dyy,&p->dzz);
	}
}

void classifySupports(structureModel* m)
{
    for (int i = 0; i < m->supportCount; i++) {
        Support* s = &m->supports[i];
        int nt = s->r[0] + s->r[1] + s->r[2];    // restrained translations
        int nr = s->r[3] + s->r[4] + s->r[5];    // restrained rotations

        if      (nt + nr == 0)        s->type = SUPPORT_NONE;
        else if (nt == 3 && nr == 3)  s->type = SUPPORT_FIXED;
        else if (nt == 3 && nr == 0)  s->type = SUPPORT_PINNED;
        else if (nr == 0)             s->type = SUPPORT_ROLLER;
        else                          s->type = SUPPORT_GENERAL;
    }
}

void loadModel(const char* address,structureModel* model){
	const char* tempName = "temp.3dd";
	FILE* fp,*tmpf;
	char line[256];
	
	fp = fopen(address,"r");
	if(!fp){
		fprintf(stderr,"couldn't open input file!\n");
		exit(12);
	}
	
	tmpf = fopen(tempName,"w");
	if(!tmpf){
		fprintf(stderr,"couldn't create temporary file!\n");
		exit(12);
	}

	do {
		getline_no_comment(fp, line, 256);
		if (line[0] == EOF) break;                              /* 1 */
		if (line[strspn(line, " \t\r\n")] == '\0') continue;    /* 2 */
		fprintf(tmpf, "%s\n", line);                            /* 3 */
	} while (line[0] != '_');
	fclose(tmpf);
	fclose(fp);
	
	FILE* f = fopen(tempName,"r");
	if(!f){
		fprintf(stderr,"couldn't open temporary created file for reading!\n");
		exit(12);
	}
	
	char title[256];
	fgets(title,sizeof(title),f);
	
	int sr = fscanf(f,"%d",&(model->nodeCount));
	char nodeBuf[256];
	fgets(nodeBuf,sizeof(nodeBuf),f);
	model->nodes = (Node*)malloc(sizeof(Node)*model->nodeCount);
	for(int i=0;i<model->nodeCount;i++){
		int id;
		double x,y,z,r;
		fscanf(f,"%d %lf %lf %lf %lf ",&id,&x,&y,&z,&r);
		if(id-1 < 0 || id> model->nodeCount){
			fprintf(stderr,"ERROR: in node coordinate data, node number out of range\n(node number %d is <= 0 or > %d)\n",id,model->nodeCount);
			exit(41);
		}
		model->nodes[id-1].x = x;model->nodes[id-1].y = y;model->nodes[id-1].z = z;model->nodes[id-1].r = r;
		model->nodes[id-1].id = id-1;
	}
	
	fscanf(f, "%d", &model->supportCount);
	model->supports = (Support*)calloc(model->supportCount, sizeof(Support));
	for (int i = 0; i < model->supportCount; i++) {
		int n;
		fscanf(f, "%d", &n);
		if (n < 1 || n > model->nodeCount) { fprintf(stderr, "ERROR: support node %d out of range\n", n); exit(41); }
		model->supports[i].node = n - 1;
		for (int k = 0; k < 6; k++) fscanf(f, "%d", &model->supports[i].r[k]);
	}
	classifySupports(model);
	
	char tempBuf[256];
	
	sr = fscanf(f,"%d",&(model->elementCount));
	char elBuf[256];
	fgets(elBuf,sizeof(elBuf),f);
	model->elements = (frameElement*)malloc(sizeof(frameElement)*model->elementCount);
	for(int i=0;i<model->elementCount;i++){
		char elementLine[256];
		int id,n1,n2;
		double S;
		if (!fgets(elementLine, sizeof elementLine, f) ||
			sscanf(elementLine, "%d %d %d %lf", &id, &n1, &n2,&S) != 4) {
			fprintf(stderr, "ERROR: bad element line %d\n", i + 1);
			exit(41);
		}
		if (id < 1 || id > model->elementCount) {
			fprintf(stderr, "ERROR: element number %d is <= 0 or > %d\n", id, model->elementCount);
			exit(41);
		}
		if (n1 < 1 || n1 > model->nodeCount) {
			fprintf(stderr, "ERROR: element %d: node number %d is <= 0 or > %d\n", id, n1, model->nodeCount);
			exit(41);
		}
		if (n2 < 1 || n2 > model->nodeCount) {
			fprintf(stderr, "ERROR: element %d: node number %d is <= 0 or > %d\n", id, n2, model->nodeCount);
			exit(41);
		}
		id-=1;n1-=1;n2-=1;
		model->elements[id].id = id;
		model->elements[id].node1 = n1;
		model->elements[id].node2 = n2;
		model->elements[id].S = S;
	}
	
	// pass some not needed data for now
	for(int i=0;i<5;i++){
		fgets(tempBuf,sizeof(tempBuf),f);
	}
	
	
	fscanf(f,"%d",&(model->caseCount));
	if(model->caseCount <= 0){
		fprintf(stderr, "ERROR: static load cases are  %d <= 0\n",model->caseCount);
		exit(41);
	}
	model->cases = (staticLoadCase*)malloc(sizeof(staticLoadCase)*model->caseCount);
	for(int i=0;i<model->caseCount;i++){
		loadCase(f,&(model->cases[i]),model);
	}
	
	fscanf(f,"%d",&(model->mode_count));
	for(int j = 0; j < 3; j++) {
		model->box.min[j] = FLT_MAX;
		model->box.max[j] = -FLT_MAX;
	}
	calculateModelSize(model,&model->box);
	float dx = model->box.max[0] - model->box.min[0];
	float dy = model->box.max[1] - model->box.min[1];
	float dz = model->box.max[2] - model->box.min[2];
	model->modelSize = sqrtf(dx*dx+dy*dy+dz*dz);
	if(model->modelSize < 1e-4){
		model->modelSize = 1.0;
	}
	model->viewScale = 2.0/model->modelSize;
	
	float Lmx = 1e-9;
	float Lmn = 1e9;
	float Lavg = 0.0;
	for(int i=0;i<model->elementCount;i++){
		Node node1 = model->nodes[model->elements[i].node1];
		Node node2 = model->nodes[model->elements[i].node2];
		float x = node2.x-node1.x,y = node2.y-node1.y,z = node2.z-node1.z;
		float l = sqrtf(x*x+y*y+z*z);
		Lavg+=l;
		if(l>Lmx){
			Lmx = l;
		}
		if(l<Lmn){
			Lmn = l;
		}
	}
	Lavg/=(float)model->elementCount;
	model->Lavg = Lavg;
	model->Lmin = Lmn;
	model->Lmax = Lmx;
	fclose(f);
	if (remove(tempName) != 0) {
		perror("could not delete temp file \n");
	}
}










