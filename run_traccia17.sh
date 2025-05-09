#!/bin/bash

mkdir -p results

for flows in {1..9}; do
  for run in {1..5}; do  # 5 run diversi
    echo "Running DSDV - Flows: $flows - Run: $run"
    ./ns3 run "scratch/quesito1 --useAodv=false --flowsNumber=$flows --runNumber=$run" > results/dsdv_flows_${flows}_run${run}.txt 2>&1

    echo "Running AODV - Flows: $flows - Run: $run"
    ./ns3 run "scratch/quesito1 --useAodv=true --flowsNumber=$flows --runNumber=$run" > results/aodv_flows_${flows}_run${run}.txt 2>&1
  done
done

echo "Simulazioni completate!"
