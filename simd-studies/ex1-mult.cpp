//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-18, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For details about use and distribution, please read RAJA/LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "RAJA/RAJA.hpp"
#include "RAJA/util/Timer.hpp"
/*
 *  Simd test 1 - adds vectors
 */

//#define ADD_ALIGN_HINT  


#if defined(ADD_ALIGN_HINT)

#define MULT_BODY \
  y[i] = alpha*x[i] ;
#else
#define MULT_BODY \
  b[i] = alpha*a[i];
#endif


using realType = double;
using TFloat = realType * const RAJA_RESTRICT;

void mult_noVec(TFloat a, TFloat b, const double alpha, RAJA::Index_type N) 
{  

#if defined(ADD_ALIGN_HINT)
  realType *x = RAJA::align_hint(a);
  realType *y = RAJA::align_hint(b);
#endif

  RAJA_NO_SIMD
  for(RAJA::Index_type i=0; i<N; ++i){
    MULT_BODY
  }
}

void mult_native(TFloat a, TFloat b, const double alpha, RAJA::Index_type N) 
{  

#if defined(ADD_ALIGN_HINT)
  realType *x = RAJA::align_hint(a);
  realType *y = RAJA::align_hint(b);
#endif

  for(RAJA::Index_type i=0; i<N; ++i){
    MULT_BODY;
  }

}

void mult_simd(TFloat a, TFloat b, const double alpha, RAJA::Index_type N) 
{  

#if defined(ADD_ALIGN_HINT)
  realType *x = RAJA::align_hint(a);
  realType *y = RAJA::align_hint(b);
#endif
  RAJA_SIMD
  for(RAJA::Index_type i=0; i<N; ++i){
    MULT_BODY;
  }
  
}

template<typename POL>
void mult_RAJA(TFloat a, TFloat b, const double alpha, RAJA::Index_type N)
{

#if defined(ADD_ALIGN_HINT)
  realType *x = RAJA::align_hint(a);
  realType *y = RAJA::align_hint(b);
#endif

  RAJA::forall<POL>(RAJA::RangeSegment(0, N), [=] (RAJA::Index_type i) {
      MULT_BODY;
    });

}


template<typename T>
void checkResult(T res, RAJA::Index_type len);

int main(int argc, char *argv[])
{

  if(argc !=2 ){
  exit(-1);
  }

//
// Define vector length
//
  RAJA::Timer::ElapsedType runTime; 
  const RAJA::Index_type N = atoi(argv[1]);
  //const RAJA::Index_type N = 2048;  // RAJA code runs slower

#if defined(ADD_ALIGN_HINT)
  std::cout << "\n\nRAJA vector addition benchmark with alignment hint...\n";
#else
  std::cout << "\n\nRAJA vector addition benchmark...\n";
#endif
  std::cout<<"No of entries "<<N<<"\n\n"<<std::endl;
  
  auto timer = RAJA::Timer();
  const RAJA::Index_type Niter = 1000000;

  const double alpha = 10;
  TFloat a = RAJA::allocate_aligned_type<realType>(RAJA::DATA_ALIGN, N*sizeof(realType));
  TFloat b = RAJA::allocate_aligned_type<realType>(RAJA::DATA_ALIGN, N*sizeof(realType));
  
  //Intialize data 
  for(RAJA::Index_type i=0; i<N; ++i)
    {
      a[i] = 1./10.;
    }

  //---------------------------------------------------------
  std::cout<<"Native C - strictly sequential"<<std::endl;
  //---------------------------------------------------------
  std::memset(b, 0, N*sizeof(realType));
  timer.start();
  for(RAJA::Index_type it = 0; it < Niter; ++it){

    mult_noVec(a, b, alpha, N);
    
  }
  timer.stop();
  runTime = timer.elapsed();
  timer.reset();
  std::cout<< "\trun time : " << runTime << " seconds" << std::endl;
  checkResult(b, N);


  //---------------------------------------------------------
  std::cout<<"Native C - raw loop"<<std::endl;
  //---------------------------------------------------------
  std::memset(b, 0, N*sizeof(realType));
  timer.start();
  for(RAJA::Index_type it = 0; it < Niter; ++it){

    mult_native(a, b, alpha, N);
    
  }
  timer.stop();
  runTime = timer.elapsed();
  timer.reset();
  std::cout<< "\trun time : " << runTime << " seconds" << std::endl;
  checkResult(b, N);

  //---------------------------------------------------------
  std::cout<<"Native C - with vectorization hint"<<std::endl;
  //---------------------------------------------------------
  std::memset(b, 0, N*sizeof(realType));
  timer.start();
  for(RAJA::Index_type it = 0; it < Niter; ++it){

    mult_simd(a, b, alpha, N);
    
  }
  timer.stop();
  runTime = timer.elapsed();
  timer.reset();
  std::cout<< "\trun time : " << runTime << " seconds" << std::endl;
  checkResult(b, N);

  //---------------------------------------------------------
  std::cout<<"RAJA - strictly sequential"<<std::endl;
  //---------------------------------------------------------
  std::memset(b, 0, N*sizeof(realType));
  timer.start();
  for(RAJA::Index_type it = 0; it < Niter; ++it){

  mult_RAJA<RAJA::seq_exec>(a, b, alpha, N);
    
  }
  timer.stop();
  runTime = timer.elapsed();
  timer.reset();
  std::cout<< "\trun time : " << runTime << " seconds" << std::endl;
  checkResult(b, N);

  //---------------------------------------------------------
  std::cout<<"RAJA - raw loop"<<std::endl;
  //---------------------------------------------------------
  std::memset(b, 0, N*sizeof(realType));
  timer.start();
  for(RAJA::Index_type it = 0; it < Niter; ++it){

  mult_RAJA<RAJA::loop_exec>(a, b, alpha, N);
    
  }
  timer.stop();
  runTime = timer.elapsed();
  timer.reset();
  std::cout<< "\trun time : " << runTime << " seconds" << std::endl;
  checkResult(b, N);

  //---------------------------------------------------------
  std::cout<<"RAJA - with vectorization hint"<<std::endl;
  //---------------------------------------------------------
  std::memset(b, 0, N*sizeof(realType));
  timer.start();
  for(RAJA::Index_type it = 0; it < Niter; ++it){

  mult_RAJA<RAJA::simd_exec>(a, b, alpha, N);
    
  }
  timer.stop();
  runTime = timer.elapsed();
  timer.reset();
  std::cout<< "\trun time : " << runTime << " seconds" << std::endl;
  checkResult(b, N);
  //---------------------------------------------------------

  
  std::cout << "\n DONE!...\n";
  return 0;
}

//
// Function to check result and report P/F.
//
template<typename T>
void checkResult(T res, RAJA::Index_type len) 
{
  bool correct = true;
  for (RAJA::Index_type i = 0; i < len; i++) {
    if ( std::abs(res[i] - 1.0) > 1e-9 ) { correct = false; }
  }
  if ( correct ) {
    std::cout << "\t result -- PASS\n\n";
  } else {
    std::cout << "\t result -- FAIL\n\n";
  }
}