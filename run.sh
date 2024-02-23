#!/bin/bash

 ./src/build/c_phase2 &
    sleep 1
  ./src/build/p0_phase2 &
    sleep 3
 ./src/build/p1_phase2 &
    sleep 3
 ./src/build/p2_phase2 &
    wait