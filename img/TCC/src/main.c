/* Alex Malins - alex.malins@gmail.com */
/* DynamicTCC: A topological cluster classification code with temporal tracking of clusters. */
/* Not for general consumption */	


#include <string.h>
// #include <mpi.h>
#include "globals.h"
#include "setup.h"
#include "tools.h"
#include "potentials.h"
#include "bonds.h"
#include "rings.h"
#include "clusters.h"
#include "tcc.h"



void Update_bl_mom(int f) {
	int i;
	
	for (i=0; i<nsp3[f]; i++) mean_bl_mom_sp3+=bl_mom_sp3[i];
	for (i=0; i<nsp3a[f]; i++) mean_bl_mom_sp3a+=bl_mom_sp3a[i];
	for (i=0; i<nsp3b[f]; i++) mean_bl_mom_sp3b+=bl_mom_sp3b[i];
	for (i=0; i<nsp3c[f]; i++) mean_bl_mom_sp3c+=bl_mom_sp3c[i];
	
	for (i=0; i<nsp4[f]; i++) mean_bl_mom_sp4+=bl_mom_sp4[i];
	for (i=0; i<nsp4a[f]; i++) mean_bl_mom_sp4a+=bl_mom_sp4a[i];
	for (i=0; i<nsp4b[f]; i++) mean_bl_mom_sp4b+=bl_mom_sp4b[i];
	for (i=0; i<nsp4c[f]; i++) mean_bl_mom_sp4c+=bl_mom_sp4c[i];
	for (i=0; i<n6A[f]; i++) mean_bl_mom_6A+=bl_mom_6A[i];
	
	for (i=0; i<nsp5[f]; i++) mean_bl_mom_sp5+=bl_mom_sp5[i];
	for (i=0; i<nsp5a[f]; i++) mean_bl_mom_sp5a+=bl_mom_sp5a[i];
	for (i=0; i<nsp5b[f]; i++) mean_bl_mom_sp5b+=bl_mom_sp5b[i];
	for (i=0; i<nsp5c[f]; i++) mean_bl_mom_sp5c+=bl_mom_sp5c[i];
}



int icell(int tix, int tiy, int tiz) { 	// returns cell number (from 1 to ncells) for given (tix,tiy,tiz) coordinate
	return 1 + (tix-1+M)%M + M*((tiy-1+M)%M) + M*M*((tiz-1+M)%M); 
}



//// START: Ring routines
void Rings_gSP3(int f, int n0) {	// get SP3/4/5 rings including particle n0
	int i,j;
	int n1, n2;
	
	for (i=0; i<cnb[n0]-1; i++){
		n1=bNums[n0][i];
		if (n1 < n0) continue;
		for (j=i+1; j<cnb[n0]; ++j){
			n2=bNums[n0][j];
			if (n2<n0) continue;
			if (Bonds_BondCheck(n1,n2)) { // is n1 bonded to n2
				if (n1<n2) Rings_aSP3(f,n0,n1,n2); // SP3 found, check type and store
				else Rings_aSP3(f,n0,n2,n1); // SP3 found, check type and store
			} 
			else { // not SP3, search for SP4 & SP5
				if (dosp4==1) {
					if (n1<n2) Rings_gSP4(f,n0,n1,n2);
					else Rings_gSP4(f,n0,n2,n1);
				}
			}
		} 		
	}
}



void Write_dyn(char *filename, int clusSize, int *dyn_n, int **dyn_l, int **dyn_hc, int do_sub, int nsub, char *sub, int **dyn_sub, int doBinaryCen, int binaryCen) {
	int i,j, cntA, cntA_cen, cntA_shell;
	char errMsg[1000];
	FILE *writeout;
	double longestSingleLifetime,totalLifetime,maxLifetime;
	double ratioTotalLifetimeMaxLifetime;
	
	writeout=fopen(filename,"w");
	if (writeout==NULL)  {
		sprintf(errMsg,"Write_dyn(): Error opening file %s",filename);	// Always test file open
		Error(errMsg);
	}
	
	fprintf(writeout,"%s\n",filename);
	fprintf(writeout,"%d\n",(*dyn_n));
	fprintf(writeout,"i	longest_single_lifetime	total_lifetime	max_lifetime	ratio");
	if (doBinary==1) {
		if (doBinaryCen==0) fprintf(writeout,"	N_A");
		else  fprintf(writeout,"	N_A	N_A_cen	N_A_shell");
	}
	for (j=0;j<clusSize;j++) fprintf(writeout,"	p%d",j+1);
	if (doSubClusts==1 && do_sub==1) fprintf(writeout,"	%s",sub);
	fprintf(writeout,"	events"); 
	for (j=0;j<dyn_MaxLives;j++) fprintf(writeout,"	l_s_%d	l_e_%d",j,j);
	fprintf(writeout,"\n");
	for (i=0;i<(*dyn_n);i++) {
		fprintf(writeout,"%d",i);
		longestSingleLifetime=totalLifetime=maxLifetime=0.0;
		ratioTotalLifetimeMaxLifetime=0.0;
		maxLifetime=(dyn_l[i][2*dyn_l[i][0]]-dyn_l[i][1]+1)*FRAMETSTEP*SAMPLEFREQ/talpha;
		for (j=1;j<=dyn_l[i][0];j++) {
			totalLifetime+=(dyn_l[i][2*j]-dyn_l[i][2*j-1]+1)*FRAMETSTEP*SAMPLEFREQ/talpha;
			if (longestSingleLifetime<(dyn_l[i][2*j]-dyn_l[i][2*j-1]+1)*FRAMETSTEP*SAMPLEFREQ/talpha) longestSingleLifetime=(dyn_l[i][2*j]-dyn_l[i][2*j-1]+1)*FRAMETSTEP*SAMPLEFREQ/talpha;
		}
		ratioTotalLifetimeMaxLifetime=totalLifetime/maxLifetime;
		fprintf(writeout,"	%.15lg	%.15lg	%.15lg	%.15lg",longestSingleLifetime,totalLifetime,maxLifetime,ratioTotalLifetimeMaxLifetime);
		if (doBinary==1) {
			cntA=cntA_cen=cntA_shell=0;
			if (doBinaryCen==0) {
				for (j=0;j<clusSize;j++) {
					if (rtype[dyn_hc[i][j]]==1) cntA++;
				}
				fprintf(writeout,"	%d",cntA);
			}
			else {
				for (j=0;j<clusSize;j++) {
					if (rtype[dyn_hc[i][j]]==1) cntA++;
					if (j==binaryCen) if (rtype[dyn_hc[i][j]]==1) cntA_cen++;
					if (j!=binaryCen) if (rtype[dyn_hc[i][j]]==1) cntA_shell++;
				}
				fprintf(writeout,"	%d	%d	%d",cntA,cntA_cen,cntA_shell);
			}
		}
		for (j=0;j<clusSize;j++) fprintf(writeout,"	%d",dyn_hc[i][j]);
		if (doSubClusts==1 && do_sub==1) {
			for (j=0; j<nsub; j++) fprintf(writeout,"	%d",dyn_sub[i][j]);
		}
		fprintf(writeout,"	%d",dyn_l[i][0]);
		for (j=1;j<2*dyn_l[i][0]+1;j++) fprintf(writeout,"	%d",dyn_l[i][j]);
		fprintf(writeout,"\n");
	}
	fclose(writeout);
	printf("d%d Written %s\n",rank,filename);
}

void Dyn_Analyse_Init() {
	int f;
	char errMsg[1000];
	
	sum_hist_sp3=sum_hist_sp3a=sum_hist_sp3b=sum_hist_sp3c=0;	// sum_hist_ax size of **sp** arrays in disum_hist_ension i
	sum_hist_sp4=sum_hist_sp4a=sum_hist_sp4b=sum_hist_6A=0;	// sum_hist_ax size of **sp** arrays in disum_hist_ension i
	sum_hist_sp5=sum_hist_sp5a=sum_hist_sp5b=sum_hist_sp5c=0;	// sum_hist_ax size of **sp** arrays in disum_hist_ension i
	sum_hist_6Z=sum_hist_7K=0;	// sum_hist_ax size of sum_hist_** arrays in disum_hist_ension i
	sum_hist_8A=sum_hist_8B=sum_hist_8K=0;	// sum_hist_ax size of sum_hist_** arrays in disum_hist_ension i
	sum_hist_9A=sum_hist_9B=sum_hist_9K=0;	// sum_hist_ax size of sum_hist_** arrays in disum_hist_ension i
	sum_hist_10A=sum_hist_10B=sum_hist_10K=sum_hist_10W=0;	// sum_hist_ax size of sum_hist_** arrays in disum_hist_ension i
	sum_hist_11A=sum_hist_11B=sum_hist_11C=sum_hist_11E=sum_hist_11F=sum_hist_11W=0;	// sum_hist_ax size of sum_hist_** arrays in disum_hist_ension i
	sum_hist_12A=sum_hist_12B=sum_hist_12D=sum_hist_12E=sum_hist_12K=0;	// sum_hist_ax size of sum_hist_** arrays in disum_hist_ension i
	sum_hist_13A=sum_hist_13B=sum_hist_13K=0;	// max size of sum_hist_** arrays in dimension i
	sum_hist_FCC=sum_hist_HCP=sum_hist_BCC_9=sum_hist_BCC_15=0;	// sum_hist_ax size of **sp** arrays in disum_hist_ension i

	sum_max_hist_sp3=sum_max_hist_sp3a=sum_max_hist_sp3b=sum_max_hist_sp3c=0;	// sum_max_hist_ax size of **sp** arrays in disum_max_hist_ension i
	sum_max_hist_sp4=sum_max_hist_sp4a=sum_max_hist_sp4b=sum_max_hist_6A=0;	// sum_max_hist_ax size of **sp** arrays in disum_max_hist_ension i
	sum_max_hist_sp5=sum_max_hist_sp5a=sum_max_hist_sp5b=sum_max_hist_sp5c=0;	// sum_max_hist_ax size of **sp** arrays in disum_max_hist_ension i
	sum_max_hist_6Z=sum_max_hist_7K=0;	// sum_max_hist_ax size of sum_max_hist_** arrays in disum_max_hist_ension i
	sum_max_hist_8A=sum_max_hist_8B=sum_max_hist_8K=0;	// sum_max_hist_ax size of sum_max_hist_** arrays in disum_max_hist_ension i
	sum_max_hist_9A=sum_max_hist_9B=sum_max_hist_9K=0;	// sum_max_hist_ax size of sum_max_hist_** arrays in disum_max_hist_ension i
	sum_max_hist_10A=sum_max_hist_10B=sum_max_hist_10K=sum_max_hist_10W=0;	// sum_max_hist_ax size of sum_max_hist_** arrays in disum_max_hist_ension i
	sum_max_hist_11A=sum_max_hist_11B=sum_max_hist_11C=sum_max_hist_11E=sum_max_hist_11F=sum_max_hist_11W=0;	// sum_max_hist_ax size of sum_max_hist_** arrays in disum_max_hist_ension i
	sum_max_hist_12A=sum_max_hist_12B=sum_max_hist_12D=sum_max_hist_12E=sum_max_hist_12K=0;	// sum_max_hist_ax size of sum_max_hist_** arrays in disum_max_hist_ension i
	sum_max_hist_13A=sum_max_hist_13B=sum_max_hist_13K=0;	// max size of sum_max_hist_** arrays in dimension i
	sum_max_hist_FCC=sum_max_hist_HCP=sum_max_hist_BCC_9=sum_max_hist_BCC_15=0;	// sum_max_hist_ax size of **sp** arrays in disum_max_hist_ension i
	
	if (dyn_msp3!=-1) { 
		dyn_hist_sp3=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp3[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp3[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp3=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp3=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp3[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp3[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp3=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp3==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp3[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp3a!=-1) { 
		dyn_hist_sp3a=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp3a[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp3a[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp3a=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp3a=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp3a[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp3a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp3a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp3a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp3a[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp3b!=-1) { 
		dyn_hist_sp3b=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp3b[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp3b[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp3b=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp3b=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp3b[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp3b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp3b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp3b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp3b[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp3c!=-1) { 
		dyn_hist_sp3c=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp3c[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp3c[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp3c=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp3c=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp3c[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp3c[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp3c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp3c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp3c[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp4!=-1) { 
		dyn_hist_sp4=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp4[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp4[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp4=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		
		dyn_max_hist_sp4=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp4[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp4[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp4=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp4==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp4[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp4a!=-1) { 
		dyn_hist_sp4a=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp4a[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp4a[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp4a=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		
		dyn_max_hist_sp4a=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp4a[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp4a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp4a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp4a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp4a[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp4b!=-1) { 
		dyn_hist_sp4b=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp4b[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp4b[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp4b=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp4b=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp4b[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp4b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp4b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp4b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp4b[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m6A!=-1) { 
		dyn_hist_6A=malloc(FRAMES*sizeof(int)); if (dyn_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_6A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_6A[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_6A=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_6A[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_6A=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_6A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_6A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_6A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_6A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_6A[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp5!=-1) { 
		dyn_hist_sp5=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp5[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp5[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp5=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp5=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp5[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp5[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp5=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp5==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp5[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp5a!=-1) { 
		dyn_hist_sp5a=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp5a[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp5a[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp5a=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp5a=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp5a[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp5a[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp5a=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp5a==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp5a[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp5b!=-1) { 
		dyn_hist_sp5b=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp5b[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp5b[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp5b=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp5b=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp5b[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp5b[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp5b=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp5b==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp5b[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_msp5c!=-1) { 
		dyn_hist_sp5c=malloc(FRAMES*sizeof(int)); if (dyn_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_sp5c[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_sp5c[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_sp5c=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_sp5c=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_sp5c[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_sp5c[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_sp5c=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_sp5c==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_sp5c[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m6Z!=-1) { 
		dyn_hist_6Z=malloc(FRAMES*sizeof(int)); if (dyn_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_6Z[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_6Z[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_6Z=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_6Z=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_6Z[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_6Z[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_6Z=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_6Z==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_6Z[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m7K!=-1) { 
		dyn_hist_7K=malloc(FRAMES*sizeof(int)); if (dyn_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_7K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_7K[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_7K=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_7K[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_7K=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_7K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_7K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_7K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_7K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_7K[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m8A!=-1) { 
		dyn_hist_8A=malloc(FRAMES*sizeof(int)); if (dyn_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_8A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_8A[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_8A=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_8A[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_8A=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_8A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_8A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_8A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_8A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_8A[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m8B!=-1) { 
		dyn_hist_8B=malloc(FRAMES*sizeof(int)); if (dyn_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_8B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_8B[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_8B=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_8B[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_8B=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_8B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_8B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_8B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_8B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_8B[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m8K!=-1) { 
		dyn_hist_8K=malloc(FRAMES*sizeof(int)); if (dyn_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_8K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_8K[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_8K=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_8K[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_8K=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_8K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_8K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_8K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_8K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_8K[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m9A!=-1) { 
		dyn_hist_9A=malloc(FRAMES*sizeof(int)); if (dyn_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_9A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_9A[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_9A=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_9A[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_9A=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_9A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_9A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_9A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_9A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_9A[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m9B!=-1) { 
		dyn_hist_9B=malloc(FRAMES*sizeof(int)); if (dyn_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_9B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_9B[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_9B=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_9B[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_9B=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_9B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_9B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_9B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_9B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_9B[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m9K!=-1) { 
		dyn_hist_9K=malloc(FRAMES*sizeof(int)); if (dyn_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_9K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_9K[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_9K=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_9K[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_9K=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_9K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_9K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_9K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_9K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_9K[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m10A!=-1) { 
		dyn_hist_10A=malloc(FRAMES*sizeof(int)); if (dyn_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_10A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_10A[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_10A=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_10A[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_10A=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_10A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_10A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_10A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_10A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_10A[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m10B!=-1) { 
		dyn_hist_10B=malloc(FRAMES*sizeof(int)); if (dyn_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_10B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_10B[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_10B=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_10B[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_10B=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_10B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_10B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_10B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_10B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_10B[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m10K!=-1) { 
		dyn_hist_10K=malloc(FRAMES*sizeof(int)); if (dyn_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_10K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_10K[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_10K=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_10K[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_10K=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_10K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_10K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_10K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_10K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_10K[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m10W!=-1) { 
		dyn_hist_10W=malloc(FRAMES*sizeof(int)); if (dyn_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_10W[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_10W[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_10W=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_10W[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_10W=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_10W[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_10W[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_10W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_10W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_10W[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m11A!=-1) { 
		dyn_hist_11A=malloc(FRAMES*sizeof(int)); if (dyn_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_11A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_11A[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_11A=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_11A[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_11A=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_11A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_11A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_11A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_11A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_11A[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m11B!=-1) { 
		dyn_hist_11B=malloc(FRAMES*sizeof(int)); if (dyn_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_11B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_11B[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_11B=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_11B[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_11B=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_11B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_11B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_11B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_11B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_11B[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m11C!=-1) { 
		dyn_hist_11C=malloc(FRAMES*sizeof(int)); if (dyn_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_11C[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_11C[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_11C=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_11C[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_11C=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_11C[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_11C[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_11C=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_11C==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_11C[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m11E!=-1) { 
		dyn_hist_11E=malloc(FRAMES*sizeof(int)); if (dyn_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_11E[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_11E[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_11E=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_11E[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_11E=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_11E[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_11E[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_11E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_11E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_11E[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m11F!=-1) { 
		dyn_hist_11F=malloc(FRAMES*sizeof(int)); if (dyn_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_11F[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_11F[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_11F=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_11F[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_11F=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_11F[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_11F[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_11F=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_11F==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_11F[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m11W!=-1) { 
		dyn_hist_11W=malloc(FRAMES*sizeof(int)); if (dyn_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_11W[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_11W[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_11W=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_11W[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_11W=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_11W[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_11W[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_11W=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_11W==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_11W[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m12A!=-1) { 
		dyn_hist_12A=malloc(FRAMES*sizeof(int)); if (dyn_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_12A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_12A[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_12A=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_12A[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_12A=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_12A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_12A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_12A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_12A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_12A[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m12B!=-1) { 
		dyn_hist_12B=malloc(FRAMES*sizeof(int)); if (dyn_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_12B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_12B[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_12B=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_12B[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_12B=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_12B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_12B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_12B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_12B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_12B[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m12D!=-1) { 
		dyn_hist_12D=malloc(FRAMES*sizeof(int)); if (dyn_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_12D[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_12D[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_12D=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_12D[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_12D=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_12D[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_12D[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_12D=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_12D==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_12D[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m12E!=-1) { 
		dyn_hist_12E=malloc(FRAMES*sizeof(int)); if (dyn_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_12E[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_12E[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_12E=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_12E[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_12E=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_12E[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_12E[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_12E=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_12E==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_12E[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m12K!=-1) { 
		dyn_hist_12K=malloc(FRAMES*sizeof(int)); if (dyn_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_12K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_12K[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_12K=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_12K[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_12K=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_12K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_12K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_12K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_12K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_12K[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m13A!=-1) { 
		dyn_hist_13A=malloc(FRAMES*sizeof(int)); if (dyn_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_13A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_13A[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_13A=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_13A[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_13A=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_13A[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_13A[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_13A=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_13A==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_13A[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m13B!=-1) { 
		dyn_hist_13B=malloc(FRAMES*sizeof(int)); if (dyn_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_13B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_13B[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_13B=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_13B[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_13B=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_13B[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_13B[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_13B=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_13B==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_13B[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_m13K!=-1) { 
		dyn_hist_13K=malloc(FRAMES*sizeof(int)); if (dyn_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_13K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_13K[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_13K=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_13K[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_13K=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_13K[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_13K[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_13K=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_13K==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_13K[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_mFCC!=-1) { 
		dyn_hist_FCC=malloc(FRAMES*sizeof(int)); if (dyn_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_FCC[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_FCC[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_FCC=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_FCC=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_FCC[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_FCC[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_FCC=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_FCC==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_FCC[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_mHCP!=-1) { 
		dyn_hist_HCP=malloc(FRAMES*sizeof(int)); if (dyn_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_HCP[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_HCP[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_HCP=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_HCP=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_HCP[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_HCP[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_HCP=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_HCP==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_HCP[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_mBCC_9!=-1) { 
		dyn_hist_BCC_9=malloc(FRAMES*sizeof(int)); if (dyn_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_BCC_9=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_BCC_9=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_BCC_9=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_BCC_9==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_BCC_9[] malloc out of memory\n"); Error(errMsg); } 
	}
	if (dyn_mBCC_15!=-1) { 
		dyn_hist_BCC_15=malloc(FRAMES*sizeof(int)); if (dyn_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_norm_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); } 
		
		dyn_total_hist_BCC_15=malloc(FRAMES*sizeof(int)); if (dyn_total_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_total_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_total_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_norm_total_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_total_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_total_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_total_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_total_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
	
		dyn_max_hist_BCC_15=malloc(FRAMES*sizeof(int)); if (dyn_max_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_max_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_max_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_norm_max_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_max_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_norm_corrected_max_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_norm_corrected_max_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_norm_corrected_max_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); } 
		dyn_decay_norm_max_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_max_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_max_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); }
		dyn_decay_norm_corrected_max_hist_BCC_15=malloc(FRAMES*sizeof(double)); if (dyn_decay_norm_corrected_max_hist_BCC_15==NULL) { sprintf(errMsg,"Dyn_Analyse_Init(): dyn_decay_norm_corrected_max_hist_BCC_15[] malloc out of memory\n"); Error(errMsg); } 
	}
	
	for (f=0; f<FRAMES; f++) {
		if (dyn_msp3!=-1) {
			dyn_hist_sp3[f]=0;
			dyn_norm_hist_sp3[f]=0.0;
			dyn_norm_corrected_hist_sp3[f]=0.0;
			dyn_decay_norm_hist_sp3[f]=0.0;
			dyn_decay_norm_corrected_hist_sp3[f]=0.0;
			dyn_total_hist_sp3[f]=0;
			dyn_norm_total_hist_sp3[f]=0.0;
			dyn_decay_norm_total_hist_sp3[f]=0.0;
			dyn_max_hist_sp3[f]=0;
			dyn_norm_max_hist_sp3[f]=0.0;
			dyn_norm_corrected_max_hist_sp3[f]=0.0;
			dyn_decay_norm_max_hist_sp3[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp3[f]=0.0;
		}
		if (dyn_msp3a!=-1) {
			dyn_hist_sp3a[f]=0;
			dyn_norm_hist_sp3a[f]=0.0;
			dyn_norm_corrected_hist_sp3a[f]=0.0;
			dyn_decay_norm_hist_sp3a[f]=0.0;
			dyn_decay_norm_corrected_hist_sp3a[f]=0.0;
			dyn_total_hist_sp3a[f]=0;
			dyn_norm_total_hist_sp3a[f]=0.0;
			dyn_decay_norm_total_hist_sp3a[f]=0.0;
			dyn_max_hist_sp3a[f]=0;
			dyn_norm_max_hist_sp3a[f]=0.0;
			dyn_norm_corrected_max_hist_sp3a[f]=0.0;
			dyn_decay_norm_max_hist_sp3a[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp3a[f]=0.0;
		}
		if (dyn_msp3b!=-1) {
			dyn_hist_sp3b[f]=0;
			dyn_norm_hist_sp3b[f]=0.0;
			dyn_norm_corrected_hist_sp3b[f]=0.0;
			dyn_decay_norm_hist_sp3b[f]=0.0;
			dyn_decay_norm_corrected_hist_sp3b[f]=0.0;
			dyn_total_hist_sp3b[f]=0;
			dyn_norm_total_hist_sp3b[f]=0.0;
			dyn_decay_norm_total_hist_sp3b[f]=0.0;
			dyn_max_hist_sp3b[f]=0;
			dyn_norm_max_hist_sp3b[f]=0.0;
			dyn_norm_corrected_max_hist_sp3b[f]=0.0;
			dyn_decay_norm_max_hist_sp3b[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp3b[f]=0.0;
		}
		if (dyn_msp3c!=-1) {
			dyn_hist_sp3c[f]=0;
			dyn_norm_hist_sp3c[f]=0.0;
			dyn_norm_corrected_hist_sp3c[f]=0.0;
			dyn_decay_norm_hist_sp3c[f]=0.0;
			dyn_decay_norm_corrected_hist_sp3c[f]=0.0;
			dyn_total_hist_sp3c[f]=0;
			dyn_norm_total_hist_sp3c[f]=0.0;
			dyn_decay_norm_total_hist_sp3c[f]=0.0;
			dyn_max_hist_sp3c[f]=0;
			dyn_norm_max_hist_sp3c[f]=0.0;
			dyn_norm_corrected_max_hist_sp3c[f]=0.0;
			dyn_decay_norm_max_hist_sp3c[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp3c[f]=0.0;
		}
		if (dyn_msp4!=-1) {
			dyn_hist_sp4[f]=0;
			dyn_norm_hist_sp4[f]=0.0;
			dyn_norm_corrected_hist_sp4[f]=0.0;
			dyn_decay_norm_hist_sp4[f]=0.0;
			dyn_decay_norm_corrected_hist_sp4[f]=0.0;
			dyn_total_hist_sp4[f]=0;
			dyn_norm_total_hist_sp4[f]=0.0;
			dyn_decay_norm_total_hist_sp4[f]=0.0;
			dyn_max_hist_sp4[f]=0;
			dyn_norm_max_hist_sp4[f]=0.0;
			dyn_norm_corrected_max_hist_sp4[f]=0.0;
			dyn_decay_norm_max_hist_sp4[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp4[f]=0.0;
		}
		if (dyn_msp4a!=-1) {
			dyn_hist_sp4a[f]=0;
			dyn_norm_hist_sp4a[f]=0.0;
			dyn_norm_corrected_hist_sp4a[f]=0.0;
			dyn_decay_norm_hist_sp4a[f]=0.0;
			dyn_decay_norm_corrected_hist_sp4a[f]=0.0;
			dyn_total_hist_sp4a[f]=0;
			dyn_norm_total_hist_sp4a[f]=0.0;
			dyn_decay_norm_total_hist_sp4a[f]=0.0;
			dyn_max_hist_sp4a[f]=0;
			dyn_norm_max_hist_sp4a[f]=0.0;
			dyn_norm_corrected_max_hist_sp4a[f]=0.0;
			dyn_decay_norm_max_hist_sp4a[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp4a[f]=0.0;
		}
		if (dyn_msp4b!=-1) {
			dyn_hist_sp4b[f]=0;
			dyn_norm_hist_sp4b[f]=0.0;
			dyn_norm_corrected_hist_sp4b[f]=0.0;
			dyn_decay_norm_hist_sp4b[f]=0.0;
			dyn_decay_norm_corrected_hist_sp4b[f]=0.0;
			dyn_total_hist_sp4b[f]=0;
			dyn_norm_total_hist_sp4b[f]=0.0;
			dyn_decay_norm_total_hist_sp4b[f]=0.0;
			dyn_max_hist_sp4b[f]=0;
			dyn_norm_max_hist_sp4b[f]=0.0;
			dyn_norm_corrected_max_hist_sp4b[f]=0.0;
			dyn_decay_norm_max_hist_sp4b[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp4b[f]=0.0;
		}
		if (dyn_m6A!=-1) {
			dyn_hist_6A[f]=0;
			dyn_norm_hist_6A[f]=0.0;
			dyn_norm_corrected_hist_6A[f]=0.0;
			dyn_decay_norm_hist_6A[f]=0.0;
			dyn_decay_norm_corrected_hist_6A[f]=0.0;
			dyn_total_hist_6A[f]=0;
			dyn_norm_total_hist_6A[f]=0.0;
			dyn_decay_norm_total_hist_6A[f]=0.0;
			dyn_max_hist_6A[f]=0;
			dyn_norm_max_hist_6A[f]=0.0;
			dyn_norm_corrected_max_hist_6A[f]=0.0;
			dyn_decay_norm_max_hist_6A[f]=0.0;
			dyn_decay_norm_corrected_max_hist_6A[f]=0.0;
		}
		if (dyn_msp5!=-1) {
			dyn_hist_sp5[f]=0;
			dyn_norm_hist_sp5[f]=0.0;
			dyn_norm_corrected_hist_sp5[f]=0.0;
			dyn_decay_norm_hist_sp5[f]=0.0;
			dyn_decay_norm_corrected_hist_sp5[f]=0.0;
			dyn_total_hist_sp5[f]=0;
			dyn_norm_total_hist_sp5[f]=0.0;
			dyn_decay_norm_total_hist_sp5[f]=0.0;
			dyn_max_hist_sp5[f]=0;
			dyn_norm_max_hist_sp5[f]=0.0;
			dyn_norm_corrected_max_hist_sp5[f]=0.0;
			dyn_decay_norm_max_hist_sp5[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp5[f]=0.0;
		}
		if (dyn_msp5a!=-1) {
			dyn_hist_sp5a[f]=0;
			dyn_norm_hist_sp5a[f]=0.0;
			dyn_norm_corrected_hist_sp5a[f]=0.0;
			dyn_decay_norm_hist_sp5a[f]=0.0;
			dyn_decay_norm_corrected_hist_sp5a[f]=0.0;
			dyn_total_hist_sp5a[f]=0;
			dyn_norm_total_hist_sp5a[f]=0.0;
			dyn_decay_norm_total_hist_sp5a[f]=0.0;
			dyn_max_hist_sp5a[f]=0;
			dyn_norm_max_hist_sp5a[f]=0.0;
			dyn_norm_corrected_max_hist_sp5a[f]=0.0;
			dyn_decay_norm_max_hist_sp5a[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp5a[f]=0.0;
		}
		if (dyn_msp5b!=-1) {
			dyn_hist_sp5b[f]=0;
			dyn_norm_hist_sp5b[f]=0.0;
			dyn_norm_corrected_hist_sp5b[f]=0.0;
			dyn_decay_norm_hist_sp5b[f]=0.0;
			dyn_decay_norm_corrected_hist_sp5b[f]=0.0;
			dyn_total_hist_sp5b[f]=0;
			dyn_norm_total_hist_sp5b[f]=0.0;
			dyn_decay_norm_total_hist_sp5b[f]=0.0;
			dyn_max_hist_sp5b[f]=0;
			dyn_norm_max_hist_sp5b[f]=0.0;
			dyn_norm_corrected_max_hist_sp5b[f]=0.0;
			dyn_decay_norm_max_hist_sp5b[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp5b[f]=0.0;
		}
		if (dyn_msp5c!=-1) {
			dyn_hist_sp5c[f]=0;
			dyn_norm_hist_sp5c[f]=0.0;
			dyn_norm_corrected_hist_sp5c[f]=0.0;
			dyn_decay_norm_hist_sp5c[f]=0.0;
			dyn_decay_norm_corrected_hist_sp5c[f]=0.0;
			dyn_total_hist_sp5c[f]=0;
			dyn_norm_total_hist_sp5c[f]=0.0;
			dyn_decay_norm_total_hist_sp5c[f]=0.0;
			dyn_max_hist_sp5c[f]=0;
			dyn_norm_max_hist_sp5c[f]=0.0;
			dyn_norm_corrected_max_hist_sp5c[f]=0.0;
			dyn_decay_norm_max_hist_sp5c[f]=0.0;
			dyn_decay_norm_corrected_max_hist_sp5c[f]=0.0;
		}
		if (dyn_m6Z!=-1) {
			dyn_hist_6Z[f]=0;
			dyn_norm_hist_6Z[f]=0.0;
			dyn_norm_corrected_hist_6Z[f]=0.0;
			dyn_decay_norm_hist_6Z[f]=0.0;
			dyn_decay_norm_corrected_hist_6Z[f]=0.0;
			dyn_total_hist_6Z[f]=0;
			dyn_norm_total_hist_6Z[f]=0.0;
			dyn_decay_norm_total_hist_6Z[f]=0.0;
			dyn_max_hist_6Z[f]=0;
			dyn_norm_max_hist_6Z[f]=0.0;
			dyn_norm_corrected_max_hist_6Z[f]=0.0;
			dyn_decay_norm_max_hist_6Z[f]=0.0;
			dyn_decay_norm_corrected_max_hist_6Z[f]=0.0;
		}
		if (dyn_m7K!=-1) {
			dyn_hist_7K[f]=0;
			dyn_norm_hist_7K[f]=0.0;
			dyn_norm_corrected_hist_7K[f]=0.0;
			dyn_decay_norm_hist_7K[f]=0.0;
			dyn_decay_norm_corrected_hist_7K[f]=0.0;
			dyn_total_hist_7K[f]=0;
			dyn_norm_total_hist_7K[f]=0.0;
			dyn_decay_norm_total_hist_7K[f]=0.0;
			dyn_max_hist_7K[f]=0;
			dyn_norm_max_hist_7K[f]=0.0;
			dyn_norm_corrected_max_hist_7K[f]=0.0;
			dyn_decay_norm_max_hist_7K[f]=0.0;
			dyn_decay_norm_corrected_max_hist_7K[f]=0.0;
		}
		if (dyn_m8A!=-1) {
			dyn_hist_8A[f]=0;
			dyn_norm_hist_8A[f]=0.0;
			dyn_norm_corrected_hist_8A[f]=0.0;
			dyn_decay_norm_hist_8A[f]=0.0;
			dyn_decay_norm_corrected_hist_8A[f]=0.0;
			dyn_total_hist_8A[f]=0;
			dyn_norm_total_hist_8A[f]=0.0;
			dyn_decay_norm_total_hist_8A[f]=0.0;
			dyn_max_hist_8A[f]=0;
			dyn_norm_max_hist_8A[f]=0.0;
			dyn_norm_corrected_max_hist_8A[f]=0.0;
			dyn_decay_norm_max_hist_8A[f]=0.0;
			dyn_decay_norm_corrected_max_hist_8A[f]=0.0;
		}
		if (dyn_m8B!=-1) {
			dyn_hist_8B[f]=0;
			dyn_norm_hist_8B[f]=0.0;
			dyn_norm_corrected_hist_8B[f]=0.0;
			dyn_decay_norm_hist_8B[f]=0.0;
			dyn_decay_norm_corrected_hist_8B[f]=0.0;
			dyn_total_hist_8B[f]=0;
			dyn_norm_total_hist_8B[f]=0.0;
			dyn_decay_norm_total_hist_8B[f]=0.0;
			dyn_max_hist_8B[f]=0;
			dyn_norm_max_hist_8B[f]=0.0;
			dyn_norm_corrected_max_hist_8B[f]=0.0;
			dyn_decay_norm_max_hist_8B[f]=0.0;
			dyn_decay_norm_corrected_max_hist_8B[f]=0.0;
		}
		if (dyn_m8K!=-1) {
			dyn_hist_8K[f]=0;
			dyn_norm_hist_8K[f]=0.0;
			dyn_norm_corrected_hist_8K[f]=0.0;
			dyn_decay_norm_hist_8K[f]=0.0;
			dyn_decay_norm_corrected_hist_8K[f]=0.0;
			dyn_total_hist_8K[f]=0;
			dyn_norm_total_hist_8K[f]=0.0;
			dyn_decay_norm_total_hist_8K[f]=0.0;
			dyn_max_hist_8K[f]=0;
			dyn_norm_max_hist_8K[f]=0.0;
			dyn_norm_corrected_max_hist_8K[f]=0.0;
			dyn_decay_norm_max_hist_8K[f]=0.0;
			dyn_decay_norm_corrected_max_hist_8K[f]=0.0;
		}
		if (dyn_m9A!=-1) {
			dyn_hist_9A[f]=0;
			dyn_norm_hist_9A[f]=0.0;
			dyn_norm_corrected_hist_9A[f]=0.0;
			dyn_decay_norm_hist_9A[f]=0.0;
			dyn_decay_norm_corrected_hist_9A[f]=0.0;
			dyn_total_hist_9A[f]=0;
			dyn_norm_total_hist_9A[f]=0.0;
			dyn_decay_norm_total_hist_9A[f]=0.0;
			dyn_max_hist_9A[f]=0;
			dyn_norm_max_hist_9A[f]=0.0;
			dyn_norm_corrected_max_hist_9A[f]=0.0;
			dyn_decay_norm_max_hist_9A[f]=0.0;
			dyn_decay_norm_corrected_max_hist_9A[f]=0.0;
		}
		if (dyn_m9B!=-1) {
			dyn_hist_9B[f]=0;
			dyn_norm_hist_9B[f]=0.0;
			dyn_norm_corrected_hist_9B[f]=0.0;
			dyn_decay_norm_hist_9B[f]=0.0;
			dyn_decay_norm_corrected_hist_9B[f]=0.0;
			dyn_total_hist_9B[f]=0;
			dyn_norm_total_hist_9B[f]=0.0;
			dyn_decay_norm_total_hist_9B[f]=0.0;
			dyn_max_hist_9B[f]=0;
			dyn_norm_max_hist_9B[f]=0.0;
			dyn_norm_corrected_max_hist_9B[f]=0.0;
			dyn_decay_norm_max_hist_9B[f]=0.0;
			dyn_decay_norm_corrected_max_hist_9B[f]=0.0;
		}
		if (dyn_m9K!=-1) {
			dyn_hist_9K[f]=0;
			dyn_norm_hist_9K[f]=0.0;
			dyn_norm_corrected_hist_9K[f]=0.0;
			dyn_decay_norm_hist_9K[f]=0.0;
			dyn_decay_norm_corrected_hist_9K[f]=0.0;
			dyn_total_hist_9K[f]=0;
			dyn_norm_total_hist_9K[f]=0.0;
			dyn_decay_norm_total_hist_9K[f]=0.0;
			dyn_max_hist_9K[f]=0;
			dyn_norm_max_hist_9K[f]=0.0;
			dyn_norm_corrected_max_hist_9K[f]=0.0;
			dyn_decay_norm_max_hist_9K[f]=0.0;
			dyn_decay_norm_corrected_max_hist_9K[f]=0.0;
		}
		if (dyn_m10A!=-1) {
			dyn_hist_10A[f]=0;
			dyn_norm_hist_10A[f]=0.0;
			dyn_norm_corrected_hist_10A[f]=0.0;
			dyn_decay_norm_hist_10A[f]=0.0;
			dyn_decay_norm_corrected_hist_10A[f]=0.0;
			dyn_total_hist_10A[f]=0;
			dyn_norm_total_hist_10A[f]=0.0;
			dyn_decay_norm_total_hist_10A[f]=0.0;
			dyn_max_hist_10A[f]=0;
			dyn_norm_max_hist_10A[f]=0.0;
			dyn_norm_corrected_max_hist_10A[f]=0.0;
			dyn_decay_norm_max_hist_10A[f]=0.0;
			dyn_decay_norm_corrected_max_hist_10A[f]=0.0;
		}
		if (dyn_m10B!=-1) {
			dyn_hist_10B[f]=0;
			dyn_norm_hist_10B[f]=0.0;
			dyn_norm_corrected_hist_10B[f]=0.0;
			dyn_decay_norm_hist_10B[f]=0.0;
			dyn_decay_norm_corrected_hist_10B[f]=0.0;
			dyn_total_hist_10B[f]=0;
			dyn_norm_total_hist_10B[f]=0.0;
			dyn_decay_norm_total_hist_10B[f]=0.0;
			dyn_max_hist_10B[f]=0;
			dyn_norm_max_hist_10B[f]=0.0;
			dyn_norm_corrected_max_hist_10B[f]=0.0;
			dyn_decay_norm_max_hist_10B[f]=0.0;
			dyn_decay_norm_corrected_max_hist_10B[f]=0.0;
		}
		if (dyn_m10K!=-1) {
			dyn_hist_10K[f]=0;
			dyn_norm_hist_10K[f]=0.0;
			dyn_norm_corrected_hist_10K[f]=0.0;
			dyn_decay_norm_hist_10K[f]=0.0;
			dyn_decay_norm_corrected_hist_10K[f]=0.0;
			dyn_total_hist_10K[f]=0;
			dyn_norm_total_hist_10K[f]=0.0;
			dyn_decay_norm_total_hist_10K[f]=0.0;
			dyn_max_hist_10K[f]=0;
			dyn_norm_max_hist_10K[f]=0.0;
			dyn_norm_corrected_max_hist_10K[f]=0.0;
			dyn_decay_norm_max_hist_10K[f]=0.0;
			dyn_decay_norm_corrected_max_hist_10K[f]=0.0;
		}
		if (dyn_m10W!=-1) {
			dyn_hist_10W[f]=0;
			dyn_norm_hist_10W[f]=0.0;
			dyn_norm_corrected_hist_10W[f]=0.0;
			dyn_decay_norm_hist_10W[f]=0.0;
			dyn_decay_norm_corrected_hist_10W[f]=0.0;
			dyn_total_hist_10W[f]=0;
			dyn_norm_total_hist_10W[f]=0.0;
			dyn_decay_norm_total_hist_10W[f]=0.0;
			dyn_max_hist_10W[f]=0;
			dyn_norm_max_hist_10W[f]=0.0;
			dyn_norm_corrected_max_hist_10W[f]=0.0;
			dyn_decay_norm_max_hist_10W[f]=0.0;
			dyn_decay_norm_corrected_max_hist_10W[f]=0.0;
		}
		if (dyn_m11A!=-1) {
			dyn_hist_11A[f]=0;
			dyn_norm_hist_11A[f]=0.0;
			dyn_norm_corrected_hist_11A[f]=0.0;
			dyn_decay_norm_hist_11A[f]=0.0;
			dyn_decay_norm_corrected_hist_11A[f]=0.0;
			dyn_total_hist_11A[f]=0;
			dyn_norm_total_hist_11A[f]=0.0;
			dyn_decay_norm_total_hist_11A[f]=0.0;
			dyn_max_hist_11A[f]=0;
			dyn_norm_max_hist_11A[f]=0.0;
			dyn_norm_corrected_max_hist_11A[f]=0.0;
			dyn_decay_norm_max_hist_11A[f]=0.0;
			dyn_decay_norm_corrected_max_hist_11A[f]=0.0;
		}
		if (dyn_m11B!=-1) {
			dyn_hist_11B[f]=0;
			dyn_norm_hist_11B[f]=0.0;
			dyn_norm_corrected_hist_11B[f]=0.0;
			dyn_decay_norm_hist_11B[f]=0.0;
			dyn_decay_norm_corrected_hist_11B[f]=0.0;
			dyn_total_hist_11B[f]=0;
			dyn_norm_total_hist_11B[f]=0.0;
			dyn_decay_norm_total_hist_11B[f]=0.0;
			dyn_max_hist_11B[f]=0;
			dyn_norm_max_hist_11B[f]=0.0;
			dyn_norm_corrected_max_hist_11B[f]=0.0;
			dyn_decay_norm_max_hist_11B[f]=0.0;
			dyn_decay_norm_corrected_max_hist_11B[f]=0.0;
		}
		if (dyn_m11C!=-1) {
			dyn_hist_11C[f]=0;
			dyn_norm_hist_11C[f]=0.0;
			dyn_norm_corrected_hist_11C[f]=0.0;
			dyn_decay_norm_hist_11C[f]=0.0;
			dyn_decay_norm_corrected_hist_11C[f]=0.0;
			dyn_total_hist_11C[f]=0;
			dyn_norm_total_hist_11C[f]=0.0;
			dyn_decay_norm_total_hist_11C[f]=0.0;
			dyn_max_hist_11C[f]=0;
			dyn_norm_max_hist_11C[f]=0.0;
			dyn_norm_corrected_max_hist_11C[f]=0.0;
			dyn_decay_norm_max_hist_11C[f]=0.0;
			dyn_decay_norm_corrected_max_hist_11C[f]=0.0;
		}
		if (dyn_m11E!=-1) {
			dyn_hist_11E[f]=0;
			dyn_norm_hist_11E[f]=0.0;
			dyn_norm_corrected_hist_11E[f]=0.0;
			dyn_decay_norm_hist_11E[f]=0.0;
			dyn_decay_norm_corrected_hist_11E[f]=0.0;
			dyn_total_hist_11E[f]=0;
			dyn_norm_total_hist_11E[f]=0.0;
			dyn_decay_norm_total_hist_11E[f]=0.0;
			dyn_max_hist_11E[f]=0;
			dyn_norm_max_hist_11E[f]=0.0;
			dyn_norm_corrected_max_hist_11E[f]=0.0;
			dyn_decay_norm_max_hist_11E[f]=0.0;
			dyn_decay_norm_corrected_max_hist_11E[f]=0.0;
		}
		if (dyn_m11F!=-1) {
			dyn_hist_11F[f]=0;
			dyn_norm_hist_11F[f]=0.0;
			dyn_norm_corrected_hist_11F[f]=0.0;
			dyn_decay_norm_hist_11F[f]=0.0;
			dyn_decay_norm_corrected_hist_11F[f]=0.0;
			dyn_total_hist_11F[f]=0;
			dyn_norm_total_hist_11F[f]=0.0;
			dyn_decay_norm_total_hist_11F[f]=0.0;
			dyn_max_hist_11F[f]=0;
			dyn_norm_max_hist_11F[f]=0.0;
			dyn_norm_corrected_max_hist_11F[f]=0.0;
			dyn_decay_norm_max_hist_11F[f]=0.0;
			dyn_decay_norm_corrected_max_hist_11F[f]=0.0;
		}
		if (dyn_m11W!=-1) {
			dyn_hist_11W[f]=0;
			dyn_norm_hist_11W[f]=0.0;
			dyn_norm_corrected_hist_11W[f]=0.0;
			dyn_decay_norm_hist_11W[f]=0.0;
			dyn_decay_norm_corrected_hist_11W[f]=0.0;
			dyn_total_hist_11W[f]=0;
			dyn_norm_total_hist_11W[f]=0.0;
			dyn_decay_norm_total_hist_11W[f]=0.0;
			dyn_max_hist_11W[f]=0;
			dyn_norm_max_hist_11W[f]=0.0;
			dyn_norm_corrected_max_hist_11W[f]=0.0;
			dyn_decay_norm_max_hist_11W[f]=0.0;
			dyn_decay_norm_corrected_max_hist_11W[f]=0.0;
		}
		if (dyn_m12A!=-1) {
			dyn_hist_12A[f]=0;
			dyn_norm_hist_12A[f]=0.0;
			dyn_norm_corrected_hist_12A[f]=0.0;
			dyn_decay_norm_hist_12A[f]=0.0;
			dyn_decay_norm_corrected_hist_12A[f]=0.0;
			dyn_total_hist_12A[f]=0;
			dyn_norm_total_hist_12A[f]=0.0;
			dyn_decay_norm_total_hist_12A[f]=0.0;
			dyn_max_hist_12A[f]=0;
			dyn_norm_max_hist_12A[f]=0.0;
			dyn_norm_corrected_max_hist_12A[f]=0.0;
			dyn_decay_norm_max_hist_12A[f]=0.0;
			dyn_decay_norm_corrected_max_hist_12A[f]=0.0;
		}
		if (dyn_m12B!=-1) {
			dyn_hist_12B[f]=0;
			dyn_norm_hist_12B[f]=0.0;
			dyn_norm_corrected_hist_12B[f]=0.0;
			dyn_decay_norm_hist_12B[f]=0.0;
			dyn_decay_norm_corrected_hist_12B[f]=0.0;
			dyn_total_hist_12B[f]=0;
			dyn_norm_total_hist_12B[f]=0.0;
			dyn_decay_norm_total_hist_12B[f]=0.0;
			dyn_max_hist_12B[f]=0;
			dyn_norm_max_hist_12B[f]=0.0;
			dyn_norm_corrected_max_hist_12B[f]=0.0;
			dyn_decay_norm_max_hist_12B[f]=0.0;
			dyn_decay_norm_corrected_max_hist_12B[f]=0.0;
		}
		if (dyn_m12D!=-1) {
			dyn_hist_12D[f]=0;
			dyn_norm_hist_12D[f]=0.0;
			dyn_norm_corrected_hist_12D[f]=0.0;
			dyn_decay_norm_hist_12D[f]=0.0;
			dyn_decay_norm_corrected_hist_12D[f]=0.0;
			dyn_total_hist_12D[f]=0;
			dyn_norm_total_hist_12D[f]=0.0;
			dyn_decay_norm_total_hist_12D[f]=0.0;
			dyn_max_hist_12D[f]=0;
			dyn_norm_max_hist_12D[f]=0.0;
			dyn_norm_corrected_max_hist_12D[f]=0.0;
			dyn_decay_norm_max_hist_12D[f]=0.0;
			dyn_decay_norm_corrected_max_hist_12D[f]=0.0;
		}
		if (dyn_m12E!=-1) {
			dyn_hist_12E[f]=0;
			dyn_norm_hist_12E[f]=0.0;
			dyn_norm_corrected_hist_12E[f]=0.0;
			dyn_decay_norm_hist_12E[f]=0.0;
			dyn_decay_norm_corrected_hist_12E[f]=0.0;
			dyn_total_hist_12E[f]=0;
			dyn_norm_total_hist_12E[f]=0.0;
			dyn_decay_norm_total_hist_12E[f]=0.0;
			dyn_max_hist_12E[f]=0;
			dyn_norm_max_hist_12E[f]=0.0;
			dyn_norm_corrected_max_hist_12E[f]=0.0;
			dyn_decay_norm_max_hist_12E[f]=0.0;
			dyn_decay_norm_corrected_max_hist_12E[f]=0.0;
		}
		if (dyn_m12K!=-1) {
			dyn_hist_12K[f]=0;
			dyn_norm_hist_12K[f]=0.0;
			dyn_norm_corrected_hist_12K[f]=0.0;
			dyn_decay_norm_hist_12K[f]=0.0;
			dyn_decay_norm_corrected_hist_12K[f]=0.0;
			dyn_total_hist_12K[f]=0;
			dyn_norm_total_hist_12K[f]=0.0;
			dyn_decay_norm_total_hist_12K[f]=0.0;
			dyn_max_hist_12K[f]=0;
			dyn_norm_max_hist_12K[f]=0.0;
			dyn_norm_corrected_max_hist_12K[f]=0.0;
			dyn_decay_norm_max_hist_12K[f]=0.0;
			dyn_decay_norm_corrected_max_hist_12K[f]=0.0;
		}
		if (dyn_m13A!=-1) {
			dyn_hist_13A[f]=0;
			dyn_norm_hist_13A[f]=0.0;
			dyn_norm_corrected_hist_13A[f]=0.0;
			dyn_decay_norm_hist_13A[f]=0.0;
			dyn_decay_norm_corrected_hist_13A[f]=0.0;
			dyn_total_hist_13A[f]=0;
			dyn_norm_total_hist_13A[f]=0.0;
			dyn_decay_norm_total_hist_13A[f]=0.0;
			dyn_max_hist_13A[f]=0;
			dyn_norm_max_hist_13A[f]=0.0;
			dyn_norm_corrected_max_hist_13A[f]=0.0;
			dyn_decay_norm_max_hist_13A[f]=0.0;
			dyn_decay_norm_corrected_max_hist_13A[f]=0.0;
		}
		if (dyn_m13B!=-1) {
			dyn_hist_13B[f]=0;
			dyn_norm_hist_13B[f]=0.0;
			dyn_norm_corrected_hist_13B[f]=0.0;
			dyn_decay_norm_hist_13B[f]=0.0;
			dyn_decay_norm_corrected_hist_13B[f]=0.0;
			dyn_total_hist_13B[f]=0;
			dyn_norm_total_hist_13B[f]=0.0;
			dyn_decay_norm_total_hist_13B[f]=0.0;
			dyn_max_hist_13B[f]=0;
			dyn_norm_max_hist_13B[f]=0.0;
			dyn_norm_corrected_max_hist_13B[f]=0.0;
			dyn_decay_norm_max_hist_13B[f]=0.0;
			dyn_decay_norm_corrected_max_hist_13B[f]=0.0;
		}
		if (dyn_m13K!=-1) {
			dyn_hist_13K[f]=0;
			dyn_norm_hist_13K[f]=0.0;
			dyn_norm_corrected_hist_13K[f]=0.0;
			dyn_decay_norm_hist_13K[f]=0.0;
			dyn_decay_norm_corrected_hist_13K[f]=0.0;
			dyn_total_hist_13K[f]=0;
			dyn_norm_total_hist_13K[f]=0.0;
			dyn_decay_norm_total_hist_13K[f]=0.0;
			dyn_max_hist_13K[f]=0;
			dyn_norm_max_hist_13K[f]=0.0;
			dyn_norm_corrected_max_hist_13K[f]=0.0;
			dyn_decay_norm_max_hist_13K[f]=0.0;
			dyn_decay_norm_corrected_max_hist_13K[f]=0.0;
		}
		if (dyn_mFCC!=-1) {
			dyn_hist_FCC[f]=0;
			dyn_norm_hist_FCC[f]=0.0;
			dyn_norm_corrected_hist_FCC[f]=0.0;
			dyn_decay_norm_hist_FCC[f]=0.0;
			dyn_decay_norm_corrected_hist_FCC[f]=0.0;
			dyn_total_hist_FCC[f]=0;
			dyn_norm_total_hist_FCC[f]=0.0;
			dyn_decay_norm_total_hist_FCC[f]=0.0;
			dyn_max_hist_FCC[f]=0;
			dyn_norm_max_hist_FCC[f]=0.0;
			dyn_norm_corrected_max_hist_FCC[f]=0.0;
			dyn_decay_norm_max_hist_FCC[f]=0.0;
			dyn_decay_norm_corrected_max_hist_FCC[f]=0.0;
		}
		if (dyn_mHCP!=-1) {
			dyn_hist_HCP[f]=0;
			dyn_norm_hist_HCP[f]=0.0;
			dyn_norm_corrected_hist_HCP[f]=0.0;
			dyn_decay_norm_hist_HCP[f]=0.0;
			dyn_decay_norm_corrected_hist_HCP[f]=0.0;
			dyn_total_hist_HCP[f]=0;
			dyn_norm_total_hist_HCP[f]=0.0;
			dyn_decay_norm_total_hist_HCP[f]=0.0;
			dyn_max_hist_HCP[f]=0;
			dyn_norm_max_hist_HCP[f]=0.0;
			dyn_norm_corrected_max_hist_HCP[f]=0.0;
			dyn_decay_norm_max_hist_HCP[f]=0.0;
			dyn_decay_norm_corrected_max_hist_HCP[f]=0.0;
		}
		if (dyn_mBCC_9!=-1) {
			dyn_hist_BCC_9[f]=0;
			dyn_norm_hist_BCC_9[f]=0.0;
			dyn_norm_corrected_hist_BCC_9[f]=0.0;
			dyn_decay_norm_hist_BCC_9[f]=0.0;
			dyn_decay_norm_corrected_hist_BCC_9[f]=0.0;
			dyn_total_hist_BCC_9[f]=0;
			dyn_norm_total_hist_BCC_9[f]=0.0;
			dyn_decay_norm_total_hist_BCC_9[f]=0.0;
			dyn_max_hist_BCC_9[f]=0;
			dyn_norm_max_hist_BCC_9[f]=0.0;
			dyn_norm_corrected_max_hist_BCC_9[f]=0.0;
			dyn_decay_norm_max_hist_BCC_9[f]=0.0;
			dyn_decay_norm_corrected_max_hist_BCC_9[f]=0.0;
		}
		if (dyn_mBCC_15!=-1) {
			dyn_hist_BCC_15[f]=0;
			dyn_norm_hist_BCC_15[f]=0.0;
			dyn_norm_corrected_hist_BCC_15[f]=0.0;
			dyn_decay_norm_hist_BCC_15[f]=0.0;
			dyn_decay_norm_corrected_hist_BCC_15[f]=0.0;
			dyn_total_hist_BCC_15[f]=0;
			dyn_norm_total_hist_BCC_15[f]=0.0;
			dyn_decay_norm_total_hist_BCC_15[f]=0.0;
			dyn_max_hist_BCC_15[f]=0;
			dyn_norm_max_hist_BCC_15[f]=0.0;
			dyn_norm_corrected_max_hist_BCC_15[f]=0.0;
			dyn_decay_norm_max_hist_BCC_15[f]=0.0;
			dyn_decay_norm_corrected_max_hist_BCC_15[f]=0.0;
		}
	}
}


void Dyn_Analyse(int *dyn_a, int *dyn_e, int *dyn_n, int* **dyn_l, 
	int* *dyn_hist, double *sum_hist, double* *dyn_norm_hist, double* *dyn_norm_corrected_hist, double* *dyn_decay_norm_hist, double* *dyn_decay_norm_corrected_hist,
	int* *dyn_total_hist,  double* *dyn_norm_total_hist, double* *dyn_decay_norm_total_hist,
	int* *dyn_max_hist,  double *sum_max_hist, double* *dyn_norm_max_hist, double* *dyn_norm_corrected_max_hist,  double* *dyn_decay_norm_max_hist, double* *dyn_decay_norm_corrected_max_hist
	) {
	int f, i, j, temptotal;
	f=0;

	(*dyn_a)=(*dyn_e)=0;
	for (i=0; i<(*dyn_n); i++) {
		if ((*dyn_l)[i][0]>dyn_MaxLives) dyn_MaxLives=(*dyn_l)[i][0];

		temptotal=0;
		for (j=0; j<(*dyn_l)[i][0]; j++) {
			f=(*dyn_l)[i][2*j+2]-(*dyn_l)[i][2*j+1];
			(*dyn_hist)[f]++;
			(*dyn_e)=(*dyn_e)+1;
			temptotal+=f+1;
		}
		(*dyn_total_hist)[temptotal-1]++;
		
		f=(*dyn_l)[i][2*(*dyn_l)[i][0]]-(*dyn_l)[i][1];
		(*dyn_max_hist)[f]++;
		(*dyn_a)++;
	}
	
	for (f=0; f<FRAMES; f++) {		
		(*dyn_norm_corrected_hist)[f]=(double)(*dyn_hist)[f]/(FRAMES-f);
		(*sum_hist)=(*sum_hist)+(*dyn_norm_corrected_hist)[f];
		(*dyn_norm_corrected_max_hist)[f]=(double)(*dyn_max_hist)[f]/(FRAMES-f);
		(*sum_max_hist)=(*sum_max_hist)+(*dyn_norm_corrected_max_hist)[f];
	}
	
	for (f=0; f<FRAMES; f++) {
		if ((*dyn_e)!=0) (*dyn_norm_hist)[f]=(double)(*dyn_hist)[f]/(*dyn_e);
		if ((*sum_hist)!=0) (*dyn_norm_corrected_hist)[f]/=(*sum_hist);
		if ((*dyn_a)!=0) (*dyn_norm_total_hist)[f]=(double)(*dyn_total_hist)[f]/(*dyn_a);
		if ((*dyn_a)!=0) (*dyn_norm_max_hist)[f]=(double)(*dyn_max_hist)[f]/(*dyn_a);
		if ((*sum_max_hist)!=0) (*dyn_norm_corrected_max_hist)[f]/=(*sum_max_hist);
	}
	
	(*dyn_decay_norm_hist)[FRAMES-1]=(*dyn_norm_hist)[FRAMES-1];
	(*dyn_decay_norm_corrected_hist)[FRAMES-1]=(*dyn_norm_corrected_hist)[FRAMES-1];
	(*dyn_decay_norm_total_hist)[FRAMES-1]=(*dyn_norm_total_hist)[FRAMES-1];
	(*dyn_decay_norm_max_hist)[FRAMES-1]=(*dyn_norm_max_hist)[FRAMES-1];
	(*dyn_decay_norm_corrected_max_hist)[FRAMES-1]=(*dyn_norm_corrected_max_hist)[FRAMES-1];
	for (f=FRAMES-2; f>=0; f--) {
		(*dyn_decay_norm_hist)[f]=(*dyn_norm_hist)[f]+(*dyn_decay_norm_hist)[f+1];
		(*dyn_decay_norm_corrected_hist)[f]=(*dyn_norm_corrected_hist)[f]+(*dyn_decay_norm_corrected_hist)[f+1];
		(*dyn_decay_norm_total_hist)[f]=(*dyn_norm_total_hist)[f]+(*dyn_decay_norm_total_hist)[f+1];
		(*dyn_decay_norm_max_hist)[f]=(*dyn_norm_max_hist)[f]+(*dyn_decay_norm_max_hist)[f+1];
		(*dyn_decay_norm_corrected_max_hist)[f]=(*dyn_norm_corrected_max_hist)[f]+(*dyn_decay_norm_corrected_max_hist)[f+1];
	}
	
	dyn_TotClus+=(*dyn_n);
	dyn_TotEvents+=(*dyn_e);
	dyn_Tot_a_Events+=(*dyn_a);
}

void Dyn_Report(char *filename) {
	char errMsg[1000];
	FILE *writeout;
	
	writeout=fopen(filename,"w");
	if (writeout==NULL)  {
		sprintf(errMsg,"Dyn_Report(): Error opening file %s",filename);	// Always test file open
		Error(errMsg);
	}

	printf("d%d Clust	Uniq_#	Events_#	Av_Events_#\n",rank);
	if (dyn_msp3!=-1) printf("d%d sp3	%d	%d	%d\n",rank,dyn_nsp3,dyn_esp3,dyn_asp3);
	if (dyn_msp3a!=-1) printf("d%d sp3a	%d	%d	%d\n",rank,dyn_nsp3a,dyn_esp3a,dyn_asp3a);
	if (dyn_msp3b!=-1) printf("d%d sp3b	%d	%d	%d\n",rank,dyn_nsp3b,dyn_esp3b,dyn_asp3b);
	if (dyn_msp3c!=-1) printf("d%d 5A_D3h	%d	%d	%d\n",rank,dyn_nsp3c,dyn_esp3c,dyn_asp3c);
	if (dyn_msp4!=-1) printf("d%d sp4	%d	%d	%d\n",rank,dyn_nsp4,dyn_esp4,dyn_asp4);
	if (dyn_msp4a!=-1) printf("d%d sp4a	%d	%d	%d\n",rank,dyn_nsp4a,dyn_esp4a,dyn_asp4a);
	if (dyn_msp4b!=-1) printf("d%d sp4b	%d	%d	%d\n",rank,dyn_nsp4b,dyn_esp4b,dyn_asp4b);
	if (dyn_m6A!=-1) printf("d%d 6A_Oh	%d	%d	%d\n",rank,dyn_n6A,dyn_e6A,dyn_a6A); 
	if (dyn_m6Z!=-1) printf("d%d 6Z_C2v	%d	%d	%d\n",rank,dyn_n6Z,dyn_e6Z,dyn_a6Z);
	if (dyn_msp5!=-1) printf("d%d sp5	%d	%d	%d\n",rank,dyn_nsp5,dyn_esp5,dyn_asp5);
	if (dyn_msp5a!=-1) printf("d%d sp5a	%d	%d	%d\n",rank,dyn_nsp5a,dyn_esp5a,dyn_asp5a);
	if (dyn_msp5b!=-1) printf("d%d sp5b	%d	%d	%d\n",rank,dyn_nsp5b,dyn_esp5b,dyn_asp5b);
	if (dyn_msp5c!=-1) printf("d%d 7A_D5h	%d	%d	%d\n",rank,dyn_nsp5c,dyn_esp5c,dyn_asp5c);
	if (dyn_m7K!=-1) printf("d%d 7K	%d	%d	%d\n",rank,dyn_n7K,dyn_e7K,dyn_a7K);
	if (dyn_m8A!=-1) printf("d%d 8A_D2d	%d	%d	%d\n",rank,dyn_n8A,dyn_e8A,dyn_a8A);
	if (dyn_m8B!=-1) printf("d%d 8B_Cs	%d	%d	%d\n",rank,dyn_n8B,dyn_e8B,dyn_a8B);
	if (dyn_m8K!=-1) printf("d%d 8K	%d	%d	%d\n",rank,dyn_n8K,dyn_e8K,dyn_a8K);
	if (dyn_m9A!=-1) printf("d%d 9A_D3h	%d	%d	%d\n",rank,dyn_n9A,dyn_e9A,dyn_a9A);
	if (dyn_m9B!=-1) printf("d%d 9B_C2v	%d	%d	%d\n",rank,dyn_n9B,dyn_e9B,dyn_a9B);
	if (dyn_m9K!=-1) printf("d%d 9K	%d	%d	%d\n",rank,dyn_n9K,dyn_e9K,dyn_a9K);
	if (dyn_m10A!=-1) printf("d%d 10A_D4d	%d	%d	%d\n",rank,dyn_n10A,dyn_e10A,dyn_a10A);
	if (dyn_m10B!=-1) printf("d%d 10B_C3v	%d	%d	%d\n",rank,dyn_n10B,dyn_e10B,dyn_a10B);
	if (dyn_m10K!=-1) printf("d%d 10K	%d	%d	%d\n",rank,dyn_n10K,dyn_e10K,dyn_a10K);
	if (dyn_m10W!=-1) printf("d%d 10W	%d	%d	%d\n",rank,dyn_n10W,dyn_e10W,dyn_a10W);
	if (dyn_m11A!=-1) printf("d%d 11A_D4d	%d	%d	%d\n",rank,dyn_n11A,dyn_e11A,dyn_a11A);
	if (dyn_m11B!=-1) printf("d%d 11B_C2v	%d	%d	%d\n",rank,dyn_n11B,dyn_e11B,dyn_a11B);
	if (dyn_m11C!=-1) printf("d%d 11CD	%d	%d	%d\n",rank,dyn_n11C,dyn_e11C,dyn_a11C);
	if (dyn_m11E!=-1) printf("d%d 11E_C2	%d	%d	%d\n",rank,dyn_n11E,dyn_e11E,dyn_a11E);
	if (dyn_m11F!=-1) printf("d%d 11F_C2v	%d	%d	%d\n",rank,dyn_n11F,dyn_e11F,dyn_a11F);
	if (dyn_m11W!=-1) printf("d%d 11W_Cs	%d	%d	%d\n",rank,dyn_n11W,dyn_e11W,dyn_a11W);
	if (dyn_m12A!=-1) printf("d%d 12A_C2v	%d	%d	%d\n",rank,dyn_n12A,dyn_e12A,dyn_a12A);
	if (dyn_m12B!=-1) printf("d%d 12BC	%d	%d	%d\n",rank,dyn_n12B,dyn_e12B,dyn_a12B);
	if (dyn_m12D!=-1) printf("d%d 12D_D2d	%d	%d	%d\n",rank,dyn_n12D,dyn_e12D,dyn_a12D);
	if (dyn_m12E!=-1) printf("d%d 12E_D3h	%d	%d	%d\n",rank,dyn_n12E,dyn_e12E,dyn_a12E);
	if (dyn_m12K!=-1) printf("d%d 12K	%d	%d	%d\n",rank,dyn_n12K,dyn_e12K,dyn_a12K);
	if (dyn_m13A!=-1) printf("d%d 13A_Ih	%d	%d	%d\n",rank,dyn_n13A,dyn_e13A,dyn_a13A);
	if (dyn_m13B!=-1) printf("d%d 13B_D5h	%d	%d	%d\n",rank,dyn_n13B,dyn_e13B,dyn_a13B);
	if (dyn_m13K!=-1) printf("d%d 13K	%d	%d	%d\n",rank,dyn_n13K,dyn_e13K,dyn_a13K);
	if (dyn_mFCC!=-1) printf("d%d FCC_m13	%d	%d	%d\n",rank,dyn_nFCC,dyn_eFCC,dyn_aFCC);
	if (dyn_mHCP!=-1) printf("d%d HCP_m13	%d	%d	%d\n",rank,dyn_nHCP,dyn_eHCP,dyn_aHCP);
	if (dyn_mBCC_9!=-1) printf("d%d BCC_m9	%d	%d	%d\n",rank,dyn_nBCC_9,dyn_eBCC_9,dyn_aBCC_9);
	if (dyn_mBCC_15!=-1) printf("d%d BCC_m15	%d	%d	%d\n",rank,dyn_nBCC_15,dyn_eBCC_15,dyn_aBCC_15);
	printf("d%d totals	%d	%d	%d\n",rank,dyn_TotClus,dyn_TotEvents,dyn_Tot_a_Events);
	printf("d%d Maximum number of dynamic events for any unique cluster	%d\n",rank,dyn_MaxLives);
	
	fprintf(writeout,"%s\n",filename);
	fprintf(writeout,"Clust	Uniq_#	Events_#	Av_Events_#\n");
	if (dyn_msp3!=-1) fprintf(writeout,"sp3	%d	%d	%d\n",dyn_nsp3,dyn_esp3,dyn_asp3);
	if (dyn_msp3a!=-1) fprintf(writeout,"sp3a	%d	%d	%d\n",dyn_nsp3a,dyn_esp3a,dyn_asp3a);
	if (dyn_msp3b!=-1) fprintf(writeout,"sp3b	%d	%d	%d\n",dyn_nsp3b,dyn_esp3b,dyn_asp3b);
	if (dyn_msp3c!=-1) fprintf(writeout,"5A_D3h	%d	%d	%d\n",dyn_nsp3c,dyn_esp3c,dyn_asp3c);
	if (dyn_msp4!=-1) fprintf(writeout,"sp4	%d	%d	%d\n",dyn_nsp4,dyn_esp4,dyn_asp4);
	if (dyn_msp4a!=-1) fprintf(writeout,"sp4a	%d	%d	%d\n",dyn_nsp4a,dyn_esp4a,dyn_asp4a);
	if (dyn_msp4b!=-1) fprintf(writeout,"sp4b	%d	%d	%d\n",dyn_nsp4b,dyn_esp4b,dyn_asp4b);
	if (dyn_m6A!=-1) fprintf(writeout,"6A_Oh	%d	%d	%d\n",dyn_n6A,dyn_e6A,dyn_a6A); 
	if (dyn_m6Z!=-1) fprintf(writeout,"6Z_C2v	%d	%d	%d\n",dyn_n6Z,dyn_e6Z,dyn_a6Z);
	if (dyn_msp5!=-1) fprintf(writeout,"sp5	%d	%d	%d\n",dyn_nsp5,dyn_esp5,dyn_asp5);
	if (dyn_msp5a!=-1) fprintf(writeout,"sp5a	%d	%d	%d\n",dyn_nsp5a,dyn_esp5a,dyn_asp5a);
	if (dyn_msp5b!=-1) fprintf(writeout,"sp5b	%d	%d	%d\n",dyn_nsp5b,dyn_esp5b,dyn_asp5b);
	if (dyn_msp5c!=-1) fprintf(writeout,"7A_D5h	%d	%d	%d\n",dyn_nsp5c,dyn_esp5c,dyn_asp5c);
	if (dyn_m7K!=-1) fprintf(writeout,"7K	%d	%d	%d\n",dyn_n7K,dyn_e7K,dyn_a7K);
	if (dyn_m8A!=-1) fprintf(writeout,"8A_D2d	%d	%d	%d\n",dyn_n8A,dyn_e8A,dyn_a8A);
	if (dyn_m8B!=-1) fprintf(writeout,"8B_Cs	%d	%d	%d\n",dyn_n8B,dyn_e8B,dyn_a8B);
	if (dyn_m8K!=-1) fprintf(writeout,"8K	%d	%d	%d\n",dyn_n8K,dyn_e8K,dyn_a8K);
	if (dyn_m9A!=-1) fprintf(writeout,"9A_D3h	%d	%d	%d\n",dyn_n9A,dyn_e9A,dyn_a9A);
	if (dyn_m9B!=-1) fprintf(writeout,"9B_C2v	%d	%d	%d\n",dyn_n9B,dyn_e9B,dyn_a9B);
	if (dyn_m9K!=-1) fprintf(writeout,"9K	%d	%d	%d\n",dyn_n9K,dyn_e9K,dyn_a9K);
	if (dyn_m10A!=-1) fprintf(writeout,"10A_D4d	%d	%d	%d\n",dyn_n10A,dyn_e10A,dyn_a10A);
	if (dyn_m10B!=-1) fprintf(writeout,"10B_C3v	%d	%d	%d\n",dyn_n10B,dyn_e10B,dyn_a10B);
	if (dyn_m10K!=-1) fprintf(writeout,"10K	%d	%d	%d\n",dyn_n10K,dyn_e10K,dyn_a10K);
	if (dyn_m10W!=-1) fprintf(writeout,"10W	%d	%d	%d\n",dyn_n10W,dyn_e10W,dyn_a10W);
	if (dyn_m11A!=-1) fprintf(writeout,"11A_D4d	%d	%d	%d\n",dyn_n11A,dyn_e11A,dyn_a11A);
	if (dyn_m11B!=-1) fprintf(writeout,"11B_C2v	%d	%d	%d\n",dyn_n11B,dyn_e11B,dyn_a11B);
	if (dyn_m11C!=-1) fprintf(writeout,"11CD	%d	%d	%d\n",dyn_n11C,dyn_e11C,dyn_a11C);
	if (dyn_m11E!=-1) fprintf(writeout,"11E_C2	%d	%d	%d\n",dyn_n11E,dyn_e11E,dyn_a11E);
	if (dyn_m11F!=-1) fprintf(writeout,"11F_C2v	%d	%d	%d\n",dyn_n11F,dyn_e11F,dyn_a11F);
	if (dyn_m11W!=-1) fprintf(writeout,"11W_Cs	%d	%d	%d\n",dyn_n11W,dyn_e11W,dyn_a11W);
	if (dyn_m12A!=-1) fprintf(writeout,"12A_C2v	%d	%d	%d\n",dyn_n12A,dyn_e12A,dyn_a12A);
	if (dyn_m12B!=-1) fprintf(writeout,"12BC	%d	%d	%d\n",dyn_n12B,dyn_e12B,dyn_a12B);
	if (dyn_m12D!=-1) fprintf(writeout,"12D_D2d	%d	%d	%d\n",dyn_n12D,dyn_e12D,dyn_a12D);
	if (dyn_m12E!=-1) fprintf(writeout,"12E_D3h	%d	%d	%d\n",dyn_n12E,dyn_e12E,dyn_a12E);
	if (dyn_m12K!=-1) fprintf(writeout,"12K	%d	%d	%d\n",dyn_n12K,dyn_e12K,dyn_a12K);
	if (dyn_m13A!=-1) fprintf(writeout,"13A_Ih	%d	%d	%d\n",dyn_n13A,dyn_e13A,dyn_a13A);
	if (dyn_m13B!=-1) fprintf(writeout,"13B_D5h	%d	%d	%d\n",dyn_n13B,dyn_e13B,dyn_a13B);
	if (dyn_m13K!=-1) fprintf(writeout,"13K	%d	%d	%d\n",dyn_n13K,dyn_e13K,dyn_a13K);
	if (dyn_mFCC!=-1) fprintf(writeout,"FCC_m13	%d	%d	%d\n",dyn_nFCC,dyn_eFCC,dyn_aFCC);
	if (dyn_mHCP!=-1) fprintf(writeout,"HCP_m13	%d	%d	%d\n",dyn_nHCP,dyn_eHCP,dyn_aHCP);
	if (dyn_mBCC_9!=-1) fprintf(writeout,"BCC_m9	%d	%d	%d\n",dyn_nBCC_9,dyn_eBCC_9,dyn_aBCC_9);
	if (dyn_mBCC_15!=-1) fprintf(writeout,"BCC_m15	%d	%d	%d\n",dyn_nBCC_15,dyn_eBCC_15,dyn_aBCC_15);
	fprintf(writeout,"totals	%d	%d	%d\n",dyn_TotClus,dyn_TotEvents,dyn_Tot_a_Events);
	fprintf(writeout,"Maximum number of dynamic events for any unique cluster	%d\n",dyn_MaxLives);
	
	fclose(writeout);
}
//// START: Dynamics routines

//// START: Stats routines
void Stats_Init() {
	int i;
	char errMsg[1000];
	
	a5=malloc(N*sizeof(int));	if (a5==NULL) { sprintf(errMsg,"Stats_Init(): a5[] malloc out of memory\n");	Error(errMsg); }
	a6=malloc(N*sizeof(int));	if (a6==NULL) { sprintf(errMsg,"Stats_Init(): a6[] malloc out of memory\n");	Error(errMsg); }
	a7=malloc(N*sizeof(int));	if (a7==NULL) { sprintf(errMsg,"Stats_Init(): a7[] malloc out of memory\n");	Error(errMsg); }
	a8=malloc(N*sizeof(int));	if (a8==NULL) { sprintf(errMsg,"Stats_Init(): a8[] malloc out of memory\n");	Error(errMsg); }
	a9=malloc(N*sizeof(int));	if (a9==NULL) { sprintf(errMsg,"Stats_Init(): a9[] malloc out of memory\n");	Error(errMsg); }
	a10=malloc(N*sizeof(int));	if (a10==NULL) { sprintf(errMsg,"Stats_Init(): a10[] malloc out of memory\n");	Error(errMsg); }
	a11=malloc(N*sizeof(int));	if (a11==NULL) { sprintf(errMsg,"Stats_Init(): a11[] malloc out of memory\n");	Error(errMsg); }
	a12=malloc(N*sizeof(int));	if (a12==NULL) { sprintf(errMsg,"Stats_Init(): a12[] malloc out of memory\n");	Error(errMsg); }
	a13=malloc(N*sizeof(int));	if (a13==NULL) { sprintf(errMsg,"Stats_Init(): a13[] malloc out of memory\n");	Error(errMsg); }
	a15=malloc(N*sizeof(int));	if (a15==NULL) { sprintf(errMsg,"Stats_Init(): a15[] malloc out of memory\n");	Error(errMsg); }
	
	for(i=0;i<N;i++) {
		a5[i]=a6[i]=a7[i]=a8[i]=a9[i]=a10[i]=a11[i]=a12[i]=a13[i]=a15[i]=0;
	}
	
	ngsp3=ngsp3a=ngsp3b=ng5A=0;
	ngsp4=ngsp4a=ngsp4b=ng6A=0;
	ngsp5=ngsp5a=ngsp5b=ng7A=0;
	ng6Z=ng7K=0;
	ng8A=ng8B=ng8K=0;
	ng9A=ng9B=ng9K=0;
	ng10A=ng10B=ng10K=ng10W=0;
	ng11A=ng11B=ng11C=ng11E=ng11F=ng11W=0;	// number of particles in clusers gross
	ng12A=ng12B=ng12D=ng12E=ng12K=0;
	ng13A=ng13B=ng13K=0;
	ngFCC=ngHCP=ngBCC_9=ngBCC_15=0;
	
	nn5A=0;
	nn6A=0;
	nn7A=0;
	nn6Z=nn7K=0;
	nn8A=nn8B=nn8K=0;
	nn9A=nn9B=nn9K=0;
	nn10A=nn10B=nn10K=nn10W=0;
	nn11A=nn11B=nn11C=nn11E=nn11F=nn11W=0;	// number of particles in clusers gross
	nn12A=nn12B=nn12D=nn12E=nn12K=0;
	nn13A=nn13B=nn13K=0;
	nnFCC=nnHCP=nnBCC_9=nnBCC_15=0;

	ncsp3=ncsp3a=ncsp3b=nc5A=0;
	ncsp4=ncsp4a=ncsp4b=nc6A=0;
	ncsp5=ncsp5a=ncsp5b=nc7A=0;
	nc6Z=nc7K=0;
	nc8A=nc8B=nc8K=0;
	nc9A=nc9B=nc9K=0;
	nc10A=nc10B=nc10K=nc10W=0;
	nc11A=nc11B=nc11C=nc11E=nc11F=nc11W=0;	// number of particles in clusers gross
	nc12A=nc12B=nc12D=nc12E=nc12K=0;
	nc13A=nc13B=nc13K=0;
	ncFCC=ncHCP=ncBCC_9=ncBCC_15=0;
	
	ncsp3_excess_spindles=ncsp4_excess_spindles=ncsp5_excess_spindles=0;	// total number of _excess_spindlesed basic clusters
	ncsp3c_spindlebonds=ncsp4c_spindlebonds=nc6A_spindlebonds=ncsp5c_spindlebonds=0;
}

void Stats_Reset() {
	int i;
	
	for(i=0;i<N;i++) {
		a5[i]=a6[i]=a7[i]=a8[i]=a9[i]=a10[i]=a11[i]=a12[i]=a13[i]=a15[i]=0;
	}	
}


void Stats_FreeMem() {	
	free(a5);
	free(a6);
	free(a7);
	free(a8);
	free(a9);
	free(a10);
	free(a11);
	free(a12);	
	free(a13);
	free(a15);
}

void Stats_Analyse() {
	int i, Nclus;
	int flg;
   
	for(i=0; i<N; ++i){
		if(ssp3[i] != 'C') ++ngsp3;
		if(ssp3a[i] != 'C') ++ngsp3a;
		if(ssp3b[i] != 'C') ++ngsp3b;
		if(s5A[i] != 'C') ++ng5A;
		if(ssp4[i] != 'C') ++ngsp4;
		if(ssp4a[i] != 'C') ++ngsp4a;
		if(ssp4b[i] != 'C') ++ngsp4b;
		if(s6A[i] != 'C') ++ng6A;
		if(s6Z[i] != 'C') ++ng6Z;
		if(s7K[i] != 'C') ++ng7K;
		if(ssp5[i] != 'C') ++ngsp5;
		if(ssp5a[i] != 'C') ++ngsp5a;
		if(ssp5b[i] != 'C') ++ngsp5b;
		if(s7A[i] != 'C') ++ng7A; 
		if(s8A[i] != 'C') ++ng8A;
		if(s8B[i] != 'C') ++ng8B;
		if(s8K[i] != 'C') ++ng8K;
		if(s9A[i] != 'C') ++ng9A;
		if(s9B[i] != 'C') ++ng9B;
		if(s9K[i] != 'C') ++ng9K;
		if(s10A[i] != 'C') ++ng10A;
		if(s10B[i] != 'C') ++ng10B;
		if(s10K[i] != 'C') ++ng10K;
		if(s10W[i] != 'C') ++ng10W;
		if(s11A[i] != 'C') ++ng11A;
		if(s11B[i] != 'C') ++ng11B;
		if(s11C[i] != 'C') ++ng11C;
		if(s11E[i] != 'C') ++ng11E;
		if(s11F[i] != 'C') ++ng11F;
		if(s11W[i] != 'C') ++ng11W;
		if(s12A[i] != 'C') ++ng12A;
		if(s12B[i] != 'C') ++ng12B;
		if(s12D[i] != 'C') ++ng12D;
		if(s12E[i] != 'C') ++ng12E;
		if(s12K[i] != 'C') ++ng12K;
		if(s13A[i] != 'C') ++ng13A;
		if(s13B[i] != 'C') ++ng13B;
		if(s13K[i] != 'C') ++ng13K;
		if(sFCC[i] != 'C') ++ngFCC;
		if(sHCP[i] != 'C') ++ngHCP;
		if(sBCC_9[i] != 'C') ++ngBCC_9;
		if(sBCC_15[i] != 'C') ++ngBCC_15;		
	}
	Stats_SetA();
	for(i=0; i<N; ++i){
		if(s5A[i] != 'C' && !a6[i]) ++nn5A;
		if(s6A[i] != 'C' && !a7[i]) ++nn6A;
		if(s6Z[i] != 'C' && !a7[i]) ++nn6Z;
		if(s7K[i] != 'C' && !a7[i]) ++nn7K;
		if(s7A[i] != 'C' && !a8[i]) ++nn7A; 
		if(s8A[i] != 'C' && !a9[i]) ++nn8A;
		if(s8B[i] != 'C' && !a9[i]) ++nn8B;
		if(s8K[i] != 'C' && !a9[i]) ++nn8K;
		if(s9A[i] != 'C' && !a10[i]) ++nn9A;
		if(s9B[i] != 'C' && !a10[i]) ++nn9B;
		if(s9K[i] != 'C' && !a10[i]) ++nn9K;
		if(s10A[i] != 'C' && !a11[i]) ++nn10A;
		if(s10B[i] != 'C' && !a11[i]) ++nn10B;
		if(s10K[i] != 'C' && !a11[i]) ++nn10K;
		if(s10W[i] != 'C' && !a11[i]) ++nn10W;
		if(s11A[i] != 'C' && !a12[i]) ++nn11A;
		if(s11B[i] != 'C' && !a12[i]) ++nn11B;
		if(s11C[i] != 'C' && !a12[i]) ++nn11C;
		if(s11E[i] != 'C' && !a12[i]) ++nn11E;
		if(s11F[i] != 'C' && !a12[i]) ++nn11F;
		if(s11W[i] != 'C' && !a12[i]) ++nn11W;
		if(s12A[i] != 'C' && !a13[i]) ++nn12A;
		if(s12B[i] != 'C' && !a13[i]) ++nn12B;
		if(s12D[i] != 'C' && !a13[i]) ++nn12D;
		if(s12E[i] != 'C' && !a13[i]) ++nn12E;
		if(s12K[i] != 'C' && !a13[i]) ++nn12K;
		if(s13A[i] != 'C') ++nn13A;
		if(s13B[i] != 'C') ++nn13B;
		if(s13K[i] != 'C') ++nn13K;
		if(sFCC[i] != 'C') ++nnFCC;
		if(sHCP[i] != 'C') ++nnHCP; 
		if(sBCC_9[i] != 'C') ++nnBCC_9;
		if(sBCC_15[i] != 'C') ++nnBCC_15;
	}
	Nclus = 0;
	for(i=0; i<N; ++i){
		flg = 0;
		if(s5A[i] != 'C') flg=1;
		if(s6A[i] != 'C') flg=1;
		if(s6Z[i] != 'C') flg=1;
		if(s7K[i] != 'C') flg=1;
		if(s7A[i] != 'C') flg=1;
		if(s8A[i] != 'C') flg=1;
		if(s8B[i] != 'C') flg=1;
		if(s8K[i] != 'C') flg=1;
		if(s9A[i] != 'C') flg=1;
		if(s9B[i] != 'C') flg=1;
		if(s9K[i] != 'C') flg=1;
		if(s10A[i] != 'C') flg=1;
		if(s10B[i] != 'C') flg=1;
		if(s10K[i] != 'C') flg=1;
		if(s10W[i] != 'C') flg=1;
		if(s11A[i] != 'C') flg=1;
		if(s11B[i] != 'C') flg=1;
		if(s11C[i] != 'C') flg=1;
		if(s11E[i] != 'C') flg=1;
		if(s11F[i] != 'C') flg=1;
		if(s11W[i] != 'C') flg=1;
		if(s12A[i] != 'C') flg=1;
		if(s12B[i] != 'C') flg=1;
		if(s12D[i] != 'C') flg=1;
		if(s12E[i] != 'C') flg=1;
		if(s12K[i] != 'C') flg=1;
		if(s13A[i] != 'C') flg=1;
		if(s13B[i] != 'C') flg=1;
		if(s13K[i] != 'C') flg=1;
		if(sFCC[i] != 'C') flg=1;
		if(sHCP[i] != 'C') flg=1;
		if(sBCC_9[i] != 'C') flg=1;
		if(sBCC_15[i] != 'C') flg=1;
		if(flg) ++Nclus;
	}
	totNclus+=Nclus;
	if (PRINTINFO==1) printf("d%d Nclus = %d\n",rank,Nclus);
}

void Stats_SetA() { // Set arrays to true if the ith particle is a member of any clusters with this or a larger number of particles
	int flg1, flg2;
	int i;
	
	for (i=0;i<N;i++) { 
		flg1 = sFCC[i] != 'C' || sHCP[i] != 'C' || sBCC_15[i] != 'C' || s13A[i] != 'C' || s13B[i] != 'C' || s13K[i] != 'C';
		if(flg1==1) a13[i] = 1; 
	}
	for (i=0;i<N;i++) { 
		flg1 = s12A[i] != 'C' || s12B[i] != 'C' || s12D[i] != 'C' || s12E[i] != 'C' || s12K[i] != 'C';
		flg2 = a13[i];
		if(flg1==1 || flg2==1) a12[i] = 1;
	}
	for (i=0;i<N;i++) {  
		flg1 = s11A[i] != 'C' || s11B[i] != 'C' || s11C[i] != 'C' || s11E[i] != 'C' || s11F[i] != 'C' || s11W[i] != 'C';
		flg2 = a12[i];
		if(flg1==1 || flg2==1) a11[i] = 1;
	}
	for (i=0;i<N;i++) { 
		flg1 = s10A[i] != 'C' || s10B[i] != 'C' || s10K[i] != 'C' || s10W[i] != 'C';
		flg2 = a11[i];
		if(flg1==1 || flg2==1) a10[i] = 1;
	}	
	for (i=0;i<N;i++) { 
		flg1 = s9A[i] != 'C' || s9B[i] != 'C' || s9K[i] != 'C' || sBCC_9[i] != 'C';
		flg2 = a10[i];
		if(flg1==1 || flg2==1) a9[i] = 1;
	}
	for (i=0;i<N;i++) { 
		flg1 = s8A[i] != 'C' || s8B[i] != 'C' || s8K[i] != 'C';
		flg2 = a9[i];
		if(flg1==1 || flg2==1) a8[i] = 1;
	}
	for (i=0;i<N;i++) {  
		flg1 = s7A[i] != 'C' || s7K[i] != 'C';
		flg2 = a8[i];
		if(flg1==1 || flg2==1) a7[i] = 1;
	}
	for (i=0;i<N;i++) {  
		flg1 = s6A[i] != 'C' || s6Z[i] != 'C';
		flg2 = a7[i];
		if(flg1==1 || flg2==1) a6[i] = 1;
	}
}

void Stats_Report(char *filename) {
	int f;
	int clusTot=0;
	double temp;
	char errMsg[1000];
	FILE *writeout;
	
	for (f=0; f<FRAMES; f++) {
		ncsp3+=nsp3[f];
		ncsp3a+=nsp3a[f];
		ncsp3b+=nsp3b[f];
		nc5A+=nsp3c[f];
		ncsp3_excess_spindles+=nsp3_excess_spindles[f];
		ncsp3c_spindlebonds+=nsp3c_spindlebonds[f];
		ncsp4+=nsp4[f];
		ncsp4a+=nsp4a[f];
		ncsp4b+=nsp4b[f];
		ncsp4c+=nsp4c[f];
		ncsp4_excess_spindles+=nsp4_excess_spindles[f];
		ncsp4c_spindlebonds+=nsp4c_spindlebonds[f];
		nc6A+=n6A[f];
		nc6A_spindlebonds+=n6A_spindlebonds[f];
		nc6Z+=n6Z[f];
		nc7K+=n7K[f];
		ncsp5+=nsp5[f];
		ncsp5a+=nsp5a[f];
		ncsp5b+=nsp5b[f];
		nc7A+=nsp5c[f];
		ncsp5_excess_spindles+=nsp5_excess_spindles[f];
		ncsp5c_spindlebonds+=nsp5c_spindlebonds[f];
		nc8A+=n8A[f];
		nc8B+=n8B[f];
		nc8K+=n8K[f];
		nc9A+=n9A[f];
		nc9B+=n9B[f];
		nc9K+=n9K[f];
		nc10A+=n10A[f];
		nc10B+=n10B[f];
		nc10K+=n10K[f];
		nc10W+=n10W[f];
		nc11A+=n11A[f];
		nc11B+=n11B[f];
		nc11C+=n11C[f];
		nc11E+=n11E[f];
		nc11F+=n11F[f];
		nc11W+=n11W[f];
		nc12A+=n12A[f];
		nc12B+=n12B[f];
		nc12D+=n12D[f];
		nc12E+=n12E[f];
		nc12K+=n12K[f];
		nc13A+=n13A[f];
		nc13B+=n13B[f];
		nc13K+=n13K[f];
		ncFCC+=nFCC[f];
		ncHCP+=nHCP[f];
		ncBCC_9+=nBCC_9[f];
		ncBCC_15+=nBCC_15[f];
		clusTot+=nsp3a[f]+nsp3b[f]+nsp3c[f]+nsp4a[f]+nsp4b[f]+n6A[f]+nsp5a[f]+nsp5b[f]+nsp5c[f];
		clusTot+=n6Z[f]+n7K[f]+n8A[f]+n8B[f]+n8K[f]+n9A[f]+n9B[f]+n9K[f]+n10A[f]+n10B[f]+n10K[f]+n10W[f];
		clusTot+=n11A[f]+n11B[f]+n11C[f]+n11E[f]+n11F[f]+n11W[f];
		clusTot+=n12A[f]+n12B[f]+n12D[f]+n12E[f]+n12K[f];
		clusTot+=n13A[f]+n13B[f]+n13K[f]+nFCC[f]+nHCP[f]+nBCC_9[f]+nBCC_15[f];
	}
	
	printf("d%d Clust	Number	Gross	Net	Mean Pop Per Frame\n",rank);
	printf("d%d sp3	%d	%d	%d	%.5lg\n",rank,ncsp3,ngsp3,0,mean_pop_per_frame_sp3);
	printf("d%d sp3a	%d	%d	%d	%.5lg\n",rank,ncsp3a,ngsp3a,0,mean_pop_per_frame_sp3a);
	printf("d%d sp3b	%d	%d	%d	%.5lg\n",rank,ncsp3b,ngsp3b,0,mean_pop_per_frame_sp3b);
	printf("d%d 5A_D3h	%d	%d	%d	%.5lg\n",rank,nc5A,ng5A,nn5A,mean_pop_per_frame_sp3c);
	printf("d%d sp4	%d	%d	%d	%.5lg\n",rank,ncsp4,ngsp4,0,mean_pop_per_frame_sp4);
	printf("d%d sp4a	%d	%d	%d	%.5lg\n",rank,ncsp4a,ngsp4a,0,mean_pop_per_frame_sp4a);
	printf("d%d sp4b	%d	%d	%d	%.5lg\n",rank,ncsp4b,ngsp4b,0,mean_pop_per_frame_sp4b);
	printf("d%d sp4c	%d	%d	%d	%.5lg\n",rank,ncsp4c,ng6A,0,mean_pop_per_frame_6A); 
	printf("d%d 6A_Oh	%d	%d	%d	%.5lg\n",rank,nc6A,ng6A,nn6A,mean_pop_per_frame_6A); 
	printf("d%d 6Z_C2v	%d	%d	%d	%.5lg\n",rank,nc6Z,ng6Z,nn6Z,mean_pop_per_frame_6Z);
	printf("d%d sp5	%d	%d	%d	%.5lg\n",rank,ncsp5,ngsp5,0,mean_pop_per_frame_sp5);
	printf("d%d sp5a	%d	%d	%d	%.5lg\n",rank,ncsp5a,ngsp5a,0,mean_pop_per_frame_sp5a);
	printf("d%d sp5b	%d	%d	%d	%.5lg\n",rank,ncsp5b,ngsp5b,0,mean_pop_per_frame_sp5b);
	printf("d%d 7A_D5h	%d	%d	%d	%.5lg\n",rank,nc7A,ng7A,nn7A,mean_pop_per_frame_sp5c);
	printf("d%d 7K	%d	%d	%d	%.5lg\n",rank,nc7K,ng7K,nn7K,mean_pop_per_frame_7K);
	printf("d%d 8A_D2d	%d	%d	%d	%.5lg\n",rank,nc8A,ng8A,nn8A,mean_pop_per_frame_8A);
	printf("d%d 8B_Cs	%d	%d	%d	%.5lg\n",rank,nc8B,ng8B,nn8B,mean_pop_per_frame_8B);
	printf("d%d 8K	%d	%d	%d	%.5lg\n",rank,nc8K,ng8K,nn8K,mean_pop_per_frame_8K);
	printf("d%d 9A_D3h	%d	%d	%d	%.5lg\n",rank,nc9A,ng9A,nn9A,mean_pop_per_frame_9A);
	printf("d%d 9B_C2v	%d	%d	%d	%.5lg\n",rank,nc9B,ng9B,nn9B,mean_pop_per_frame_9B);
	printf("d%d 9K	%d	%d	%d	%.5lg\n",rank,nc9K,ng9K,nn9K,mean_pop_per_frame_9K);
	printf("d%d 10A_D4d	%d	%d	%d	%.5lg\n",rank,nc10A,ng10A,nn10A,mean_pop_per_frame_10A);
	printf("d%d 10B_C3v	%d	%d	%d	%.5lg\n",rank,nc10B,ng10B,nn10B,mean_pop_per_frame_10B);
	printf("d%d 10K	%d	%d	%d	%.5lg\n",rank,nc10K,ng10K,nn10K,mean_pop_per_frame_10K);
	printf("d%d 10W	%d	%d	%d	%.5lg\n",rank,nc10W,ng10W,nn10W,mean_pop_per_frame_10W);
	printf("d%d 11A_D4d	%d	%d	%d	%.5lg\n",rank,nc11A,ng11A,nn11A,mean_pop_per_frame_11A);
	printf("d%d 11B_C2v	%d	%d	%d	%.5lg\n",rank,nc11B,ng11B,nn11B,mean_pop_per_frame_11B);
	printf("d%d 11CD	%d	%d	%d	%.5lg\n",rank,nc11C,ng11C,nn11C,mean_pop_per_frame_11C);
	printf("d%d 11E_C2	%d	%d	%d	%.5lg\n",rank,nc11E,ng11E,nn11E,mean_pop_per_frame_11E);
	printf("d%d 11F_C2v	%d	%d	%d	%.5lg\n",rank,nc11F,ng11F,nn11F,mean_pop_per_frame_11F);
	printf("d%d 11W_Cs	%d	%d	%d	%.5lg\n",rank,nc11W,ng11W,nn11W,mean_pop_per_frame_11W);
	printf("d%d 12A_C2v	%d	%d	%d	%.5lg\n",rank,nc12A,ng12A,nn12A,mean_pop_per_frame_12A);
	printf("d%d 12BC	%d	%d	%d	%.5lg\n",rank,nc12B,ng12B,nn12B,mean_pop_per_frame_12B);
	printf("d%d 12D_D2d	%d	%d	%d	%.5lg\n",rank,nc12D,ng12D,nn12D,mean_pop_per_frame_12D);
	printf("d%d 12E_D3h	%d	%d	%d	%.5lg\n",rank,nc12E,ng12E,nn12E,mean_pop_per_frame_12E);
	printf("d%d 12K	%d	%d	%d	%.5lg\n",rank,nc12K,ng12K,nn12K,mean_pop_per_frame_12K);
	printf("d%d 13A_Ih	%d	%d	%d	%.5lg\n",rank,nc13A,ng13A,nn13A,mean_pop_per_frame_13A);
	printf("d%d 13B_D5h	%d	%d	%d	%.5lg\n",rank,nc13B,ng13B,nn13B,mean_pop_per_frame_13B);
	printf("d%d 13K	%d	%d	%d	%.5lg\n",rank,nc13K,ng13K,nn13K,mean_pop_per_frame_13K);
	printf("d%d FCC_m13	%d	%d	%d	%.5lg\n",rank,ncFCC,ngFCC,nnFCC,mean_pop_per_frame_FCC);
	printf("d%d HCP_m13	%d	%d	%d	%.5lg\n",rank,ncHCP,ngHCP,nnHCP,mean_pop_per_frame_HCP);
	printf("d%d BCC_m9	%d	%d	%d	%.5lg\n",rank,ncBCC_9,ngBCC_9,nnBCC_9,mean_pop_per_frame_BCC_9);
	printf("d%d BCC_m15	%d	%d	%d	%.5lg\n",rank,ncBCC_15,ngBCC_15,nnBCC_15,mean_pop_per_frame_BCC_15);
	printf("d%d totals	%d	%d	%d\n",rank,clusTot,totNclus,N*FRAMES);
	printf("d%d maxnB	%d\n",rank,maxnb);
	printf("d%d max to sp3	%d\n",rank,maxto3);
	printf("d%d max to sp4	%d\n",rank,maxto4);
	printf("d%d max to sp5	%d\n",rank,maxto5);
	printf("d%d excess spindles sp3	%d\n",rank,ncsp3_excess_spindles);
	printf("d%d excess spindles sp4	%d\n",rank,ncsp4_excess_spindles);
	printf("d%d excess spindles sp5	%d\n",rank,ncsp5_excess_spindles);
	if (doBLDistros==1) {
		printf("d%d mean bl samples	%.15lg	%d\n",rank,meanBL,BLDistroNoSamples);
		if (doBinary==1) {
			printf("d%d mean AA bl samples	%.15lg	%d\n",rank,meanBLAA,BLDistroNoSamplesAA);
			printf("d%d mean AB bl samples	%.15lg	%d\n",rank,meanBLAB,BLDistroNoSamplesAB);
			printf("d%d mean BB bl samples	%.15lg	%d\n",rank,meanBLBB,BLDistroNoSamplesBB);
		}
	}
	if (donbDistros==1) {
		printf("d%d mean nB	%.15lg\n",rank,meannb);
		if (doBinary==1) {
			printf("d%d mean A->A nB	%.15lg\n",rank,meannbAA);
			printf("d%d mean A->B nB	%.15lg\n",rank,meannbAB);
			printf("d%d mean B->A nB	%.15lg\n",rank,meannbBA);
			printf("d%d mean B->B nB	%.15lg\n",rank,meannbBB);
		}
	}
	temp=(double)ncsp3c_spindlebonds/(double)nc5A;
	printf("d%d 5A spindle bonds	%d	%.15lg\n",rank,ncsp3c_spindlebonds,temp);
	temp=(double)ncsp4c_spindlebonds/(double)ncsp4c;
	printf("d%d sp4c spindle bonds	%d	%.15lg\n",rank,ncsp4c_spindlebonds,temp);
	temp=(double)nc6A_spindlebonds/(double)nc6A;
	printf("d%d 6A spindle bonds	%d	%.15lg\n",rank,nc6A_spindlebonds,temp);
	temp=(double)ncsp5c_spindlebonds/(double)nc7A;
	printf("d%d 7A spindle bonds	%d	%.15lg\n",rank,ncsp5c_spindlebonds,temp);
	printf("d%d correctedBonds %d per frame %.5lg per part per frame %.5lg\n",rank,correctedBonds,(double)correctedBonds/FRAMES,(double)correctedBonds/(FRAMES*N));
	
	if (doDynamics==0) {
		writeout=fopen(filename,"w");
		if (writeout==NULL)  {
			sprintf(errMsg,"Stats_Report(): Error opening file %s",filename);	// Always test file open
			Error(errMsg);
		}
		fprintf(writeout,"%s\n",filename);
		fprintf(writeout,"Clust	Number	Gross	Net	Mean Pop Per Frame\n");
		fprintf(writeout,"sp3	%d	%d	%d	%.5lg\n",ncsp3,ngsp3,0,mean_pop_per_frame_sp3);
		fprintf(writeout,"sp3a	%d	%d	%d	%.5lg\n",ncsp3a,ngsp3a,0,mean_pop_per_frame_sp3a);
		fprintf(writeout,"sp3b	%d	%d	%d	%.5lg\n",ncsp3b,ngsp3b,0,mean_pop_per_frame_sp3b);
		fprintf(writeout,"5A_D3h	%d	%d	%d	%.5lg\n",nc5A,ng5A,nn5A,mean_pop_per_frame_sp3c);
		fprintf(writeout,"sp4	%d	%d	%d	%.5lg\n",ncsp4,ngsp4,0,mean_pop_per_frame_sp4);
		fprintf(writeout,"sp4a	%d	%d	%d	%.5lg\n",ncsp4a,ngsp4a,0,mean_pop_per_frame_sp4a);
		fprintf(writeout,"sp4b	%d	%d	%d	%.5lg\n",ncsp4b,ngsp4b,0,mean_pop_per_frame_sp4b);
		fprintf(writeout,"sp4c	%d	%d	%d	%.5lg\n",ncsp4c,ng6A,0,mean_pop_per_frame_6A); 
		fprintf(writeout,"6A_Oh	%d	%d	%d	%.5lg\n",nc6A,ng6A,nn6A,mean_pop_per_frame_6A);
		fprintf(writeout,"6Z_C2v	%d	%d	%d	%.5lg\n",nc6Z,ng6Z,nn6Z,mean_pop_per_frame_6Z);
		fprintf(writeout,"sp5	%d	%d	%d	%.5lg\n",ncsp5,ngsp5,0,mean_pop_per_frame_sp5);
		fprintf(writeout,"sp5a	%d	%d	%d	%.5lg\n",ncsp5a,ngsp5a,0,mean_pop_per_frame_sp5a);
		fprintf(writeout,"sp5b	%d	%d	%d	%.5lg\n",ncsp5b,ngsp5b,0,mean_pop_per_frame_sp5b);
		fprintf(writeout,"7A_D5h	%d	%d	%d	%.5lg\n",nc7A,ng7A,nn7A,mean_pop_per_frame_sp5c);
		fprintf(writeout,"7K	%d	%d	%d	%.5lg\n",nc7K,ng7K,nn7K,mean_pop_per_frame_7K);
		fprintf(writeout,"8A_D2d	%d	%d	%d	%.5lg\n",nc8A,ng8A,nn8A,mean_pop_per_frame_8A);
		fprintf(writeout,"8B_Cs	%d	%d	%d	%.5lg\n",nc8B,ng8B,nn8B,mean_pop_per_frame_8B);
		fprintf(writeout,"8K	%d	%d	%d	%.5lg\n",nc8K,ng8K,nn8K,mean_pop_per_frame_8K);
		fprintf(writeout,"9A_D3h	%d	%d	%d	%.5lg\n",nc9A,ng9A,nn9A,mean_pop_per_frame_9A);
		fprintf(writeout,"9B_C2v	%d	%d	%d	%.5lg\n",nc9B,ng9B,nn9B,mean_pop_per_frame_9B);
		fprintf(writeout,"9K	%d	%d	%d	%.5lg\n",nc9K,ng9K,nn9K,mean_pop_per_frame_9K);
		fprintf(writeout,"10A_D4d	%d	%d	%d	%.5lg\n",nc10A,ng10A,nn10A,mean_pop_per_frame_10A);
		fprintf(writeout,"10B_C3v	%d	%d	%d	%.5lg\n",nc10B,ng10B,nn10B,mean_pop_per_frame_10B);
		fprintf(writeout,"10K	%d	%d	%d	%.5lg\n",nc10K,ng10K,nn10K,mean_pop_per_frame_10K);
		fprintf(writeout,"10W	%d	%d	%d	%.5lg\n",nc10W,ng10W,nn10W,mean_pop_per_frame_10W);
		fprintf(writeout,"11A_D4d	%d	%d	%d	%.5lg\n",nc11A,ng11A,nn11A,mean_pop_per_frame_11A);
		fprintf(writeout,"11B_C2v	%d	%d	%d	%.5lg\n",nc11B,ng11B,nn11B,mean_pop_per_frame_11B);
		fprintf(writeout,"11CD	%d	%d	%d	%.5lg\n",nc11C,ng11C,nn11C,mean_pop_per_frame_11C);
		fprintf(writeout,"11E_C2	%d	%d	%d	%.5lg\n",nc11E,ng11E,nn11E,mean_pop_per_frame_11E);
		fprintf(writeout,"11F_C2v	%d	%d	%d	%.5lg\n",nc11F,ng11F,nn11F,mean_pop_per_frame_11F);
		fprintf(writeout,"11W_Cs	%d	%d	%d	%.5lg\n",nc11W,ng11W,nn11W,mean_pop_per_frame_11W);
		fprintf(writeout,"12A_C2v	%d	%d	%d	%.5lg\n",nc12A,ng12A,nn12A,mean_pop_per_frame_12A);
		fprintf(writeout,"12BC	%d	%d	%d	%.5lg\n",nc12B,ng12B,nn12B,mean_pop_per_frame_12B);
		fprintf(writeout,"12D_D2d	%d	%d	%d	%.5lg\n",nc12D,ng12D,nn12D,mean_pop_per_frame_12D);
		fprintf(writeout,"12E_D3h	%d	%d	%d	%.5lg\n",nc12E,ng12E,nn12E,mean_pop_per_frame_12E);
		fprintf(writeout,"12K	%d	%d	%d	%.5lg\n",nc12K,ng12K,nn12K,mean_pop_per_frame_12K);
		fprintf(writeout,"13A_Ih	%d	%d	%d	%.5lg\n",nc13A,ng13A,nn13A,mean_pop_per_frame_13A);
		fprintf(writeout,"13B_D5h	%d	%d	%d	%.5lg\n",nc13B,ng13B,nn13B,mean_pop_per_frame_13B);
		fprintf(writeout,"13K	%d	%d	%d	%.5lg\n",nc13K,ng13K,nn13K,mean_pop_per_frame_13K);
		fprintf(writeout,"FCC_m13	%d	%d	%d	%.5lg\n",ncFCC,ngFCC,nnFCC,mean_pop_per_frame_FCC);
		fprintf(writeout,"HCP_m13	%d	%d	%d	%.5lg\n",ncHCP,ngHCP,nnHCP,mean_pop_per_frame_HCP);
		fprintf(writeout,"BCC_m9	%d	%d	%d	%.5lg\n",ncBCC_9,ngBCC_9,nnBCC_9,mean_pop_per_frame_BCC_9);
		fprintf(writeout,"BCC_m15	%d	%d	%d	%.5lg\n",ncBCC_15,ngBCC_15,nnBCC_15,mean_pop_per_frame_BCC_15);
		fprintf(writeout,"totals	%d	%d	%d\n",clusTot,totNclus,N*FRAMES);
		fprintf(writeout,"maxnB	%d\n",maxnb);
		fprintf(writeout,"max to sp3	%d\n",maxto3);
		fprintf(writeout,"max to sp4	%d\n",maxto4);
		fprintf(writeout,"max to sp5	%d\n",maxto5);
		fprintf(writeout,"excess spindles sp3	%d\n",ncsp3_excess_spindles);
		fprintf(writeout,"excess spindles sp4	%d\n",ncsp4_excess_spindles);
		fprintf(writeout,"excess spindles sp5	%d\n",ncsp5_excess_spindles);
		if (doBLDistros==1) fprintf(writeout,"mean bl samples	%.15lg	%d\n",meanBL,BLDistroNoSamples);
		if (doBinary==1 && doBLDistros==1) {
			fprintf(writeout,"mean AA bl samples	%.15lg	%d\n",meanBLAA,BLDistroNoSamplesAA);
			fprintf(writeout,"mean AB bl samples	%.15lg	%d\n",meanBLAB,BLDistroNoSamplesAB);
			fprintf(writeout,"mean BB bl samples	%.15lg	%d\n",meanBLBB,BLDistroNoSamplesBB);
		}
		if (donbDistros==1) {
			fprintf(writeout,"mean nB	%.15lg\n",meannb);
			if (doBinary==1) {
				fprintf(writeout,"mean A->A nB	%.15lg\n",meannbAA);
				fprintf(writeout,"mean A->B nB	%.15lg\n",meannbAB);
				fprintf(writeout,"mean B->A nB	%.15lg\n",meannbBA);
				fprintf(writeout,"mean B->B nB	%.15lg\n",meannbBB);
			}
		}
		temp=(double)ncsp3c_spindlebonds/(double)nc5A;
		fprintf(writeout,"5A spindle bonds	%d	%.15lg\n",ncsp3c_spindlebonds,temp);
		temp=(double)ncsp4c_spindlebonds/(double)ncsp4c;
		fprintf(writeout,"sp4c spindle bonds	%d	%.15lg\n",ncsp4c_spindlebonds,temp);
		temp=(double)nc6A_spindlebonds/(double)nc6A;
		fprintf(writeout,"6A spindle bonds	%d	%.15lg\n",nc6A_spindlebonds,temp);
		temp=(double)ncsp5c_spindlebonds/(double)nc7A;
		fprintf(writeout,"7A spindle bonds	%d	%.15lg\n",ncsp5c_spindlebonds,temp);
		fprintf(writeout,"correctedBonds	%d	per frame	%.15lg	per part per frame	%.15lg\n",correctedBonds,(double)correctedBonds/FRAMES,(double)correctedBonds/(FRAMES*N));
		
		fclose(writeout);
	}
}
//// END: Stats routines

//// START: Write routines
void Write_Raw_Init() {
	char errMsg[1000];
	char output[1000];
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp3",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp3=fopen(output,"w");
	if (file_raw_sp3==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp3,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp3a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp3a=fopen(output,"w");
	if (file_raw_sp3a==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp3a,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp3b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp3b=fopen(output,"w");
	if (file_raw_sp3b==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp3b,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_5A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_5A=fopen(output,"w");
	if (file_raw_5A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_5A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp4",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp4=fopen(output,"w");
	if (file_raw_sp4==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp4,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp4a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp4a=fopen(output,"w");
	if (file_raw_sp4a==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp4a,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp4b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp4b=fopen(output,"w");
	if (file_raw_sp4b==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp4b,"%s\n",output);

	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_6A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_6A=fopen(output,"w");
	if (file_raw_6A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_6A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_6Z",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_6Z=fopen(output,"w");
	if (file_raw_6Z==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_6Z,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp5",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp5=fopen(output,"w");
	if (file_raw_sp5==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp5,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp5a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp5a=fopen(output,"w");
	if (file_raw_sp5a==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp5a,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_sp5b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_sp5b=fopen(output,"w");
	if (file_raw_sp5b==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_sp5b,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_7A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_7A=fopen(output,"w");
	if (file_raw_7A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_7A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_7K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_7K=fopen(output,"w");
	if (file_raw_7K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_7K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_8A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_8A=fopen(output,"w");
	if (file_raw_8A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_8A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_8B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_8B=fopen(output,"w");
	if (file_raw_8B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_8B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_8K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_8K=fopen(output,"w");
	if (file_raw_8K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_8K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_9A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_9A=fopen(output,"w");
	if (file_raw_9A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_9A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_9B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_9B=fopen(output,"w");
	if (file_raw_9B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_9B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_9K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_9K=fopen(output,"w");
	if (file_raw_9K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_9K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_10A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_10A=fopen(output,"w");
	if (file_raw_10A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_10A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_10B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_10B=fopen(output,"w");
	if (file_raw_10B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_10B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_10K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_10K=fopen(output,"w");
	if (file_raw_10K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_10K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_10W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_10W=fopen(output,"w");
	if (file_raw_10W==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_10W,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_11A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_11A=fopen(output,"w");
	if (file_raw_11A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_11A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_11B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_11B=fopen(output,"w");
	if (file_raw_11B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_11B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_11C",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_11C=fopen(output,"w");
	if (file_raw_11C==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_11C,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_11E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_11E=fopen(output,"w");
	if (file_raw_11E==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_11E,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_11F",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_11F=fopen(output,"w");
	if (file_raw_11F==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_11F,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_11W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_11W=fopen(output,"w");
	if (file_raw_11W==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_11W,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_12A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_12A=fopen(output,"w");
	if (file_raw_12A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_12A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_12B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_12B=fopen(output,"w");
	if (file_raw_12B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_12B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_12D",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_12D=fopen(output,"w");
	if (file_raw_12D==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_12D,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_12E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_12E=fopen(output,"w");
	if (file_raw_12E==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_12E,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_12K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_12K=fopen(output,"w");
	if (file_raw_12K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_12K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_13A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_13A=fopen(output,"w");
	if (file_raw_13A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_13A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_13B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_13B=fopen(output,"w");
	if (file_raw_13B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_13B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_13K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_13K=fopen(output,"w");
	if (file_raw_13K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_13K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_FCC",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_FCC=fopen(output,"w");
	if (file_raw_FCC==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_FCC,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_HCP",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_HCP=fopen(output,"w");
	if (file_raw_HCP==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_HCP,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_BCC_9",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_BCC_9=fopen(output,"w");
	if (file_raw_BCC_9==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_BCC_9,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_BCC_15",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	file_raw_BCC_15=fopen(output,"w");
	if (file_raw_BCC_15==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(file_raw_BCC_15,"%s\n",output);
}

void Write_Raw_Xmol(int f, FILE *thefile, char *sarr) {
	int i;
	
	fprintf(thefile,"%d\nframe %d of %d\n",N,f,TOTALFRAMES);
	for(i=0; i<N; i++) {
		if (sarr[i]!='C') {
			if (rtype[i]==1) fprintf(thefile,"C\n");
			else fprintf(thefile,"D\n");
		}
		else if (sarr[i]=='C') {
			if (rtype[i]==1) fprintf(thefile,"A\n");
			else fprintf(thefile,"B\n");
		}
	}
}

void Write_13A_cen_xmol(int f) {
	int i;
	
	int n13A_cen=0;
	
	for(i=0; i<N; i++) if(s13A_cen[i]=='O') ++n13A_cen;         // get total number of 13A centres 
		
	fprintf(file_13A_cen_xmol,"%d\nframe %d of %d\n",n13A_cen,f,TOTALFRAMES);
	for(i=0; i<N; i++) {
		if (s13A_cen[i]=='O') fprintf(file_13A_cen_xmol ,"O\t%.5lg\t%.5lg\t%.5lg\n", x[i], y[i], z[i]);
	}
}

        

void Write_Raw(int f) {
	Write_Raw_Xmol(f,file_raw_sp3,&ssp3[0]);
	Write_Raw_Xmol(f,file_raw_sp3a,&ssp3a[0]);
	Write_Raw_Xmol(f,file_raw_sp3b,&ssp3b[0]);
	Write_Raw_Xmol(f,file_raw_5A,&s5A[0]);
	Write_Raw_Xmol(f,file_raw_sp4,&ssp4[0]);
	Write_Raw_Xmol(f,file_raw_sp4a,&ssp4a[0]);
	Write_Raw_Xmol(f,file_raw_sp4b,&ssp4b[0]);
	Write_Raw_Xmol(f,file_raw_6A,&s6A[0]);
	Write_Raw_Xmol(f,file_raw_6Z,&s6Z[0]);
	Write_Raw_Xmol(f,file_raw_7K,&s7K[0]);
	Write_Raw_Xmol(f,file_raw_sp5,&ssp5[0]);
	Write_Raw_Xmol(f,file_raw_sp5a,&ssp5a[0]);
	Write_Raw_Xmol(f,file_raw_sp5b,&ssp5b[0]);
	Write_Raw_Xmol(f,file_raw_7A,&s7A[0]);
	Write_Raw_Xmol(f,file_raw_8A,&s8A[0]);
	Write_Raw_Xmol(f,file_raw_8B,&s8B[0]);
	Write_Raw_Xmol(f,file_raw_8K,&s8K[0]);
	Write_Raw_Xmol(f,file_raw_9A,&s9A[0]);
	Write_Raw_Xmol(f,file_raw_9B,&s9B[0]);
	Write_Raw_Xmol(f,file_raw_9K,&s9K[0]);
	Write_Raw_Xmol(f,file_raw_10A,&s10A[0]);
	Write_Raw_Xmol(f,file_raw_10B,&s10B[0]);
	Write_Raw_Xmol(f,file_raw_10K,&s10K[0]);
	Write_Raw_Xmol(f,file_raw_10W,&s10W[0]);
	Write_Raw_Xmol(f,file_raw_11A,&s11A[0]);
	Write_Raw_Xmol(f,file_raw_11B,&s11B[0]);
	Write_Raw_Xmol(f,file_raw_11C,&s11C[0]);
	Write_Raw_Xmol(f,file_raw_11E,&s11E[0]);
	Write_Raw_Xmol(f,file_raw_11F,&s11F[0]);
	Write_Raw_Xmol(f,file_raw_11W,&s11W[0]);
	Write_Raw_Xmol(f,file_raw_12A,&s12A[0]);
	Write_Raw_Xmol(f,file_raw_12B,&s12B[0]);
	Write_Raw_Xmol(f,file_raw_12D,&s12D[0]);
	Write_Raw_Xmol(f,file_raw_12E,&s12E[0]);
	Write_Raw_Xmol(f,file_raw_12K,&s12K[0]);
	Write_Raw_Xmol(f,file_raw_13A,&s13A[0]);
	Write_Raw_Xmol(f,file_raw_13B,&s13B[0]);
	Write_Raw_Xmol(f,file_raw_13K,&s13K[0]);
	Write_Raw_Xmol(f,file_raw_FCC,&sFCC[0]);
	Write_Raw_Xmol(f,file_raw_HCP,&sHCP[0]);
	Write_Raw_Xmol(f,file_raw_BCC_9,&sBCC_9[0]);
	Write_Raw_Xmol(f,file_raw_BCC_15,&sBCC_15[0]);
}

void Write_Raw_Close() {
	fclose(file_raw_sp3);
	fclose(file_raw_sp3a);
	fclose(file_raw_sp3b);
	fclose(file_raw_5A);
	fclose(file_raw_sp4);
	fclose(file_raw_sp4a);
	fclose(file_raw_sp4b);
	fclose(file_raw_6A);
	fclose(file_raw_6Z);
	fclose(file_raw_7K);
	fclose(file_raw_sp5);
	fclose(file_raw_sp5a);
	fclose(file_raw_sp5b);
	fclose(file_raw_7A);
	fclose(file_raw_8A);
	fclose(file_raw_8B);
	fclose(file_raw_8K);
	fclose(file_raw_9A);
	fclose(file_raw_9B);
	fclose(file_raw_9K);
	fclose(file_raw_10A);
	fclose(file_raw_10B);
	fclose(file_raw_10K);
	fclose(file_raw_10W);
	fclose(file_raw_11A);
	fclose(file_raw_11B);
	fclose(file_raw_11C);
	fclose(file_raw_11E);
	fclose(file_raw_11F);
	fclose(file_raw_11W);
	fclose(file_raw_12A);
	fclose(file_raw_12B);
	fclose(file_raw_12D);
	fclose(file_raw_12E);
	fclose(file_raw_12K);
	fclose(file_raw_13A);
	fclose(file_raw_13B);
	fclose(file_raw_13K);
	fclose(file_raw_FCC);
	fclose(file_raw_HCP);
	fclose(file_raw_BCC_9);
	fclose(file_raw_BCC_15);
}


void Write_Cluster_Init() {
	char errMsg[1000];
	char output[1000];
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp3",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp3=fopen(output,"w");
	if (wsp3==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp3,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp3a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp3a=fopen(output,"w");
	if (wsp3a==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp3a,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp3b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp3b=fopen(output,"w");
	if (wsp3b==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp3b,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_5A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w5A=fopen(output,"w");
	if (w5A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w5A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp4",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp4=fopen(output,"w");
	if (wsp4==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp4,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp4a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp4a=fopen(output,"w");
	if (wsp4a==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp4a,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp4b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp4b=fopen(output,"w");
	if (wsp4b==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp4b,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp4c",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp4c=fopen(output,"w");
	if (wsp4c==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp4c,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_6A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w6A=fopen(output,"w");
	if (w6A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w6A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_6Z",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w6Z=fopen(output,"w");
	if (w6Z==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w6Z,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp5",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp5=fopen(output,"w");
	if (wsp5==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp5,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp5a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp5a=fopen(output,"w");
	if (wsp5a==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp5a,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_sp5b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wsp5b=fopen(output,"w");
	if (wsp5b==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wsp5b,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_7A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w7A=fopen(output,"w");
	if (w7A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w7A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_7K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w7K=fopen(output,"w");
	if (w7K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w7K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_8A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w8A=fopen(output,"w");
	if (w8A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w8A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_8B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w8B=fopen(output,"w");
	if (w8B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w8B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_8K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w8K=fopen(output,"w");
	if (w8K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w8K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_9A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w9A=fopen(output,"w");
	if (w9A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w9A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_9B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w9B=fopen(output,"w");
	if (w9B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w9B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_9K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w9K=fopen(output,"w");
	if (w9K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w9K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_10A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w10A=fopen(output,"w");
	if (w10A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w10A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_10B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w10B=fopen(output,"w");
	if (w10B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w10B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_10K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w10K=fopen(output,"w");
	if (w10K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w10K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_10W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w10W=fopen(output,"w");
	if (w10W==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w10W,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_11A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w11A=fopen(output,"w");
	if (w11A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w11A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_11B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w11B=fopen(output,"w");
	if (w11B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w11B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_11C",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w11C=fopen(output,"w");
	if (w11C==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w11C,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_11E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w11E=fopen(output,"w");
	if (w11E==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w11E,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_11F",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w11F=fopen(output,"w");
	if (w11F==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w11F,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_11W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w11W=fopen(output,"w");
	if (w11W==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w11W,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_12A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w12A=fopen(output,"w");
	if (w12A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w12A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_12B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w12B=fopen(output,"w");
	if (w12B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w12B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_12D",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w12D=fopen(output,"w");
	if (w12D==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w12D,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_12E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w12E=fopen(output,"w");
	if (w12E==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w12E,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_12K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w12K=fopen(output,"w");
	if (w12K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w12K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_13A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w13A=fopen(output,"w");
	if (w13A==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w13A,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_13B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w13B=fopen(output,"w");
	if (w13B==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w13B,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_13K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	w13K=fopen(output,"w");
	if (w13K==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(w13K,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_FCC",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wFCC=fopen(output,"w");
	if (wFCC==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wFCC,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_HCP",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wHCP=fopen(output,"w");
	if (wHCP==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wHCP,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_BCC_9",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wBCC_9=fopen(output,"w");
	if (wBCC_9==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wBCC_9,"%s\n",output);
	
	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clusts_BCC_15",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	wBCC_15=fopen(output,"w");
	if (wBCC_15==NULL) { sprintf(errMsg,"Write_Cluster_Init(): Error opening file %s",output); Error(errMsg); }
	fprintf(wBCC_15,"%s\n",output);
}

void Write_Cluster_Close() {
	fclose(wsp3);
	fclose(wsp3a);
	fclose(wsp3b);
	fclose(w5A);
	fclose(wsp4);
	fclose(wsp4a);
	fclose(wsp4b);
	fclose(wsp4c);
	fclose(w6A);
	fclose(w6Z);
	fclose(w7K);
	fclose(wsp5);
	fclose(wsp5a);
	fclose(wsp5b);
	fclose(w7A);
	fclose(w8A);
	fclose(w8B);
	fclose(w8K);
	fclose(w9A);
	fclose(w9B);
	fclose(w9K);
	fclose(w10A);
	fclose(w10B);
	fclose(w10K);
	fclose(w10W);
	fclose(w11A);
	fclose(w11B);
	fclose(w11C);
	fclose(w11E);
	fclose(w11F);
	fclose(w11W);
	fclose(w12A);
	fclose(w12B);
	fclose(w12D);
	fclose(w12E);
	fclose(w12K);
	fclose(w13A);
	fclose(w13B);
	fclose(w13K);
	fclose(wFCC);
	fclose(wHCP);
	fclose(wBCC_9);
	fclose(wBCC_15);
}

void Write_Cluster(int f, FILE *writeout, int *n, int **hc, int clusSize) {
	int i,j;
	
	fprintf(writeout,"%d\n",n[f]);
	for (i=0;i<n[f];i++) {
		fprintf(writeout,"%d",hc[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",hc[i][j]);
		fprintf(writeout,"	%d\n",hc[i][clusSize-1]);
	}
}


void Write_Cluster_sp3(int f, FILE *writeout) {
	int i,j;
	int clusSize=3;
	
	fprintf(writeout,"%d\n",nsp3[f]);
	for (i=0;i<nsp3a[f];i++) {
		fprintf(writeout,"%d",sp3a[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp3a[i][j]);
		fprintf(writeout,"	%d\n",sp3a[i][clusSize-1]);
	}
	for (i=0;i<nsp3b[f];i++) {
		fprintf(writeout,"%d",sp3b[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp3b[i][j]);
		fprintf(writeout,"	%d\n",sp3b[i][clusSize-1]);
	}
	for (i=0;i<nsp3c[f];i++) {
		fprintf(writeout,"%d",sp3c[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp3c[i][j]);
		fprintf(writeout,"	%d\n",sp3c[i][clusSize-1]);
	}
}

void Write_Cluster_sp4(int f, FILE *writeout) {
	int i,j;
	int clusSize=4;
	
	fprintf(writeout,"%d\n",nsp4[f]);
	for (i=0;i<nsp4a[f];i++) {
		fprintf(writeout,"%d",sp4a[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp4a[i][j]);
		fprintf(writeout,"	%d\n",sp4a[i][clusSize-1]);
	}
	for (i=0;i<nsp4b[f];i++) {
		fprintf(writeout,"%d",sp4b[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp4b[i][j]);
		fprintf(writeout,"	%d\n",sp4b[i][clusSize-1]);
	}
	for (i=0;i<nsp4c[f];i++) {
		fprintf(writeout,"%d",sp4c[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp4c[i][j]);
		fprintf(writeout,"	%d\n",sp4c[i][clusSize-1]);
	}
}

void Write_Cluster_sp5(int f, FILE *writeout) {
	int i,j;
	int clusSize=5;
	
	fprintf(writeout,"%d\n",nsp5[f]);
	for (i=0;i<nsp5a[f];i++) {
		fprintf(writeout,"%d",sp5a[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp5a[i][j]);
		fprintf(writeout,"	%d\n",sp5a[i][clusSize-1]);
	}
	for (i=0;i<nsp5b[f];i++) {
		fprintf(writeout,"%d",sp5b[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp5b[i][j]);
		fprintf(writeout,"	%d\n",sp5b[i][clusSize-1]);
	}
	for (i=0;i<nsp5c[f];i++) {
		fprintf(writeout,"%d",sp5c[i][0]);
		for (j=1;j<clusSize-1;j++) fprintf(writeout,"	%d",sp5c[i][j]);
		fprintf(writeout,"	%d\n",sp5c[i][clusSize-1]);
	}
}
//// END: Write routines

void Pop_Per_Frame(int f) {
	int i;
	
	for(i=0; i<N; ++i){
		if(ssp3[i] != 'C') pop_per_frame_sp3[f]+=1.0;
		if(ssp3a[i] != 'C') pop_per_frame_sp3a[f]+=1.0;
		if(ssp3b[i] != 'C') pop_per_frame_sp3b[f]+=1.0;
		if(s5A[i] != 'C') pop_per_frame_sp3c[f]+=1.0;
		if(ssp4[i] != 'C') pop_per_frame_sp4[f]+=1.0;
		if(ssp4a[i] != 'C') pop_per_frame_sp4a[f]+=1.0;
		if(ssp4b[i] != 'C') pop_per_frame_sp4b[f]+=1.0;
		if(s6A[i] != 'C') pop_per_frame_6A[f]+=1.0;
		if(s6Z[i] != 'C') pop_per_frame_6Z[f]+=1.0;
		if(ssp5[i] != 'C') pop_per_frame_sp5[f]+=1.0;
		if(ssp5a[i] != 'C') pop_per_frame_sp5a[f]+=1.0;
		if(ssp5b[i] != 'C') pop_per_frame_sp5b[f]+=1.0;
		if(s7A[i] != 'C') pop_per_frame_sp5c[f]+=1.0;
		if(s7K[i] != 'C') pop_per_frame_7K[f]+=1.0;
		if(s8A[i] != 'C') pop_per_frame_8A[f]+=1.0;
		if(s8B[i] != 'C') pop_per_frame_8B[f]+=1.0;
		if(s8K[i] != 'C') pop_per_frame_8K[f]+=1.0;
		if(s9A[i] != 'C') pop_per_frame_9A[f]+=1.0;
		if(s9B[i] != 'C') pop_per_frame_9B[f]+=1.0;
		if(s9K[i] != 'C') pop_per_frame_9K[f]+=1.0;
		if(s10A[i] != 'C') pop_per_frame_10A[f]+=1.0;
		if(s10B[i] != 'C') pop_per_frame_10B[f]+=1.0;
		if(s10K[i] != 'C') pop_per_frame_10K[f]+=1.0;
		if(s10W[i] != 'C') pop_per_frame_10W[f]+=1.0;
		if(s11A[i] != 'C') pop_per_frame_11A[f]+=1.0;
		if(s11B[i] != 'C') pop_per_frame_11B[f]+=1.0;
		if(s11C[i] != 'C') pop_per_frame_11C[f]+=1.0;
		if(s11E[i] != 'C') pop_per_frame_11E[f]+=1.0;
		if(s11F[i] != 'C') pop_per_frame_11F[f]+=1.0;
		if(s11W[i] != 'C') pop_per_frame_11W[f]+=1.0;
		if(s12A[i] != 'C') pop_per_frame_12A[f]+=1.0;
		if(s12B[i] != 'C') pop_per_frame_12B[f]+=1.0;
		if(s12D[i] != 'C') pop_per_frame_12D[f]+=1.0;
		if(s12E[i] != 'C') pop_per_frame_12E[f]+=1.0;
		if(s12K[i] != 'C') pop_per_frame_12K[f]+=1.0;
		if(s13A[i] != 'C') pop_per_frame_13A[f]+=1.0;
		if(s13B[i] != 'C') pop_per_frame_13B[f]+=1.0;
		if(s13K[i] != 'C') pop_per_frame_13K[f]+=1.0;
		if(sFCC[i] != 'C') pop_per_frame_FCC[f]+=1.0;
		if(sHCP[i] != 'C') pop_per_frame_HCP[f]+=1.0;
		if(sBCC_9[i] != 'C') pop_per_frame_BCC_9[f]+=1.0;
		if(sBCC_15[i] != 'C') pop_per_frame_BCC_15[f]+=1.0;	
	}
	
	pop_per_frame_sp3[f]/=N;
	pop_per_frame_sp3a[f]/=N;
	pop_per_frame_sp3b[f]/=N;
	pop_per_frame_sp3c[f]/=N;
	pop_per_frame_sp4[f]/=N;
	pop_per_frame_sp4a[f]/=N;
	pop_per_frame_sp4b[f]/=N;
	pop_per_frame_6A[f]/=N;
	pop_per_frame_6Z[f]/=N;
	pop_per_frame_sp5[f]/=N;
	pop_per_frame_sp5a[f]/=N;
	pop_per_frame_sp5b[f]/=N;
	pop_per_frame_sp5c[f]/=N;
	pop_per_frame_7K[f]/=N;
	pop_per_frame_8A[f]/=N;
	pop_per_frame_8B[f]/=N;
	pop_per_frame_8K[f]/=N;
	pop_per_frame_9A[f]/=N;
	pop_per_frame_9B[f]/=N;
	pop_per_frame_9K[f]/=N;
	pop_per_frame_10A[f]/=N;
	pop_per_frame_10B[f]/=N;
	pop_per_frame_10K[f]/=N;
	pop_per_frame_10W[f]/=N;
	pop_per_frame_11A[f]/=N;
	pop_per_frame_11B[f]/=N;
	pop_per_frame_11C[f]/=N;
	pop_per_frame_11E[f]/=N;
	pop_per_frame_11F[f]/=N;
	pop_per_frame_11W[f]/=N;
	pop_per_frame_12A[f]/=N;
	pop_per_frame_12B[f]/=N;
	pop_per_frame_12D[f]/=N;
	pop_per_frame_12E[f]/=N;
	pop_per_frame_12K[f]/=N;
	pop_per_frame_13A[f]/=N;
	pop_per_frame_13B[f]/=N;
	pop_per_frame_13K[f]/=N;
	pop_per_frame_FCC[f]/=N;
	pop_per_frame_HCP[f]/=N;
	pop_per_frame_BCC_9[f]/=N;
	pop_per_frame_BCC_15[f]/=N;
	
	mean_pop_per_frame_sp3+=pop_per_frame_sp3[f];
	mean_pop_per_frame_sp3a+=pop_per_frame_sp3a[f];
	mean_pop_per_frame_sp3b+=pop_per_frame_sp3b[f];
	mean_pop_per_frame_sp3c+=pop_per_frame_sp3c[f];
	mean_pop_per_frame_sp4+=pop_per_frame_sp4[f];
	mean_pop_per_frame_sp4a+=pop_per_frame_sp4a[f];
	mean_pop_per_frame_sp4b+=pop_per_frame_sp4b[f];
	mean_pop_per_frame_6A+=pop_per_frame_6A[f];
	mean_pop_per_frame_6Z+=pop_per_frame_6Z[f];
	mean_pop_per_frame_sp5+=pop_per_frame_sp5[f];
	mean_pop_per_frame_sp5a+=pop_per_frame_sp5a[f];
	mean_pop_per_frame_sp5b+=pop_per_frame_sp5b[f];
	mean_pop_per_frame_sp5c+=pop_per_frame_sp5c[f];
	mean_pop_per_frame_7K+=pop_per_frame_7K[f];
	mean_pop_per_frame_8A+=pop_per_frame_8A[f];
	mean_pop_per_frame_8B+=pop_per_frame_8B[f];
	mean_pop_per_frame_8K+=pop_per_frame_8K[f];
	mean_pop_per_frame_9A+=pop_per_frame_9A[f];
	mean_pop_per_frame_9B+=pop_per_frame_9B[f];
	mean_pop_per_frame_9K+=pop_per_frame_9K[f];
	mean_pop_per_frame_10A+=pop_per_frame_10A[f];
	mean_pop_per_frame_10B+=pop_per_frame_10B[f];
	mean_pop_per_frame_10K+=pop_per_frame_10K[f];
	mean_pop_per_frame_10W+=pop_per_frame_10W[f];
	mean_pop_per_frame_11A+=pop_per_frame_11A[f];
	mean_pop_per_frame_11B+=pop_per_frame_11B[f];
	mean_pop_per_frame_11C+=pop_per_frame_11C[f];
	mean_pop_per_frame_11E+=pop_per_frame_11E[f];
	mean_pop_per_frame_11F+=pop_per_frame_11F[f];
	mean_pop_per_frame_11W+=pop_per_frame_11W[f];
	mean_pop_per_frame_12A+=pop_per_frame_12A[f];
	mean_pop_per_frame_12B+=pop_per_frame_12B[f];
	mean_pop_per_frame_12D+=pop_per_frame_12D[f];
	mean_pop_per_frame_12E+=pop_per_frame_12E[f];
	mean_pop_per_frame_12K+=pop_per_frame_12K[f];
	mean_pop_per_frame_13A+=pop_per_frame_13A[f];
	mean_pop_per_frame_13B+=pop_per_frame_13B[f];
	mean_pop_per_frame_13K+=pop_per_frame_13K[f];
	mean_pop_per_frame_FCC+=pop_per_frame_FCC[f];
	mean_pop_per_frame_HCP+=pop_per_frame_HCP[f];
	mean_pop_per_frame_BCC_9+=pop_per_frame_BCC_9[f];
	mean_pop_per_frame_BCC_15+=pop_per_frame_BCC_15[f];
}

void Update_Potential(int f) {
	int i;
	
	for(i=0; i<N; ++i){
		if(ssp3[i] != 'C') {
			av_pot_sp3+=part_pot[i];
		}
		if(ssp3a[i] != 'C') av_pot_sp3a+=part_pot[i];
		if(ssp3b[i] != 'C') av_pot_sp3b+=part_pot[i];
		if(s5A[i] != 'C') av_pot_sp3c+=part_pot[i];
		if(ssp4[i] != 'C') av_pot_sp4+=part_pot[i];
		if(ssp4a[i] != 'C') av_pot_sp4a+=part_pot[i];
		if(ssp4b[i] != 'C') av_pot_sp4b+=part_pot[i];
		if(s6A[i] != 'C') {
			av_pot_sp4c+=part_pot[i];
			av_pot_6A+=part_pot[i];
		}
		if(s6Z[i] != 'C') av_pot_6Z+=part_pot[i];
		if(ssp5[i] != 'C') av_pot_sp5+=part_pot[i];
		if(ssp5a[i] != 'C') av_pot_sp5a+=part_pot[i];
		if(ssp5b[i] != 'C') av_pot_sp5b+=part_pot[i];
		if(s7A[i] != 'C') av_pot_sp5c+=part_pot[i];
		if(s7K[i] != 'C') av_pot_7K+=part_pot[i];
		if(s8A[i] != 'C') av_pot_8A+=part_pot[i];
		if(s8B[i] != 'C') av_pot_8B+=part_pot[i];
		if(s8K[i] != 'C') av_pot_8K+=part_pot[i];
		if(s9A[i] != 'C') av_pot_9A+=part_pot[i];
		if(s9B[i] != 'C') av_pot_9B+=part_pot[i];
		if(s9K[i] != 'C') av_pot_9K+=part_pot[i];
		if(s10A[i] != 'C') av_pot_10A+=part_pot[i];
		if(s10B[i] != 'C') av_pot_10B+=part_pot[i];
		if(s10K[i] != 'C') av_pot_10K+=part_pot[i];
		if(s10W[i] != 'C') av_pot_10W+=part_pot[i];
		if(s11A[i] != 'C') av_pot_11A+=part_pot[i];
		if(s11B[i] != 'C') av_pot_11B+=part_pot[i];
		if(s11C[i] != 'C') av_pot_11C+=part_pot[i];
		if(s11E[i] != 'C') av_pot_11E+=part_pot[i];
		if(s11F[i] != 'C') av_pot_11F+=part_pot[i];
		if(s11W[i] != 'C') av_pot_11W+=part_pot[i];
		if(s12A[i] != 'C') av_pot_12A+=part_pot[i];
		if(s12B[i] != 'C') av_pot_12B+=part_pot[i];
		if(s12D[i] != 'C') av_pot_12D+=part_pot[i];
		if(s12E[i] != 'C') av_pot_12E+=part_pot[i];
		if(s12K[i] != 'C') av_pot_12K+=part_pot[i];
		if(s13A[i] != 'C') av_pot_13A+=part_pot[i];
		if(s13B[i] != 'C') av_pot_13B+=part_pot[i];
		if(s13K[i] != 'C') av_pot_13K+=part_pot[i];
		if(sFCC[i] != 'C') av_pot_FCC+=part_pot[i];
		if(sHCP[i] != 'C') av_pot_HCP+=part_pot[i];
		if(sBCC_9[i] != 'C') av_pot_BCC_9+=part_pot[i];
		if(sBCC_15[i] != 'C') av_pot_BCC_15+=part_pot[i];
		
		if(s9B_cen[i] != 'C') {
			av_pot_cen_9B+=part_pot[i];
			cnt_av_pot_cen_9B++;
		}
		if(s9K_cen[i] != 'C') {
			av_pot_cen_9K+=part_pot[i];
			cnt_av_pot_cen_9K++;
		}
		if(s10B_cen[i] != 'C') {
			av_pot_cen_10B+=part_pot[i];
			cnt_av_pot_cen_10B++;
		}
		if(s10K_cen[i] != 'C') {
			av_pot_cen_10K+=part_pot[i];
			cnt_av_pot_cen_10K++;
		}
		if(s10W_cen[i] != 'C') {
			av_pot_cen_10W+=part_pot[i];
			cnt_av_pot_cen_10W++;
		}
		if(s11A_cen[i] != 'C') {
			av_pot_cen_11A+=part_pot[i];
			cnt_av_pot_cen_11A++;
		}
		if(s11B_cen[i] != 'C') {
			av_pot_cen_11B+=part_pot[i];
			cnt_av_pot_cen_11B++;
		}
		if(s11C_cen[i] != 'C') {
			av_pot_cen_11C+=part_pot[i];
			cnt_av_pot_cen_11C++;
		}
		if(s11W_cen[i] != 'C') {
			av_pot_cen_11W+=part_pot[i];
			cnt_av_pot_cen_11W++;
		}
		if(s12A_cen[i] != 'C') {
			av_pot_cen_12A+=part_pot[i];
			cnt_av_pot_cen_12A++;
		}
		if(s12B_cen[i] != 'C') {
			av_pot_cen_12B+=part_pot[i];
			cnt_av_pot_cen_12B++;
		}
		if(s12K_cen[i] != 'C') {
			av_pot_cen_12K+=part_pot[i];
			cnt_av_pot_cen_12K++;
		}
		if(s13A_cen[i] != 'C') {
			av_pot_cen_13A+=part_pot[i];
			cnt_av_pot_cen_13A++;
		}
		if(s13B_cen[i] != 'C') {
			av_pot_cen_13B+=part_pot[i];
			cnt_av_pot_cen_13B++;
		}
		if(s13K_cen[i] != 'C') {
			av_pot_cen_13K+=part_pot[i];
			cnt_av_pot_cen_13K++;
		}
		if(sFCC_cen[i] != 'C') {
			av_pot_cen_FCC+=part_pot[i];
			cnt_av_pot_cen_FCC++;
		}
		if(sHCP_cen[i] != 'C') {
			av_pot_cen_HCP+=part_pot[i];
			cnt_av_pot_cen_HCP++;
		}
		if(sBCC_9_cen[i] != 'C') {
			av_pot_cen_BCC_9+=part_pot[i];
			cnt_av_pot_cen_BCC_9++;
		}
		if(sBCC_15_cen[i] != 'C') {
			av_pot_cen_BCC_15+=part_pot[i];
			cnt_av_pot_cen_BCC_15++;
		}
		
		if(s9B_shell[i] != 'C') {
			av_pot_shell_9B+=part_pot[i];
			cnt_av_pot_shell_9B++;
		}
		if(s9K_shell[i] != 'C') {
			av_pot_shell_9K+=part_pot[i];
			cnt_av_pot_shell_9K++;
		}
		if(s10B_shell[i] != 'C') {
			av_pot_shell_10B+=part_pot[i];
			cnt_av_pot_shell_10B++;
		}
		if(s10K_shell[i] != 'C') {
			av_pot_shell_10K+=part_pot[i];
			cnt_av_pot_shell_10K++;
		}
		if(s10W_shell[i] != 'C') {
			av_pot_shell_10W+=part_pot[i];
			cnt_av_pot_shell_10W++;
		}
		if(s11A_shell[i] != 'C') {
			av_pot_shell_11A+=part_pot[i];
			cnt_av_pot_shell_11A++;
		}
		if(s11B_shell[i] != 'C') {
			av_pot_shell_11B+=part_pot[i];
			cnt_av_pot_shell_11B++;
		}
		if(s11C_shell[i] != 'C') {
			av_pot_shell_11C+=part_pot[i];
			cnt_av_pot_shell_11C++;
		}
		if(s11W_shell[i] != 'C') {
			av_pot_shell_11W+=part_pot[i];
			cnt_av_pot_shell_11W++;
		}
		if(s12A_shell[i] != 'C') {
			av_pot_shell_12A+=part_pot[i];
			cnt_av_pot_shell_12A++;
		}
		if(s12B_shell[i] != 'C') {
			av_pot_shell_12B+=part_pot[i];
			cnt_av_pot_shell_12B++;
		}
		if(s12K_shell[i] != 'C') {
			av_pot_shell_12K+=part_pot[i];
			cnt_av_pot_shell_12K++;
		}
		if(s13A_shell[i] != 'C') {
			av_pot_shell_13A+=part_pot[i];
			cnt_av_pot_shell_13A++;
		}
		if(s13B_shell[i] != 'C') {
			av_pot_shell_13B+=part_pot[i];
			cnt_av_pot_shell_13B++;
		}
		if(s13K_shell[i] != 'C') {
			av_pot_shell_13K+=part_pot[i];
			cnt_av_pot_shell_13K++;
		}
		if(sFCC_shell[i] != 'C') {
			av_pot_shell_FCC+=part_pot[i];
			cnt_av_pot_shell_FCC++;
		}
		if(sHCP_shell[i] != 'C') {
			av_pot_shell_HCP+=part_pot[i];
			cnt_av_pot_shell_HCP++;
		}
		if(sBCC_9_shell[i] != 'C') {
			av_pot_shell_BCC_9+=part_pot[i];
			cnt_av_pot_shell_BCC_9++;
		}
		if(sBCC_15_shell[i] != 'C') {
			av_pot_shell_BCC_15+=part_pot[i];
			cnt_av_pot_shell_BCC_15++;
		}
	}
}

void Norm_Write_Potential(char *filename) {
	FILE *fPotentialOutput;
	char  errMsg[1000];
	
	av_potential=av_potential/(N*FRAMES);
	av_pot_check=av_pot_check/(N*2.0);
	av_pot_sp3=av_pot_sp3/(ngsp3*2.0);
	av_pot_sp3a=av_pot_sp3a/(ngsp3a*2.0);
	av_pot_sp3b=av_pot_sp3b/(ngsp3b*2.0);
	av_pot_sp3c=av_pot_sp3c/(ng5A*2.0);
	av_pot_sp4=av_pot_sp4/(ngsp4*2.0);
	av_pot_sp4a=av_pot_sp4a/(ngsp4a*2.0);
	av_pot_sp4b=av_pot_sp4b/(ngsp4b*2.0);
	av_pot_sp4c=av_pot_sp4c/(ng6A*2.0);
	av_pot_6A=av_pot_6A/(ng6A*2.0);
	av_pot_sp5=av_pot_sp5/(ngsp5*2.0);
	av_pot_sp5a=av_pot_sp5a/(ngsp5a*2.0);
	av_pot_sp5b=av_pot_sp5b/(ngsp5b*2.0);
	av_pot_sp5c=av_pot_sp5c/(ng7A*2.0);
	av_pot_6Z=av_pot_6Z/(ng6Z*2.0);
	av_pot_7K=av_pot_7K/(ng7K*2.0);
	av_pot_8A=av_pot_8A/(ng8A*2.0);
	av_pot_8B=av_pot_8B/(ng8B*2.0);
	av_pot_8K=av_pot_8K/(ng8K*2.0);
	av_pot_9A=av_pot_9A/(ng9A*2.0);
	av_pot_9B=av_pot_9B/(ng9B*2.0);
	av_pot_9K=av_pot_9K/(ng9K*2.0);
	av_pot_10A=av_pot_10A/(ng10A*2.0);
	av_pot_10B=av_pot_10B/(ng10B*2.0);
	av_pot_10K=av_pot_10K/(ng10K*2.0);
	av_pot_10W=av_pot_10W/(ng10W*2.0);
	av_pot_11A=av_pot_11A/(ng11A*2.0);
	av_pot_11B=av_pot_11B/(ng11B*2.0);
	av_pot_11C=av_pot_11C/(ng11C*2.0);
	av_pot_11E=av_pot_11E/(ng11E*2.0);
	av_pot_11F=av_pot_11F/(ng11F*2.0);
	av_pot_11W=av_pot_11W/(ng11W*2.0);
	av_pot_12A=av_pot_12A/(ng12A*2.0);
	av_pot_12B=av_pot_12B/(ng12B*2.0);
	av_pot_12D=av_pot_12D/(ng12D*2.0);
	av_pot_12E=av_pot_12E/(ng12E*2.0);
	av_pot_12K=av_pot_12K/(ng12K*2.0);
	av_pot_13A=av_pot_13A/(ng13A*2.0);
	av_pot_13B=av_pot_13B/(ng13B*2.0);
	av_pot_13K=av_pot_13K/(ng13K*2.0);
	av_pot_FCC=av_pot_FCC/(ngFCC*2.0);
	av_pot_HCP=av_pot_HCP/(ngHCP*2.0);
	av_pot_BCC_9=av_pot_BCC_9/(ngBCC_9*2.0);
	av_pot_BCC_15=av_pot_BCC_15/(ngBCC_15*2.0);
	
	av_pot_cen_9B=av_pot_cen_9B/(cnt_av_pot_cen_9B*2.0);
	av_pot_cen_9K=av_pot_cen_9K/(cnt_av_pot_cen_9K*2.0);
	av_pot_cen_10B=av_pot_cen_10B/(cnt_av_pot_cen_10B*2.0);
	av_pot_cen_10K=av_pot_cen_10K/(cnt_av_pot_cen_10K*2.0);
	av_pot_cen_10W=av_pot_cen_10W/(cnt_av_pot_cen_10W*2.0);
	av_pot_cen_11A=av_pot_cen_11A/(cnt_av_pot_cen_11A*2.0);
	av_pot_cen_11B=av_pot_cen_11B/(cnt_av_pot_cen_11B*2.0);
	av_pot_cen_11C=av_pot_cen_11C/(cnt_av_pot_cen_11C*2.0);
	av_pot_cen_11W=av_pot_cen_11W/(cnt_av_pot_cen_11W*2.0);
	av_pot_cen_12A=av_pot_cen_12A/(cnt_av_pot_cen_12A*2.0);
	av_pot_cen_12B=av_pot_cen_12B/(cnt_av_pot_cen_12B*2.0);
	av_pot_cen_12K=av_pot_cen_12K/(cnt_av_pot_cen_12K*2.0);
	av_pot_cen_13A=av_pot_cen_13A/(cnt_av_pot_cen_13A*2.0);
	av_pot_cen_13B=av_pot_cen_13B/(cnt_av_pot_cen_13B*2.0);
	av_pot_cen_13K=av_pot_cen_13K/(cnt_av_pot_cen_13K*2.0);
	av_pot_cen_FCC=av_pot_cen_FCC/(cnt_av_pot_cen_FCC*2.0);
	av_pot_cen_HCP=av_pot_cen_HCP/(cnt_av_pot_cen_HCP*2.0);
	av_pot_cen_BCC_9=av_pot_cen_BCC_9/(cnt_av_pot_cen_BCC_9*2.0);
	av_pot_cen_BCC_15=av_pot_cen_BCC_15/(cnt_av_pot_cen_BCC_15*2.0);
	
	av_pot_shell_9B=av_pot_shell_9B/(cnt_av_pot_shell_9B*2.0);
	av_pot_shell_9K=av_pot_shell_9K/(cnt_av_pot_shell_9K*2.0);
	av_pot_shell_10B=av_pot_shell_10B/(cnt_av_pot_shell_10B*2.0);
	av_pot_shell_10K=av_pot_shell_10K/(cnt_av_pot_shell_10K*2.0);
	av_pot_shell_10W=av_pot_shell_10W/(cnt_av_pot_shell_10W*2.0);
	av_pot_shell_11A=av_pot_shell_11A/(cnt_av_pot_shell_11A*2.0);
	av_pot_shell_11B=av_pot_shell_11B/(cnt_av_pot_shell_11B*2.0);
	av_pot_shell_11C=av_pot_shell_11C/(cnt_av_pot_shell_11C*2.0);
	av_pot_shell_11W=av_pot_shell_11W/(cnt_av_pot_shell_11W*2.0);
	av_pot_shell_12A=av_pot_shell_12A/(cnt_av_pot_shell_12A*2.0);
	av_pot_shell_12B=av_pot_shell_12B/(cnt_av_pot_shell_12B*2.0);
	av_pot_shell_12K=av_pot_shell_12K/(cnt_av_pot_shell_12K*2.0);
	av_pot_shell_13A=av_pot_shell_13A/(cnt_av_pot_shell_13A*2.0);
	av_pot_shell_13B=av_pot_shell_13B/(cnt_av_pot_shell_13B*2.0);
	av_pot_shell_13K=av_pot_shell_13K/(cnt_av_pot_shell_13K*2.0);
	av_pot_shell_FCC=av_pot_shell_FCC/(cnt_av_pot_shell_FCC*2.0);
	av_pot_shell_HCP=av_pot_shell_HCP/(cnt_av_pot_shell_HCP*2.0);
	av_pot_shell_BCC_9=av_pot_shell_BCC_9/(cnt_av_pot_shell_BCC_9*2.0);
	av_pot_shell_BCC_15=av_pot_shell_BCC_15/(cnt_av_pot_shell_BCC_15*2.0);
	
	printf("\nd%d Potential Energy analysis\n",rank);
	printf("d%d Clust	potential	diff from mean	central part potential	cen diff from mean	shell part potential	shell diff from mean\n",rank);
	printf("d%d sp3	%lg	%lg\n",rank,av_pot_sp3,-av_potential+av_pot_sp3);
	printf("d%d sp3a	%lg	%lg\n",rank,av_pot_sp3a,-av_potential+av_pot_sp3a);
	printf("d%d sp3b	%lg	%lg\n",rank,av_pot_sp3b,-av_potential+av_pot_sp3b);
	printf("d%d 5A_D3h	%lg	%lg\n",rank,av_pot_sp3c,-av_potential+av_pot_sp3c);
	printf("d%d sp4	%lg	%lg\n",rank,av_pot_sp4,-av_potential+av_pot_sp4);
	printf("d%d sp4a	%lg	%lg\n",rank,av_pot_sp4a,-av_potential+av_pot_sp4a);
	printf("d%d sp4b	%lg	%lg\n",rank,av_pot_sp4b,-av_potential+av_pot_sp4b);
	printf("d%d sp4c	%lg	%lg\n",rank,av_pot_sp4c,-av_potential+av_pot_sp4c); 
	printf("d%d 6A_Oh	%lg	%lg\n",rank,av_pot_6A,-av_potential+av_pot_6A); 
	printf("d%d 6Z_C2v	%lg	%lg\n",rank,av_pot_6Z,-av_potential+av_pot_6Z);
	printf("d%d sp5	%lg	%lg\n",rank,av_pot_sp5,-av_potential+av_pot_sp5);
	printf("d%d sp5a	%lg	%lg\n",rank,av_pot_sp5a,-av_potential+av_pot_sp5a);
	printf("d%d sp5b	%lg	%lg\n",rank,av_pot_sp5b,-av_potential+av_pot_sp5b);
	printf("d%d 7A_D5h	%lg	%lg\n",rank,av_pot_sp5c,-av_potential+av_pot_sp5c);
	printf("d%d 7K	%lg	%lg\n",rank,av_pot_7K,-av_potential+av_pot_7K);
	printf("d%d 8A_D2d	%lg	%lg\n",rank,av_pot_8A,-av_potential+av_pot_8A);
	printf("d%d 8B_Cs	%lg	%lg\n",rank,av_pot_8B,-av_potential+av_pot_8B);
	printf("d%d 8K	%lg	%lg\n",rank,av_pot_8K,-av_potential+av_pot_8K);
	printf("d%d 9A_D3h	%lg	%lg\n",rank,av_pot_9A,-av_potential+av_pot_9A);
	printf("d%d 9B_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_9B,-av_potential+av_pot_9B,av_pot_cen_9B,-av_potential+av_pot_cen_9B,av_pot_shell_9B,-av_potential+av_pot_shell_9B);
	printf("d%d 9K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_9K,-av_potential+av_pot_9K,av_pot_cen_9K,-av_potential+av_pot_cen_9K,av_pot_shell_9K,-av_potential+av_pot_shell_9K);
	printf("d%d 10A_D4d	%lg	%lg\n",rank,av_pot_10A,-av_potential+av_pot_10A);
	printf("d%d 10B_C3v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_10B,-av_potential+av_pot_10B,av_pot_cen_10B,-av_potential+av_pot_cen_10B,av_pot_shell_10B,-av_potential+av_pot_shell_10B);
	printf("d%d 10K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_10K,-av_potential+av_pot_10K,av_pot_cen_10K,-av_potential+av_pot_cen_10K,av_pot_shell_10K,-av_potential+av_pot_shell_10K);
	printf("d%d 10W	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_10W,-av_potential+av_pot_10W,av_pot_cen_10W,-av_potential+av_pot_cen_10W,av_pot_shell_10W,-av_potential+av_pot_shell_10W);
	printf("d%d 11A_D4d	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_11A,-av_potential+av_pot_11A,av_pot_cen_11A,-av_potential+av_pot_cen_11A,av_pot_shell_11A,-av_potential+av_pot_shell_11A);
	printf("d%d 11B_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_11B,-av_potential+av_pot_11B,av_pot_cen_11B,-av_potential+av_pot_cen_11B,av_pot_shell_11B,-av_potential+av_pot_shell_11B);
	printf("d%d 11CD	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_11C,-av_potential+av_pot_11C,av_pot_cen_11C,-av_potential+av_pot_cen_11C,av_pot_shell_11C,-av_potential+av_pot_shell_11C);
	printf("d%d 11E_C2	%lg	%lg\n",rank,av_pot_11E,-av_potential+av_pot_11E);
	printf("d%d 11F_C2v	%lg	%lg\n",rank,av_pot_11F,-av_potential+av_pot_11F);
	printf("d%d 11W	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_11W,-av_potential+av_pot_11W,av_pot_cen_11W,-av_potential+av_pot_cen_11W,av_pot_shell_11W,-av_potential+av_pot_shell_11W);
	printf("d%d 12A_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_12A,-av_potential+av_pot_12A,av_pot_cen_12A,-av_potential+av_pot_cen_12A,av_pot_shell_12A,-av_potential+av_pot_shell_12A);
	printf("d%d 12BC	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_12B,-av_potential+av_pot_12B,av_pot_cen_12B,-av_potential+av_pot_cen_12B,av_pot_shell_12B,-av_potential+av_pot_shell_12B);
	printf("d%d 12D_D2d	%lg	%lg\n",rank,av_pot_12D,-av_potential+av_pot_12D);
	printf("d%d 12E_D3h	%lg	%lg\n",rank,av_pot_12E,-av_potential+av_pot_12E);
	printf("d%d 12K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_12K,-av_potential+av_pot_12K,av_pot_cen_12K,-av_potential+av_pot_cen_12K,av_pot_shell_12K,-av_potential+av_pot_shell_12K);
	printf("d%d 13A_Ih	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_13A,-av_potential+av_pot_13A,av_pot_cen_13A,-av_potential+av_pot_cen_13A,av_pot_shell_13A,-av_potential+av_pot_shell_13A);
	printf("d%d 13B_D5h	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_13B,-av_potential+av_pot_13B,av_pot_cen_13B,-av_potential+av_pot_cen_13B,av_pot_shell_13B,-av_potential+av_pot_shell_13B);
	printf("d%d 13K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_13K,-av_potential+av_pot_13K,av_pot_cen_13K,-av_potential+av_pot_cen_13K,av_pot_shell_13K,-av_potential+av_pot_shell_13K);
	printf("d%d FCC_m13	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_FCC,-av_potential+av_pot_FCC,av_pot_cen_FCC,-av_potential+av_pot_cen_FCC,av_pot_shell_FCC,-av_potential+av_pot_shell_FCC);
	printf("d%d HCP_m13	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_HCP,-av_potential+av_pot_HCP,av_pot_cen_HCP,-av_potential+av_pot_cen_HCP,av_pot_shell_HCP,-av_potential+av_pot_shell_HCP);
	printf("d%d BCC_m9	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_BCC_9,-av_potential+av_pot_BCC_9,av_pot_cen_BCC_9,-av_potential+av_pot_cen_BCC_9,av_pot_shell_BCC_9,-av_potential+av_pot_shell_BCC_9);
	printf("d%d BCC_m15	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,av_pot_BCC_15,-av_potential+av_pot_BCC_15,av_pot_cen_BCC_15,-av_potential+av_pot_cen_BCC_15,av_pot_shell_BCC_15,-av_potential+av_pot_shell_BCC_15);
	printf("d%d mean	%lg\n",rank,av_potential);
		
	fPotentialOutput=fopen(filename,"w");
	if (fPotentialOutput==NULL)  {
		sprintf(errMsg,"Norm_Write_Potential(): Error opening file %s",filename);	// Always test file open
		Error(errMsg);
	}
	fprintf(fPotentialOutput,"%s\n",filename);
	
	fprintf(fPotentialOutput,"Clust	potential	diff from mean	central part potential	cen diff from mean	shell part potential	shell diff from mean\n");
	fprintf(fPotentialOutput,"sp3	%lg	%lg\n",av_pot_sp3,-av_potential+av_pot_sp3);
	fprintf(fPotentialOutput,"sp3a	%lg	%lg\n",av_pot_sp3a,-av_potential+av_pot_sp3a);
	fprintf(fPotentialOutput,"sp3b	%lg	%lg\n",av_pot_sp3b,-av_potential+av_pot_sp3b);
	fprintf(fPotentialOutput,"5A_D3h	%lg	%lg\n",av_pot_sp3c,-av_potential+av_pot_sp3c);
	fprintf(fPotentialOutput,"sp4	%lg	%lg\n",av_pot_sp4,-av_potential+av_pot_sp4);
	fprintf(fPotentialOutput,"sp4a	%lg	%lg\n",av_pot_sp4a,-av_potential+av_pot_sp4a);
	fprintf(fPotentialOutput,"sp4b	%lg	%lg\n",av_pot_sp4b,-av_potential+av_pot_sp4b);
	fprintf(fPotentialOutput,"sp4c	%lg	%lg\n",av_pot_sp4c,-av_potential+av_pot_sp4c); 
	fprintf(fPotentialOutput,"6A_Oh	%lg	%lg\n",av_pot_6A,-av_potential+av_pot_6A); 
	fprintf(fPotentialOutput,"6Z_C2v	%lg	%lg\n",av_pot_6Z,-av_potential+av_pot_6Z);
	fprintf(fPotentialOutput,"sp5	%lg	%lg\n",av_pot_sp5,-av_potential+av_pot_sp5);
	fprintf(fPotentialOutput,"sp5a	%lg	%lg\n",av_pot_sp5a,-av_potential+av_pot_sp5a);
	fprintf(fPotentialOutput,"sp5b	%lg	%lg\n",av_pot_sp5b,-av_potential+av_pot_sp5b);
	fprintf(fPotentialOutput,"7A_D5h	%lg	%lg\n",av_pot_sp5c,-av_potential+av_pot_sp5c);
	fprintf(fPotentialOutput,"7K_C2v	%lg	%lg\n",av_pot_7K,-av_potential+av_pot_7K);
	fprintf(fPotentialOutput,"8A_D2d	%lg	%lg\n",av_pot_8A,-av_potential+av_pot_8A);
	fprintf(fPotentialOutput,"8B_Cs	%lg	%lg\n",av_pot_8B,-av_potential+av_pot_8B);
	fprintf(fPotentialOutput,"8K	%lg	%lg\n",av_pot_8K,-av_potential+av_pot_8K);
	fprintf(fPotentialOutput,"9A_D3h	%lg	%lg\n",av_pot_9A,-av_potential+av_pot_9A);
	fprintf(fPotentialOutput,"9B_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_9B,-av_potential+av_pot_9B,av_pot_cen_9B,-av_potential+av_pot_cen_9B,av_pot_shell_9B,-av_potential+av_pot_shell_9B);
	fprintf(fPotentialOutput,"9K	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_9K,-av_potential+av_pot_9K,av_pot_cen_9K,-av_potential+av_pot_cen_9K,av_pot_shell_9K,-av_potential+av_pot_shell_9K);
	fprintf(fPotentialOutput,"10A_D4d	%lg	%lg\n",av_pot_10A,-av_potential+av_pot_10A);
	fprintf(fPotentialOutput,"10B_C3v	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_10B,-av_potential+av_pot_10B,av_pot_cen_10B,-av_potential+av_pot_cen_10B,av_pot_shell_10B,-av_potential+av_pot_shell_10B);
	fprintf(fPotentialOutput,"10K	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_10K,-av_potential+av_pot_10K,av_pot_cen_10K,-av_potential+av_pot_cen_10K,av_pot_shell_10K,-av_potential+av_pot_shell_10K);
	fprintf(fPotentialOutput,"10W	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_10W,-av_potential+av_pot_10W,av_pot_cen_10W,-av_potential+av_pot_cen_10W,av_pot_shell_10W,-av_potential+av_pot_shell_10W);
	fprintf(fPotentialOutput,"11A_D4d	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_11A,-av_potential+av_pot_11A,av_pot_cen_11A,-av_potential+av_pot_cen_11A,av_pot_shell_11A,-av_potential+av_pot_shell_11A);
	fprintf(fPotentialOutput,"11B_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_11B,-av_potential+av_pot_11B,av_pot_cen_11B,-av_potential+av_pot_cen_11B,av_pot_shell_11B,-av_potential+av_pot_shell_11B);
	fprintf(fPotentialOutput,"11CD	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_11C,-av_potential+av_pot_11C,av_pot_cen_11C,-av_potential+av_pot_cen_11C,av_pot_shell_11C,-av_potential+av_pot_shell_11C);
	fprintf(fPotentialOutput,"11E_C2	%lg	%lg\n",av_pot_11E,-av_potential+av_pot_11E);
	fprintf(fPotentialOutput,"11F_C2v	%lg	%lg\n",av_pot_11F,-av_potential+av_pot_11F);
	fprintf(fPotentialOutput,"11W	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_11W,-av_potential+av_pot_11W,av_pot_cen_11W,-av_potential+av_pot_cen_11W,av_pot_shell_11W,-av_potential+av_pot_shell_11W);
	fprintf(fPotentialOutput,"12A_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_12A,-av_potential+av_pot_12A,av_pot_cen_12A,-av_potential+av_pot_cen_12A,av_pot_shell_12A,-av_potential+av_pot_shell_12A);
	fprintf(fPotentialOutput,"12BC	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_12B,-av_potential+av_pot_12B,av_pot_cen_12B,-av_potential+av_pot_cen_12B,av_pot_shell_12B,-av_potential+av_pot_shell_12B);
	fprintf(fPotentialOutput,"12D_D2d	%lg	%lg\n",av_pot_12D,-av_potential+av_pot_12D);
	fprintf(fPotentialOutput,"12E_D3h	%lg	%lg\n",av_pot_12E,-av_potential+av_pot_12E);
	fprintf(fPotentialOutput,"12K	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_12K,-av_potential+av_pot_12K,av_pot_cen_12K,-av_potential+av_pot_cen_12K,av_pot_shell_12K,-av_potential+av_pot_shell_12K);
	fprintf(fPotentialOutput,"13A_Ih	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_13A,-av_potential+av_pot_13A,av_pot_cen_13A,-av_potential+av_pot_cen_13A,av_pot_shell_13A,-av_potential+av_pot_shell_13A);
	fprintf(fPotentialOutput,"13B_D5h	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_13B,-av_potential+av_pot_13B,av_pot_cen_13B,-av_potential+av_pot_cen_13B,av_pot_shell_13B,-av_potential+av_pot_shell_13B);
	fprintf(fPotentialOutput,"13K	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_13K,-av_potential+av_pot_13K,av_pot_cen_13K,-av_potential+av_pot_cen_13K,av_pot_shell_13K,-av_potential+av_pot_shell_13K);
	fprintf(fPotentialOutput,"FCC_m13	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_FCC,-av_potential+av_pot_FCC,av_pot_cen_FCC,-av_potential+av_pot_cen_FCC,av_pot_shell_FCC,-av_potential+av_pot_shell_FCC);
	fprintf(fPotentialOutput,"HCP_m13	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_HCP,-av_potential+av_pot_HCP,av_pot_cen_HCP,-av_potential+av_pot_cen_HCP,av_pot_shell_HCP,-av_potential+av_pot_shell_HCP);
	fprintf(fPotentialOutput,"BCC_m9	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_BCC_9,-av_potential+av_pot_BCC_9,av_pot_cen_BCC_9,-av_potential+av_pot_cen_BCC_9,av_pot_shell_BCC_9,-av_potential+av_pot_shell_BCC_9);
	fprintf(fPotentialOutput,"BCC_m15	%lg	%lg	%lg	%lg	%lg	%lg\n",av_pot_BCC_15,-av_potential+av_pot_BCC_15,av_pot_cen_BCC_15,-av_potential+av_pot_cen_BCC_15,av_pot_shell_BCC_15,-av_potential+av_pot_shell_BCC_15);
	fprintf(fPotentialOutput,"mean	%lg\n",av_potential);
	
	fclose(fPotentialOutput);
	printf("\nd%d Written %s\n\n",rank,filename);
}

void Norm_Write_bonded_to_cen_distro(char *output, int *the_array, int clusSize, int normFactor) {
	FILE *fOut;
	int i;
	char errMsg[1000];
	double tempratio;
	
	fOut=fopen(output,"w");
	if (fOut==NULL)  {
		sprintf(errMsg,"Norm_Write_bonded_to_cen_distro(): Error opening file %s",output);	// Always test file open
		Error(errMsg);
	}
	fprintf(fOut,"%s\n",output);
	
	fprintf(fOut,"N_A in clus	N_B in clus	Freq	Norm\n");
	for (i=0; i<nB+1; i++) {
		tempratio=(double)the_array[i]/(double)(normFactor);
		fprintf(fOut,"%d	%d %d	%.15lg\n",i,clusSize-i,the_array[i],tempratio);
	}
	
	fclose(fOut);
	printf("d%d Written %s\n",rank,output);
}

void Norm_Write_bonded_to_cen(char *filename) {
	FILE *fOut;
	char output[1000], errMsg[1000];
	double tempratio, normFactor;
	
	normFactor=(double)(nc9B);
	sprintf(output,"%s.bonded_to_cen_9B",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_9B[0],9,normFactor);
	normFactor=(double)(nc9K);
	sprintf(output,"%s.bonded_to_cen_9K",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_9K[0],9,normFactor);
	normFactor=(double)(nc10B);
	sprintf(output,"%s.bonded_to_cen_10B",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_10B[0],10,normFactor);
	normFactor=(double)(nc10K);
	sprintf(output,"%s.bonded_to_cen_10K",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_10K[0],9,normFactor);
	normFactor=(double)(nc10W);
	sprintf(output,"%s.bonded_to_cen_10W",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_10W[0],9,normFactor);
	normFactor=(double)(nc11A);
	sprintf(output,"%s.bonded_to_cen_11A",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_11A[0],11,normFactor);
	normFactor=(double)(nc11B);
	sprintf(output,"%s.bonded_to_cen_11B",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_11B[0],11,normFactor);
	normFactor=(double)(nc11C);
	sprintf(output,"%s.bonded_to_cen_11C",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_11C[0],11,normFactor);
	normFactor=(double)(nc11W);
	sprintf(output,"%s.bonded_to_cen_11W",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_11W[0],11,normFactor);
	normFactor=(double)(nc12A);
	sprintf(output,"%s.bonded_to_cen_12A",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_12A[0],12,normFactor);
	normFactor=(double)(nc12B);
	sprintf(output,"%s.bonded_to_cen_12B",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_12B[0],12,normFactor);
	normFactor=(double)(nc12K);
	sprintf(output,"%s.bonded_to_cen_12K",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_12K[0],9,normFactor);
	normFactor=(double)(nc13A);
	sprintf(output,"%s.bonded_to_cen_13A",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_13A[0],13,normFactor);
	normFactor=(double)(nc13B);
	sprintf(output,"%s.bonded_to_cen_13B",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_13B[0],13,normFactor);
	normFactor=(double)(nc13K);
	sprintf(output,"%s.bonded_to_cen_13K",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_13K[0],9,normFactor);
	normFactor=(double)(ncFCC);
	sprintf(output,"%s.bonded_to_cen_FCC",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_FCC[0],13,normFactor);
	normFactor=(double)(ncHCP);
	sprintf(output,"%s.bonded_to_cen_HCP",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_HCP[0],13,normFactor);
	normFactor=(double)(ncBCC_9);
	sprintf(output,"%s.bonded_to_cen_BCC_9",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_BCC_9[0],9,normFactor);
	normFactor=(double)(ncBCC_15);
	sprintf(output,"%s.bonded_to_cen_BCC_15",filename);
	Norm_Write_bonded_to_cen_distro(output,&n_distro_bonded_to_cen_BCC_15[0],15,normFactor);
	
	sprintf(output,"%s.bonded_to_cen",filename);
	fOut=fopen(output,"w");
	if (fOut==NULL)  {
		sprintf(errMsg,"Norm_Write_bonded_to_cen(): Error opening file %s",output);	// Always test file open
		Error(errMsg);
	}
	fprintf(fOut,"%s\n",output);
	
	printf("d%d Number Bonded to Central Particle Analysis\nd%d Clust Mean Bonded to Centre\n",rank,rank);
	tempratio=(double)n_bonded_to_cen_9B/(double)nc9B;
	printf("d%d 9B_C2v	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_9K/(double)nc9K;
	printf("d%d 9K	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_10B/(double)nc10B;
	printf("d%d 10B_C3v	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_11A/(double)nc11A;
	printf("d%d 11A_D4d	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_10K/(double)nc10K;
	printf("d%d 10K	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_10W/(double)nc10W;
	printf("d%d 10W	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_11B/(double)nc11B;
	printf("d%d 11B_C2v	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_11C/(double)nc11C;
	printf("d%d 11CD	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_11W/(double)nc11W;
	printf("d%d 11W	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_12A/(double)nc12A;
	printf("d%d 12A_C2v	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_12B/(double)nc12B;
	printf("d%d 12BC	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_12K/(double)nc12K;
	printf("d%d 12	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_13A/(double)nc13A;
	printf("d%d 13A_Ih	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_13B/(double)nc13B;
	printf("d%d 13B_D5h	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_13K/(double)nc13K;
	printf("d%d 13K	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_FCC/(double)ncFCC;
	printf("d%d FCC_m13	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_HCP/(double)ncHCP;
	printf("d%d HCP_m13	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_BCC_9/(double)ncBCC_9;
	printf("d%d BCC_m9	%lg\n",rank,tempratio);
	tempratio=(double)n_bonded_to_cen_BCC_15/(double)ncBCC_15;
	printf("d%d BCC_m15	%lg\n",rank,tempratio);
	
	fprintf(fOut,"\nNumber Bonded to Central Particle Analysis\nClust Mean Bonded to Centre\n");
	tempratio=(double)n_bonded_to_cen_9B/(double)nc9B;
	fprintf(fOut,"9B_C2v	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_9K/(double)nc9K;
	fprintf(fOut,"9K	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_10B/(double)nc10B;
	fprintf(fOut,"10B_C3v	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_10K/(double)nc10K;
	fprintf(fOut,"10K	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_10W/(double)nc10W;
	fprintf(fOut,"10W	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_11A/(double)nc11A;
	fprintf(fOut,"11A_D4d	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_11B/(double)nc11B;
	fprintf(fOut,"11B_C2v	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_11C/(double)nc11C;
	fprintf(fOut,"11CD	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_11W/(double)nc11W;
	fprintf(fOut,"11W	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_12A/(double)nc12A;
	fprintf(fOut,"12A_C2v	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_12B/(double)nc12B;
	fprintf(fOut,"12BC	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_12K/(double)nc12K;
	fprintf(fOut,"12K	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_13A/(double)nc13A;
	fprintf(fOut,"13A_Ih	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_13B/(double)nc13B;
	fprintf(fOut,"13B_D5h	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_13K/(double)nc13K;
	fprintf(fOut,"13K	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_FCC/(double)ncFCC;
	fprintf(fOut,"FCC_m13	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_HCP/(double)ncHCP;
	fprintf(fOut,"HCP_m13	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_BCC_9/(double)ncBCC_9;
	fprintf(fOut,"BCC_m9	%lg\n",tempratio);
	tempratio=(double)n_bonded_to_cen_BCC_15/(double)ncBCC_15;
	fprintf(fOut,"BCC_m15	%lg\n",tempratio);

	fclose(fOut);
	printf("\nd%d Written %s\n\n",rank,output);
}

void Norm_Write_ClusComp_distro(int clus_cen_shell, char *output, int *the_array, int clusSize, double normFactor) {
	FILE *fOut;
	int i;
	char errMsg[1000];
	double tempratio;
	
	fOut=fopen(output,"w");
	if (fOut==NULL)  {
		sprintf(errMsg,"Norm_Write_ClusComp_distro(): Error opening file %s",output);	// Always test file open
		Error(errMsg);
	}
	fprintf(fOut,"%s\n",output);
	
	if (clus_cen_shell==0) {
		fprintf(fOut,"N_A in clus	N_B in clus	Freq	Norm\n");
		for (i=0; i<clusSize+1; i++) {
			tempratio=(double)the_array[i]/(normFactor);
			fprintf(fOut,"%d	%d	%d	%.15lg\n",i,clusSize-i,the_array[i],tempratio);
		}
	}
	else if (clus_cen_shell==1) {
		fprintf(fOut,"N_A in cen	N_B in cen	Freq	Norm\n");
		for (i=0; i<2; i++) {
			tempratio=(double)the_array[i]/(normFactor);
			fprintf(fOut,"%d	%d	%d	%.15lg\n",i,1-i,the_array[i],tempratio);
		}
	}
	else if (clus_cen_shell==2) {
		fprintf(fOut,"N_A in shell	N_B in shell	Freq	Norm\n");
		for (i=0; i<clusSize; i++) {
			tempratio=(double)the_array[i]/(normFactor);
			fprintf(fOut,"%d	%d	%d	%.15lg\n",i,clusSize-1-i,the_array[i],tempratio);
		}
	}
	
	fclose(fOut);
	printf("d%d Written %s\n",rank,output);
}

void Norm_Write_ClusComp(char *filename) {
	FILE *fClusCompOutput;
	char output[1000], errMsg[1000];
	double tempratioA, tempratioB, tempratioAcen, tempratioBcen, tempratioAshell, tempratioBshell, normFactor;
	
	normFactor=(double)(ncsp3);
	sprintf(output,"%s.comp_sp3",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp3[0],3,normFactor);
	normFactor=(double)(ncsp3b);
	sprintf(output,"%s.comp_sp3a",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp3a[0],3,normFactor);
	normFactor=(double)(ncsp3b);
	sprintf(output,"%s.comp_sp3b",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp3b[0],4,normFactor);
	normFactor=(double)(nc5A);
	sprintf(output,"%s.comp_sp3c",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp3c[0],5,normFactor);
	normFactor=(double)(ncsp4);
	sprintf(output,"%s.comp_sp4",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp4[0],4,normFactor);
	normFactor=(double)(ncsp4a);
	sprintf(output,"%s.comp_sp4a",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp4a[0],4,normFactor);
	normFactor=(double)(ncsp4b);
	sprintf(output,"%s.comp_sp4b",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp4b[0],5,normFactor);
	normFactor=(double)(ncsp4c);
	sprintf(output,"%s.comp_sp4c",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp4c[0],6,normFactor);
	normFactor=(double)(nc6A);
	sprintf(output,"%s.comp_6A",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_6A[0],6,normFactor);
	normFactor=(double)(ncsp5);
	sprintf(output,"%s.comp_sp5",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp5[0],5,normFactor);
	normFactor=(double)(ncsp5a);
	sprintf(output,"%s.comp_sp5a",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp5a[0],5,normFactor);
	normFactor=(double)(ncsp5b);
	sprintf(output,"%s.comp_sp5b",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp5b[0],6,normFactor);
	normFactor=(double)(nc7A);
	sprintf(output,"%s.comp_sp5c",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_sp5c[0],7,normFactor);
	normFactor=(double)(nc6Z);
	sprintf(output,"%s.comp_6Z",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_6Z[0],6,normFactor);
	normFactor=(double)(nc7K);
	sprintf(output,"%s.comp_7K",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_7K[0],7,normFactor);
	normFactor=(double)(nc8A);
	sprintf(output,"%s.comp_8A",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_8A[0],8,normFactor);
	normFactor=(double)(nc8B);
	sprintf(output,"%s.comp_8B",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_8B[0],8,normFactor);
	normFactor=(double)(nc8K);
	sprintf(output,"%s.comp_8K",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_8K[0],8,normFactor);
	normFactor=(double)(nc9A);
	sprintf(output,"%s.comp_9A",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_9A[0],9,normFactor);
	normFactor=(double)(nc9B);
	sprintf(output,"%s.comp_9B",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_9B[0],9,normFactor);
	normFactor=(double)(nc9B);
	sprintf(output,"%s.comp_9B_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_9B[0],9,normFactor);
	normFactor=(double)(nc9B);
	sprintf(output,"%s.comp_9B_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_9B[0],9,normFactor);
	normFactor=(double)(nc9K);
	sprintf(output,"%s.comp_9K",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_9K[0],9,normFactor);
	normFactor=(double)(nc9K);
	sprintf(output,"%s.comp_9K_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_9K[0],9,normFactor);
	normFactor=(double)(nc9K);
	sprintf(output,"%s.comp_9K_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_9K[0],9,normFactor);
	normFactor=(double)(nc10A);
	sprintf(output,"%s.comp_10A",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_10A[0],10,normFactor);
	normFactor=(double)(nc10B);
	sprintf(output,"%s.comp_10B",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_10B[0],10,normFactor);
	normFactor=(double)(nc10B);
	sprintf(output,"%s.comp_10B_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_10B[0],10,normFactor);
	normFactor=(double)(nc10B);
	sprintf(output,"%s.comp_10B_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_10B[0],10,normFactor);
	normFactor=(double)(nc10K);
	sprintf(output,"%s.comp_10K",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_10K[0],10,normFactor);
	normFactor=(double)(nc10K);
	sprintf(output,"%s.comp_10K_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_10K[0],10,normFactor);
	normFactor=(double)(nc10K);
	sprintf(output,"%s.comp_10K_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_10K[0],10,normFactor);
	normFactor=(double)(nc10W);
	sprintf(output,"%s.comp_10W",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_10W[0],10,normFactor);
	normFactor=(double)(nc10W);
	sprintf(output,"%s.comp_10W_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_10W[0],10,normFactor);
	normFactor=(double)(nc10W);
	sprintf(output,"%s.comp_10W_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_10W[0],10,normFactor);
	normFactor=(double)(nc11A);
	sprintf(output,"%s.comp_11A",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_11A[0],11,normFactor);
	normFactor=(double)(nc11A);
	sprintf(output,"%s.comp_11A_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_11A[0],11,normFactor);
	normFactor=(double)(nc11A);
	sprintf(output,"%s.comp_11A_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_11A[0],11,normFactor);
	normFactor=(double)(nc11B);
	sprintf(output,"%s.comp_11B",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_11B[0],11,normFactor);
	normFactor=(double)(nc11B);
	sprintf(output,"%s.comp_11B_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_11B[0],11,normFactor);
	normFactor=(double)(nc11B);
	sprintf(output,"%s.comp_11B_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_11B[0],11,normFactor);
	normFactor=(double)(nc11C);
	sprintf(output,"%s.comp_11C",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_11C[0],11,normFactor);
	normFactor=(double)(nc11C);
	sprintf(output,"%s.comp_11C_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_11C[0],11,normFactor);
	normFactor=(double)(nc11C);
	sprintf(output,"%s.comp_11C_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_11C[0],11,normFactor);
	normFactor=(double)(nc11E);
	sprintf(output,"%s.comp_11E",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_11E[0],11,normFactor);
	normFactor=(double)(nc11F);
	sprintf(output,"%s.comp_11F",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_11F[0],11,normFactor);
	normFactor=(double)(nc11W);
	sprintf(output,"%s.comp_11W",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_11W[0],11,normFactor);
	normFactor=(double)(nc11W);
	sprintf(output,"%s.comp_11W_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_11W[0],11,normFactor);
	normFactor=(double)(nc11W);
	sprintf(output,"%s.comp_11W_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_11W[0],11,normFactor);
	normFactor=(double)(nc12A);
	sprintf(output,"%s.comp_12A",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_12A[0],12,normFactor);
	normFactor=(double)(nc12A);
	sprintf(output,"%s.comp_12A_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_12A[0],12,normFactor);
	normFactor=(double)(nc12A);
	sprintf(output,"%s.comp_12A_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_12A[0],12,normFactor);
	normFactor=(double)(nc12B);
	sprintf(output,"%s.comp_12B",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_12B[0],12,normFactor);
	normFactor=(double)(nc12B);
	sprintf(output,"%s.comp_12B_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_12B[0],12,normFactor);
	normFactor=(double)(nc12B);
	sprintf(output,"%s.comp_12B_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_12B[0],12,normFactor);
	normFactor=(double)(nc12D);
	sprintf(output,"%s.comp_12D",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_12D[0],12,normFactor);
	normFactor=(double)(nc12E);
	sprintf(output,"%s.comp_12E",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_12E[0],12,normFactor);
	normFactor=(double)(nc12K);
	sprintf(output,"%s.comp_12K",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_12K[0],12,normFactor);
	normFactor=(double)(nc12K);
	sprintf(output,"%s.comp_12K_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_12K[0],12,normFactor);
	normFactor=(double)(nc12K);
	sprintf(output,"%s.comp_12K_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_12K[0],12,normFactor);
	normFactor=(double)(nc13A);
	sprintf(output,"%s.comp_13A",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_13A[0],13,normFactor);
	normFactor=(double)(nc13A);
	sprintf(output,"%s.comp_13A_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_13A[0],13,normFactor);
	normFactor=(double)(nc13A);
	sprintf(output,"%s.comp_13A_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_13A[0],13,normFactor);
	normFactor=(double)(nc13B);
	sprintf(output,"%s.comp_13B",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_13B[0],13,normFactor);
	normFactor=(double)(nc13B);
	sprintf(output,"%s.comp_13B_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_13B[0],13,normFactor);
	normFactor=(double)(nc13B);
	sprintf(output,"%s.comp_13B_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_13B[0],13,normFactor);
	normFactor=(double)(nc13K);
	sprintf(output,"%s.comp_13K",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_13K[0],13,normFactor);
	normFactor=(double)(nc13K);
	sprintf(output,"%s.comp_13K_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_13K[0],13,normFactor);
	normFactor=(double)(nc13K);
	sprintf(output,"%s.comp_13K_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_13K[0],13,normFactor);
	normFactor=(double)(ncFCC);
	sprintf(output,"%s.comp_FCC",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_FCC[0],13,normFactor);
	normFactor=(double)(ncFCC);
	sprintf(output,"%s.comp_FCC_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_FCC[0],13,normFactor);
	normFactor=(double)(ncFCC);
	sprintf(output,"%s.comp_FCC_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_FCC[0],13,normFactor);
	normFactor=(double)(ncHCP);
	sprintf(output,"%s.comp_HCP",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_HCP[0],13,normFactor);
	normFactor=(double)(ncHCP);
	sprintf(output,"%s.comp_HCP_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_HCP[0],13,normFactor);
	normFactor=(double)(ncHCP);
	sprintf(output,"%s.comp_HCP_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_HCP[0],13,normFactor);
	normFactor=(double)(ncBCC_9);
	sprintf(output,"%s.comp_BCC_9",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_BCC_9[0],9,normFactor);
	normFactor=(double)(ncBCC_9);
	sprintf(output,"%s.comp_BCC_9_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_BCC_9[0],9,normFactor);
	normFactor=(double)(ncBCC_9);
	sprintf(output,"%s.comp_BCC_9_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_BCC_9[0],9,normFactor);
	normFactor=(double)(ncBCC_15);
	sprintf(output,"%s.comp_BCC_15",filename);
	Norm_Write_ClusComp_distro(0,output,&n_distro_BCC_15[0],15,normFactor);
	normFactor=(double)(ncBCC_15);
	sprintf(output,"%s.comp_BCC_15_cen",filename);
	Norm_Write_ClusComp_distro(1,output,&n_distro_cen_BCC_15[0],15,normFactor);
	normFactor=(double)(ncBCC_15);
	sprintf(output,"%s.comp_BCC_15_shell",filename);
	Norm_Write_ClusComp_distro(2,output,&n_distro_shell_BCC_15[0],15,normFactor);

	sprintf(output,"%s.cluscomp",filename);
	fClusCompOutput=fopen(output,"w");
	if (fClusCompOutput==NULL)  {
		sprintf(errMsg,"Norm_Write_ClusComp(): Error opening file %s",output);	// Always test file open
		Error(errMsg);
	}
	fprintf(fClusCompOutput,"%s\n",output);
		
	printf("\nd%d Cluster Composition analysis\n",rank);
	printf("d%d Clust	nA	nB	nA central	nB central	nA shell	nB shell\n",rank);
	
	fprintf(fClusCompOutput,"Clust	nA	nB	nA central	nB central	nA shell	nB shell\n");
	
	tempratioA=(double)nAsp3/(double)(3.0*ncsp3);
	tempratioB=(double)nBsp3/(double)(3.0*ncsp3);
	printf("d%d sp3	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp3	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp3a/(double)(3.0*ncsp3a);
	tempratioB=(double)nBsp3a/(double)(3.0*ncsp3a);
	printf("d%d sp3a	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp3a	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp3b/(double)(4.0*ncsp3b);
	tempratioB=(double)nBsp3b/(double)(4.0*ncsp3b);
	printf("d%d sp3b	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp3b	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp3c/(double)(5.0*nc5A);
	tempratioB=(double)nBsp3c/(double)(5.0*nc5A);
	printf("d%d 5A_D3h	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"5A_D3h	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp4/(double)(4.0*ncsp4);
	tempratioB=(double)nBsp4/(double)(4.0*ncsp4);
	printf("d%d sp4	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp4	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp4a/(double)(4.0*ncsp4a);
	tempratioB=(double)nBsp4a/(double)(4.0*ncsp4a);
	printf("d%d sp4a	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp4a	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp4b/(double)(5.0*ncsp4b);
	tempratioB=(double)nBsp4b/(double)(5.0*ncsp4b);
	printf("d%d sp4b	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp4b	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp4c/(double)(6.0*ncsp4c);
	tempratioB=(double)nBsp4c/(double)(6.0*ncsp4c);
	printf("d%d sp4b	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp4c	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA6A/(double)(6.0*nc6A);
	tempratioB=(double)nB6A/(double)(6.0*nc6A);
	printf("d%d 6A_Oh	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"6A_Oh	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp5/(double)(5.0*ncsp5);
	tempratioB=(double)nBsp5/(double)(5.0*ncsp5);
	printf("d%d sp5	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp5	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp5a/(double)(5.0*ncsp5a);
	tempratioB=(double)nBsp5a/(double)(5.0*ncsp5a);
	printf("d%d sp5a	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp5a	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp5b/(double)(6.0*ncsp5b);
	tempratioB=(double)nBsp5b/(double)(6.0*ncsp5b);
	printf("d%d sp5b	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"sp5b	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nAsp5c/(double)(7.0*nc7A);
	tempratioB=(double)nBsp5c/(double)(7.0*nc7A);
	printf("d%d 7A_D5h	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"7A_D5h	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA6Z/(double)(6.0*nc6Z);
	tempratioB=(double)nB6Z/(double)(6.0*nc6Z);
	printf("d%d 6Z_C2v	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"6Z_C2v	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA7K/(double)(7.0*nc7K);
	tempratioB=(double)nB7K/(double)(7.0*nc7K);
	printf("d%d 7K_C2v	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"7K_C2v	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA8A/(double)(8.0*nc8A);
	tempratioB=(double)nB8A/(double)(8.0*nc8A);
	printf("d%d 8A_D2d	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"8A_D2d	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA8B/(double)(8.0*nc8B);
	tempratioB=(double)nB8B/(double)(8.0*nc8B);
	printf("d%d 8B_Cs	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"8B_Cs	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA8K/(double)(8.0*nc8K);
	tempratioB=(double)nB8K/(double)(8.0*nc8K);
	printf("d%d 8K	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"8K	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA9A/(double)(9.0*nc9A);
	tempratioB=(double)nB9A/(double)(9.0*nc9A);
	printf("d%d 9A_D3h	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"9A_D3h	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA9B/(double)(9.0*nc9B);
	tempratioB=(double)nB9B/(double)(9.0*nc9B);
	tempratioAcen=(double)nA_cen_9B/(double)(nc9B);
	tempratioBcen=(double)nB_cen_9B/(double)(nc9B);
	tempratioAshell=(double)nA_shell_9B/(double)(8.0*nc9B);
	tempratioBshell=(double)nB_shell_9B/(double)(8.0*nc9B);
	printf("d%d 9B_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"9B_C2v	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA9K/(double)(9.0*nc9K);
	tempratioB=(double)nB9K/(double)(9.0*nc9K);
	tempratioAcen=(double)nA_cen_9K/(double)(nc9K);
	tempratioBcen=(double)nB_cen_9K/(double)(nc9K);
	tempratioAshell=(double)nA_shell_9K/(double)(8.0*nc9K);
	tempratioBshell=(double)nB_shell_9K/(double)(8.0*nc9K);
	printf("d%d 9K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"9K	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA10A/(double)(10.0*nc10A);
	tempratioB=(double)nB10A/(double)(10.0*nc10A);
	printf("d%d 10A_D4d	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"10A_D4d	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA10B/(double)(10.0*nc10B);
	tempratioB=(double)nB10B/(double)(10.0*nc10B);
	tempratioAcen=(double)nA_cen_10B/(double)(nc10B);
	tempratioBcen=(double)nB_cen_10B/(double)(nc10B);
	tempratioAshell=(double)nA_shell_10B/(double)(9.0*nc10B);
	tempratioBshell=(double)nB_shell_10B/(double)(9.0*nc10B);
	printf("d%d 10B_C3v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"10B_C3v	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA10K/(double)(10.0*nc10K);
	tempratioB=(double)nB10K/(double)(10.0*nc10K);
	tempratioAcen=(double)nA_cen_10K/(double)(nc10K);
	tempratioBcen=(double)nB_cen_10K/(double)(nc10K);
	tempratioAshell=(double)nA_shell_10K/(double)(9.0*nc10K);
	tempratioBshell=(double)nB_shell_10K/(double)(9.0*nc10K);
	printf("d%d 10K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"10K	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA10W/(double)(10.0*nc10W);
	tempratioB=(double)nB10W/(double)(10.0*nc10W);
	tempratioAcen=(double)nA_cen_10W/(double)(nc10W);
	tempratioBcen=(double)nB_cen_10W/(double)(nc10W);
	tempratioAshell=(double)nA_shell_10W/(double)(9.0*nc10W);
	tempratioBshell=(double)nB_shell_10W/(double)(9.0*nc10W);
	printf("d%d 10W	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"10W	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA11A/(double)(11.0*nc11A);
	tempratioB=(double)nB11A/(double)(11.0*nc11A);
	tempratioAcen=(double)nA_cen_11A/(double)(nc11A);
	tempratioBcen=(double)nB_cen_11A/(double)(nc11A);
	tempratioAshell=(double)nA_shell_11A/(double)(10.0*nc11A);
	tempratioBshell=(double)nB_shell_11A/(double)(10.0*nc11A);
	printf("d%d 11A_D4d	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"11A_D4d	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA11B/(double)(11.0*nc11B);
	tempratioB=(double)nB11B/(double)(11.0*nc11B);
	tempratioAcen=(double)nA_cen_11B/(double)(nc11B);
	tempratioBcen=(double)nB_cen_11B/(double)(nc11B);
	tempratioAshell=(double)nA_shell_11B/(double)(10.0*nc11B);
	tempratioBshell=(double)nB_shell_11B/(double)(10.0*nc11B);
	printf("d%d 11B_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"11B_C2v	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA11C/(double)(11.0*nc11C);
	tempratioB=(double)nB11C/(double)(11.0*nc11C);
	tempratioAcen=(double)nA_cen_11C/(double)(nc11C);
	tempratioBcen=(double)nB_cen_11C/(double)(nc11C);
	tempratioAshell=(double)nA_shell_11C/(double)(10.0*nc11C);
	tempratioBshell=(double)nB_shell_11C/(double)(10.0*nc11C);
	printf("d%d 11CD	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"11CD	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA11E/(double)(11.0*nc11E);
	tempratioB=(double)nB11E/(double)(11.0*nc11E);
	printf("d%d 11E_C2	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"11E_C2	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA11F/(double)(11.0*nc11F);
	tempratioB=(double)nB11F/(double)(11.0*nc11F);
	printf("d%d 11F_C2v	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"11F_C2v	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA11W/(double)(11.0*nc11W);
	tempratioB=(double)nB11W/(double)(11.0*nc11W);
	tempratioAcen=(double)nA_cen_11W/(double)(nc11W);
	tempratioBcen=(double)nB_cen_11W/(double)(nc11W);
	tempratioAshell=(double)nA_shell_11W/(double)(10.0*nc11W);
	tempratioBshell=(double)nB_shell_11W/(double)(10.0*nc11W);
	printf("d%d 11W	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"11W	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA12A/(double)(12.0*nc12A);
	tempratioB=(double)nB12A/(double)(12.0*nc12A);
	tempratioAcen=(double)nA_cen_12A/(double)(nc12A);
	tempratioBcen=(double)nB_cen_12A/(double)(nc12A);
	tempratioAshell=(double)nA_shell_12A/(double)(11.0*nc12A);
	tempratioBshell=(double)nB_shell_12A/(double)(11.0*nc12A);
	printf("d%d 12A_C2v	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"12A_C2v	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA12B/(double)(12.0*nc12B);
	tempratioB=(double)nB12B/(double)(12.0*nc12B);
	tempratioAcen=(double)nA_cen_12B/(double)(nc12B);
	tempratioBcen=(double)nB_cen_12B/(double)(nc12B);
	tempratioAshell=(double)nA_shell_12B/(double)(11.0*nc12B);
	tempratioBshell=(double)nB_shell_12B/(double)(11.0*nc12B);
	printf("d%d 12BC	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"12BC	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA12D/(double)(12.0*nc12D);
	tempratioB=(double)nB12D/(double)(12.0*nc12D);
	printf("d%d 12D_D2d	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"12D_D2d	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA12E/(double)(12.0*nc12E);
	tempratioB=(double)nB12E/(double)(12.0*nc12E);
	printf("d%d 12E_D3h	%lg	%lg\n",rank,tempratioA,tempratioB);
	fprintf(fClusCompOutput,"12E_D3h	%.15lg	%.15lg\n",tempratioA,tempratioB);
	
	tempratioA=(double)nA12K/(double)(12.0*nc12K);
	tempratioB=(double)nB12K/(double)(12.0*nc12K);
	tempratioAcen=(double)nA_cen_12K/(double)(nc12K);
	tempratioBcen=(double)nB_cen_12K/(double)(nc12K);
	tempratioAshell=(double)nA_shell_12K/(double)(11.0*nc12K);
	tempratioBshell=(double)nB_shell_12K/(double)(11.0*nc12K);
	printf("d%d 12K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"12K	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA13A/(double)(13.0*nc13A);
	tempratioB=(double)nB13A/(double)(13.0*nc13A);
	tempratioAcen=(double)nA_cen_13A/(double)(nc13A);
	tempratioBcen=(double)nB_cen_13A/(double)(nc13A);
	tempratioAshell=(double)nA_shell_13A/(double)(12.0*nc13A);
	tempratioBshell=(double)nB_shell_13A/(double)(12.0*nc13A);
	printf("d%d 13A_Ih	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"13A_Ih	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nA13B/(double)(13.0*nc13B);
	tempratioB=(double)nB13B/(double)(13.0*nc13B);
	tempratioAcen=(double)nA_cen_13B/(double)(nc13B);
	tempratioBcen=(double)nB_cen_13B/(double)(nc13B);
	tempratioAshell=(double)nA_shell_13B/(double)(12.0*nc13B);
	tempratioBshell=(double)nB_shell_13B/(double)(12.0*nc13B);
	printf("d%d 13B_D5h	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"13B_D5h	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);

	tempratioA=(double)nA13K/(double)(13.0*nc13K);
	tempratioB=(double)nB13K/(double)(13.0*nc13K);
	tempratioAcen=(double)nA_cen_13K/(double)(nc13K);
	tempratioBcen=(double)nB_cen_13K/(double)(nc13K);
	tempratioAshell=(double)nA_shell_13K/(double)(12.0*nc13K);
	tempratioBshell=(double)nB_shell_13K/(double)(12.0*nc13K);
	printf("d%d 13K	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"13K	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nAFCC/(double)(13.0*ncFCC);
	tempratioB=(double)nBFCC/(double)(13.0*ncFCC);
	tempratioAcen=(double)nA_cen_FCC/(double)(ncFCC);
	tempratioBcen=(double)nB_cen_FCC/(double)(ncFCC);
	tempratioAshell=(double)nA_shell_FCC/(double)(12.0*ncFCC);
	tempratioBshell=(double)nB_shell_FCC/(double)(12.0*ncFCC);
	printf("d%d FCC_m13	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"FCC_m13	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nAHCP/(double)(13.0*ncHCP);
	tempratioB=(double)nBHCP/(double)(13.0*ncHCP);
	tempratioAcen=(double)nA_cen_HCP/(double)(ncHCP);
	tempratioBcen=(double)nB_cen_HCP/(double)(ncHCP);
	tempratioAshell=(double)nA_shell_HCP/(double)(12.0*ncHCP);
	tempratioBshell=(double)nB_shell_HCP/(double)(12.0*ncHCP);
	printf("d%d HCP_m13	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"HCP_m13	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);

	tempratioA=(double)nABCC_9/(double)(9.0*ncBCC_9);
	tempratioB=(double)nBBCC_9/(double)(9.0*ncBCC_9);
	tempratioAcen=(double)nA_cen_BCC_9/(double)(ncBCC_9);
	tempratioBcen=(double)nB_cen_BCC_9/(double)(ncBCC_9);
	tempratioAshell=(double)nA_shell_BCC_9/(double)(8.0*ncBCC_9);
	tempratioBshell=(double)nB_shell_BCC_9/(double)(8.0*ncBCC_9);
	printf("d%d BCC_m9	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"BCC_m9	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	
	tempratioA=(double)nABCC_15/(double)(15.0*ncBCC_15);
	tempratioB=(double)nBBCC_15/(double)(15.0*ncBCC_15);
	tempratioAcen=(double)nA_cen_BCC_15/(double)(ncBCC_15);
	tempratioBcen=(double)nB_cen_BCC_15/(double)(ncBCC_15);
	tempratioAshell=(double)nA_shell_BCC_15/(double)(14.0*ncBCC_15);
	tempratioBshell=(double)nB_shell_BCC_15/(double)(14.0*ncBCC_15);
	printf("d%d BCC_m15	%lg	%lg	%lg	%lg	%lg	%lg\n",rank,tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);
	fprintf(fClusCompOutput,"BCC_m15	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",tempratioA,tempratioB,tempratioAcen,tempratioBcen,tempratioAshell,tempratioBshell);

	
	fclose(fClusCompOutput);
	printf("\nd%d Written %s\n\n",rank,output);
}

void Norm_Write_bl_mom(char *filename) {
	FILE *f_bl_mom;
	char  errMsg[1000];
	
	mean_bl_mom_sp3/=ncsp3;
	mean_bl_mom_sp3a/=ncsp3a;
	mean_bl_mom_sp3b/=ncsp3b;
	mean_bl_mom_sp3c/=nc5A;
	mean_bl_mom_sp4/=ncsp4;
	mean_bl_mom_sp4a/=ncsp4a;
	mean_bl_mom_sp4b/=ncsp4b;
	mean_bl_mom_sp4c/=ncsp4c;
	mean_bl_mom_6A/=nc6A;
	mean_bl_mom_sp5/=ncsp5;
	mean_bl_mom_sp5a/=ncsp5a;
	mean_bl_mom_sp5b/=ncsp5b;
	mean_bl_mom_sp5c/=nc7A;
	
	printf("\nd%d Cluster bond length 2nd moment deviation analysis\n",rank);
	printf("d%d Clust	mean 2nd moment\n",rank);
	printf("d%d sp3	%lg\n",rank,mean_bl_mom_sp3);
	printf("d%d sp3a	%lg\n",rank,mean_bl_mom_sp3a);
	printf("d%d sp3b	%lg\n",rank,mean_bl_mom_sp3b);
	printf("d%d 5A_D3h	%lg\n",rank,mean_bl_mom_sp3c);
	printf("d%d sp4	%lg\n",rank,mean_bl_mom_sp4);
	printf("d%d sp4a	%lg\n",rank,mean_bl_mom_sp4a);
	printf("d%d sp4b	%lg\n",rank,mean_bl_mom_sp4b);
	printf("d%d sp4c	%lg\n",rank,mean_bl_mom_sp4c); 
	printf("d%d 6A_Oh	%lg\n",rank,mean_bl_mom_6A); 
	printf("d%d sp5	%lg\n",rank,mean_bl_mom_sp5);
	printf("d%d sp5a	%lg\n",rank,mean_bl_mom_sp5a);
	printf("d%d sp5b	%lg\n",rank,mean_bl_mom_sp5b);
	printf("d%d 7A_D5h	%lg\n",rank,mean_bl_mom_sp5c);
	
	f_bl_mom=fopen(filename,"w");
	if (f_bl_mom==NULL)  {
		sprintf(errMsg,"Norm_Write_Potential(): Error opening file %s",filename);	// Always test file open
		Error(errMsg);
	}
	fprintf(f_bl_mom,"%s\n",filename);

	fprintf(f_bl_mom,"Clust	mean 2nd moment\n");
	fprintf(f_bl_mom,"sp3	%lg\n",mean_bl_mom_sp3);
	fprintf(f_bl_mom,"sp3a	%lg\n",mean_bl_mom_sp3a);
	fprintf(f_bl_mom,"sp3b	%lg\n",mean_bl_mom_sp3b);
	fprintf(f_bl_mom,"5A_D3h	%lg\n",mean_bl_mom_sp3c);
	fprintf(f_bl_mom,"sp4	%lg\n",mean_bl_mom_sp4);
	fprintf(f_bl_mom,"sp4a	%lg\n",mean_bl_mom_sp4a);
	fprintf(f_bl_mom,"sp4b	%lg\n",mean_bl_mom_sp4b);
	fprintf(f_bl_mom,"sp4c	%lg\n",mean_bl_mom_sp4c); 
	fprintf(f_bl_mom,"6A_Oh	%lg\n",mean_bl_mom_6A); 
	fprintf(f_bl_mom,"sp5	%lg\n",mean_bl_mom_sp5);
	fprintf(f_bl_mom,"sp5a	%lg\n",mean_bl_mom_sp5a);
	fprintf(f_bl_mom,"sp5b	%lg\n",mean_bl_mom_sp5b);
	fprintf(f_bl_mom,"7A_D5h	%lg\n",mean_bl_mom_sp5c);
	
	fclose(f_bl_mom);
	printf("\nd%d Written %s\n\n",rank,filename);
}

void Coslovich(int f) {
	int i, j, k;
	int cnt;
	int n2, n3, n4, n5, n6, n7, n8;
	
	for (i=0; i<N; i++) {
		s_s_0_2_8[i]='C';
		s_s_1_2_5_3[i]='C';
		s_s_1_2_5_2[i]='C';
		s_s_0_3_6[i]='C';
		s_s_0_0_12[i]='C';
		s_b_0_2_8_4[i]='C';
		s_b_0_2_8_5[i]='C';
		s_b_0_3_6_6[i]='C';
		s_b_0_1_10_4[i]='C';
	}
	
	for (i=0; i<N; i++) {
		if (rtype[i]!=2) continue;
		n2=n3=n4=n5=n6=n7=n8=0;
		for (j=0; j<cnb[i]; j++) {
			cnt=0;
			for (k=0; k<cnb[i]; k++) {
				if (k==j) continue;
				if (Bonds_BondCheck(bNums[i][j],bNums[i][k])==1) cnt++;
			}
			if (cnt==2) n2++;
			else if (cnt==3) n3++;
			else if (cnt==4) n4++;
			else if (cnt==5) n5++;
			else if (cnt==6) n6++;
			else if (cnt==7) n7++;
			else if (cnt==8) n8++;
		}
		
		if (n2==0 && n3==0 && n4==2 && n5==8 && n6==0 && n7==0 && n8==0) {
			nCos_s_0_2_8++;
			//printf("d%d small (0,2,8) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5);
			s_s_0_2_8[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_s_0_2_8[bNums[i][j]]=='C') s_s_0_2_8[bNums[i][j]]='B';
			}
		}
		if (n2==0 && n3==1 && n4==2 && n5==5 && n6==3 && n7==0 && n8==0) {
			nCos_s_1_2_5_3++;
			//printf("d%d small (1,2,5,3) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d n6 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5,n6);
			s_s_1_2_5_3[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_s_1_2_5_3[bNums[i][j]]=='C') s_s_1_2_5_3[bNums[i][j]]='B';
			}
		}
		if (n2==0 && n3==1 && n4==2 && n5==5 && n6==2 && n7==0 && n8==0) {
			nCos_s_1_2_5_2++;
			//printf("d%d small (1,2,5,2) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d n6 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5,n6);
			s_s_1_2_5_2[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_s_1_2_5_2[bNums[i][j]]=='C') s_s_1_2_5_2[bNums[i][j]]='B';
			}
		}
		if (n2==0 && n3==0 && n4==3 && n5==6 && n6==0 && n7==0 && n8==0) {
			nCos_s_0_3_6++;
			//printf("d%d small (0,3,6) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5);
			s_s_0_3_6[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_s_0_3_6[bNums[i][j]]=='C') s_s_0_3_6[bNums[i][j]]='B';
			}
		}
		if (n2==0 && n3==0 && n4==0 && n5==12 && n6==0 && n7==0 && n8==0) {
			nCos_s_0_0_12++;
			
			s_s_0_0_12[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_s_0_0_12[bNums[i][j]]=='C') s_s_0_0_12[bNums[i][j]]='B';
			}
		}
	}
	
	for (i=0; i<N; i++) {
		if (rtype[i]!=1) continue;
		n2=n3=n4=n5=n6=n7=n8=0;
		for (j=0; j<cnb[i]; j++) {
			cnt=0;
			for (k=0; k<cnb[i]; k++) {
				if (k==j) continue;
				if (Bonds_BondCheck(bNums[i][j],bNums[i][k])==1) cnt++;
			}
			if (cnt==2) n2++;
			else if (cnt==3) n3++;
			else if (cnt==4) n4++;
			else if (cnt==5) n5++;
			else if (cnt==6) n6++;
			else if (cnt==7) n7++;
			else if (cnt==8) n8++;
		}
		
		if (n2==0 && n3==0 && n4==2 && n5==8 && n6==4 && n7==0 && n8==0) {
			nCos_b_0_2_8_4++;
			//printf("d%d big (0,2,8,4) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d n6 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5,n6);
			s_b_0_2_8_4[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_b_0_2_8_4[bNums[i][j]]=='C') s_b_0_2_8_4[bNums[i][j]]='B';
			}
		}
		if (n2==0 && n3==0 && n4==2 && n5==8 && n6==5 && n7==0 && n8==0) {
			nCos_b_0_2_8_5++;
			//printf("d%d big (0,2,8,5) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d n6 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5,n6);
			s_b_0_2_8_5[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_b_0_2_8_5[bNums[i][j]]=='C') s_b_0_2_8_5[bNums[i][j]]='B';
			}
		}
		if (n2==0 && n3==0 && n4==3 && n5==6 && n6==6 && n7==0 && n8==0) {
			nCos_b_0_3_6_6++;
			//printf("d%d big (0,3,6,6) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d n6 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5,n6);
			s_b_0_3_6_6[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_b_0_3_6_6[bNums[i][j]]=='C') s_b_0_3_6_6[bNums[i][j]]='B';
			}
		}
		if (n2==0 && n3==0 && n4==1 && n5==10 && n6==4 && n7==0 && n8==0) {
			nCos_b_0_1_10_4++;
			//printf("d%d big (0,1,10,4) i %d rtype[i] %d cnb[i] %d n3 %d n4 %d n5 %d n6 %d\n",rank,i,rtype[i],cnb[i],n3,n4,n5,n6);
			s_b_0_1_10_4[i]='O';
			for (j=0; j<cnb[i]; j++) {
				if (s_b_0_1_10_4[bNums[i][j]]=='C') s_b_0_1_10_4[bNums[i][j]]='B';
			}
		}
	}
	
	for (i=0; i<N; i++) {
		if (s_s_0_2_8[i]!='C') {
			np_s_0_2_8++;
			pop_per_frame_s_0_2_8[f]=pop_per_frame_s_0_2_8[f]+1.0;
		}
		if (s_s_1_2_5_3[i]!='C') {
			np_s_1_2_5_3++;
			pop_per_frame_s_1_2_5_3[f]=pop_per_frame_s_1_2_5_3[f]+1.0;
		}
		if (s_s_1_2_5_2[i]!='C') {
			np_s_1_2_5_2++;
			pop_per_frame_s_1_2_5_2[f]=pop_per_frame_s_1_2_5_2[f]+1.0;
		}
		if (s_s_0_3_6[i]!='C') {
			np_s_0_3_6++;
			pop_per_frame_s_0_3_6[f]=pop_per_frame_s_0_3_6[f]+1.0;
		}
		if (s_s_0_0_12[i]!='C') {
			np_s_0_0_12++;
			pop_per_frame_s_0_0_12[f]=pop_per_frame_s_0_0_12[f]+1.0;
		}
		
		if (s_b_0_2_8_4[i]!='C') {
			np_b_0_2_8_4++;
			pop_per_frame_b_0_2_8_4[f]=pop_per_frame_b_0_2_8_4[f]+1.0;
		}
		if (s_b_0_2_8_5[i]!='C') {
			np_b_0_2_8_5++;
			pop_per_frame_b_0_2_8_5[f]=pop_per_frame_b_0_2_8_5[f]+1.0;
		}
		if (s_b_0_3_6_6[i]!='C') {
			np_b_0_3_6_6++;
			pop_per_frame_b_0_3_6_6[f]=pop_per_frame_b_0_3_6_6[f]+1.0;
		}
		if (s_b_0_1_10_4[i]!='C') {
			np_b_0_1_10_4++;
			pop_per_frame_b_0_1_10_4[f]=pop_per_frame_b_0_1_10_4[f]+1.0;
		}
	}
	pop_per_frame_s_0_2_8[f]=pop_per_frame_s_0_2_8[f]/N;
	pop_per_frame_s_1_2_5_3[f]=pop_per_frame_s_1_2_5_3[f]/N;
	pop_per_frame_s_1_2_5_2[f]=pop_per_frame_s_1_2_5_2[f]/N;
	pop_per_frame_s_0_3_6[f]=pop_per_frame_s_0_3_6[f]/N;
	pop_per_frame_s_0_0_12[f]=pop_per_frame_s_0_0_12[f]/N;
	
	pop_per_frame_b_0_2_8_4[f]=pop_per_frame_b_0_2_8_4[f]/N;
	pop_per_frame_b_0_2_8_5[f]=pop_per_frame_b_0_2_8_5[f]/N;
	pop_per_frame_b_0_3_6_6[f]=pop_per_frame_b_0_3_6_6[f]/N;
	pop_per_frame_b_0_1_10_4[f]=pop_per_frame_b_0_1_10_4[f]/N;
}
			

//// START: main() routine
int main(int argc, char **argv) {
	// MPI_Init(&argc, &argv);
	int e, f, i;
	int write, remainder;
	double tempish;
	char errMsg[1000], output[1000], other[1000];
	int ix, iy, iz;
	int imap;
	int **dummy_sub=NULL;
	FILE *rXmol;
	FILE *rSizes; //NPT_FIX
	FILE *clusBinFile, *dyn_lives_file, *dyn_decays_file, *cos_pop_per_frame, *file_s_0_0_12;
	// MPI_Comm_size(MPI_COMM_WORLD, &size);
	// MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//rank++;
	//size++;
	
	sprintf(fInputParamsName,"inputparameters.in");
	Setup_ReadInputParams(fInputParamsName);	// read input params

	Setup_ReadXmolParams(fXmolParamsName);	// read xmol params
	
	//NPT stuff
	if (ISNOTCUBIC!=0){
		if (USELIST==1) {
		sprintf(errMsg,"main() : Error! Need switch cell list off for non-cubic/NPT system");	// Always test file open
		Error(errMsg);
		}
		if (doPotential==1) {
		sprintf(errMsg,"main() : Error! Need switch off potential calc. for non-cubic/NPT system");	// Always test file open
		Error(errMsg);
		}
		
	}
	//NPT stuff
	//read in box data if noncubic/NPT
	if  (ISNOTCUBIC!=0){
		printf("reading box size data from %s\n",fBoxSizeName);
		rSizes=fopen(fBoxSizeName,"r");
		if(rSizes==NULL)  {
			sprintf(errMsg,"main() : Error opening file %s",fBoxSizeName);
			Error(errMsg);
		}
		fgets(other,1000,rSizes); //reads first line
		if  (ISNOTCUBIC==1) {
			Setup_ReadBox(rSizes);
			printf("sidex: %f, sidey: %f, sidez: %f\n", sidex,sidey,sidez);
		}
		if  (ISNOTCUBIC==3) {
	
			Setup_ReadBox(rSizes);
			printf("sidex: %f, sidey: %f, sidez: %f, tilt %f\n", sidex,sidey,sidez,tilt);
		}
	}
	
	
	printf("d%d reading coordinate frames from %s\n\n",rank,fXmolName);
	rXmol=fopen(fXmolName,"r");	// open xmol trajecotry
	if (rXmol==NULL)  {
		sprintf(errMsg,"main() : Error opening file %s",fXmolName);	// Always test file open
		Error(errMsg);
	}
	
	if (doWriteBonds==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bonds",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		bondsout=fopen(output, "w");
		if (bondsout==NULL)  {
			sprintf(errMsg,"main() : Error opening file %s",output);	// Always test file open
			Error(errMsg);
		}
	}
	
	if (USELIST==1) {
		M = (int)(side/rcutAA);	// number of cells along box side
		if (M<3) Error("main(): M<3, too few cells");
		ncells = M*M*M;	// total number of cells
	}
	
	Setup_InitStaticVars();
	
	if (doClusBLDeviation==1) {
		sprintf(fgsblName,"ground.state.bondlengths.dat");
		Setup_InitgsblVars(fgsblName);
	}
	
	if (doDynamics==1) {
		sprintf(fDynMemSizeName,"dynamic.memsize.dat");
		Setup_InitDynamicVars(fDynMemSizeName);
		printf("d%d created dynamic variables in memory, running TCC :)\n",rank);
	}
	
	if (USELIST==1) {
		cellSide = side/M;	// length of cells
		invcellSide = 1.0/cellSide;	// invcellSide
		printf("d%d m %d ncells %d cellside %.15lg\n", rank,M, ncells, cellSide);
		// routine to create the thirteen nearest neighbours array map[] of each cell 
		for (iz=1; iz<=M; iz++) {
			for (iy=1; iy<=M; iy++) {
				for (ix=1; ix<=M; ix++) {
					imap = (icell(ix,iy,iz)-1)*13;
					map[imap+1 ]=icell(ix+1,iy	,iz	);
					map[imap+2 ]=icell(ix+1,iy+1,iz	);
					map[imap+3 ]=icell(ix	 ,iy+1,iz	);
					map[imap+4 ]=icell(ix-1 ,iy+1,iz	);
					map[imap+5 ]=icell(ix+1,iy	,iz-1	);
					map[imap+6 ]=icell(ix+1,iy+1,iz-1	);
					map[imap+7 ]=icell(ix	 ,iy+1,iz-1	);
					map[imap+8 ]=icell(ix-1 ,iy+1,iz-1	);
					map[imap+9 ]=icell(ix+1,iy	,iz+1	);
					map[imap+10]=icell(ix+1,iy+1,iz+1	);
					map[imap+11]=icell(ix	 ,iy+1,iz+1	);
					map[imap+12]=icell(ix-1 ,iy+1,iz+1);
					map[imap+13]=icell(ix	 ,iy	,iz+1	);
				}
			}
		}
	}

	if (doPotential==1) {
		sprintf(fPotentialParamsName,"potentialparams.in");
		Setup_InitPotentialVars(fPotentialParamsName);
		Setup_print_U_r();
		if (USELIST==1) {
		
			map_pot=malloc(((13*ncells_pot)+1)*sizeof(int));	if (map_pot==NULL) { sprintf(errMsg,"main(): map_pot[] malloc out of memory\n");	Error_no_free(errMsg); }
			for (i=0; i<(13*ncells_pot+1); i++) map_pot[i]=0;
			head_pot=malloc((ncells_pot+1)*sizeof(int));	if (head_pot==NULL) { sprintf(errMsg,"main(): head_pot[] malloc out of memory\n");	Error_no_free(errMsg); }
			for (i=0; i<(ncells_pot+1); i++) head_pot[i]=0;
			llist_pot=malloc((N+1)*sizeof(int));	if (llist_pot==NULL) { sprintf(errMsg,"main(): llist_pot[] malloc out of memory\n");	Error_no_free(errMsg); }
			for (i=0; i<(N+1); i++) llist_pot[i]=0;
			
			for (iz=1; iz<=M_pot; iz++) {
				for (iy=1; iy<=M_pot; iy++) {
					for (ix=1; ix<=M_pot; ix++) {
						imap = (icell_pot(ix,iy,iz)-1)*13;
						map_pot[imap+1 ]=icell_pot(ix+1,iy	,iz	);
						map_pot[imap+2 ]=icell_pot(ix+1,iy+1,iz	);
						map_pot[imap+3 ]=icell_pot(ix	 ,iy+1,iz	);
						map_pot[imap+4 ]=icell_pot(ix-1 ,iy+1,iz	);
						map_pot[imap+5 ]=icell_pot(ix+1,iy	,iz-1	);
						map_pot[imap+6 ]=icell_pot(ix+1,iy+1,iz-1	);
						map_pot[imap+7 ]=icell_pot(ix	 ,iy+1,iz-1	);
						map_pot[imap+8 ]=icell_pot(ix-1 ,iy+1,iz-1	);
						map_pot[imap+9 ]=icell_pot(ix+1,iy	,iz+1	);
						map_pot[imap+10]=icell_pot(ix+1,iy+1,iz+1	);
						map_pot[imap+11]=icell_pot(ix	 ,iy+1,iz+1	);
						map_pot[imap+12]=icell_pot(ix-1 ,iy+1,iz+1);
						map_pot[imap+13]=icell_pot(ix	 ,iy	,iz+1	);
					}
				}
			}
		}
	}
	
	printf("d%d initializing static variables...",rank);
	Stats_Init();
	printf("d%d completed\n",rank);
	
	if (doWriteClus==1) {
		printf("\nd%d initializing cluster files...",rank);
		Write_Cluster_Init();
		printf("d%d completed\n",rank);
	}
	
	if (doWriteRaw==1) {
		printf("\nd%d initializing raw cluster xmol files...",rank);
		Write_Raw_Init();
		printf("d%d completed\n",rank);
	}
	
	if (do13AcenXmol==1) {
		printf("\nd%d initializing 13A centre xmol files...",rank);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.raw_13A_cen.xmol",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		file_13A_cen_xmol=fopen(output, "w");
		printf("d%d completed\n",rank);
	}
	
	if (doCoslovich==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.s_0_0_12",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		file_s_0_0_12=fopen(output, "w");
		if (file_s_0_0_12==NULL)  {
			sprintf(errMsg,"main() : Error opening file %s",output);	// Always test file open
			Error(errMsg);
		}
		
		s_s_0_2_8=malloc(N*sizeof(char));	if (s_s_0_2_8==NULL) { sprintf(errMsg,"main(): s_s_0_2_8[] malloc out of memory\n");	Error_no_free(errMsg); }
		s_s_1_2_5_3=malloc(N*sizeof(char));	if (s_s_1_2_5_3==NULL) { sprintf(errMsg,"main(): s_s_1_2_5_3[] malloc out of memory\n");	Error_no_free(errMsg); }
		s_s_1_2_5_2=malloc(N*sizeof(char));	if (s_s_1_2_5_2==NULL) { sprintf(errMsg,"main(): s_s_1_2_5_2[] malloc out of memory\n");	Error_no_free(errMsg); }
		s_s_0_3_6=malloc(N*sizeof(char));	if (s_s_0_3_6==NULL) { sprintf(errMsg,"main(): s_s_0_3_6[] malloc out of memory\n");	Error_no_free(errMsg); }
		s_s_0_0_12=malloc(N*sizeof(char));	if (s_s_0_0_12==NULL) { sprintf(errMsg,"main(): s_s_0_0_12[] malloc out of memory\n");	Error_no_free(errMsg); }
		
		pop_per_frame_s_0_2_8=malloc(FRAMES*sizeof(double));	if (pop_per_frame_s_0_2_8==NULL) { sprintf(errMsg,"main(): pop_per_frame_s_0_2_8[] malloc out of memory\n");	Error_no_free(errMsg); }
		pop_per_frame_s_1_2_5_3=malloc(FRAMES*sizeof(double));	if (pop_per_frame_s_1_2_5_3==NULL) { sprintf(errMsg,"main(): pop_per_frame_s_1_2_5_3[] malloc out of memory\n");	Error_no_free(errMsg); }
		pop_per_frame_s_1_2_5_2=malloc(FRAMES*sizeof(double));	if (pop_per_frame_s_1_2_5_2==NULL) { sprintf(errMsg,"main(): pop_per_frame_s_1_2_5_2[] malloc out of memory\n");	Error_no_free(errMsg); }
		pop_per_frame_s_0_3_6=malloc(FRAMES*sizeof(double));	if (pop_per_frame_s_0_3_6==NULL) { sprintf(errMsg,"main(): pop_per_frame_s_0_3_6[] malloc out of memory\n");	Error_no_free(errMsg); }
		pop_per_frame_s_0_0_12=malloc(FRAMES*sizeof(double));	if (pop_per_frame_s_0_0_12==NULL) { sprintf(errMsg,"main(): pop_per_frame_s_0_0_12[] malloc out of memory\n");	Error_no_free(errMsg); }
		
		s_b_0_2_8_4=malloc(N*sizeof(char));	if (s_b_0_2_8_4==NULL) { sprintf(errMsg,"main(): s_b_0_2_8_4[] malloc out of memory\n");	Error_no_free(errMsg); }
		s_b_0_2_8_5=malloc(N*sizeof(char));	if (s_b_0_2_8_5==NULL) { sprintf(errMsg,"main(): s_b_0_2_8_5[] malloc out of memory\n");	Error_no_free(errMsg); }
		s_b_0_3_6_6=malloc(N*sizeof(char));	if (s_b_0_3_6_6==NULL) { sprintf(errMsg,"main(): s_b_0_3_6_6[] malloc out of memory\n");	Error_no_free(errMsg); }
		s_b_0_1_10_4=malloc(N*sizeof(char));	if (s_b_0_1_10_4==NULL) { sprintf(errMsg,"main(): s_b_0_1_10_4[] malloc out of memory\n");	Error_no_free(errMsg); }
		
		pop_per_frame_b_0_2_8_4=malloc(FRAMES*sizeof(double));	if (pop_per_frame_b_0_2_8_4==NULL) { sprintf(errMsg,"main(): pop_per_frame_b_0_2_8[] malloc out of memory\n");	Error_no_free(errMsg); }
		pop_per_frame_b_0_2_8_5=malloc(FRAMES*sizeof(double));	if (pop_per_frame_b_0_2_8_5==NULL) { sprintf(errMsg,"main(): pop_per_frame_b_0_2_8_5[] malloc out of memory\n");	Error_no_free(errMsg); }
		pop_per_frame_b_0_3_6_6=malloc(FRAMES*sizeof(double));	if (pop_per_frame_b_0_3_6_6==NULL) { sprintf(errMsg,"main(): pop_per_frame_b_0_3_6[] malloc out of memory\n");	Error_no_free(errMsg); }
		pop_per_frame_b_0_1_10_4=malloc(FRAMES*sizeof(double));	if (pop_per_frame_b_0_1_10_4==NULL) { sprintf(errMsg,"main(): pop_per_frame_b_0_1_10_4[] malloc out of memory\n");	Error_no_free(errMsg); }
		
		for (i=0; i<N; i++) {
			s_s_0_2_8[i]='C';
			s_s_1_2_5_3[i]='C';
			s_s_1_2_5_2[i]='C';
			s_s_0_3_6[i]='C';
			s_s_0_0_12[i]='C';
			
			s_b_0_2_8_4[i]='C';
			s_b_0_2_8_5[i]='C';
			s_b_0_3_6_6[i]='C';
			s_b_0_1_10_4[i]='C';
		}
		for (i=0; i<FRAMES; i++) {
			pop_per_frame_s_0_2_8[i]=0.0;
			pop_per_frame_s_1_2_5_3[i]=0.0;
			pop_per_frame_s_1_2_5_2[i]=0.0;
			pop_per_frame_s_0_3_6[i]=0.0;
			pop_per_frame_s_0_0_12[i]=0.0;
			
			pop_per_frame_b_0_2_8_4[i]=0.0;
			pop_per_frame_b_0_2_8_5[i]=0.0;
			pop_per_frame_b_0_3_6_6[i]=0.0;
			pop_per_frame_b_0_1_10_4[i]=0.0;
		}
		nCos_s_0_2_8=nCos_s_1_2_5_3=nCos_s_1_2_5_2=nCos_s_0_3_6=nCos_s_0_0_12=0;
		np_s_0_2_8=np_s_1_2_5_3=np_s_1_2_5_2=np_s_0_3_6=np_s_0_0_12=0;
		nCos_b_0_2_8_4=nCos_b_0_2_8_5=nCos_b_0_3_6_6=nCos_b_0_1_10_4=0;
		np_b_0_2_8_4=np_b_0_2_8_5=np_b_0_3_6_6=np_b_0_1_10_4=0;
	}
	
	printf("d%d begin main loop\n",rank);

	f=0;
	for (e=0;e<TOTALFRAMES;e++) {
		remainder=e%SAMPLEFREQ;
		if (remainder==0 && f<FRAMES && e>=STARTFROM) {
			write=1;
		}
		else write=0;
		if (write==1) Setup_ResetStaticVars(f);
		if (ISNOTCUBIC==2) Setup_ReadBox(rSizes);
		Setup_Readxyz(e,write,f,rXmol);
		
		if (write==1) {
			if (doPotential==1) {
				if (USELIST==0) { 
					if (WHICHPOTENTIAL==0) BLJ();
					else if (WHICHPOTENTIAL==1) BLJSF();
					else if (WHICHPOTENTIAL==2) MorYuk();
					else if (WHICHPOTENTIAL==4)  BIPL();
					else if (WHICHPOTENTIAL==5)  BLJ_WCA_s();
					else if (WHICHPOTENTIAL==6)  SFBIPL();
					else if (WHICHPOTENTIAL==7)  CRVT();
				}
				else {
					links_pot();
					if (WHICHPOTENTIAL==0) listBLJ();
					else if (WHICHPOTENTIAL==1) listBLJSF();
					else if (WHICHPOTENTIAL==2) listMorYuk();
					else if (WHICHPOTENTIAL==4)  listBIPL();
					else if (WHICHPOTENTIAL==5)  listBLJ_WCA_s();
					else if (WHICHPOTENTIAL==6)  listSFBIPL();
				}
				av_potential+=potential;
				for (i=0; i<N; i++) av_pot_check+=part_pot[i];
			}
			Bonds_GetBonds(f);

			for(i=0; i<N; i++) {
				if (cnb[i]>maxnb) maxnb=cnb[i];
				if (dosp3==1) Rings_gSP3(f,i);
			}
			if (dosp3==1) Rings_setSP3c(f);			
			if (dosp4==1) Rings_setSP4c(f);
			if (dosp5==1) Rings_setSP5c(f);
			if (do6Z==1) Clusters_Get6Z_C2v(f);
			if (do7K==1) Clusters_Get7K(f);
			if (do8A==1) Clusters_Get8A_D2d(f);
			if (do8B==1) Clusters_Get8B_Cs(f);
			if (do8K==1) Clusters_Get8K(f);
			if (do9A==1) Clusters_Get9A_D3h(f);
			if (do9B==1) Clusters_Get9B_10B_11B_11E_12D(f);
			if (do9K==1) Clusters_Get9K_10K(f);
			if (do10A==1) Clusters_Get10A_C3v(f);
			if (do10W==1) Clusters_Get10W(f);
			if (do11A==1) Clusters_Get11A_12K(f);
			if (do11C==1) Clusters_Get11C_12A(f);
			if (do11F==1) Clusters_Get11F_12E_13K(f);
			if (do12B==1) Clusters_Get12B_13A(f);
			if (do13B==1) Clusters_Get13B_D5h(f);
			if (doFCC==1) Clusters_GetFCC(f);
			if (doHCP==1) Clusters_GetHCP(f);
			if (doBCC9==1) Clusters_GetBCC_9(f);
			if (doBCC15==1) Clusters_GetBCC_15(f);

			if (doCoslovich==1) {
				Coslovich(f);
				fprintf(file_s_0_0_12,"%d\nframe %d of %d\n",N,f,TOTALFRAMES);
				for(i=0; i<N; i++) {
					if (s_s_0_0_12[i]!='C') {
						if (rtype[i]==1) fprintf(file_s_0_0_12,"C\n");
						else fprintf(file_s_0_0_12,"D\n");
					}
					else if (s_s_0_0_12[i]=='C') {
						if (rtype[i]==1) fprintf(file_s_0_0_12,"A\n");
						else fprintf(file_s_0_0_12,"B\n");
					}
				}
			}

			if (doWriteClus==1) {
				Write_Cluster_sp3(f,wsp3);
				Write_Cluster(f,wsp3a,nsp3a,sp3a,3);
				Write_Cluster(f,wsp3b,nsp3b,sp3b,4);
				Write_Cluster(f,w5A,nsp3c,sp3c,5);
				Write_Cluster_sp4(f,wsp4);
				Write_Cluster(f,wsp4a,nsp4a,sp4a,4);
				Write_Cluster(f,wsp4b,nsp4b,sp4b,5);
				Write_Cluster(f,wsp4c,nsp4c,sp4c,6);
				Write_Cluster(f,w6A,n6A,hc6A,6);
				Write_Cluster_sp5(f,wsp5);
				Write_Cluster(f,wsp5a,nsp5a,sp5a,5);
				Write_Cluster(f,wsp5b,nsp5b,sp5b,6);
				Write_Cluster(f,w7A,nsp5c,sp5c,7);
				Write_Cluster(f,w6Z,n6Z,hc6Z,6);
				Write_Cluster(f,w7K,n7K,hc7K,7);
				Write_Cluster(f,w8A,n8A,hc8A,8);
				Write_Cluster(f,w8B,n8B,hc8B,8);
				Write_Cluster(f,w8K,n8K,hc8K,8);
				Write_Cluster(f,w9A,n9A,hc9A,9);
				Write_Cluster(f,w9B,n9B,hc9B,9);
				Write_Cluster(f,w9K,n9K,hc9K,9);
				Write_Cluster(f,w10A,n10A,hc10A,10);
				Write_Cluster(f,w10B,n10B,hc10B,10);
				Write_Cluster(f,w10K,n10K,hc10K,10);
				Write_Cluster(f,w10W,n10W,hc10W,10);
				Write_Cluster(f,w11A,n11A,hc11A,11);
				Write_Cluster(f,w11B,n11B,hc11B,11);
				Write_Cluster(f,w11C,n11C,hc11C,11);
				Write_Cluster(f,w11E,n11E,hc11E,11);
				Write_Cluster(f,w11F,n11F,hc11F,11);
				Write_Cluster(f,w11W,n11W,hc11W,11);
				Write_Cluster(f,w12A,n12A,hc12A,12);
				Write_Cluster(f,w12B,n12B,hc12B,12);
				Write_Cluster(f,w12D,n12D,hc12D,12);
				Write_Cluster(f,w12E,n12E,hc12E,12);
				Write_Cluster(f,w12K,n12K,hc12K,12);
				Write_Cluster(f,w13A,n13A,hc13A,13);
				Write_Cluster(f,w13B,n13B,hc13B,13);
				Write_Cluster(f,w13K,n13K,hc13K,13);
				Write_Cluster(f,wFCC,nFCC,hcFCC,13);
				Write_Cluster(f,wHCP,nHCP,hcHCP,13);
				Write_Cluster(f,wBCC_9,nBCC_9,hcBCC_9,9);
				Write_Cluster(f,wBCC_15,nBCC_15,hcBCC_15,15);
			}
			if (PRINTINFO==1) printf("\n");

			if (doWriteRaw==1) Write_Raw(f);
			
			if (do13AcenXmol==1) Write_13A_cen_xmol(f);
		    
			
			Stats_Reset();
			Stats_Analyse();
			Pop_Per_Frame(f);
			if (doClusBLDeviation==1) Update_bl_mom(f);
			if (doPotential==1) Update_Potential(f);
			
			printf("d%d f%d complete\n",rank,f);
			f++;
		}
		if (f==FRAMES) break;
	}
	fclose(rXmol);
	if (doWriteBonds==1) fclose(bondsout);
	
	if (f!=FRAMES) {
		printf("\n\n\nd%d !!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!\n\n\n",rank);
		printf("d%d Analysed frames %d less than expected number of FRAMES %d from %s\n\n",rank,f,FRAMES,fInputParamsName);
		printf("d%d Error could be in xmol file check %s\n\n\n",rank,fXmolParamsName);
	}
	
	if (doWriteClus==1) Write_Cluster_Close();
	
	mean_pop_per_frame_sp3/=FRAMES;
	mean_pop_per_frame_sp3a/=FRAMES;
	mean_pop_per_frame_sp3b/=FRAMES;
	mean_pop_per_frame_sp3c/=FRAMES;
	mean_pop_per_frame_sp4/=FRAMES;
	mean_pop_per_frame_sp4a/=FRAMES;
	mean_pop_per_frame_sp4b/=FRAMES;
	mean_pop_per_frame_6A/=FRAMES;
	mean_pop_per_frame_6Z/=FRAMES;
	mean_pop_per_frame_sp5/=FRAMES;
	mean_pop_per_frame_sp5a/=FRAMES;
	mean_pop_per_frame_sp5b/=FRAMES;
	mean_pop_per_frame_sp5c/=FRAMES;
	mean_pop_per_frame_7K/=FRAMES;
	mean_pop_per_frame_8A/=FRAMES;
	mean_pop_per_frame_8B/=FRAMES;
	mean_pop_per_frame_8K/=FRAMES;
	mean_pop_per_frame_9A/=FRAMES;
	mean_pop_per_frame_9B/=FRAMES;
	mean_pop_per_frame_9K/=FRAMES;
	mean_pop_per_frame_10A/=FRAMES;
	mean_pop_per_frame_10B/=FRAMES;
	mean_pop_per_frame_10K/=FRAMES;
	mean_pop_per_frame_10W/=FRAMES;
	mean_pop_per_frame_11A/=FRAMES;
	mean_pop_per_frame_11B/=FRAMES;
	mean_pop_per_frame_11C/=FRAMES;
	mean_pop_per_frame_11E/=FRAMES;
	mean_pop_per_frame_11F/=FRAMES;
	mean_pop_per_frame_11W/=FRAMES;
	mean_pop_per_frame_12A/=FRAMES;
	mean_pop_per_frame_12B/=FRAMES;
	mean_pop_per_frame_12D/=FRAMES;
	mean_pop_per_frame_12E/=FRAMES;
	mean_pop_per_frame_12K/=FRAMES;
	mean_pop_per_frame_13A/=FRAMES;
	mean_pop_per_frame_13B/=FRAMES;
	mean_pop_per_frame_13K/=FRAMES;
	mean_pop_per_frame_FCC/=FRAMES;
	mean_pop_per_frame_HCP/=FRAMES;
	mean_pop_per_frame_BCC_9/=FRAMES;
	mean_pop_per_frame_BCC_15/=FRAMES;
	
	if (doWritePopPerFrame==1) {
		printf("d%d Writing pop_per_frame %s ....",rank,output);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.pop_per_frame",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		fPopPerFrame=fopen(output, "w");
		if (fPopPerFrame==NULL)  {
			sprintf(errMsg,"main() : Error opening file %s",output);	// Always test file open
			Error(errMsg);
		}
		fprintf(fPopPerFrame,"%s\n",output);
		
		fprintf(fPopPerFrame,"frame	time	time_norm_t_a	sp3	sp3a	sp3b	5A	sp4	sp4a	sp4b	6A	6Z	sp5	sp5a	sp5b	7A	7K	8A	8B	8K	9A	9B	9K	10A	10B	10K	10W");
		fprintf(fPopPerFrame,"	11A	11B	11C	11E	11F	11W	12A	12B	12D	12E	12K	13A	13B	13K	FCC	HCP	BCC_9	BCC_15\n");
		
		
		fprintf(fPopPerFrame,"mean	-	-	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",mean_pop_per_frame_sp3,mean_pop_per_frame_sp3a,mean_pop_per_frame_sp3b,mean_pop_per_frame_sp3c,mean_pop_per_frame_sp4,mean_pop_per_frame_sp4a,mean_pop_per_frame_sp4b,mean_pop_per_frame_6A,mean_pop_per_frame_6Z,mean_pop_per_frame_sp5,mean_pop_per_frame_sp5a,mean_pop_per_frame_sp5b,mean_pop_per_frame_sp5c,mean_pop_per_frame_7K,mean_pop_per_frame_8A,mean_pop_per_frame_8B,mean_pop_per_frame_8K,mean_pop_per_frame_9A,mean_pop_per_frame_9B,mean_pop_per_frame_9K,mean_pop_per_frame_10A,mean_pop_per_frame_10B,mean_pop_per_frame_10K,mean_pop_per_frame_10W);
		fprintf(fPopPerFrame,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",mean_pop_per_frame_11A,mean_pop_per_frame_11B,mean_pop_per_frame_11C,mean_pop_per_frame_11E,mean_pop_per_frame_11F,mean_pop_per_frame_11W,mean_pop_per_frame_12A,mean_pop_per_frame_12B,mean_pop_per_frame_12D,mean_pop_per_frame_12E,mean_pop_per_frame_12K,mean_pop_per_frame_13A,mean_pop_per_frame_13B,mean_pop_per_frame_13K,mean_pop_per_frame_FCC,mean_pop_per_frame_HCP,mean_pop_per_frame_BCC_9,mean_pop_per_frame_BCC_15);
		for (f=0;f<FRAMES;f++) {
			fprintf(fPopPerFrame,"%d	%.15lg	%.15lg",f,(double)f*FRAMETSTEP*SAMPLEFREQ,(double)f*FRAMETSTEP*SAMPLEFREQ/talpha);
			fprintf(fPopPerFrame,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",pop_per_frame_sp3[f],pop_per_frame_sp3a[f],pop_per_frame_sp3b[f],pop_per_frame_sp3c[f],pop_per_frame_sp4[f],pop_per_frame_sp4a[f],pop_per_frame_sp4b[f],pop_per_frame_6A[f],pop_per_frame_6Z[f],pop_per_frame_sp5[f],pop_per_frame_sp5a[f],pop_per_frame_sp5b[f],pop_per_frame_sp5c[f],pop_per_frame_7K[f],pop_per_frame_8A[f],pop_per_frame_8B[f],pop_per_frame_8K[f],pop_per_frame_9A[f],pop_per_frame_9B[f],pop_per_frame_9K[f],pop_per_frame_10A[f],pop_per_frame_10B[f],pop_per_frame_10K[f],pop_per_frame_10W[f]);
			fprintf(fPopPerFrame,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",pop_per_frame_11A[f],pop_per_frame_11B[f],pop_per_frame_11C[f],pop_per_frame_11E[f],pop_per_frame_11F[f],pop_per_frame_11W[f],pop_per_frame_12A[f],pop_per_frame_12B[f],pop_per_frame_12D[f],pop_per_frame_12E[f],pop_per_frame_12K[f],pop_per_frame_13A[f],pop_per_frame_13B[f],pop_per_frame_13K[f],pop_per_frame_FCC[f],pop_per_frame_HCP[f],pop_per_frame_BCC_9[f],pop_per_frame_BCC_15[f]);
		}
		fclose(fPopPerFrame);
		printf("d%d Closed file %s\n\n",rank,output);
	}	
	
	if (doWriteRaw==1) {
		printf("d%d Closing raw cluster xmol files....",rank);
		Write_Raw_Close();
		printf("closed!\n\n");
	}
	
	if (do13AcenXmol==1) {
		printf("d%d Closing 13A centre xmol files....",rank);
		fclose(file_13A_cen_xmol);
		printf("closed!\n\n");
	}
	
	if (doCoslovich==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.cos_pop_per_frame",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		cos_pop_per_frame=fopen(output, "w");
		if (cos_pop_per_frame==NULL)  {
			sprintf(errMsg,"main() : Error opening file %s",output);	// Always test file open
			Error(errMsg);
		}
		fprintf(cos_pop_per_frame,"%s\n",output);
		
		fprintf(cos_pop_per_frame,"frame	time	time_norm_t_a	s(0,0,12)	s(0,2,8)	s(1,2,5,3)	s(1,2,5,2)	s(0,3,6)	b(0,2,8,4)	b(0,2,8,5)	b(0,3,6,6)	b(0,1,10,4)\n");
		fprintf(cos_pop_per_frame,"mean	-	-	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",(double)np_s_0_0_12/(N*FRAMES),(double)np_s_0_2_8/(N*FRAMES),(double)np_s_1_2_5_3/(N*FRAMES),(double)np_s_1_2_5_2/(N*FRAMES),(double)np_s_0_3_6/(N*FRAMES),(double)np_b_0_2_8_4/(N*FRAMES),(double)np_b_0_2_8_5/(N*FRAMES),(double)np_b_0_3_6_6/(N*FRAMES),(double)np_b_0_1_10_4/(N*FRAMES));
		for (f=0;f<FRAMES;f++) {
			fprintf(cos_pop_per_frame,"%d	%.15lg	%.15lg",f,(double)f*FRAMETSTEP*SAMPLEFREQ,(double)f*FRAMETSTEP*SAMPLEFREQ/talpha);
			fprintf(cos_pop_per_frame,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",(double)pop_per_frame_s_0_0_12[f],(double)pop_per_frame_s_0_2_8[f],(double)pop_per_frame_s_1_2_5_3[f],(double)pop_per_frame_s_1_2_5_2[f],(double)pop_per_frame_s_0_3_6[f],(double)pop_per_frame_b_0_2_8_4[f],(double)pop_per_frame_b_0_2_8_5[f],(double)pop_per_frame_b_0_3_6_6[f],(double)pop_per_frame_b_0_1_10_4[f]);
		}
		fclose(cos_pop_per_frame);
		printf("d%d Written %s\n\n",rank,output);
		fclose(file_s_0_0_12);
	}
	
	if (doBLDistros==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistro(output,BLDistro,&BLDistroNoSamples,&meanBL);
		if (doBinary==1){
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_AA",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Bonds_WriteBLDistro(output,BLDistroAA,&BLDistroNoSamplesAA,&meanBLAA);
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_AB",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Bonds_WriteBLDistro(output,BLDistroAB,&BLDistroNoSamplesAB,&meanBLAB);
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_BB",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Bonds_WriteBLDistro(output,BLDistroBB,&BLDistroNoSamplesBB,&meanBLBB);
		}
	}

	if (doClusBLDistros==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp3",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp3,&BLDistroNoSamplessp3,&meanBLsp3);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp3a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp3a,&BLDistroNoSamplessp3a,&meanBLsp3a);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp3b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp3b,&BLDistroNoSamplessp3b,&meanBLsp3b);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp3c",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp3c,&BLDistroNoSamplessp3c,&meanBLsp3c);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp4",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp4,&BLDistroNoSamplessp4,&meanBLsp4);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp4a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp4a,&BLDistroNoSamplessp4a,&meanBLsp4a);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp4b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp4b,&BLDistroNoSamplessp4b,&meanBLsp4b);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp4c",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp4c,&BLDistroNoSamplessp4c,&meanBLsp4c);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_6A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro6A,&BLDistroNoSamples6A,&meanBL6A);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp5",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp5,&BLDistroNoSamplessp5,&meanBLsp5);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp5a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp5a,&BLDistroNoSamplessp5a,&meanBLsp5a);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp5b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp5b,&BLDistroNoSamplessp5b,&meanBLsp5b);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_sp5c",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistrosp5c,&BLDistroNoSamplessp5c,&meanBLsp5c);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_6Z",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro6Z,&BLDistroNoSamples6Z,&meanBL6Z);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_7K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro7K,&BLDistroNoSamples7K,&meanBL7K);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_8A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro8A,&BLDistroNoSamples8A,&meanBL8A);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_8B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro8B,&BLDistroNoSamples8B,&meanBL8B);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_8K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro8K,&BLDistroNoSamples8K,&meanBL8K);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_9A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro9A,&BLDistroNoSamples9A,&meanBL9A);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_9B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro9B,&BLDistroNoSamples9B,&meanBL9B);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_9K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro9K,&BLDistroNoSamples9K,&meanBL9K);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_10A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro10A,&BLDistroNoSamples10A,&meanBL10A);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_10B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro10B,&BLDistroNoSamples10B,&meanBL10B);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_10K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro10K,&BLDistroNoSamples10K,&meanBL10K);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_10W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro10W,&BLDistroNoSamples10W,&meanBL10W);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_11A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro11A,&BLDistroNoSamples11A,&meanBL11A);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_11B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro11B,&BLDistroNoSamples11B,&meanBL11B);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_11C",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro11C,&BLDistroNoSamples11C,&meanBL11C);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_11E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro11E,&BLDistroNoSamples11E,&meanBL11E);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_11F",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro11F,&BLDistroNoSamples11F,&meanBL11F);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_11W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro11W,&BLDistroNoSamples11W,&meanBL11W);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_12A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro12A,&BLDistroNoSamples12A,&meanBL12A);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_12B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro12B,&BLDistroNoSamples12B,&meanBL12B);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_12D",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro12D,&BLDistroNoSamples12D,&meanBL12D);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_12E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro12E,&BLDistroNoSamples12E,&meanBL12E);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_12K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro12K,&BLDistroNoSamples12K,&meanBL12K);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_13A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro13A,&BLDistroNoSamples13A,&meanBL13A);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_13B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro13B,&BLDistroNoSamples13B,&meanBL13B);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_13K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistro13K,&BLDistroNoSamples13K,&meanBL13K);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_FCC",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistroFCC,&BLDistroNoSamplesFCC,&meanBLFCC);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_HCP",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistroHCP,&BLDistroNoSamplesHCP,&meanBLHCP);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_BCC_9",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistroBCC_9,&BLDistroNoSamplesBCC_9,&meanBLBCC_9);
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.bond_length_BCC_15",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WriteBLDistroClust(output,BLDistroBCC_15,&BLDistroNoSamplesBCC_15,&meanBLBCC_15);
		
		printf("\nd%d Cluster based bond length analysis\n",rank);
		printf("d%d Clust	mean bond length\n",rank);
		printf("d%d sp3	%lg	%lg\n",rank,meanBLsp3,-meanBL+meanBLsp3);
		printf("d%d sp3a	%lg	%lg\n",rank,meanBLsp3a,-meanBL+meanBLsp3a);
		printf("d%d sp3b	%lg	%lg\n",rank,meanBLsp3b,-meanBL+meanBLsp3b);
		printf("d%d 5A_D3h	%lg	%lg\n",rank,meanBLsp3c,-meanBL+meanBLsp3c);
		printf("d%d sp4	%lg	%lg\n",rank,meanBLsp4,-meanBL+meanBLsp4);
		printf("d%d sp4a	%lg	%lg\n",rank,meanBLsp4a,-meanBL+meanBLsp4a);
		printf("d%d sp4b	%lg	%lg\n",rank,meanBLsp4b,-meanBL+meanBLsp4b);
		printf("d%d sp4c	%lg	%lg\n",rank,meanBLsp4c,-meanBL+meanBLsp4c); 
		printf("d%d 6A_Oh	%lg	%lg\n",rank,meanBL6A,-meanBL+meanBL6A); 
		printf("d%d 6Z_C2v	%lg	%lg\n",rank,meanBL6Z,-meanBL+meanBL6Z);
		printf("d%d 7K	%lg	%lg\n",rank,meanBL7K,-meanBL+meanBL7K);
		printf("d%d sp5	%lg	%lg\n",rank,meanBLsp5,-meanBL+meanBLsp5);
		printf("d%d sp5a	%lg	%lg\n",rank,meanBLsp5a,-meanBL+meanBLsp5a);
		printf("d%d sp5b	%lg	%lg\n",rank,meanBLsp5b,-meanBL+meanBLsp5b);
		printf("d%d 7A_D5h	%lg	%lg\n",rank,meanBLsp5c,-meanBL+meanBLsp5c);
		printf("d%d 8A_D2d	%lg	%lg\n",rank,meanBL8A,-meanBL+meanBL8A);
		printf("d%d 8B_Cs	%lg	%lg\n",rank,meanBL8B,-meanBL+meanBL8B);
		printf("d%d 8K	%lg	%lg\n",rank,meanBL8K,-meanBL+meanBL8K);
		printf("d%d 9A_D3h	%lg	%lg\n",rank,meanBL9A,-meanBL+meanBL9A);
		printf("d%d 9B_C2v	%lg	%lg\n",rank,meanBL9B,-meanBL+meanBL9B);
		printf("d%d 9K	%lg	%lg\n",rank,meanBL9K,-meanBL+meanBL9K);
		printf("d%d 10A_D4d	%lg	%lg\n",rank,meanBL10A,-meanBL+meanBL10A);
		printf("d%d 10B_C3v	%lg	%lg\n",rank,meanBL10B,-meanBL+meanBL10B);
		printf("d%d 10K	%lg	%lg\n",rank,meanBL10K,-meanBL+meanBL10K);
		printf("d%d 10W	%lg	%lg\n",rank,meanBL10W,-meanBL+meanBL10W);
		printf("d%d 11A_D4d	%lg	%lg\n",rank,meanBL11A,-meanBL+meanBL11A);
		printf("d%d 11B_C2v	%lg	%lg\n",rank,meanBL11B,-meanBL+meanBL11B);
		printf("d%d 11CD	%lg	%lg\n",rank,meanBL11C,-meanBL+meanBL11C);
		printf("d%d 11E_C2	%lg	%lg\n",rank,meanBL11E,-meanBL+meanBL11E);
		printf("d%d 11F_C2v	%lg	%lg\n",rank,meanBL11F,-meanBL+meanBL11F);
		printf("d%d 11W_Cs	%lg	%lg\n",rank,meanBL11W,-meanBL+meanBL11W);
		printf("d%d 12A_C2v	%lg	%lg\n",rank,meanBL12A,-meanBL+meanBL12A);
		printf("d%d 12BC	%lg	%lg\n",rank,meanBL12B,-meanBL+meanBL12B);
		printf("d%d 12D_D2d	%lg	%lg\n",rank,meanBL12D,-meanBL+meanBL12D);
		printf("d%d 12E_D3h	%lg	%lg\n",rank,meanBL12E,-meanBL+meanBL12E);
		printf("d%d 12K	%lg	%lg\n",rank,meanBL12K,-meanBL+meanBL12K);
		printf("d%d 13A_Ih	%lg	%lg\n",rank,meanBL13A,-meanBL+meanBL13A);
		printf("d%d 13B_D5h	%lg	%lg\n",rank,meanBL13B,-meanBL+meanBL13B);
		printf("d%d 13K	%lg	%lg\n",rank,meanBL13K,-meanBL+meanBL13K);
		printf("d%d FCC_m13	%lg	%lg\n",rank,meanBLFCC,-meanBL+meanBLFCC);
		printf("d%d HCP_m13	%lg	%lg\n",rank,meanBLHCP,-meanBL+meanBLHCP);
		printf("d%d BCC_m9	%lg	%lg\n",rank,meanBLBCC_9,-meanBL+meanBLBCC_9);
		printf("d%d BCC_m15	%lg	%lg\n",rank,meanBLBCC_15,-meanBL+meanBLBCC_15);
		if (doBLDistros==1) printf("d%d mean bl	%lg	%lg\n",rank,meanBL,0.0);
		
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.meanBL",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		clusBinFile=fopen(output,"w");
		if (clusBinFile==NULL)  {
			sprintf(errMsg,"main(): Error opening file %s",output);	// Always test file open
			Error(errMsg);
		}
		fprintf(clusBinFile,"%s\n",output);
		
		fprintf(clusBinFile,"Clust	mean bond length\n");
		fprintf(clusBinFile,"sp3	%lg	%lg\n",meanBLsp3,-meanBL+meanBLsp3);
		fprintf(clusBinFile,"sp3a	%lg	%lg\n",meanBLsp3a,-meanBL+meanBLsp3a);
		fprintf(clusBinFile,"sp3b	%lg	%lg\n",meanBLsp3b,-meanBL+meanBLsp3b);
		fprintf(clusBinFile,"5A_D3h	%lg	%lg\n",meanBLsp3c,-meanBL+meanBLsp3c);
		fprintf(clusBinFile,"sp4	%lg	%lg\n",meanBLsp4,-meanBL+meanBLsp4);
		fprintf(clusBinFile,"sp4a	%lg	%lg\n",meanBLsp4a,-meanBL+meanBLsp4a);
		fprintf(clusBinFile,"sp4b	%lg	%lg\n",meanBLsp4b,-meanBL+meanBLsp4b);
		fprintf(clusBinFile,"sp4c	%lg	%lg\n",meanBLsp4c,-meanBL+meanBLsp4c); 
		fprintf(clusBinFile,"6A_Oh	%lg	%lg\n",meanBL6A,-meanBL+meanBL6A); 
		fprintf(clusBinFile,"6Z_C2v	%lg	%lg\n",meanBL6Z,-meanBL+meanBL6Z);
		fprintf(clusBinFile,"7K	%lg	%lg\n",meanBL7K,-meanBL+meanBL7K);
		fprintf(clusBinFile,"sp5	%lg	%lg\n",meanBLsp5,-meanBL+meanBLsp5);
		fprintf(clusBinFile,"sp5a	%lg	%lg\n",meanBLsp5a,-meanBL+meanBLsp5a);
		fprintf(clusBinFile,"sp5b	%lg	%lg\n",meanBLsp5b,-meanBL+meanBLsp5b);
		fprintf(clusBinFile,"7A_D5h	%lg	%lg\n",meanBLsp5c,-meanBL+meanBLsp5c);
		fprintf(clusBinFile,"8A_D2d	%lg	%lg\n",meanBL8A,-meanBL+meanBL8A);
		fprintf(clusBinFile,"8B_Cs	%lg	%lg\n",meanBL8B,-meanBL+meanBL8B);
		fprintf(clusBinFile,"8K	%lg	%lg\n",meanBL8K,-meanBL+meanBL8K);
		fprintf(clusBinFile,"9A_D3h	%lg	%lg\n",meanBL9A,-meanBL+meanBL9A);
		fprintf(clusBinFile,"9B_C2v	%lg	%lg\n",meanBL9B,-meanBL+meanBL9B);
		fprintf(clusBinFile,"9K	%lg	%lg\n",meanBL9K,-meanBL+meanBL9K);
		fprintf(clusBinFile,"10A_D4d	%lg	%lg\n",meanBL10A,-meanBL+meanBL10A);
		fprintf(clusBinFile,"10B_C3v	%lg	%lg\n",meanBL10B,-meanBL+meanBL10B);
		fprintf(clusBinFile,"10K	%lg	%lg\n",meanBL10K,-meanBL+meanBL10K);
		fprintf(clusBinFile,"10W	%lg	%lg\n",meanBL10W,-meanBL+meanBL10W);
		fprintf(clusBinFile,"11A_D4d	%lg	%lg\n",meanBL11A,-meanBL+meanBL11A);
		fprintf(clusBinFile,"11B_C2v	%lg	%lg\n",meanBL11B,-meanBL+meanBL11B);
		fprintf(clusBinFile,"11CD	%lg	%lg\n",meanBL11C,-meanBL+meanBL11C);
		fprintf(clusBinFile,"11E_C2	%lg	%lg\n",meanBL11E,-meanBL+meanBL11E);
		fprintf(clusBinFile,"11F_C2v	%lg	%lg\n",meanBL11F,-meanBL+meanBL11F);
		fprintf(clusBinFile,"11W_Cs	%lg	%lg\n",meanBL11W,-meanBL+meanBL11W);
		fprintf(clusBinFile,"12A_C2v	%lg	%lg\n",meanBL12A,-meanBL+meanBL12A);
		fprintf(clusBinFile,"12BC	%lg	%lg\n",meanBL12B,-meanBL+meanBL12B);
		fprintf(clusBinFile,"12D_D2d	%lg	%lg\n",meanBL12D,-meanBL+meanBL12D);
		fprintf(clusBinFile,"12E_D3h	%lg	%lg\n",meanBL12E,-meanBL+meanBL12E);
		fprintf(clusBinFile,"12K	%lg	%lg\n",meanBL12K,-meanBL+meanBL12K);
		fprintf(clusBinFile,"13A_Ih	%lg	%lg\n",meanBL13A,-meanBL+meanBL13A);
		fprintf(clusBinFile,"13B_D5h	%lg	%lg\n",meanBL13B,-meanBL+meanBL13B);
		fprintf(clusBinFile,"13K	%lg	%lg\n",meanBL13K,-meanBL+meanBL13K);
		fprintf(clusBinFile,"FCC_m13	%lg	%lg\n",meanBLFCC,-meanBL+meanBLFCC);
		fprintf(clusBinFile,"HCP_m13	%lg	%lg\n",meanBLHCP,-meanBL+meanBLHCP);
		fprintf(clusBinFile,"BCC_m9	%lg	%lg\n",meanBLBCC_9,-meanBL+meanBLBCC_9);
		fprintf(clusBinFile,"BCC_m15	%lg	%lg\n",meanBLBCC_15,-meanBL+meanBLBCC_15);
		if (doBLDistros==1) fprintf(clusBinFile,"mean bl	%lg	%lg\n",meanBL,0.0);
		
		fclose(clusBinFile);
		printf("\nd%d Written %s\n\n",rank,output);
	}
	
	if (donbDistros==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.nb_histo",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Bonds_WritenbDistro(output,nbDistro,&nbDistroNoSamples,&meannb);
		if (doBinary==1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.nb_histo_AA",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Bonds_WritenbDistro(output,nbDistroAA,&nbDistroNoSamplesAA,&meannbAA);
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.nb_histo_AB",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Bonds_WritenbDistro(output,nbDistroAB,&nbDistroNoSamplesAB,&meannbAB);
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.nb_histo_BA",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Bonds_WritenbDistro(output,nbDistroBA,&nbDistroNoSamplesBA,&meannbBA);
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.nb_histo_BB",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Bonds_WritenbDistro(output,nbDistroBB,&nbDistroNoSamplesBB,&meannbBB);
		}
	}
	

	sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.static_clust",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
	printf("\n");
	Stats_Report(output);
	printf("\nd%d Written %s\n\n",rank,output);

	if (doPotential==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.potential",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Norm_Write_Potential(output);
	}
	
	if (doClusComp==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Norm_Write_ClusComp(output);
		printf("\n");
	}
	
	if (doBondedCen==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Norm_Write_bonded_to_cen(output);
	}
	
	if (doClusBLDeviation==1) {
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.clus_bl_mom",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Norm_Write_bl_mom(output);
	}
	
	if (doDynamics==1) {
		Dyn_Analyse_Init();
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_lives",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		dyn_lives_file=fopen(output,"w");
		if (dyn_lives_file==NULL)  {
			sprintf(errMsg,"main(): Error opening file %s",output);	// Always test file open
			Error(errMsg);
		}
		fprintf(dyn_lives_file,"%s\n",output);
		
		if (dyn_msp3!=-1) {
			Dyn_Analyse(&dyn_asp3, &dyn_esp3, &dyn_nsp3, &dyn_lsp3, 
			&dyn_hist_sp3, &sum_hist_sp3, &dyn_norm_hist_sp3, &dyn_norm_corrected_hist_sp3, &dyn_decay_norm_hist_sp3, &dyn_decay_norm_corrected_hist_sp3,
			&dyn_total_hist_sp3,  &dyn_norm_total_hist_sp3, &dyn_decay_norm_total_hist_sp3,
			&dyn_max_hist_sp3,  &sum_max_hist_sp3, &dyn_norm_max_hist_sp3, &dyn_norm_corrected_max_hist_sp3,  &dyn_decay_norm_max_hist_sp3, &dyn_decay_norm_corrected_max_hist_sp3);
		}
		if (dyn_msp3a!=-1) {
			Dyn_Analyse(&dyn_asp3a, &dyn_esp3a, &dyn_nsp3a, &dyn_lsp3a, 
			&dyn_hist_sp3a, &sum_hist_sp3a, &dyn_norm_hist_sp3a, &dyn_norm_corrected_hist_sp3a, &dyn_decay_norm_hist_sp3a, &dyn_decay_norm_corrected_hist_sp3a,
			&dyn_total_hist_sp3a,  &dyn_norm_total_hist_sp3a, &dyn_decay_norm_total_hist_sp3a,
			&dyn_max_hist_sp3a,  &sum_max_hist_sp3a, &dyn_norm_max_hist_sp3a, &dyn_norm_corrected_max_hist_sp3a,  &dyn_decay_norm_max_hist_sp3a, &dyn_decay_norm_corrected_max_hist_sp3a);
		}
		if (dyn_msp3b!=-1) {
			Dyn_Analyse(&dyn_asp3b, &dyn_esp3b, &dyn_nsp3b, &dyn_lsp3b, 
			&dyn_hist_sp3b, &sum_hist_sp3b, &dyn_norm_hist_sp3b, &dyn_norm_corrected_hist_sp3b, &dyn_decay_norm_hist_sp3b, &dyn_decay_norm_corrected_hist_sp3b,
			&dyn_total_hist_sp3b,  &dyn_norm_total_hist_sp3b, &dyn_decay_norm_total_hist_sp3b,
			&dyn_max_hist_sp3b,  &sum_max_hist_sp3b, &dyn_norm_max_hist_sp3b, &dyn_norm_corrected_max_hist_sp3b,  &dyn_decay_norm_max_hist_sp3b, &dyn_decay_norm_corrected_max_hist_sp3b);
		}
		if (dyn_msp3c!=-1) {
			Dyn_Analyse(&dyn_asp3c, &dyn_esp3c, &dyn_nsp3c, &dyn_lsp3c, 
			&dyn_hist_sp3c, &sum_hist_sp3c, &dyn_norm_hist_sp3c, &dyn_norm_corrected_hist_sp3c, &dyn_decay_norm_hist_sp3c, &dyn_decay_norm_corrected_hist_sp3c,
			&dyn_total_hist_sp3c,  &dyn_norm_total_hist_sp3c, &dyn_decay_norm_total_hist_sp3c,
			&dyn_max_hist_sp3c,  &sum_max_hist_sp3c, &dyn_norm_max_hist_sp3c, &dyn_norm_corrected_max_hist_sp3c,  &dyn_decay_norm_max_hist_sp3c, &dyn_decay_norm_corrected_max_hist_sp3c);
		}
		if (dyn_msp4!=-1) {
			Dyn_Analyse(&dyn_asp4, &dyn_esp4, &dyn_nsp4, &dyn_lsp4, 
			&dyn_hist_sp4, &sum_hist_sp4, &dyn_norm_hist_sp4, &dyn_norm_corrected_hist_sp4, &dyn_decay_norm_hist_sp4, &dyn_decay_norm_corrected_hist_sp4,
			&dyn_total_hist_sp4,  &dyn_norm_total_hist_sp4, &dyn_decay_norm_total_hist_sp4,
			&dyn_max_hist_sp4,  &sum_max_hist_sp4, &dyn_norm_max_hist_sp4, &dyn_norm_corrected_max_hist_sp4,  &dyn_decay_norm_max_hist_sp4, &dyn_decay_norm_corrected_max_hist_sp4);
		}
		if (dyn_msp4a!=-1) {
			Dyn_Analyse(&dyn_asp4a, &dyn_esp4a, &dyn_nsp4a, &dyn_lsp4a, 
			&dyn_hist_sp4a, &sum_hist_sp4a, &dyn_norm_hist_sp4a, &dyn_norm_corrected_hist_sp4a, &dyn_decay_norm_hist_sp4a, &dyn_decay_norm_corrected_hist_sp4a,
			&dyn_total_hist_sp4a,  &dyn_norm_total_hist_sp4a, &dyn_decay_norm_total_hist_sp4a,
			&dyn_max_hist_sp4a,  &sum_max_hist_sp4a, &dyn_norm_max_hist_sp4a, &dyn_norm_corrected_max_hist_sp4a,  &dyn_decay_norm_max_hist_sp4a, &dyn_decay_norm_corrected_max_hist_sp4a);
		}
		if (dyn_msp4b!=-1) {
			Dyn_Analyse(&dyn_asp4b, &dyn_esp4b, &dyn_nsp4b, &dyn_lsp4b, 
			&dyn_hist_sp4b, &sum_hist_sp4b, &dyn_norm_hist_sp4b, &dyn_norm_corrected_hist_sp4b, &dyn_decay_norm_hist_sp4b, &dyn_decay_norm_corrected_hist_sp4b,
			&dyn_total_hist_sp4b,  &dyn_norm_total_hist_sp4b, &dyn_decay_norm_total_hist_sp4b,
			&dyn_max_hist_sp4b,  &sum_max_hist_sp4b, &dyn_norm_max_hist_sp4b, &dyn_norm_corrected_max_hist_sp4b,  &dyn_decay_norm_max_hist_sp4b, &dyn_decay_norm_corrected_max_hist_sp4b);
		}
		if (dyn_m6A!=-1) {
			Dyn_Analyse(&dyn_a6A, &dyn_e6A, &dyn_n6A, &dyn_l6A, 
			&dyn_hist_6A, &sum_hist_6A, &dyn_norm_hist_6A, &dyn_norm_corrected_hist_6A, &dyn_decay_norm_hist_6A, &dyn_decay_norm_corrected_hist_6A,
			&dyn_total_hist_6A,  &dyn_norm_total_hist_6A, &dyn_decay_norm_total_hist_6A,
			&dyn_max_hist_6A,  &sum_max_hist_6A, &dyn_norm_max_hist_6A, &dyn_norm_corrected_max_hist_6A,  &dyn_decay_norm_max_hist_6A, &dyn_decay_norm_corrected_max_hist_6A);
		}
		if (dyn_msp5!=-1) {
			Dyn_Analyse(&dyn_asp5, &dyn_esp5, &dyn_nsp5, &dyn_lsp5, 
			&dyn_hist_sp5, &sum_hist_sp5, &dyn_norm_hist_sp5, &dyn_norm_corrected_hist_sp5, &dyn_decay_norm_hist_sp5, &dyn_decay_norm_corrected_hist_sp5,
			&dyn_total_hist_sp5,  &dyn_norm_total_hist_sp5, &dyn_decay_norm_total_hist_sp5,
			&dyn_max_hist_sp5,  &sum_max_hist_sp5, &dyn_norm_max_hist_sp5, &dyn_norm_corrected_max_hist_sp5,  &dyn_decay_norm_max_hist_sp5, &dyn_decay_norm_corrected_max_hist_sp5);
		}
		if (dyn_msp5a!=-1) {
			Dyn_Analyse(&dyn_asp5a, &dyn_esp5a, &dyn_nsp5a, &dyn_lsp5a, 
			&dyn_hist_sp5a, &sum_hist_sp5a, &dyn_norm_hist_sp5a, &dyn_norm_corrected_hist_sp5a, &dyn_decay_norm_hist_sp5a, &dyn_decay_norm_corrected_hist_sp5a,
			&dyn_total_hist_sp5a,  &dyn_norm_total_hist_sp5a, &dyn_decay_norm_total_hist_sp5a,
			&dyn_max_hist_sp5a,  &sum_max_hist_sp5a, &dyn_norm_max_hist_sp5a, &dyn_norm_corrected_max_hist_sp5a,  &dyn_decay_norm_max_hist_sp5a, &dyn_decay_norm_corrected_max_hist_sp5a);
		}
		if (dyn_msp5b!=-1) {
			Dyn_Analyse(&dyn_asp5b, &dyn_esp5b, &dyn_nsp5b, &dyn_lsp5b, 
			&dyn_hist_sp5b, &sum_hist_sp5b, &dyn_norm_hist_sp5b, &dyn_norm_corrected_hist_sp5b, &dyn_decay_norm_hist_sp5b, &dyn_decay_norm_corrected_hist_sp5b,
			&dyn_total_hist_sp5b,  &dyn_norm_total_hist_sp5b, &dyn_decay_norm_total_hist_sp5b,
			&dyn_max_hist_sp5b,  &sum_max_hist_sp5b, &dyn_norm_max_hist_sp5b, &dyn_norm_corrected_max_hist_sp5b,  &dyn_decay_norm_max_hist_sp5b, &dyn_decay_norm_corrected_max_hist_sp5b);
		}
		if (dyn_msp5c!=-1) {
			Dyn_Analyse(&dyn_asp5c, &dyn_esp5c, &dyn_nsp5c, &dyn_lsp5c, 
			&dyn_hist_sp5c, &sum_hist_sp5c, &dyn_norm_hist_sp5c, &dyn_norm_corrected_hist_sp5c, &dyn_decay_norm_hist_sp5c, &dyn_decay_norm_corrected_hist_sp5c,
			&dyn_total_hist_sp5c,  &dyn_norm_total_hist_sp5c, &dyn_decay_norm_total_hist_sp5c,
			&dyn_max_hist_sp5c,  &sum_max_hist_sp5c, &dyn_norm_max_hist_sp5c, &dyn_norm_corrected_max_hist_sp5c,  &dyn_decay_norm_max_hist_sp5c, &dyn_decay_norm_corrected_max_hist_sp5c);
		}
		if (dyn_m6Z!=-1) {
			Dyn_Analyse(&dyn_a6Z, &dyn_e6Z, &dyn_n6Z, &dyn_l6Z, 
			&dyn_hist_6Z, &sum_hist_6Z, &dyn_norm_hist_6Z, &dyn_norm_corrected_hist_6Z, &dyn_decay_norm_hist_6Z, &dyn_decay_norm_corrected_hist_6Z,
			&dyn_total_hist_6Z,  &dyn_norm_total_hist_6Z, &dyn_decay_norm_total_hist_6Z,
			&dyn_max_hist_6Z,  &sum_max_hist_6Z, &dyn_norm_max_hist_6Z, &dyn_norm_corrected_max_hist_6Z,  &dyn_decay_norm_max_hist_6Z, &dyn_decay_norm_corrected_max_hist_6Z);
		}
		if (dyn_m7K!=-1) {
			Dyn_Analyse(&dyn_a7K, &dyn_e7K, &dyn_n7K, &dyn_l7K, 
			&dyn_hist_7K, &sum_hist_7K, &dyn_norm_hist_7K, &dyn_norm_corrected_hist_7K, &dyn_decay_norm_hist_7K, &dyn_decay_norm_corrected_hist_7K,
			&dyn_total_hist_7K,  &dyn_norm_total_hist_7K, &dyn_decay_norm_total_hist_7K,
			&dyn_max_hist_7K,  &sum_max_hist_7K, &dyn_norm_max_hist_7K, &dyn_norm_corrected_max_hist_7K,  &dyn_decay_norm_max_hist_7K, &dyn_decay_norm_corrected_max_hist_7K);
		}
		if (dyn_m8A!=-1) {
			Dyn_Analyse(&dyn_a8A, &dyn_e8A, &dyn_n8A, &dyn_l8A, 
			&dyn_hist_8A, &sum_hist_8A, &dyn_norm_hist_8A, &dyn_norm_corrected_hist_8A, &dyn_decay_norm_hist_8A, &dyn_decay_norm_corrected_hist_8A,
			&dyn_total_hist_8A,  &dyn_norm_total_hist_8A, &dyn_decay_norm_total_hist_8A,
			&dyn_max_hist_8A,  &sum_max_hist_8A, &dyn_norm_max_hist_8A, &dyn_norm_corrected_max_hist_8A,  &dyn_decay_norm_max_hist_8A, &dyn_decay_norm_corrected_max_hist_8A);
		}
		if (dyn_m8B!=-1) {
			Dyn_Analyse(&dyn_a8B, &dyn_e8B, &dyn_n8B, &dyn_l8B, 
			&dyn_hist_8B, &sum_hist_8B, &dyn_norm_hist_8B, &dyn_norm_corrected_hist_8B, &dyn_decay_norm_hist_8B, &dyn_decay_norm_corrected_hist_8B,
			&dyn_total_hist_8B,  &dyn_norm_total_hist_8B, &dyn_decay_norm_total_hist_8B,
			&dyn_max_hist_8B,  &sum_max_hist_8B, &dyn_norm_max_hist_8B, &dyn_norm_corrected_max_hist_8B,  &dyn_decay_norm_max_hist_8B, &dyn_decay_norm_corrected_max_hist_8B);
		}
		if (dyn_m8K!=-1) {
			Dyn_Analyse(&dyn_a8K, &dyn_e8K, &dyn_n8K, &dyn_l8K, 
			&dyn_hist_8K, &sum_hist_8K, &dyn_norm_hist_8K, &dyn_norm_corrected_hist_8K, &dyn_decay_norm_hist_8K, &dyn_decay_norm_corrected_hist_8K,
			&dyn_total_hist_8K,  &dyn_norm_total_hist_8K, &dyn_decay_norm_total_hist_8K,
			&dyn_max_hist_8K,  &sum_max_hist_8K, &dyn_norm_max_hist_8K, &dyn_norm_corrected_max_hist_8K,  &dyn_decay_norm_max_hist_8K, &dyn_decay_norm_corrected_max_hist_8K);
		}
		if (dyn_m9A!=-1) {
			Dyn_Analyse(&dyn_a9A, &dyn_e9A, &dyn_n9A, &dyn_l9A, 
			&dyn_hist_9A, &sum_hist_9A, &dyn_norm_hist_9A, &dyn_norm_corrected_hist_9A, &dyn_decay_norm_hist_9A, &dyn_decay_norm_corrected_hist_9A,
			&dyn_total_hist_9A,  &dyn_norm_total_hist_9A, &dyn_decay_norm_total_hist_9A,
			&dyn_max_hist_9A,  &sum_max_hist_9A, &dyn_norm_max_hist_9A, &dyn_norm_corrected_max_hist_9A,  &dyn_decay_norm_max_hist_9A, &dyn_decay_norm_corrected_max_hist_9A);
		}
		if (dyn_m9B!=-1) {
			Dyn_Analyse(&dyn_a9B, &dyn_e9B, &dyn_n9B, &dyn_l9B, 
			&dyn_hist_9B, &sum_hist_9B, &dyn_norm_hist_9B, &dyn_norm_corrected_hist_9B, &dyn_decay_norm_hist_9B, &dyn_decay_norm_corrected_hist_9B,
			&dyn_total_hist_9B,  &dyn_norm_total_hist_9B, &dyn_decay_norm_total_hist_9B,
			&dyn_max_hist_9B,  &sum_max_hist_9B, &dyn_norm_max_hist_9B, &dyn_norm_corrected_max_hist_9B,  &dyn_decay_norm_max_hist_9B, &dyn_decay_norm_corrected_max_hist_9B);
		}
		if (dyn_m9K!=-1) {
			Dyn_Analyse(&dyn_a9K, &dyn_e9K, &dyn_n9K, &dyn_l9K, 
			&dyn_hist_9K, &sum_hist_9K, &dyn_norm_hist_9K, &dyn_norm_corrected_hist_9K, &dyn_decay_norm_hist_9K, &dyn_decay_norm_corrected_hist_9K,
			&dyn_total_hist_9K,  &dyn_norm_total_hist_9K, &dyn_decay_norm_total_hist_9K,
			&dyn_max_hist_9K,  &sum_max_hist_9K, &dyn_norm_max_hist_9K, &dyn_norm_corrected_max_hist_9K,  &dyn_decay_norm_max_hist_9K, &dyn_decay_norm_corrected_max_hist_9K);
		}
		if (dyn_m10A!=-1) {
			Dyn_Analyse(&dyn_a10A, &dyn_e10A, &dyn_n10A, &dyn_l10A, 
			&dyn_hist_10A, &sum_hist_10A, &dyn_norm_hist_10A, &dyn_norm_corrected_hist_10A, &dyn_decay_norm_hist_10A, &dyn_decay_norm_corrected_hist_10A,
			&dyn_total_hist_10A,  &dyn_norm_total_hist_10A, &dyn_decay_norm_total_hist_10A,
			&dyn_max_hist_10A,  &sum_max_hist_10A, &dyn_norm_max_hist_10A, &dyn_norm_corrected_max_hist_10A,  &dyn_decay_norm_max_hist_10A, &dyn_decay_norm_corrected_max_hist_10A);
		}
		if (dyn_m10B!=-1) {
			Dyn_Analyse(&dyn_a10B, &dyn_e10B, &dyn_n10B, &dyn_l10B, 
			&dyn_hist_10B, &sum_hist_10B, &dyn_norm_hist_10B, &dyn_norm_corrected_hist_10B, &dyn_decay_norm_hist_10B, &dyn_decay_norm_corrected_hist_10B,
			&dyn_total_hist_10B,  &dyn_norm_total_hist_10B, &dyn_decay_norm_total_hist_10B,
			&dyn_max_hist_10B,  &sum_max_hist_10B, &dyn_norm_max_hist_10B, &dyn_norm_corrected_max_hist_10B,  &dyn_decay_norm_max_hist_10B, &dyn_decay_norm_corrected_max_hist_10B);
		}
		if (dyn_m10K!=-1) {
			Dyn_Analyse(&dyn_a10K, &dyn_e10K, &dyn_n10K, &dyn_l10K, 
			&dyn_hist_10K, &sum_hist_10K, &dyn_norm_hist_10K, &dyn_norm_corrected_hist_10K, &dyn_decay_norm_hist_10K, &dyn_decay_norm_corrected_hist_10K,
			&dyn_total_hist_10K,  &dyn_norm_total_hist_10K, &dyn_decay_norm_total_hist_10K,
			&dyn_max_hist_10K,  &sum_max_hist_10K, &dyn_norm_max_hist_10K, &dyn_norm_corrected_max_hist_10K,  &dyn_decay_norm_max_hist_10K, &dyn_decay_norm_corrected_max_hist_10K);
		}
		if (dyn_m10W!=-1) {
			Dyn_Analyse(&dyn_a10W, &dyn_e10W, &dyn_n10W, &dyn_l10W, 
			&dyn_hist_10W, &sum_hist_10W, &dyn_norm_hist_10W, &dyn_norm_corrected_hist_10W, &dyn_decay_norm_hist_10W, &dyn_decay_norm_corrected_hist_10W,
			&dyn_total_hist_10W,  &dyn_norm_total_hist_10W, &dyn_decay_norm_total_hist_10W,
			&dyn_max_hist_10W,  &sum_max_hist_10W, &dyn_norm_max_hist_10W, &dyn_norm_corrected_max_hist_10W,  &dyn_decay_norm_max_hist_10W, &dyn_decay_norm_corrected_max_hist_10W);
		}
		if (dyn_m11A!=-1) {
			Dyn_Analyse(&dyn_a11A, &dyn_e11A, &dyn_n11A, &dyn_l11A, 
			&dyn_hist_11A, &sum_hist_11A, &dyn_norm_hist_11A, &dyn_norm_corrected_hist_11A, &dyn_decay_norm_hist_11A, &dyn_decay_norm_corrected_hist_11A,
			&dyn_total_hist_11A,  &dyn_norm_total_hist_11A, &dyn_decay_norm_total_hist_11A,
			&dyn_max_hist_11A,  &sum_max_hist_11A, &dyn_norm_max_hist_11A, &dyn_norm_corrected_max_hist_11A,  &dyn_decay_norm_max_hist_11A, &dyn_decay_norm_corrected_max_hist_11A);
		}
		if (dyn_m11B!=-1) {
			Dyn_Analyse(&dyn_a11B, &dyn_e11B, &dyn_n11B, &dyn_l11B, 
			&dyn_hist_11B, &sum_hist_11B, &dyn_norm_hist_11B, &dyn_norm_corrected_hist_11B, &dyn_decay_norm_hist_11B, &dyn_decay_norm_corrected_hist_11B,
			&dyn_total_hist_11B,  &dyn_norm_total_hist_11B, &dyn_decay_norm_total_hist_11B,
			&dyn_max_hist_11B,  &sum_max_hist_11B, &dyn_norm_max_hist_11B, &dyn_norm_corrected_max_hist_11B,  &dyn_decay_norm_max_hist_11B, &dyn_decay_norm_corrected_max_hist_11B);
		}
		if (dyn_m11C!=-1) {
			Dyn_Analyse(&dyn_a11C, &dyn_e11C, &dyn_n11C, &dyn_l11C, 
			&dyn_hist_11C, &sum_hist_11C, &dyn_norm_hist_11C, &dyn_norm_corrected_hist_11C, &dyn_decay_norm_hist_11C, &dyn_decay_norm_corrected_hist_11C,
			&dyn_total_hist_11C,  &dyn_norm_total_hist_11C, &dyn_decay_norm_total_hist_11C,
			&dyn_max_hist_11C,  &sum_max_hist_11C, &dyn_norm_max_hist_11C, &dyn_norm_corrected_max_hist_11C,  &dyn_decay_norm_max_hist_11C, &dyn_decay_norm_corrected_max_hist_11C);
		}
		if (dyn_m11E!=-1) {
			Dyn_Analyse(&dyn_a11E, &dyn_e11E, &dyn_n11E, &dyn_l11E, 
			&dyn_hist_11E, &sum_hist_11E, &dyn_norm_hist_11E, &dyn_norm_corrected_hist_11E, &dyn_decay_norm_hist_11E, &dyn_decay_norm_corrected_hist_11E,
			&dyn_total_hist_11E,  &dyn_norm_total_hist_11E, &dyn_decay_norm_total_hist_11E,
			&dyn_max_hist_11E,  &sum_max_hist_11E, &dyn_norm_max_hist_11E, &dyn_norm_corrected_max_hist_11E,  &dyn_decay_norm_max_hist_11E, &dyn_decay_norm_corrected_max_hist_11E);
		}
		if (dyn_m11F!=-1) {
			Dyn_Analyse(&dyn_a11F, &dyn_e11F, &dyn_n11F, &dyn_l11F, 
			&dyn_hist_11F, &sum_hist_11F, &dyn_norm_hist_11F, &dyn_norm_corrected_hist_11F, &dyn_decay_norm_hist_11F, &dyn_decay_norm_corrected_hist_11F,
			&dyn_total_hist_11F,  &dyn_norm_total_hist_11F, &dyn_decay_norm_total_hist_11F,
			&dyn_max_hist_11F,  &sum_max_hist_11F, &dyn_norm_max_hist_11F, &dyn_norm_corrected_max_hist_11F,  &dyn_decay_norm_max_hist_11F, &dyn_decay_norm_corrected_max_hist_11F);
		}
		if (dyn_m11W!=-1) {
			Dyn_Analyse(&dyn_a11W, &dyn_e11W, &dyn_n11W, &dyn_l11W, 
			&dyn_hist_11W, &sum_hist_11W, &dyn_norm_hist_11W, &dyn_norm_corrected_hist_11W, &dyn_decay_norm_hist_11W, &dyn_decay_norm_corrected_hist_11W,
			&dyn_total_hist_11W,  &dyn_norm_total_hist_11W, &dyn_decay_norm_total_hist_11W,
			&dyn_max_hist_11W,  &sum_max_hist_11W, &dyn_norm_max_hist_11W, &dyn_norm_corrected_max_hist_11W,  &dyn_decay_norm_max_hist_11W, &dyn_decay_norm_corrected_max_hist_11W);
		}
		if (dyn_m12A!=-1) {
			Dyn_Analyse(&dyn_a12A, &dyn_e12A, &dyn_n12A, &dyn_l12A, 
			&dyn_hist_12A, &sum_hist_12A, &dyn_norm_hist_12A, &dyn_norm_corrected_hist_12A, &dyn_decay_norm_hist_12A, &dyn_decay_norm_corrected_hist_12A,
			&dyn_total_hist_12A,  &dyn_norm_total_hist_12A, &dyn_decay_norm_total_hist_12A,
			&dyn_max_hist_12A,  &sum_max_hist_12A, &dyn_norm_max_hist_12A, &dyn_norm_corrected_max_hist_12A,  &dyn_decay_norm_max_hist_12A, &dyn_decay_norm_corrected_max_hist_12A);
		}
		if (dyn_m12B!=-1) {
			Dyn_Analyse(&dyn_a12B, &dyn_e12B, &dyn_n12B, &dyn_l12B, 
			&dyn_hist_12B, &sum_hist_12B, &dyn_norm_hist_12B, &dyn_norm_corrected_hist_12B, &dyn_decay_norm_hist_12B, &dyn_decay_norm_corrected_hist_12B,
			&dyn_total_hist_12B,  &dyn_norm_total_hist_12B, &dyn_decay_norm_total_hist_12B,
			&dyn_max_hist_12B,  &sum_max_hist_12B, &dyn_norm_max_hist_12B, &dyn_norm_corrected_max_hist_12B,  &dyn_decay_norm_max_hist_12B, &dyn_decay_norm_corrected_max_hist_12B);
		}
		if (dyn_m12D!=-1) {
			Dyn_Analyse(&dyn_a12D, &dyn_e12D, &dyn_n12D, &dyn_l12D, 
			&dyn_hist_12D, &sum_hist_12D, &dyn_norm_hist_12D, &dyn_norm_corrected_hist_12D, &dyn_decay_norm_hist_12D, &dyn_decay_norm_corrected_hist_12D,
			&dyn_total_hist_12D,  &dyn_norm_total_hist_12D, &dyn_decay_norm_total_hist_12D,
			&dyn_max_hist_12D,  &sum_max_hist_12D, &dyn_norm_max_hist_12D, &dyn_norm_corrected_max_hist_12D,  &dyn_decay_norm_max_hist_12D, &dyn_decay_norm_corrected_max_hist_12D);
		}
		if (dyn_m12E!=-1) {
			Dyn_Analyse(&dyn_a12E, &dyn_e12E, &dyn_n12E, &dyn_l12E, 
			&dyn_hist_12E, &sum_hist_12E, &dyn_norm_hist_12E, &dyn_norm_corrected_hist_12E, &dyn_decay_norm_hist_12E, &dyn_decay_norm_corrected_hist_12E,
			&dyn_total_hist_12E,  &dyn_norm_total_hist_12E, &dyn_decay_norm_total_hist_12E,
			&dyn_max_hist_12E,  &sum_max_hist_12E, &dyn_norm_max_hist_12E, &dyn_norm_corrected_max_hist_12E,  &dyn_decay_norm_max_hist_12E, &dyn_decay_norm_corrected_max_hist_12E);
		}
		if (dyn_m12K!=-1) {
			Dyn_Analyse(&dyn_a12K, &dyn_e12K, &dyn_n12K, &dyn_l12K, 
			&dyn_hist_12K, &sum_hist_12K, &dyn_norm_hist_12K, &dyn_norm_corrected_hist_12K, &dyn_decay_norm_hist_12K, &dyn_decay_norm_corrected_hist_12K,
			&dyn_total_hist_12K,  &dyn_norm_total_hist_12K, &dyn_decay_norm_total_hist_12K,
			&dyn_max_hist_12K,  &sum_max_hist_12K, &dyn_norm_max_hist_12K, &dyn_norm_corrected_max_hist_12K,  &dyn_decay_norm_max_hist_12K, &dyn_decay_norm_corrected_max_hist_12K);
		}
		if (dyn_m13A!=-1) {
			Dyn_Analyse(&dyn_a13A, &dyn_e13A, &dyn_n13A, &dyn_l13A, 
			&dyn_hist_13A, &sum_hist_13A, &dyn_norm_hist_13A, &dyn_norm_corrected_hist_13A, &dyn_decay_norm_hist_13A, &dyn_decay_norm_corrected_hist_13A,
			&dyn_total_hist_13A,  &dyn_norm_total_hist_13A, &dyn_decay_norm_total_hist_13A,
			&dyn_max_hist_13A,  &sum_max_hist_13A, &dyn_norm_max_hist_13A, &dyn_norm_corrected_max_hist_13A,  &dyn_decay_norm_max_hist_13A, &dyn_decay_norm_corrected_max_hist_13A);
		}
		if (dyn_m13B!=-1) {
			Dyn_Analyse(&dyn_a13B, &dyn_e13B, &dyn_n13B, &dyn_l13B, 
			&dyn_hist_13B, &sum_hist_13B, &dyn_norm_hist_13B, &dyn_norm_corrected_hist_13B, &dyn_decay_norm_hist_13B, &dyn_decay_norm_corrected_hist_13B,
			&dyn_total_hist_13B,  &dyn_norm_total_hist_13B, &dyn_decay_norm_total_hist_13B,
			&dyn_max_hist_13B,  &sum_max_hist_13B, &dyn_norm_max_hist_13B, &dyn_norm_corrected_max_hist_13B,  &dyn_decay_norm_max_hist_13B, &dyn_decay_norm_corrected_max_hist_13B);
		}
		if (dyn_m13K!=-1) {
			Dyn_Analyse(&dyn_a13K, &dyn_e13K, &dyn_n13K, &dyn_l13K, 
			&dyn_hist_13K, &sum_hist_13K, &dyn_norm_hist_13K, &dyn_norm_corrected_hist_13K, &dyn_decay_norm_hist_13K, &dyn_decay_norm_corrected_hist_13K,
			&dyn_total_hist_13K,  &dyn_norm_total_hist_13K, &dyn_decay_norm_total_hist_13K,
			&dyn_max_hist_13K,  &sum_max_hist_13K, &dyn_norm_max_hist_13K, &dyn_norm_corrected_max_hist_13K,  &dyn_decay_norm_max_hist_13K, &dyn_decay_norm_corrected_max_hist_13K);
		}
		if (dyn_mFCC!=-1) {
			Dyn_Analyse(&dyn_aFCC, &dyn_eFCC, &dyn_nFCC, &dyn_lFCC, 
			&dyn_hist_FCC, &sum_hist_FCC, &dyn_norm_hist_FCC, &dyn_norm_corrected_hist_FCC, &dyn_decay_norm_hist_FCC, &dyn_decay_norm_corrected_hist_FCC,
			&dyn_total_hist_FCC,  &dyn_norm_total_hist_FCC, &dyn_decay_norm_total_hist_FCC,
			&dyn_max_hist_FCC,  &sum_max_hist_FCC, &dyn_norm_max_hist_FCC, &dyn_norm_corrected_max_hist_FCC,  &dyn_decay_norm_max_hist_FCC, &dyn_decay_norm_corrected_max_hist_FCC);
		}
		if (dyn_mHCP!=-1) {
			Dyn_Analyse(&dyn_aHCP, &dyn_eHCP, &dyn_nHCP, &dyn_lHCP, 
			&dyn_hist_HCP, &sum_hist_HCP, &dyn_norm_hist_HCP, &dyn_norm_corrected_hist_HCP, &dyn_decay_norm_hist_HCP, &dyn_decay_norm_corrected_hist_HCP,
			&dyn_total_hist_HCP,  &dyn_norm_total_hist_HCP, &dyn_decay_norm_total_hist_HCP,
			&dyn_max_hist_HCP,  &sum_max_hist_HCP, &dyn_norm_max_hist_HCP, &dyn_norm_corrected_max_hist_HCP,  &dyn_decay_norm_max_hist_HCP, &dyn_decay_norm_corrected_max_hist_HCP);
		}
		if (dyn_mBCC_9!=-1) {
			Dyn_Analyse(&dyn_aBCC_9, &dyn_eBCC_9, &dyn_nBCC_9, &dyn_lBCC_9, 
			&dyn_hist_BCC_9, &sum_hist_BCC_9, &dyn_norm_hist_BCC_9, &dyn_norm_corrected_hist_BCC_9, &dyn_decay_norm_hist_BCC_9, &dyn_decay_norm_corrected_hist_BCC_9,
			&dyn_total_hist_BCC_9,  &dyn_norm_total_hist_BCC_9, &dyn_decay_norm_total_hist_BCC_9,
			&dyn_max_hist_BCC_9,  &sum_max_hist_BCC_9, &dyn_norm_max_hist_BCC_9, &dyn_norm_corrected_max_hist_BCC_9,  &dyn_decay_norm_max_hist_BCC_9, &dyn_decay_norm_corrected_max_hist_BCC_9);
		}
		if (dyn_mBCC_15!=-1) {
			Dyn_Analyse(&dyn_aBCC_15, &dyn_eBCC_15, &dyn_nBCC_15, &dyn_lBCC_15, 
			&dyn_hist_BCC_15, &sum_hist_BCC_15, &dyn_norm_hist_BCC_15, &dyn_norm_corrected_hist_BCC_15, &dyn_decay_norm_hist_BCC_15, &dyn_decay_norm_corrected_hist_BCC_15,
			&dyn_total_hist_BCC_15,  &dyn_norm_total_hist_BCC_15, &dyn_decay_norm_total_hist_BCC_15,
			&dyn_max_hist_BCC_15,  &sum_max_hist_BCC_15, &dyn_norm_max_hist_BCC_15, &dyn_norm_corrected_max_hist_BCC_15,  &dyn_decay_norm_max_hist_BCC_15, &dyn_decay_norm_corrected_max_hist_BCC_15);
		}
		
		
		fprintf(dyn_lives_file,"frames	frames/t_alpha");
		fprintf(dyn_lives_file,"	sp3_events	sp3_norm_events	sp3_corrected_events	sp3_norm_corrected_events	sp3_total_events	sp3_norm_total_events	sp3_max_events	sp3_norm_max_events	sp3_corrected_max_events	sp3_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp3a_events	sp3a_norm_events	sp3a_corrected_events	sp3a_norm_corrected_events	sp3a_total_events	sp3a_norm_total_events	sp3a_max_events	sp3a_norm_max_events	sp3a_corrected_max_events	sp3a_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp3b_events	sp3b_norm_events	sp3b_corrected_events	sp3b_norm_corrected_events	sp3b_total_events	sp3b_norm_total_events	sp3b_max_events	sp3b_norm_max_events	sp3b_corrected_max_events	sp3b_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	5A_events	5A_norm_events	5A_corrected_events	5A_norm_corrected_events	5A_total_events	5A_norm_total_events	5A_max_events	5A_norm_max_events	5A_corrected_max_events	5A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp4_events	sp4_norm_events	sp4_corrected_events	sp4_norm_corrected_events	sp4_total_events	sp4_norm_total_events	sp4_max_events	sp4_norm_max_events	sp4_corrected_max_events	sp4_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp4a_events	sp4a_norm_events	sp4a_corrected_events	sp4a_norm_corrected_events	sp4a_total_events	sp4a_norm_total_events	sp4a_max_events	sp4a_norm_max_events	sp4a_corrected_max_events	sp4a_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp4b_events	sp4b_norm_events	sp4b_corrected_events	sp4b_norm_corrected_events	sp4b_total_events	sp4b_norm_total_events	sp4b_max_events	sp4b_norm_max_events	sp4b_corrected_max_events	sp4b_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	6A_events	6A_norm_events	6A_corrected_events	6A_norm_corrected_events	6A_total_events	6A_norm_total_events	6A_max_events	6A_norm_max_events	6A_corrected_max_events	6A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	6Z_events	6Z_norm_events	6Z_corrected_events	6Z_norm_corrected_events	6Z_total_events	6Z_norm_total_events	6Z_max_events	6Z_norm_max_events	6Z_corrected_max_events	6Z_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp5_events	sp5_norm_events	sp5_corrected_events	sp5_norm_corrected_events	sp5_total_events	sp5_norm_total_events	sp5_max_events	sp5_norm_max_events	sp5_corrected_max_events	sp5_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp5a_events	sp5a_norm_events	sp5a_corrected_events	sp5a_norm_corrected_events	sp5a_total_events	sp5a_norm_total_events	sp5a_max_events	sp5a_norm_max_events	sp5a_corrected_max_events	sp5a_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	sp5b_events	sp5b_norm_events	sp5b_corrected_events	sp5b_norm_corrected_events	sp5b_total_events	sp5b_norm_total_events	sp5b_max_events	sp5b_norm_max_events	sp5b_corrected_max_events	sp5b_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	7A_events	7A_norm_events	7A_corrected_events	7A_norm_corrected_events	7A_total_events	7A_norm_total_events	7A_max_events	7A_norm_max_events	7A_corrected_max_events	7A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	7K_events	7K_norm_events	7K_corrected_events	7K_norm_corrected_events	7K_total_events	7K_norm_total_events	7K_max_events	7K_norm_max_events	7K_corrected_max_events	7K_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	8A_events	8A_norm_events	8A_corrected_events	8A_norm_corrected_events	8A_total_events	8A_norm_total_events	8A_max_events	8A_norm_max_events	8A_corrected_max_events	8A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	8B_events	8B_norm_events	8B_corrected_events	8B_norm_corrected_events	8B_total_events	8B_norm_total_events	8B_max_events	8B_norm_max_events	8B_corrected_max_events	8B_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	8K_events	8K_norm_events	8K_corrected_events	8K_norm_corrected_events	8K_total_events	8K_norm_total_events	8K_max_events	8K_norm_max_events	8K_corrected_max_events	8K_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	9A_events	9A_norm_events	9A_corrected_events	9A_norm_corrected_events	9A_total_events	9A_norm_total_events	9A_max_events	9A_norm_max_events	9A_corrected_max_events	9A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	9B_events	9B_norm_events	9B_corrected_events	9B_norm_corrected_events	9B_total_events	9B_norm_total_events	9B_max_events	9B_norm_max_events	9B_corrected_max_events	9B_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	9K_events	9K_norm_events	9K_corrected_events	9K_norm_corrected_events	9K_total_events	9K_norm_total_events	9K_max_events	9K_norm_max_events	9K_corrected_max_events	9K_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	10A_events	10A_norm_events	10A_corrected_events	10A_norm_corrected_events	10A_total_events	10A_norm_total_events	10A_max_events	10A_norm_max_events	10A_corrected_max_events	10A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	10B_events	10B_norm_events	10B_corrected_events	10B_norm_corrected_events	10B_total_events	10B_norm_total_events	10B_max_events	10B_norm_max_events	10B_corrected_max_events	10B_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	10K_events	10K_norm_events	10K_corrected_events	10K_norm_corrected_events	10K_total_events	10K_norm_total_events	10K_max_events	10K_norm_max_events	10K_corrected_max_events	10K_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	10W_events	10W_norm_events	10W_corrected_events	10W_norm_corrected_events	10W_total_events	10W_norm_total_events	10W_max_events	10W_norm_max_events	10W_corrected_max_events	10W_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	11A_events	11A_norm_events	11A_corrected_events	11A_norm_corrected_events	11A_total_events	11A_norm_total_events	11A_max_events	11A_norm_max_events	11A_corrected_max_events	11A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	11B_events	11B_norm_events	11B_corrected_events	11B_norm_corrected_events	11B_total_events	11B_norm_total_events	11B_max_events	11B_norm_max_events	11B_corrected_max_events	11B_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	11C_events	11C_norm_events	11C_corrected_events	11C_norm_corrected_events	11C_total_events	11C_norm_total_events	11C_max_events	11C_norm_max_events	11C_corrected_max_events	11C_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	11E_events	11E_norm_events	11E_corrected_events	11E_norm_corrected_events	11E_total_events	11E_norm_total_events	11E_max_events	11E_norm_max_events	11E_corrected_max_events	11E_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	11F_events	11F_norm_events	11F_corrected_events	11F_norm_corrected_events	11F_total_events	11F_norm_total_events	11F_max_events	11F_norm_max_events	11F_corrected_max_events	11F_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	11W_events	11W_norm_events	11W_corrected_events	11W_norm_corrected_events	11W_total_events	11W_norm_total_events	11W_max_events	11W_norm_max_events	11W_corrected_max_events	11W_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	12A_events	12A_norm_events	12A_corrected_events	12A_norm_corrected_events	12A_total_events	12A_norm_total_events	12A_max_events	12A_norm_max_events	12A_corrected_max_events	12A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	12B_events	12B_norm_events	12B_corrected_events	12B_norm_corrected_events	12B_total_events	12B_norm_total_events	12B_max_events	12B_norm_max_events	12B_corrected_max_events	12B_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	12D_events	12D_norm_events	12D_corrected_events	12D_norm_corrected_events	12D_total_events	12D_norm_total_events	12D_max_events	12D_norm_max_events	12D_corrected_max_events	12D_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	12E_events	12E_norm_events	12E_corrected_events	12E_norm_corrected_events	12E_total_events	12E_norm_total_events	12E_max_events	12E_norm_max_events	12E_corrected_max_events	12E_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	12K_events	12K_norm_events	12K_corrected_events	12K_norm_corrected_events	12K_total_events	12K_norm_total_events	12K_max_events	12K_norm_max_events	12K_corrected_max_events	12K_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	13A_events	13A_norm_events	13A_corrected_events	13A_norm_corrected_events	13A_total_events	13A_norm_total_events	13A_max_events	13A_norm_max_events	13A_corrected_max_events	13A_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	13B_events	13B_norm_events	13B_corrected_events	13B_norm_corrected_events	13B_total_events	13B_norm_total_events	13B_max_events	13B_norm_max_events	13B_corrected_max_events	13B_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	13K_events	13K_norm_events	13K_corrected_events	13K_norm_corrected_events	13K_total_events	13K_norm_total_events	13K_max_events	13K_norm_max_events	13K_corrected_max_events	13K_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	FCC_events	FCC_norm_events	FCC_corrected_events	FCC_norm_corrected_events	FCC_total_events	FCC_norm_total_events	FCC_max_events	FCC_norm_max_events	FCC_corrected_max_events	FCC_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	HCP_events	HCP_norm_events	HCP_corrected_events	HCP_norm_corrected_events	HCP_total_events	HCP_norm_total_events	HCP_max_events	HCP_norm_max_events	HCP_corrected_max_events	HCP_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	BCC_9_events	BCC_9_norm_events	BCC_9_corrected_events	BCC_9_norm_corrected_events	BCC_9_total_events	BCC_9_norm_total_events	BCC_9_max_events	BCC_9_norm_max_events	BCC_9_corrected_max_events	BCC_9_norm_corrected_max_events");
		fprintf(dyn_lives_file,"	BCC_15_events	BCC_15_norm_events	BCC_15_corrected_events	BCC_15_norm_corrected_events	BCC_15_total_events	BCC_15_norm_total_events	BCC_15_max_events	BCC_15_norm_max_events	BCC_15_corrected_max_events	BCC_15_norm_corrected_max_events\n");
		
		for (f=0; f<FRAMES; f++) {
			fprintf(dyn_lives_file,"%d	%.15lg",(f+1),(f+1)*FRAMETSTEP*SAMPLEFREQ/talpha);
			
			if (dyn_msp3!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp3[f],dyn_norm_hist_sp3[f],(double)dyn_hist_sp3[f]/(FRAMES-f),dyn_norm_corrected_hist_sp3[f],dyn_total_hist_sp3[f],dyn_norm_total_hist_sp3[f],dyn_max_hist_sp3[f],dyn_norm_max_hist_sp3[f],(double)dyn_max_hist_sp3[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp3[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp3a!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp3a[f],dyn_norm_hist_sp3a[f],(double)dyn_hist_sp3a[f]/(FRAMES-f),dyn_norm_corrected_hist_sp3a[f],dyn_total_hist_sp3a[f],dyn_norm_total_hist_sp3a[f],dyn_max_hist_sp3a[f],dyn_norm_max_hist_sp3a[f],(double)dyn_max_hist_sp3a[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp3a[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp3b!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp3b[f],dyn_norm_hist_sp3b[f],(double)dyn_hist_sp3b[f]/(FRAMES-f),dyn_norm_corrected_hist_sp3b[f],dyn_total_hist_sp3b[f],dyn_norm_total_hist_sp3b[f],dyn_max_hist_sp3b[f],dyn_norm_max_hist_sp3b[f],(double)dyn_max_hist_sp3b[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp3b[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp3c!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp3c[f],dyn_norm_hist_sp3c[f],(double)dyn_hist_sp3c[f]/(FRAMES-f),dyn_norm_corrected_hist_sp3c[f],dyn_total_hist_sp3c[f],dyn_norm_total_hist_sp3c[f],dyn_max_hist_sp3c[f],dyn_norm_max_hist_sp3c[f],(double)dyn_max_hist_sp3c[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp3c[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp4!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp4[f],dyn_norm_hist_sp4[f],(double)dyn_hist_sp4[f]/(FRAMES-f),dyn_norm_corrected_hist_sp4[f],dyn_total_hist_sp4[f],dyn_norm_total_hist_sp4[f],dyn_max_hist_sp4[f],dyn_norm_max_hist_sp4[f],(double)dyn_max_hist_sp4[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp4[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp4a!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp4a[f],dyn_norm_hist_sp4a[f],(double)dyn_hist_sp4a[f]/(FRAMES-f),dyn_norm_corrected_hist_sp4a[f],dyn_total_hist_sp4a[f],dyn_norm_total_hist_sp4a[f],dyn_max_hist_sp4a[f],dyn_norm_max_hist_sp4a[f],(double)dyn_max_hist_sp4a[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp4a[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp4b!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp4b[f],dyn_norm_hist_sp4b[f],(double)dyn_hist_sp4b[f]/(FRAMES-f),dyn_norm_corrected_hist_sp4b[f],dyn_total_hist_sp4b[f],dyn_norm_total_hist_sp4b[f],dyn_max_hist_sp4b[f],dyn_norm_max_hist_sp4b[f],(double)dyn_max_hist_sp4b[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp4b[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m6A!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_6A[f],dyn_norm_hist_6A[f],(double)dyn_hist_6A[f]/(FRAMES-f),dyn_norm_corrected_hist_6A[f],dyn_total_hist_6A[f],dyn_norm_total_hist_6A[f],dyn_max_hist_6A[f],dyn_norm_max_hist_6A[f],(double)dyn_max_hist_6A[f]/(FRAMES-f),dyn_norm_corrected_max_hist_6A[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m6Z!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_6Z[f],dyn_norm_hist_6Z[f],(double)dyn_hist_6Z[f]/(FRAMES-f),dyn_norm_corrected_hist_6Z[f],dyn_total_hist_6Z[f],dyn_norm_total_hist_6Z[f],dyn_max_hist_6Z[f],dyn_norm_max_hist_6Z[f],(double)dyn_max_hist_6Z[f]/(FRAMES-f),dyn_norm_corrected_max_hist_6Z[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp5!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp5[f],dyn_norm_hist_sp5[f],(double)dyn_hist_sp5[f]/(FRAMES-f),dyn_norm_corrected_hist_sp5[f],dyn_total_hist_sp5[f],dyn_norm_total_hist_sp5[f],dyn_max_hist_sp5[f],dyn_norm_max_hist_sp5[f],(double)dyn_max_hist_sp5[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp5[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp5a!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp5a[f],dyn_norm_hist_sp5a[f],(double)dyn_hist_sp5a[f]/(FRAMES-f),dyn_norm_corrected_hist_sp5a[f],dyn_total_hist_sp5a[f],dyn_norm_total_hist_sp5a[f],dyn_max_hist_sp5a[f],dyn_norm_max_hist_sp5a[f],(double)dyn_max_hist_sp5a[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp5a[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp5b!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp5b[f],dyn_norm_hist_sp5b[f],(double)dyn_hist_sp5b[f]/(FRAMES-f),dyn_norm_corrected_hist_sp5b[f],dyn_total_hist_sp5b[f],dyn_norm_total_hist_sp5b[f],dyn_max_hist_sp5b[f],dyn_norm_max_hist_sp5b[f],(double)dyn_max_hist_sp5b[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp5b[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_msp5c!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_sp5c[f],dyn_norm_hist_sp5c[f],(double)dyn_hist_sp5c[f]/(FRAMES-f),dyn_norm_corrected_hist_sp5c[f],dyn_total_hist_sp5c[f],dyn_norm_total_hist_sp5c[f],dyn_max_hist_sp5c[f],dyn_norm_max_hist_sp5c[f],(double)dyn_max_hist_sp5c[f]/(FRAMES-f),dyn_norm_corrected_max_hist_sp5c[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m7K!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_7K[f],dyn_norm_hist_7K[f],(double)dyn_hist_7K[f]/(FRAMES-f),dyn_norm_corrected_hist_7K[f],dyn_total_hist_7K[f],dyn_norm_total_hist_7K[f],dyn_max_hist_7K[f],dyn_norm_max_hist_7K[f],(double)dyn_max_hist_7K[f]/(FRAMES-f),dyn_norm_corrected_max_hist_7K[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m8A!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_8A[f],dyn_norm_hist_8A[f],(double)dyn_hist_8A[f]/(FRAMES-f),dyn_norm_corrected_hist_8A[f],dyn_total_hist_8A[f],dyn_norm_total_hist_8A[f],dyn_max_hist_8A[f],dyn_norm_max_hist_8A[f],(double)dyn_max_hist_8A[f]/(FRAMES-f),dyn_norm_corrected_max_hist_8A[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m8B!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_8B[f],dyn_norm_hist_8B[f],(double)dyn_hist_8B[f]/(FRAMES-f),dyn_norm_corrected_hist_8B[f],dyn_total_hist_8B[f],dyn_norm_total_hist_8B[f],dyn_max_hist_8B[f],dyn_norm_max_hist_8B[f],(double)dyn_max_hist_8B[f]/(FRAMES-f),dyn_norm_corrected_max_hist_8B[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m8K!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_8K[f],dyn_norm_hist_8K[f],(double)dyn_hist_8K[f]/(FRAMES-f),dyn_norm_corrected_hist_8K[f],dyn_total_hist_8K[f],dyn_norm_total_hist_8K[f],dyn_max_hist_8K[f],dyn_norm_max_hist_8K[f],(double)dyn_max_hist_8K[f]/(FRAMES-f),dyn_norm_corrected_max_hist_8K[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m9A!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_9A[f],dyn_norm_hist_9A[f],(double)dyn_hist_9A[f]/(FRAMES-f),dyn_norm_corrected_hist_9A[f],dyn_total_hist_9A[f],dyn_norm_total_hist_9A[f],dyn_max_hist_9A[f],dyn_norm_max_hist_9A[f],(double)dyn_max_hist_9A[f]/(FRAMES-f),dyn_norm_corrected_max_hist_9A[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m9B!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_9B[f],dyn_norm_hist_9B[f],(double)dyn_hist_9B[f]/(FRAMES-f),dyn_norm_corrected_hist_9B[f],dyn_total_hist_9B[f],dyn_norm_total_hist_9B[f],dyn_max_hist_9B[f],dyn_norm_max_hist_9B[f],(double)dyn_max_hist_9B[f]/(FRAMES-f),dyn_norm_corrected_max_hist_9B[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m9K!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_9K[f],dyn_norm_hist_9K[f],(double)dyn_hist_9K[f]/(FRAMES-f),dyn_norm_corrected_hist_9K[f],dyn_total_hist_9K[f],dyn_norm_total_hist_9K[f],dyn_max_hist_9K[f],dyn_norm_max_hist_9K[f],(double)dyn_max_hist_9K[f]/(FRAMES-f),dyn_norm_corrected_max_hist_9K[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m10A!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_10A[f],dyn_norm_hist_10A[f],(double)dyn_hist_10A[f]/(FRAMES-f),dyn_norm_corrected_hist_10A[f],dyn_total_hist_10A[f],dyn_norm_total_hist_10A[f],dyn_max_hist_10A[f],dyn_norm_max_hist_10A[f],(double)dyn_max_hist_10A[f]/(FRAMES-f),dyn_norm_corrected_max_hist_10A[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m10B!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_10B[f],dyn_norm_hist_10B[f],(double)dyn_hist_10B[f]/(FRAMES-f),dyn_norm_corrected_hist_10B[f],dyn_total_hist_10B[f],dyn_norm_total_hist_10B[f],dyn_max_hist_10B[f],dyn_norm_max_hist_10B[f],(double)dyn_max_hist_10B[f]/(FRAMES-f),dyn_norm_corrected_max_hist_10B[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m10K!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_10K[f],dyn_norm_hist_10K[f],(double)dyn_hist_10K[f]/(FRAMES-f),dyn_norm_corrected_hist_10K[f],dyn_total_hist_10K[f],dyn_norm_total_hist_10K[f],dyn_max_hist_10K[f],dyn_norm_max_hist_10K[f],(double)dyn_max_hist_10K[f]/(FRAMES-f),dyn_norm_corrected_max_hist_10K[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m10W!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_10W[f],dyn_norm_hist_10W[f],(double)dyn_hist_10W[f]/(FRAMES-f),dyn_norm_corrected_hist_10W[f],dyn_total_hist_10W[f],dyn_norm_total_hist_10W[f],dyn_max_hist_10W[f],dyn_norm_max_hist_10W[f],(double)dyn_max_hist_10W[f]/(FRAMES-f),dyn_norm_corrected_max_hist_10W[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m11A!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_11A[f],dyn_norm_hist_11A[f],(double)dyn_hist_11A[f]/(FRAMES-f),dyn_norm_corrected_hist_11A[f],dyn_total_hist_11A[f],dyn_norm_total_hist_11A[f],dyn_max_hist_11A[f],dyn_norm_max_hist_11A[f],(double)dyn_max_hist_11A[f]/(FRAMES-f),dyn_norm_corrected_max_hist_11A[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m11B!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_11B[f],dyn_norm_hist_11B[f],(double)dyn_hist_11B[f]/(FRAMES-f),dyn_norm_corrected_hist_11B[f],dyn_total_hist_11B[f],dyn_norm_total_hist_11B[f],dyn_max_hist_11B[f],dyn_norm_max_hist_11B[f],(double)dyn_max_hist_11B[f]/(FRAMES-f),dyn_norm_corrected_max_hist_11B[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m11C!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_11C[f],dyn_norm_hist_11C[f],(double)dyn_hist_11C[f]/(FRAMES-f),dyn_norm_corrected_hist_11C[f],dyn_total_hist_11C[f],dyn_norm_total_hist_11C[f],dyn_max_hist_11C[f],dyn_norm_max_hist_11C[f],(double)dyn_max_hist_11C[f]/(FRAMES-f),dyn_norm_corrected_max_hist_11C[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m11E!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_11E[f],dyn_norm_hist_11E[f],(double)dyn_hist_11E[f]/(FRAMES-f),dyn_norm_corrected_hist_11E[f],dyn_total_hist_11E[f],dyn_norm_total_hist_11E[f],dyn_max_hist_11E[f],dyn_norm_max_hist_11E[f],(double)dyn_max_hist_11E[f]/(FRAMES-f),dyn_norm_corrected_max_hist_11E[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m11F!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_11F[f],dyn_norm_hist_11F[f],(double)dyn_hist_11F[f]/(FRAMES-f),dyn_norm_corrected_hist_11F[f],dyn_total_hist_11F[f],dyn_norm_total_hist_11F[f],dyn_max_hist_11F[f],dyn_norm_max_hist_11F[f],(double)dyn_max_hist_11F[f]/(FRAMES-f),dyn_norm_corrected_max_hist_11F[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m11W!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_11W[f],dyn_norm_hist_11W[f],(double)dyn_hist_11W[f]/(FRAMES-f),dyn_norm_corrected_hist_11W[f],dyn_total_hist_11W[f],dyn_norm_total_hist_11W[f],dyn_max_hist_11W[f],dyn_norm_max_hist_11W[f],(double)dyn_max_hist_11W[f]/(FRAMES-f),dyn_norm_corrected_max_hist_11W[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m12A!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_12A[f],dyn_norm_hist_12A[f],(double)dyn_hist_12A[f]/(FRAMES-f),dyn_norm_corrected_hist_12A[f],dyn_total_hist_12A[f],dyn_norm_total_hist_12A[f],dyn_max_hist_12A[f],dyn_norm_max_hist_12A[f],(double)dyn_max_hist_12A[f]/(FRAMES-f),dyn_norm_corrected_max_hist_12A[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m12B!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_12B[f],dyn_norm_hist_12B[f],(double)dyn_hist_12B[f]/(FRAMES-f),dyn_norm_corrected_hist_12B[f],dyn_total_hist_12B[f],dyn_norm_total_hist_12B[f],dyn_max_hist_12B[f],dyn_norm_max_hist_12B[f],(double)dyn_max_hist_12B[f]/(FRAMES-f),dyn_norm_corrected_max_hist_12B[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m12D!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_12D[f],dyn_norm_hist_12D[f],(double)dyn_hist_12D[f]/(FRAMES-f),dyn_norm_corrected_hist_12D[f],dyn_total_hist_12D[f],dyn_norm_total_hist_12D[f],dyn_max_hist_12D[f],dyn_norm_max_hist_12D[f],(double)dyn_max_hist_12D[f]/(FRAMES-f),dyn_norm_corrected_max_hist_12D[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m12E!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_12E[f],dyn_norm_hist_12E[f],(double)dyn_hist_12E[f]/(FRAMES-f),dyn_norm_corrected_hist_12E[f],dyn_total_hist_12E[f],dyn_norm_total_hist_12E[f],dyn_max_hist_12E[f],dyn_norm_max_hist_12E[f],(double)dyn_max_hist_12E[f]/(FRAMES-f),dyn_norm_corrected_max_hist_12E[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m12K!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_12K[f],dyn_norm_hist_12K[f],(double)dyn_hist_12K[f]/(FRAMES-f),dyn_norm_corrected_hist_12K[f],dyn_total_hist_12K[f],dyn_norm_total_hist_12K[f],dyn_max_hist_12K[f],dyn_norm_max_hist_12K[f],(double)dyn_max_hist_12K[f]/(FRAMES-f),dyn_norm_corrected_max_hist_12K[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m13A!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_13A[f],dyn_norm_hist_13A[f],(double)dyn_hist_13A[f]/(FRAMES-f),dyn_norm_corrected_hist_13A[f],dyn_total_hist_13A[f],dyn_norm_total_hist_13A[f],dyn_max_hist_13A[f],dyn_norm_max_hist_13A[f],(double)dyn_max_hist_13A[f]/(FRAMES-f),dyn_norm_corrected_max_hist_13A[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m13B!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_13B[f],dyn_norm_hist_13B[f],(double)dyn_hist_13B[f]/(FRAMES-f),dyn_norm_corrected_hist_13B[f],dyn_total_hist_13B[f],dyn_norm_total_hist_13B[f],dyn_max_hist_13B[f],dyn_norm_max_hist_13B[f],(double)dyn_max_hist_13B[f]/(FRAMES-f),dyn_norm_corrected_max_hist_13B[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_m13K!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_13K[f],dyn_norm_hist_13K[f],(double)dyn_hist_13K[f]/(FRAMES-f),dyn_norm_corrected_hist_13K[f],dyn_total_hist_13K[f],dyn_norm_total_hist_13K[f],dyn_max_hist_13K[f],dyn_norm_max_hist_13K[f],(double)dyn_max_hist_13K[f]/(FRAMES-f),dyn_norm_corrected_max_hist_13K[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_mFCC!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_FCC[f],dyn_norm_hist_FCC[f],(double)dyn_hist_FCC[f]/(FRAMES-f),dyn_norm_corrected_hist_FCC[f],dyn_total_hist_FCC[f],dyn_norm_total_hist_FCC[f],dyn_max_hist_FCC[f],dyn_norm_max_hist_FCC[f],(double)dyn_max_hist_FCC[f]/(FRAMES-f),dyn_norm_corrected_max_hist_FCC[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_mHCP!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_HCP[f],dyn_norm_hist_HCP[f],(double)dyn_hist_HCP[f]/(FRAMES-f),dyn_norm_corrected_hist_HCP[f],dyn_total_hist_HCP[f],dyn_norm_total_hist_HCP[f],dyn_max_hist_HCP[f],dyn_norm_max_hist_HCP[f],(double)dyn_max_hist_HCP[f]/(FRAMES-f),dyn_norm_corrected_max_hist_HCP[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_mBCC_9!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg",dyn_hist_BCC_9[f],dyn_norm_hist_BCC_9[f],(double)dyn_hist_BCC_9[f]/(FRAMES-f),dyn_norm_corrected_hist_BCC_9[f],dyn_total_hist_BCC_9[f],dyn_norm_total_hist_BCC_9[f],dyn_max_hist_BCC_9[f],dyn_norm_max_hist_BCC_9[f],(double)dyn_max_hist_BCC_9[f]/(FRAMES-f),dyn_norm_corrected_max_hist_BCC_9[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0");
			if (dyn_mBCC_15!=-1) fprintf(dyn_lives_file,"	%d	%.15lg	%.15lg	%.15lg	%d	%.15lg	%d	%.15lg	%.15lg	%.15lg\n",dyn_hist_BCC_15[f],dyn_norm_hist_BCC_15[f],(double)dyn_hist_BCC_15[f]/(FRAMES-f),dyn_norm_corrected_hist_BCC_15[f],dyn_total_hist_BCC_15[f],dyn_norm_total_hist_BCC_15[f],dyn_max_hist_BCC_15[f],dyn_norm_max_hist_BCC_15[f],(double)dyn_max_hist_BCC_15[f]/(FRAMES-f),dyn_norm_corrected_max_hist_BCC_15[f]);
			else fprintf(dyn_lives_file,"	0	0	0	0	0	0	0	0	0	0\n");
		}
		fprintf(dyn_lives_file,"-	-");
		fprintf(dyn_lives_file,"	n_unique_sp3	%d	n_samples_sp3	%d	n_av_samples_sp3	%d	-	-	-	-",dyn_nsp3,dyn_esp3,dyn_asp3);
		fprintf(dyn_lives_file,"	n_unique_sp3a	%d	n_samples_sp3a	%d	n_av_samples_sp3a	%d	-	-	-	-",dyn_nsp3a,dyn_esp3a,dyn_asp3a);
		fprintf(dyn_lives_file,"	n_unique_sp3b	%d	n_samples_sp3b	%d	n_av_samples_sp3b	%d	-	-	-	-",dyn_nsp3b,dyn_esp3b,dyn_asp3b);
		fprintf(dyn_lives_file,"	n_unique_5A	%d	n_samples_5A	%d	n_av_samples_5A	%d	-	-	-	-",dyn_nsp3c,dyn_nsp3c,dyn_nsp3c);
		fprintf(dyn_lives_file,"	n_unique_sp4	%d	n_samples_sp4	%d	n_av_samples_sp4	%d	-	-	-	-",dyn_nsp4,dyn_esp4,dyn_asp4);
		fprintf(dyn_lives_file,"	n_unique_sp4a	%d	n_samples_sp4a	%d	n_av_samples_sp4a	%d	-	-	-	-",dyn_nsp4a,dyn_esp4a,dyn_asp4a);
		fprintf(dyn_lives_file,"	n_unique_sp4b	%d	n_samples_sp4b	%d	n_av_samples_sp4b	%d	-	-	-	-",dyn_nsp4b,dyn_esp4b,dyn_asp4b);
		fprintf(dyn_lives_file,"	n_unique_6A	%d	n_samples_6A	%d	n_av_samples_6A	%d	-	-	-	-",dyn_n6A,dyn_e6A,dyn_a6A);
		fprintf(dyn_lives_file,"	n_unique_6Z	%d	n_samples_6Z	%d	n_av_samples_6Z	%d	-	-	-	-",dyn_n6Z,dyn_e6Z,dyn_a6Z);
		fprintf(dyn_lives_file,"	n_unique_sp5	%d	n_samples_sp5	%d	n_av_samples_sp5	%d	-	-	-	-",dyn_nsp5,dyn_esp5,dyn_asp5);
		fprintf(dyn_lives_file,"	n_unique_sp5a	%d	n_samples_sp5a	%d	n_av_samples_sp5a	%d	-	-	-	-",dyn_nsp5a,dyn_esp5a,dyn_asp5a);
		fprintf(dyn_lives_file,"	n_unique_sp5b	%d	n_samples_sp5b	%d	n_av_samples_sp5b	%d	-	-	-	-",dyn_nsp5b,dyn_esp5b,dyn_asp5b);
		fprintf(dyn_lives_file,"	n_unique_7A	%d	n_samples_7A	%d	n_av_samples_7A	%d	-	-	-	-",dyn_nsp5c,dyn_nsp5c,dyn_nsp5c);
		fprintf(dyn_lives_file,"	n_unique_7K	%d	n_samples_7K	%d	n_av_samples_7K	%d	-	-	-	-",dyn_n7K,dyn_e7K,dyn_a7K);
		fprintf(dyn_lives_file,"	n_unique_8A	%d	n_samples_8A	%d	n_av_samples_8A	%d	-	-	-	-",dyn_n8A,dyn_e8A,dyn_a8A);
		fprintf(dyn_lives_file,"	n_unique_8B	%d	n_samples_8B	%d	n_av_samples_8B	%d	-	-	-	-",dyn_n8B,dyn_e8B,dyn_a8B);
		fprintf(dyn_lives_file,"	n_unique_8K	%d	n_samples_8K	%d	n_av_samples_8K	%d	-	-	-	-",dyn_n8K,dyn_e8K,dyn_a8K);
		fprintf(dyn_lives_file,"	n_unique_9A	%d	n_samples_9A	%d	n_av_samples_9A	%d	-	-	-	-",dyn_n9A,dyn_e9A,dyn_a9A);
		fprintf(dyn_lives_file,"	n_unique_9B	%d	n_samples_9B	%d	n_av_samples_9B	%d	-	-	-	-",dyn_n9B,dyn_e9B,dyn_a9B);
		fprintf(dyn_lives_file,"	n_unique_9K	%d	n_samples_9K	%d	n_av_samples_9K	%d	-	-	-	-",dyn_n9K,dyn_e9K,dyn_a9K);
		fprintf(dyn_lives_file,"	n_unique_10A	%d	n_samples_10A	%d	n_av_samples_10A	%d	-	-	-	-",dyn_n10A,dyn_e10A,dyn_a10A);
		fprintf(dyn_lives_file,"	n_unique_10B	%d	n_samples_10B	%d	n_av_samples_10B	%d	-	-	-	-",dyn_n10B,dyn_e10B,dyn_a10B);
		fprintf(dyn_lives_file,"	n_unique_10K	%d	n_samples_10K	%d	n_av_samples_10K	%d	-	-	-	-",dyn_n10K,dyn_e10K,dyn_a10K);
		fprintf(dyn_lives_file,"	n_unique_10W	%d	n_samples_10W	%d	n_av_samples_10W	%d	-	-	-	-",dyn_n10W,dyn_e10W,dyn_a10W);
		fprintf(dyn_lives_file,"	n_unique_11A	%d	n_samples_11A	%d	n_av_samples_11A	%d	-	-	-	-",dyn_n11A,dyn_e11A,dyn_a11A);
		fprintf(dyn_lives_file,"	n_unique_11B	%d	n_samples_11B	%d	n_av_samples_11B	%d	-	-	-	-",dyn_n11B,dyn_e11B,dyn_a11B);
		fprintf(dyn_lives_file,"	n_unique_11C	%d	n_samples_11C	%d	n_av_samples_11C	%d	-	-	-	-",dyn_n11C,dyn_e11C,dyn_a11C);
		fprintf(dyn_lives_file,"	n_unique_11E	%d	n_samples_11E	%d	n_av_samples_11E	%d	-	-	-	-",dyn_n11E,dyn_e11E,dyn_a11E);
		fprintf(dyn_lives_file,"	n_unique_11F	%d	n_samples_11F	%d	n_av_samples_11F	%d	-	-	-	-",dyn_n11F,dyn_e11F,dyn_a11F);
		fprintf(dyn_lives_file,"	n_unique_11W	%d	n_samples_11W	%d	n_av_samples_11W	%d	-	-	-	-",dyn_n11W,dyn_e11W,dyn_a11W);
		fprintf(dyn_lives_file,"	n_unique_12A	%d	n_samples_12A	%d	n_av_samples_12A	%d	-	-	-	-",dyn_n12A,dyn_e12A,dyn_a12A);
		fprintf(dyn_lives_file,"	n_unique_12B	%d	n_samples_12B	%d	n_av_samples_12B	%d	-	-	-	-",dyn_n12B,dyn_e12B,dyn_a12B);
		fprintf(dyn_lives_file,"	n_unique_12D	%d	n_samples_12D	%d	n_av_samples_12D	%d	-	-	-	-",dyn_n12D,dyn_e12D,dyn_a12D);
		fprintf(dyn_lives_file,"	n_unique_12E	%d	n_samples_12E	%d	n_av_samples_12E	%d	-	-	-	-",dyn_n12E,dyn_e12E,dyn_a12E);
		fprintf(dyn_lives_file,"	n_unique_12K	%d	n_samples_12K	%d	n_av_samples_12K	%d	-	-	-	-",dyn_n12K,dyn_e12K,dyn_a12K);
		fprintf(dyn_lives_file,"	n_unique_13A	%d	n_samples_13A	%d	n_av_samples_13A	%d	-	-	-	-",dyn_n13A,dyn_e13A,dyn_a13A);
		fprintf(dyn_lives_file,"	n_unique_13B	%d	n_samples_13B	%d	n_av_samples_13B	%d	-	-	-	-",dyn_n13B,dyn_e13B,dyn_a13B);
		fprintf(dyn_lives_file,"	n_unique_13K	%d	n_samples_13K	%d	n_av_samples_13K	%d	-	-	-	-",dyn_n13K,dyn_e13K,dyn_a13K);
		fprintf(dyn_lives_file,"	n_unique_FCC	%d	n_samples_FCC	%d	n_av_samples_FCC	%d	-	-	-	-",dyn_nFCC,dyn_eFCC,dyn_aFCC);
		fprintf(dyn_lives_file,"	n_unique_HCP	%d	n_samples_HCP	%d	n_av_samples_HCP	%d	-	-	-	-",dyn_nHCP,dyn_eHCP,dyn_aHCP);
		fprintf(dyn_lives_file,"	n_unique_BCC_9	%d	n_samples_BCC_9	%d	n_av_samples_BCC_9	%d	-	-	-	-",dyn_nBCC_9,dyn_eBCC_9,dyn_aBCC_9);
		fprintf(dyn_lives_file,"	n_unique_BCC_15	%d	n_samples_BCC_15	%d	n_av_samples_BCC_15	%d	-	-	-	-\n",dyn_nBCC_15,dyn_eBCC_15,dyn_aBCC_15);
		
		fclose(dyn_lives_file);
		printf("d%d Written %s\n\n",rank,output);
		
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_decays",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		dyn_decays_file=fopen(output,"w");
		if (dyn_decays_file==NULL)  {
			sprintf(errMsg,"main(): Error opening file %s",output);	// Always test file open
			Error(errMsg);
		}
		fprintf(dyn_decays_file,"%s\n",output);
		
		fprintf(dyn_decays_file,"frames	frames/t_alpha");
		fprintf(dyn_decays_file,"	sp3_norm_hist	sp3_norm_corrected_hist	sp3_norm_total_hist	sp3_norm_max_hist	sp3_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp3a_norm_hist	sp3a_norm_corrected_hist	sp3a_norm_total_hist	sp3a_norm_max_hist	sp3a_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp3b_norm_hist	sp3b_norm_corrected_hist	sp3b_norm_total_hist	sp3b_norm_max_hist	sp3b_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	5A_norm_hist	5A_norm_corrected_hist	5A_norm_total_hist	5A_norm_max_hist	5A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp4_norm_hist	sp4_norm_corrected_hist	sp4_norm_total_hist	sp4_norm_max_hist	sp4_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp4a_norm_hist	sp4a_norm_corrected_hist	sp4a_norm_total_hist	sp4a_norm_max_hist	sp4a_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp4b_norm_hist	sp4b_norm_corrected_hist	sp4b_norm_total_hist	sp4b_norm_max_hist	sp4b_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	6A_norm_hist	6A_norm_corrected_hist	6A_norm_total_hist	6A_norm_max_hist	6A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	6Z_norm_hist	6Z_norm_corrected_hist	6Z_norm_total_hist	6Z_norm_max_hist	6Z_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp5_norm_hist	sp5_norm_corrected_hist	sp5_norm_total_hist	sp5_norm_max_hist	sp5_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp5a_norm_hist	sp5a_norm_corrected_hist	sp5a_norm_total_hist	sp5a_norm_max_hist	sp5a_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	sp5b_norm_hist	sp5b_norm_corrected_hist	sp5b_norm_total_hist	sp5b_norm_max_hist	sp5b_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	7A_norm_hist	7A_norm_corrected_hist	7A_norm_total_hist	7A_norm_max_hist	7A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	7K_norm_hist	7K_norm_corrected_hist	7K_norm_total_hist	7K_norm_max_hist	7K_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	8A_norm_hist	8A_norm_corrected_hist	8A_norm_total_hist	8A_norm_max_hist	8A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	8B_norm_hist	8B_norm_corrected_hist	8B_norm_total_hist	8B_norm_max_hist	8B_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	8K_norm_hist	8K_norm_corrected_hist	8K_norm_total_hist	8K_norm_max_hist	8K_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	9A_norm_hist	9A_norm_corrected_hist	9A_norm_total_hist	9A_norm_max_hist	9A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	9B_norm_hist	9B_norm_corrected_hist	9B_norm_total_hist	9B_norm_max_hist	9B_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	9K_norm_hist	9K_norm_corrected_hist	9K_norm_total_hist	9K_norm_max_hist	9K_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	10A_norm_hist	10A_norm_corrected_hist	10A_norm_total_hist	10A_norm_max_hist	10A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	10B_norm_hist	10B_norm_corrected_hist	10B_norm_total_hist	10B_norm_max_hist	10B_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	10K_norm_hist	10K_norm_corrected_hist	10K_norm_total_hist	10K_norm_max_hist	10K_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	10W_norm_hist	10W_norm_corrected_hist	10W_norm_total_hist	10W_norm_max_hist	10W_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	11A_norm_hist	11A_norm_corrected_hist	11A_norm_total_hist	11A_norm_max_hist	11A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	11B_norm_hist	11B_norm_corrected_hist	11B_norm_total_hist	11B_norm_max_hist	11B_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	11C_norm_hist	11C_norm_corrected_hist	11C_norm_total_hist	11C_norm_max_hist	11C_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	11E_norm_hist	11E_norm_corrected_hist	11E_norm_total_hist	11E_norm_max_hist	11E_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	11F_norm_hist	11F_norm_corrected_hist	11F_norm_total_hist	11F_norm_max_hist	11F_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	11W_norm_hist	11W_norm_corrected_hist	11W_norm_total_hist	11W_norm_max_hist	11W_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	12A_norm_hist	12A_norm_corrected_hist	12A_norm_total_hist	12A_norm_max_hist	12A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	12B_norm_hist	12B_norm_corrected_hist	12B_norm_total_hist	12B_norm_max_hist	12B_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	12D_norm_hist	12D_norm_corrected_hist	12D_norm_total_hist	12D_norm_max_hist	12D_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	12E_norm_hist	12E_norm_corrected_hist	12E_norm_total_hist	12E_norm_max_hist	12E_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	12K_norm_hist	12K_norm_corrected_hist	12K_norm_total_hist	12K_norm_max_hist	12K_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	13A_norm_hist	13A_norm_corrected_hist	13A_norm_total_hist	13A_norm_max_hist	13A_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	13B_norm_hist	13B_norm_corrected_hist	13B_norm_total_hist	13B_norm_max_hist	13B_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	13K_norm_hist	13K_norm_corrected_hist	13K_norm_total_hist	13K_norm_max_hist	13K_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	FCC_norm_hist	FCC_norm_corrected_hist	FCC_norm_total_hist	FCC_norm_max_hist	FCC_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	HCP_norm_hist	HCP_norm_corrected_hist	HCP_norm_total_hist	HCP_norm_max_hist	HCP_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	BCC_9_norm_hist	BCC_9_norm_corrected_hist	BCC_9_norm_total_hist	BCC_9_norm_max_hist	BCC_9_norm_corrected_max_hist");
		fprintf(dyn_decays_file,"	BCC_15_norm_hist	BCC_15_norm_corrected_hist	BCC_15_norm_total_hist	BCC_15_norm_max_hist	BCC_15_norm_corrected_max_hist\n");
		
		for (f=0; f<FRAMES; f++) {
			fprintf(dyn_decays_file,"%d	%.15lg",(f+1),(f+1)*FRAMETSTEP*SAMPLEFREQ/talpha);
			
			if (dyn_msp3!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp3[f],dyn_decay_norm_corrected_hist_sp3[f],dyn_decay_norm_total_hist_sp3[f],dyn_decay_norm_max_hist_sp3[f],dyn_decay_norm_corrected_max_hist_sp3[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp3a!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp3a[f],dyn_decay_norm_corrected_hist_sp3a[f],dyn_decay_norm_total_hist_sp3a[f],dyn_decay_norm_max_hist_sp3a[f],dyn_decay_norm_corrected_max_hist_sp3a[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp3b!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp3b[f],dyn_decay_norm_corrected_hist_sp3b[f],dyn_decay_norm_total_hist_sp3b[f],dyn_decay_norm_max_hist_sp3b[f],dyn_decay_norm_corrected_max_hist_sp3b[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp3c!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp3c[f],dyn_decay_norm_corrected_hist_sp3c[f],dyn_decay_norm_total_hist_sp3c[f],dyn_decay_norm_max_hist_sp3c[f],dyn_decay_norm_corrected_max_hist_sp3c[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp4!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp4[f],dyn_decay_norm_corrected_hist_sp4[f],dyn_decay_norm_total_hist_sp4[f],dyn_decay_norm_max_hist_sp4[f],dyn_decay_norm_corrected_max_hist_sp4[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp4a!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp4a[f],dyn_decay_norm_corrected_hist_sp4a[f],dyn_decay_norm_total_hist_sp4a[f],dyn_decay_norm_max_hist_sp4a[f],dyn_decay_norm_corrected_max_hist_sp4a[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp4b!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp4b[f],dyn_decay_norm_corrected_hist_sp4b[f],dyn_decay_norm_total_hist_sp4b[f],dyn_decay_norm_max_hist_sp4b[f],dyn_decay_norm_corrected_max_hist_sp4b[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m6A!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_6A[f],dyn_decay_norm_corrected_hist_6A[f],dyn_decay_norm_total_hist_6A[f],dyn_decay_norm_max_hist_6A[f],dyn_decay_norm_corrected_max_hist_6A[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m6Z!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_6Z[f],dyn_decay_norm_corrected_hist_6Z[f],dyn_decay_norm_total_hist_6Z[f],dyn_decay_norm_max_hist_6Z[f],dyn_decay_norm_corrected_max_hist_6Z[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp5!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp5[f],dyn_decay_norm_corrected_hist_sp5[f],dyn_decay_norm_total_hist_sp5[f],dyn_decay_norm_max_hist_sp5[f],dyn_decay_norm_corrected_max_hist_sp5[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp5a!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp5a[f],dyn_decay_norm_corrected_hist_sp5a[f],dyn_decay_norm_total_hist_sp5a[f],dyn_decay_norm_max_hist_sp5a[f],dyn_decay_norm_corrected_max_hist_sp5a[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp5b!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp5b[f],dyn_decay_norm_corrected_hist_sp5b[f],dyn_decay_norm_total_hist_sp5b[f],dyn_decay_norm_max_hist_sp5b[f],dyn_decay_norm_corrected_max_hist_sp5b[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_msp5c!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_sp5c[f],dyn_decay_norm_corrected_hist_sp5c[f],dyn_decay_norm_total_hist_sp5c[f],dyn_decay_norm_max_hist_sp5c[f],dyn_decay_norm_corrected_max_hist_sp5c[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m7K!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_7K[f],dyn_decay_norm_corrected_hist_7K[f],dyn_decay_norm_total_hist_7K[f],dyn_decay_norm_max_hist_7K[f],dyn_decay_norm_corrected_max_hist_7K[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m8A!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_8A[f],dyn_decay_norm_corrected_hist_8A[f],dyn_decay_norm_total_hist_8A[f],dyn_decay_norm_max_hist_8A[f],dyn_decay_norm_corrected_max_hist_8A[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m8B!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_8B[f],dyn_decay_norm_corrected_hist_8B[f],dyn_decay_norm_total_hist_8B[f],dyn_decay_norm_max_hist_8B[f],dyn_decay_norm_corrected_max_hist_8B[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m8K!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_8K[f],dyn_decay_norm_corrected_hist_8K[f],dyn_decay_norm_total_hist_8K[f],dyn_decay_norm_max_hist_8K[f],dyn_decay_norm_corrected_max_hist_8K[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m9A!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_9A[f],dyn_decay_norm_corrected_hist_9A[f],dyn_decay_norm_total_hist_9A[f],dyn_decay_norm_max_hist_9A[f],dyn_decay_norm_corrected_max_hist_9A[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m9B!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_9B[f],dyn_decay_norm_corrected_hist_9B[f],dyn_decay_norm_total_hist_9B[f],dyn_decay_norm_max_hist_9B[f],dyn_decay_norm_corrected_max_hist_9B[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m9K!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_9K[f],dyn_decay_norm_corrected_hist_9K[f],dyn_decay_norm_total_hist_9K[f],dyn_decay_norm_max_hist_9K[f],dyn_decay_norm_corrected_max_hist_9K[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m10A!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_10A[f],dyn_decay_norm_corrected_hist_10A[f],dyn_decay_norm_total_hist_10A[f],dyn_decay_norm_max_hist_10A[f],dyn_decay_norm_corrected_max_hist_10A[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m10B!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_10B[f],dyn_decay_norm_corrected_hist_10B[f],dyn_decay_norm_total_hist_10B[f],dyn_decay_norm_max_hist_10B[f],dyn_decay_norm_corrected_max_hist_10B[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m10K!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_10K[f],dyn_decay_norm_corrected_hist_10K[f],dyn_decay_norm_total_hist_10K[f],dyn_decay_norm_max_hist_10K[f],dyn_decay_norm_corrected_max_hist_10K[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m10W!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_10W[f],dyn_decay_norm_corrected_hist_10W[f],dyn_decay_norm_total_hist_10W[f],dyn_decay_norm_max_hist_10W[f],dyn_decay_norm_corrected_max_hist_10W[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m11A!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_11A[f],dyn_decay_norm_corrected_hist_11A[f],dyn_decay_norm_total_hist_11A[f],dyn_decay_norm_max_hist_11A[f],dyn_decay_norm_corrected_max_hist_11A[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m11B!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_11B[f],dyn_decay_norm_corrected_hist_11B[f],dyn_decay_norm_total_hist_11B[f],dyn_decay_norm_max_hist_11B[f],dyn_decay_norm_corrected_max_hist_11B[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m11C!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_11C[f],dyn_decay_norm_corrected_hist_11C[f],dyn_decay_norm_total_hist_11C[f],dyn_decay_norm_max_hist_11C[f],dyn_decay_norm_corrected_max_hist_11C[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m11E!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_11E[f],dyn_decay_norm_corrected_hist_11E[f],dyn_decay_norm_total_hist_11E[f],dyn_decay_norm_max_hist_11E[f],dyn_decay_norm_corrected_max_hist_11E[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m11F!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_11F[f],dyn_decay_norm_corrected_hist_11F[f],dyn_decay_norm_total_hist_11F[f],dyn_decay_norm_max_hist_11F[f],dyn_decay_norm_corrected_max_hist_11F[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m11W!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_11W[f],dyn_decay_norm_corrected_hist_11W[f],dyn_decay_norm_total_hist_11W[f],dyn_decay_norm_max_hist_11W[f],dyn_decay_norm_corrected_max_hist_11W[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m12A!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_12A[f],dyn_decay_norm_corrected_hist_12A[f],dyn_decay_norm_total_hist_12A[f],dyn_decay_norm_max_hist_12A[f],dyn_decay_norm_corrected_max_hist_12A[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m12B!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_12B[f],dyn_decay_norm_corrected_hist_12B[f],dyn_decay_norm_total_hist_12B[f],dyn_decay_norm_max_hist_12B[f],dyn_decay_norm_corrected_max_hist_12B[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m12D!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_12D[f],dyn_decay_norm_corrected_hist_12D[f],dyn_decay_norm_total_hist_12D[f],dyn_decay_norm_max_hist_12D[f],dyn_decay_norm_corrected_max_hist_12D[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m12E!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_12E[f],dyn_decay_norm_corrected_hist_12E[f],dyn_decay_norm_total_hist_12E[f],dyn_decay_norm_max_hist_12E[f],dyn_decay_norm_corrected_max_hist_12E[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m12K!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_12K[f],dyn_decay_norm_corrected_hist_12K[f],dyn_decay_norm_total_hist_12K[f],dyn_decay_norm_max_hist_12K[f],dyn_decay_norm_corrected_max_hist_12K[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m13A!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_13A[f],dyn_decay_norm_corrected_hist_13A[f],dyn_decay_norm_total_hist_13A[f],dyn_decay_norm_max_hist_13A[f],dyn_decay_norm_corrected_max_hist_13A[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m13B!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_13B[f],dyn_decay_norm_corrected_hist_13B[f],dyn_decay_norm_total_hist_13B[f],dyn_decay_norm_max_hist_13B[f],dyn_decay_norm_corrected_max_hist_13B[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_m13K!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_13K[f],dyn_decay_norm_corrected_hist_13K[f],dyn_decay_norm_total_hist_13K[f],dyn_decay_norm_max_hist_13K[f],dyn_decay_norm_corrected_max_hist_13K[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_mFCC!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_FCC[f],dyn_decay_norm_corrected_hist_FCC[f],dyn_decay_norm_total_hist_FCC[f],dyn_decay_norm_max_hist_FCC[f],dyn_decay_norm_corrected_max_hist_FCC[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_mHCP!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_HCP[f],dyn_decay_norm_corrected_hist_HCP[f],dyn_decay_norm_total_hist_HCP[f],dyn_decay_norm_max_hist_HCP[f],dyn_decay_norm_corrected_max_hist_HCP[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_mBCC_9!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg",dyn_decay_norm_hist_BCC_9[f],dyn_decay_norm_corrected_hist_BCC_9[f],dyn_decay_norm_total_hist_BCC_9[f],dyn_decay_norm_max_hist_BCC_9[f],dyn_decay_norm_corrected_max_hist_BCC_9[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0");
			if (dyn_mBCC_15!=-1) fprintf(dyn_decays_file,"	%.15lg	%.15lg	%.15lg	%.15lg	%.15lg\n",dyn_decay_norm_hist_BCC_15[f],dyn_decay_norm_corrected_hist_BCC_15[f],dyn_decay_norm_total_hist_BCC_15[f],dyn_decay_norm_max_hist_BCC_15[f],dyn_decay_norm_corrected_max_hist_BCC_15[f]);
			else fprintf(dyn_decays_file,"	0	0	0	0	0\n");
		}
	
		fclose(dyn_decays_file);
		printf("d%d Written %s\n\n",rank,output);
		
		if (dyn_msp3!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp3",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 3, &dyn_nsp3, dyn_lsp3, dyn_sp3, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp3a!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp3a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 3, &dyn_nsp3a, dyn_lsp3a, dyn_sp3a, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp3b!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp3b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 4, &dyn_nsp3b, dyn_lsp3b, dyn_sp3b, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp3c!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_5A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 5, &dyn_nsp3c, dyn_lsp3c, dyn_sp3c, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp4!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp4",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 4, &dyn_nsp4, dyn_lsp4, dyn_sp4, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp4a!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp4a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 4, &dyn_nsp4a, dyn_lsp4a, dyn_sp4a, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp4b!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp4b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 5, &dyn_nsp4b, dyn_lsp4b, dyn_sp4b, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m6A!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_6A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 6, &dyn_n6A, dyn_l6A, dyn_hc6A, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp5!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp5",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 5, &dyn_nsp5, dyn_lsp5, dyn_sp5, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp5a!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp5a",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 5, &dyn_nsp5a, dyn_lsp5a, dyn_sp5a, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp5b!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_sp5b",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 6, &dyn_nsp5b, dyn_lsp5b, dyn_sp5b, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_msp5c!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_7A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 7, &dyn_nsp5c, dyn_lsp5c, dyn_sp5c, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m6Z!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_6Z",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3c!=-1) Write_dyn(output, 6, &dyn_n6Z, dyn_l6Z, dyn_hc6Z, 1, 2, "5A_i	5A_j", dyn_sub_6Z, 0, -1);
			else Write_dyn(output, 6, &dyn_n6Z, dyn_l6Z, dyn_hc6Z, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m7K!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_7K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3c!=-1) Write_dyn(output, 7, &dyn_n7K, dyn_l7K, dyn_hc7K, 1, 2, "5A_i	5A_j", dyn_sub_7K, 0, -1);
			else Write_dyn(output, 7, &dyn_n7K, dyn_l7K, dyn_hc7K, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m8A!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_8A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5b!=-1 && dyn_msp5c!=-1) Write_dyn(output, 8, &dyn_n8A, dyn_l8A, dyn_hc8A, 1, 12, "sp5b_i_1	sp5b_j_1	sp5c_i_2	sp5c_j_2	sp5b_i_3	sp5c_j_3	sp5b_i_4	sp5b_j_4	sp5c_i_5	sp5c_j_5	sp5b_i_6	sp5c_j_6", dyn_sub_8A, 0, -1);
			else Write_dyn(output, 8, &dyn_n8A, dyn_l8A, dyn_hc8A, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m8B!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_8B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 8, &dyn_n8B, dyn_l8B, dyn_hc8B, 1, 1, "sp5c", dyn_sub_8B, 0, -1);
			else Write_dyn(output, 8, &dyn_n8B, dyn_l8B, dyn_hc8B, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m8K!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_8K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3c!=-1) Write_dyn(output, 8, &dyn_n8K, dyn_l8K, dyn_hc8K, 1, 3, "5A_i	5A_j	5A_k", dyn_sub_8K, 0, -1);
			else Write_dyn(output, 8, &dyn_n8K, dyn_l8K, dyn_hc8K, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m9A!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_9A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp4b!=-1) Write_dyn(output, 9, &dyn_n9A, dyn_l9A, dyn_hc9A, 1, 3, "sp4b_i	sp4b_j	sp4b_k", dyn_sub_9A, 0, -1);
			else Write_dyn(output, 9, &dyn_n9A, dyn_l9A, dyn_hc9A, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m9B!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_9B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 9, &dyn_n9B, dyn_l9B, dyn_hc9B, 1, 2, "7A_i	7A_j", dyn_sub_9B, 1, 8);
			else Write_dyn(output, 9, &dyn_n9B, dyn_l9B, dyn_hc9B, 0, 0, "", dummy_sub, 1, 8);
		}
		if (dyn_m9K!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_9K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m6A!=-1) Write_dyn(output, 9, &dyn_n9K, dyn_l9K, dyn_hc9K, 1, 2, "6A_i	6A_j", dyn_sub_9K, 1, 8);
			else Write_dyn(output, 9, &dyn_n9K, dyn_l9K, dyn_hc9K, 0, 0, "", dummy_sub, 1, 8);
		}
		if (dyn_m10A!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_10A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp4b!=-1) Write_dyn(output, 10, &dyn_n10A, dyn_l10A, dyn_hc10A, 1, 2, "sp4b_i	sp4b_j", dyn_sub_10A, 0 , -1);
			else Write_dyn(output, 10, &dyn_n10A, dyn_l10A, dyn_hc10A, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m10B!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_10B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 10, &dyn_n10B, dyn_l10B, dyn_hc10B, 1, 3, "7A_i	7A_j	7A_k", dyn_sub_10B, 1, 9);
			else Write_dyn(output, 10, &dyn_n10B, dyn_l10B, dyn_hc10B, 0, 0, "", dummy_sub, 1, 9);
		}
		if (dyn_m10K!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_10K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m9K!=-1) Write_dyn(output, 10, &dyn_n10K, dyn_l10K, dyn_hc10K, 1, 1, "9K", dyn_sub_10K, 1, 8);
			else Write_dyn(output, 10, &dyn_n10K, dyn_l10K, dyn_hc10K, 0, 0, "", dummy_sub, 1, 8);
		}
		if (dyn_m10W!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_10W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5b!=-1) Write_dyn(output, 10, &dyn_n10W, dyn_l10W, dyn_hc10W, 1, 6, "sp5b_i	sp5b_j	sp5b_k	sp5b_l	sp5b_m	sp5b_n", dyn_sub_10W, 1, 0);
			else Write_dyn(output, 10, &dyn_n10W, dyn_l10W, dyn_hc10W, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_m11A!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_11A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m6A!=-1) Write_dyn(output, 11, &dyn_n11A, dyn_l11A, dyn_hc11A, 1, 2, "6A_i	6A_j", dyn_sub_11A, 1, 10);
			else Write_dyn(output, 11, &dyn_n11A, dyn_l11A, dyn_hc11A, 0, 0, "", dummy_sub, 1, 10);
		}
		if (dyn_m11B!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_11B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m9B!=-1) Write_dyn(output, 11, &dyn_n11B, dyn_l11B, dyn_hc11B, 1, 1, "9B", dyn_sub_11B, 1, 8);
			else Write_dyn(output, 11, &dyn_n11B, dyn_l11B, dyn_hc11B, 0, 0, "", dummy_sub, 1, 8);
		}
		if (dyn_m11C!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_11CD",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 11, &dyn_n11C, dyn_l11C, dyn_hc11C, 1, 2, "7A_i	7A_j", dyn_sub_11C, 1, 0);
			else Write_dyn(output, 11, &dyn_n11C, dyn_l11C, dyn_hc11C, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_m11E!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_11E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 11, &dyn_n11E, dyn_l11E, dyn_hc11E, 1, 3, "7A_i	7A_j	7A_k", dyn_sub_11E, 0, -1);
			else Write_dyn(output, 11, &dyn_n11E, dyn_l11E, dyn_hc11E, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m11F!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_11F",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3c!=-1 && dyn_m6A!=-1) Write_dyn(output, 11, &dyn_n11F, dyn_l11F, dyn_hc11F, 1, 4, "5A_i	5A_j	6A_i	6A_j", dyn_sub_11F, 0, -1);
			else Write_dyn(output, 11, &dyn_n11F, dyn_l11F, dyn_hc11F, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m11W!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_11W",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m10B!=-1) Write_dyn(output, 11, &dyn_n11W, dyn_l11W, dyn_hc11W, 1, 1, "10B", dyn_sub_11W, 1, 9);
			else Write_dyn(output, 11, &dyn_n11W, dyn_l11W, dyn_hc11W, 0, 0, "", dummy_sub, 1, 9);
		}
		if (dyn_m12A!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_12A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m11C!=-1) Write_dyn(output, 12, &dyn_n12A, dyn_l12A, dyn_hc12A, 1, 1, "11C", dyn_sub_12A, 1, 0);
			else Write_dyn(output, 12, &dyn_n12A, dyn_l12A, dyn_hc12A, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_m12B!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_12BC",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 12, &dyn_n12B, dyn_l12B, dyn_hc12B, 1, 6, "7A_i	7A_j	7A_k	7A_l	7A_m	7A_n", dyn_sub_12B, 1, 0);
			else Write_dyn(output, 12, &dyn_n12B, dyn_l12B, dyn_hc12B, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_m12D!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_12D",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 12, &dyn_n12D, dyn_l12D, dyn_hc12D, 1, 4, "7A_i	7A_j	7A_k	7A_l", dyn_sub_12D, 0, -1);
			else Write_dyn(output, 12, &dyn_n12D, dyn_l12D, dyn_hc12D, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m12E!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_12E",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3c!=-1) Write_dyn(output, 12, &dyn_n12E, dyn_l12E, dyn_hc12E, 1, 3, "5A_i	5A_j	5A_k", dyn_sub_12E, 0, -1);
			else Write_dyn(output, 12, &dyn_n12E, dyn_l12E, dyn_hc12E, 0, 0, "", dummy_sub, 0, -1);
		}
		if (dyn_m12K!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_12K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m11A!=-1) Write_dyn(output, 12, &dyn_n12K, dyn_l12K, dyn_hc12K, 1, 1, "11A", dyn_sub_12K, 1, 10);
			else Write_dyn(output, 12, &dyn_n12K, dyn_l12K, dyn_hc12K, 0, 0, "", dummy_sub, 1, 10);
		}
		if (dyn_m13A!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_13A",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			Write_dyn(output, 13, &dyn_n13A, dyn_l13A, dyn_hc13A, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_m13B!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_13B",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp5c!=-1) Write_dyn(output, 13, &dyn_n13B, dyn_l13B, dyn_hc13B, 1, 2, "7A_i	7A_j", dyn_sub_13B, 1, 0);
			else Write_dyn(output, 13, &dyn_n13B, dyn_l13B, dyn_hc13B, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_m13K!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_13K",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3c!=-1 && dyn_m11F!=-1) Write_dyn(output, 13, &dyn_n13K, dyn_l13K, dyn_hc13K, 1, 3, "11F	5A_i	5A_j", dyn_sub_13K, 1, 0);
			else Write_dyn(output, 13, &dyn_n13K, dyn_l13K, dyn_hc13K, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_mFCC!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_FCC",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3b!=-1 && dyn_msp3c!=-1) Write_dyn(output, 13, &dyn_nFCC, dyn_lFCC, dyn_hcFCC, 1, 5, "sp3b_i	sp3b_j	sp3b_k	sp3b_l	5A_l", dyn_sub_FCC, 1, 0);
			else Write_dyn(output, 13, &dyn_nFCC, dyn_lFCC, dyn_hcFCC, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_mHCP!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_HCP",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp3c!=-1) Write_dyn(output, 13, &dyn_nHCP, dyn_lHCP, dyn_hcHCP, 1, 3, "5A_i	5A_j	5A_k", dyn_sub_HCP, 1, 0);
			else Write_dyn(output, 13, &dyn_nHCP, dyn_lHCP, dyn_hcHCP, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_mBCC_9!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_BCC_9",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_msp4b!=-1 && dyn_m6A!=-1) Write_dyn(output, 9, &dyn_nBCC_9, dyn_lBCC_9, dyn_hcBCC_9, 1, 6, "sp4b_i_1	sp4b_j_1	6A_i_2	6A_j_2	sp4b_i_3	6A_j_3", dyn_sub_BCC_9, 1, 0);
			else Write_dyn(output, 9, &dyn_nBCC_9, dyn_lBCC_9, dyn_hcBCC_9, 0, 0, "", dummy_sub, 1, 0);
		}
		if (dyn_mBCC_15!=-1) {
			sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_BCC_15",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
			if (doSubClusts==1 && dyn_m6A!=-1) Write_dyn(output, 15, &dyn_nBCC_15, dyn_lBCC_15, dyn_hcBCC_15, 1, 6, "6A_i	6A_j	6A_k	6A_l	6A_m	6A_n", dyn_sub_BCC_15, 1, 0);
			else Write_dyn(output, 15, &dyn_nBCC_15, dyn_lBCC_15, dyn_hcBCC_15, 0, 0, "", dummy_sub, 1, 0);
		}
		printf("\n");
		sprintf(output,"%s.rcAA%lg.rcAB%lg.rcBB%lg.Vor%d.fc%lg.PBCs%d.dyn_clust",fXmolName,rcutAA,rcutAB,rcutBB,Vor,fc,PBCs);
		Dyn_Report(output);
		printf("\n");
	}
	
	if (doCoslovich==1) {
		printf("d%d small (0,2,8) %d ",rank,nCos_s_0_2_8);
		tempish=(double)nCos_s_0_2_8/(double)(FRAMES*(N-NA));
		printf("d%d denom %.15lg small (0,2,8) %.15lg\n",rank,(double)(FRAMES*(N-NA)),tempish);
		
		printf("d%d small (1,2,5,3) %d ",rank,nCos_s_1_2_5_3);
		tempish=(double)nCos_s_1_2_5_3/(double)(FRAMES*(N-NA));
		printf("d%d denom %.15lg small (1,2,5,3) %.15lg\n",rank,(double)(FRAMES*(N-NA)),tempish);
		
		printf("d%d small (1,2,5,2) %d ",rank,nCos_s_1_2_5_2);
		tempish=(double)nCos_s_1_2_5_2/(double)(FRAMES*(N-NA));
		printf("d%d denom %.15lg small (1,2,5,2) %.15lg\n",rank,(double)(FRAMES*(N-NA)),tempish);
		
		printf("d%d small (0,3,6) %d ",rank,nCos_s_0_3_6);
		tempish=(double)nCos_s_0_3_6/(double)(FRAMES*(N-NA));
		printf("d%d denom %.15lg small (0,3,6) %.15lg\n",rank,(double)(FRAMES*(N-NA)),tempish);
		
		printf("d%d big (0,2,8,4) %d ",rank,nCos_b_0_2_8_4);
		tempish=(double)nCos_b_0_2_8_4/(double)(FRAMES*(NA));
		printf("d%d denom %.15lg big (0,2,8,4) %.15lg\n",rank,(double)(FRAMES*(NA)),tempish);
		
		printf("d%d big (0,2,8,5) %d ",rank,nCos_b_0_2_8_5);
		tempish=(double)nCos_b_0_2_8_5/(double)(FRAMES*(NA));
		printf("d%d denom %.15lg big (0,2,8,5) %.15lg\n",rank,(double)(FRAMES*(NA)),tempish);
		
		printf("d%d big (0,3,6,6) %d ",rank,nCos_b_0_3_6_6);
		tempish=(double)nCos_b_0_3_6_6/(double)(FRAMES*(NA));
		printf("d%d denom %.15lg big (0,3,6,6) %.15lg\n",rank,(double)(FRAMES*(NA)),tempish);
		
		printf("d%d big (0,1,10,4) %d ",rank,nCos_b_0_1_10_4);
		tempish=(double)nCos_b_0_1_10_4/(double)(FRAMES*(NA));
		printf("d%d denom %.15lg big (0,1,10,4) %.15lg\n",rank,(double)(FRAMES*(NA)),tempish);
	}
	
	// MPI_Barrier(MPI_COMM_WORLD);
	
	Setup_FreeStaticVars();
	Stats_FreeMem();
	if (doPotential==1) Setup_FreePotentialVars();
	if (doDynamics==1) {
		Setup_FreeDynamicVars();
		Dyn_Analyse_Free();
	}
	
	printf("\n\nd%d FIN \n\n",rank);
	
	// MPI_Barrier(MPI_COMM_WORLD);
	printf("\n\nd%d after barrier \n\n",rank);
	// MPI_Finalize();
	printf("\n\nd%d after finalize \n\n",rank);
	return 0;
}
