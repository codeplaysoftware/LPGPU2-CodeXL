#ifndef INCLUDE_Throttle_hpp
#define INCLUDE_Throttle_hpp
/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */



#include <vector>
#include <iostream>
#include <cinttypes>

#include <sys/time.h>
#include <unistd.h>



class Throttle {
 std::vector<double> dat; // time taken for each frame

 double tgt; // ideal total time for the block = tgt tgt * number of frames

 int ind; // index for head of the cyclic buffer

 struct timeval epoch; // initial timestamp

 double toc; // previous time value

public:
 Throttle(double s, int n=11):dat(n),tgt(1/s),ind(0) {gettimeofday(&epoch,0); toc=now()-tgt; for(double&t:dat) t=tgt;}

 // Number of seconds since start
 double now() const {struct timeval tv; gettimeofday(&tv,0); return (tv.tv_sec-epoch.tv_sec)+tv.tv_usec*1.e-6;}

 // Average frame time
 double average() const {double s=0; for(double t:dat) s+=t; return s/size();}

 // Number of frames used for average
 size_t size() const {return dat.size();}

 // Target frame time
 double target() const {return tgt;}

 // Frames per second
 double fps() const {return 1/average();}

 // Sleep for a period calculated to bring the app to target frame time
 void wait(int n=1) {
  double tic=now(); dat[ind]=tic-toc;

  double delay = tgt-average();

  toc=tic+delay; ind++; if (ind==size()) ind=0;

  if (delay>0) {delay *= 1.e6/n; while(n--) usleep(delay);}
 }
}; // Throttle



#endif
