#!/bin/bash

iterations=5
cpp1="CircleAlgorithm.cpp"
cpp2="CircleAlgorithmParallel.cpp"
exe1="CircleSequential"
exe2="CircleParallel"
input_file="./inputs/Input11.txt"
build_dir="./builds"

# Global variable to hold the average runtime
avg_time_ns=0

# Handle CLI overrides
while [[ "$#" -gt 0 ]]; do
  case $1 in
    --cpp1) cpp1="$2"; shift ;;
    --cpp2) cpp2="$2"; shift ;;
    --exe1) exe1="$2"; shift ;;
    --exe2) exe2="$2"; shift ;;
    --input) input_file="$2"; shift ;;
    *) echo "Unknown parameter passed: $1"; exit 1 ;;
  esac
  shift
done

mkdir -p "$build_dir"

# Use gdate for nanosecond timing on macOS
if command -v gdate &> /dev/null; then
  date_cmd="gdate"
else
  date_cmd="date"
fi

# Compile the programs
g++ -O2 -pthread -o "$build_dir/$exe1" "$cpp1" || { echo "Compilation of $cpp1 failed."; exit 1; }
g++ -O2 -pthread -o "$build_dir/$exe2" "$cpp2" || { echo "Compilation of $cpp2 failed."; exit 1; }

benchmark_program() {
  local exe=$1
  local label=$2
  local total_time_ns=0

  echo "Benchmarking $label for $iterations runs (nanoseconds)..."
  for ((i=1; i<=iterations; i++)); do
    start_time=$($date_cmd +%s%N | sed 's/[^0-9]//g')
    "$build_dir/$exe" < "$input_file" > /dev/null 2>&1
    end_time=$($date_cmd +%s%N | sed 's/[^0-9]//g')
    elapsed_ns=$((end_time - start_time))
    elapsed_sec=$(echo "scale=9; $elapsed_ns / 1000000000" | bc)
    
    echo "Run $i: ${elapsed_ns} ns (${elapsed_sec} s)"
    total_time_ns=$((total_time_ns + elapsed_ns))
  done

  avg_time_ns=$((total_time_ns / iterations))
  avg_time_sec=$(echo "scale=9; $avg_time_ns / 1000000000" | bc)
  echo "Average time for $label: $avg_time_ns ns (${avg_time_sec} s)"
  echo
}

# Run both benchmarks
benchmark_program "$exe1" "$exe1"
avg1=$avg_time_ns

benchmark_program "$exe2" "$exe2"
avg2=$avg_time_ns

# Compare performance
if [ "$avg1" -gt "$avg2" ]; then
  time_saved=$((avg1 - avg2))
  time_saved_sec=$(echo "scale=9; $time_saved / 1000000000" | bc)
  percent_saved=$(echo "scale=2; 100 * $time_saved / $avg1" | bc)
  echo "✅ $exe2 is faster than $exe1 by $time_saved ns (${time_saved_sec} s) ($percent_saved% time saved)"
elif [ "$avg2" -gt "$avg1" ]; then
  time_saved=$((avg2 - avg1))
  time_saved_sec=$(echo "scale=9; $time_saved / 1000000000" | bc)
  percent_saved=$(echo "scale=2; 100 * $time_saved / $avg2" | bc)
  echo "❌ $exe1 is faster than $exe2 by $time_saved ns (${time_saved_sec} s) ($percent_saved% time saved)"
else
  echo "⚖️  Both programs have the same average runtime."
fi
