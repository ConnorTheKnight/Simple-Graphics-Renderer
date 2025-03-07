circle-benchmark:
  ./benchmark.sh --cpp1 CircleAlgorithm.cpp --cpp2 CircleAlgorithm.cpp --exe1 CircleSequential --exe2 CircleParallel

square-benchmark:
  ./benchmark.sh --cpp1 SquareAlgorithm.cpp --cpp2 SquareAlgorithmParallel.cpp --exe1 SquareSequential --exe2 SquareParallel
