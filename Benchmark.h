#pragma once

#include <string>
#include <string_view>

struct BenchmarkConfig {
    int rows = 500;
    int cols = 500;

    // Initial steps excluded from final statistics.
    // They reduce cache/allocation/startup noise in the measured interval.
    int warmupSteps = 100;

    // Number of simulation steps included in benchmark statistics.
    int measuredSteps = 1000;

    // Initial percentage of alive cells.
    int initialAlivePercent = 50;

    // Fixed seed makes benchmark input reproducible.
    unsigned int seed = 12345;

    // OpenMP thread count. 0 means: use OpenMP runtime default.
    int threads = 0;
};

struct BenchmarkResult {
    std::string_view modeName = "unknown";

    int rows = 0;
    int cols = 0;
    int totalCells = 0;

    int warmupSteps = 0;
    int measuredSteps = 0;
    int threads = 1;

    unsigned int initialAliveCells = 0;
    unsigned int finalAliveCells = 0;

    double totalMs = 0.0;
    double avgStepMs = 0.0;
    double minStepMs = 0.0;
    double maxStepMs = 0.0;

    double stepsPerSecond = 0.0;
    double cellsPerSecond = 0.0;
};

BenchmarkResult RunSequentialBenchmark(const BenchmarkConfig& config);
BenchmarkResult RunOpenMpBenchmark(const BenchmarkConfig& config);

void PrintBenchmarkResult(const BenchmarkResult& result);

// Builds a CSV filename from the result: benchmark_[mode]_[rows]x[cols].csv
// placed in cfg::BENCHMARK_OUTPUT_DIR.
std::string BuildCsvPath(const BenchmarkResult& result);

// Writes result to a new CSV file (never appends to an existing file).
// The file path is built automatically via BuildCsvPath.
void AppendBenchmarkCsv(const BenchmarkResult& result);