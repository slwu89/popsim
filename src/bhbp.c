/* --------------------------------------------------------------------------------
#
#   Bellman-Harris Branching Process
#   from Simon Frost: https://gist.github.com/sdwfrost/b3d0c4cbff7ff460562162affceffb17
#   in turn from: https://github.com/jriou/wcov
#   Sean Wu (slwu89@berkeley.edu)
#   January 2020
#
-------------------------------------------------------------------------------- */

#include "bhbp.h"
#include "slist.h"

// BHHP
SEXP bhbp_C(SEXP R0r, SEXP kr, SEXP shaper, SEXP scaler, SEXP index_casesr, SEXP max_casesr,SEXP max_timer){

  GetRNGstate();

  int max_cases = Rf_asInteger(max_casesr);
  double max_time = Rf_asReal(max_timer);

  double shape = Rf_asReal(shaper);
  double scale = Rf_asReal(scaler);

  double R0 = Rf_asReal(R0r);
  double k = Rf_asReal(kr);

  int index_cases = Rf_asInteger(index_casesr);
  int cases = index_cases;

  // times,t
  double* t = (double*)malloc(index_cases*sizeof(double));

  double_slist times;
  init_double_slist(&times);
  for(int j=0; j<index_cases; j++){
    add_double_slist(&times,0);
    t[j] = 0;
  }

  int* secondary = NULL;
  double* t_new = NULL;

  // simulation loop
  while((cases > 0) && (times.size < max_cases)){

    // sample num of secondary cases each primary case produces
    secondary = (int*)realloc(secondary,sizeof(int)*cases);
    int secondary_n = 0;
    for(int j=0; j<cases; j++){
      secondary[j] = (int)rnbinom_mu(k,R0);
      secondary_n += secondary[j];
    }

    // sample their times of appearance
    t_new = (double*)realloc(t_new,secondary_n*sizeof(double));
    int t_iter = 0;
    for(int j=0; j<cases; j++){
      // if this case had secondaries sample their branching times
      if(secondary[j] > 0){
        for(int k=0; k<secondary[j]; k++){
          t_new[t_iter] = t[j] + (double)rgamma(shape,scale);
          t_iter++;
        }
      }
    }

    // check if any have overrun max_time
    int overrun_n = 0;
    for(int j=0; j<secondary_n; j++){
      if(t_new[j] >= max_time){
        overrun_n++;
      }
    }

    // updating logic accounting for overruns
    if(overrun_n > 0){

      cases = secondary_n-overrun_n;
      t = (double*)realloc(t,sizeof(double)*(secondary_n-overrun_n));
      int t_iter = 0;
      for(int k=0; k<cases; k++){
        if(t_new[k] < max_time){
          t[t_iter] = t_new[k];
          t_iter++;
          add_double_slist(&times,t_new[k]);
        }
      }

    } else {

      cases = secondary_n;
      t = (double*)realloc(t,sizeof(double)*cases);
      for(int k=0; k<cases; k++){
        t[k] = t_new[k];
        add_double_slist(&times,t_new[k]);
      }

    }

  }

  // prepare output to R
  SEXP times_out = PROTECT(Rf_allocVector(REALSXP,times.size));
  double* times_out_ptr = REAL(times_out);
  int i = 0;
  double_node* t_node = times.head;
  while(t_node != NULL){
    times_out_ptr[i] = fmax(0.0,t_node->value); // sometimes this is < 0, not sure why
    i++;
    t_node = t_node->next;
  }

  R_rsort(times_out_ptr,times.size);

  free_double_slist(&times);

  free(secondary);
  free(t_new);
  free(t);

  PutRNGstate();
  UNPROTECT(1);
  return times_out;
};
