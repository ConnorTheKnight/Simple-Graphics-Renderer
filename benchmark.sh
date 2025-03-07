#!/bin/bash

# Number of benchmark runs for each program.
iterations=5

# Default names of your source files and executables.
cpp1="CircleAlgorithm.cpp"
cpp2="CircleAlgorithmParallel.cpp"
exe1="CircleSequential"
exe2="CircleParallel"
input_file="SampeInput100Shapes.txt"
build_dir="./builds"

# Parse command-line flags to override defaults.
while [[ "$#" -gt 0 ]]; do
  case $1 in
    --cpp1) cpp1="$2"; shift ;;
    --cpp2) cpp2="$2"; shift ;;
    --exe1) exe1="$2"; shift ;;
    --exe2) exe2="$2"; shift ;;
    *) echo "Unknown parameter passed: $1"; exit 1 ;;
  esac
  shift
done

# Create the builds directory if it doesn't exist.
mkdir -p "$build_dir"

# Compile the first C++ file.
g++ -O2 -o "$build_dir/$exe1" "$cpp1"
if [ $? -ne 0 ]; then
  echo "Compilation of $cpp1 failed."
  exit 1
fi

# Compile the second C++ file.
g++ -O2 -o "$build_dir/$exe2" "$cpp2"
if [ $? -ne 0 ]; then
  echo "Compilation of $cpp2 failed."
  exit 1
fi

echo "Benchmarking $exe1 for $iterations runs (nanoseconds)..."
total_time1=0
for ((i=1; i<=iterations; i++)); do
    start=$(date +%s%N)
    "$build_dir/$exe1" < "$input_file" > /dev/null 2>&1
    end=$(date +%s%N)
    elapsed=$(( end - start ))
    echo "Run $i: $elapsed ns"
    total_time1=$(( total_time1 + elapsed ))
done
avg_time1=$(( total_time1 / iterations ))
echo "Average time for $exe1: $avg_time1 ns"

echo "Benchmarking $exe2 for $iterations runs (nanoseconds)..."
total_time2=0
for ((i=1; i<=iterations; i++)); do
    start=$(date +%s%N)
    "$build_dir/$exe2" < "$input_file" > /dev/null 2>&1
    end=$(date +%s%N)
    elapsed=$(( end - start ))
    echo "Run $i: $elapsed ns"
    total_time2=$(( total_time2 + elapsed ))
done
avg_time2=$(( total_time2 / iterations ))
echo "Average time for $exe2: $avg_time2 ns"
