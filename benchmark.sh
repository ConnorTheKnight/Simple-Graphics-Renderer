#!/bin/bash

# Number of benchmark runs for each program.
iterations=5

# Names of your source files and executables.
cpp1="CircleAlgorithm.cpp"
cpp2="CircleAlgorithmParallel.cpp"
exe1="prog1"
exe2="prog2"
input_file="SampeInput100Shapes.txt"

# Compile the first C++ file.
g++ -O2 -o "$exe1" "$cpp1"
if [ $? -ne 0 ]; then
  echo "Compilation of $cpp1 failed."
  exit 1
fi

# Compile the second C++ file.
g++ -O2 -o "$exe2" "$cpp2"
if [ $? -ne 0 ]; then
  echo "Compilation of $cpp2 failed."
  exit 1
fi

echo "Benchmarking $exe1 for $iterations runs..."
total_time1=0
for ((i=1; i<=iterations; i++)); do
    # Run the program with input redirection and capture its execution time.
    run_time=$( { /usr/bin/time -f "%e" ./"$exe1" < "$input_file"; } 2>&1 )
    echo "Run $i: $run_time seconds"
    total_time1=$(echo "$total_time1 + $run_time" | bc)
done
avg_time1=$(echo "scale=3; $total_time1 / $iterations" | bc)
echo "Average time for $exe1: $avg_time1 seconds"

echo "Benchmarking $exe2 for $iterations runs..."
total_time2=0
for ((i=1; i<=iterations; i++)); do
    run_time=$( { /usr/bin/time -f "%e" ./"$exe2" < "$input_file"; } 2>&1 )
    echo "Run $i: $run_time seconds"
    total_time2=$(echo "$total_time2 + $run_time" | bc)
done
avg_time2=$(echo "scale=3; $total_time2 / $iterations" | bc)
echo "Average time for $exe2: $avg_time2 seconds"
