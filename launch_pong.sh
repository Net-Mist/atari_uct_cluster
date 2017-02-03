#!/bin/bash

for i in `seq 36 800`;
do
    echo $i

    cd /hpctmp2/e0046667
    mkdir output$i
    cd output$i
    mkdir frames dataset
    cd dataset
    mkdir 0 3 4
    cd
    cd atari_uct_cluster/code/build
    bsub -q serial -o outfile -e errorfile ./atari_uct -save_path=/hpctmp2/e0046667/output$i/
    
    sleep 2
done

