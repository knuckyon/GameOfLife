#include "Benchmark.h"

#include "AppConfig.h"
#include "GameLogic.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {
    using Clock = std::chrono::steady_clock;
    using StepFunction = void (*)(GameState&);

    double ToMilliseconds(const Clock::duration& duration) {
        return std::chrono::duration<double, std::milli>(duration).count();
    }

    int ClampPercent(int value) {
        return std::max(0, std::min(value, 100));
    }

    void InitializeBenchmarkGame(GameState& game, const BenchmarkConfig& config) {
        game.rows = config.rows;
        game.cols = config.cols;
        game.pendingRows = config.rows;
        game.pendingCols = config.cols;
        game.initialAlivePercent = ClampPercent(config.initialAlivePercent);

        const int totalCells = game.rows * game.cols;
        game.cells.assign(static_cast<std::size_t>(totalCells), 0);
        game.nextCells.assign(static_cast<std::size_t>(totalCells), 0);

        game.generation = 0;
        game.isPaused = false;
        game.aliveCells = 0;

        std::mt19937 rng(config.seed);
        std::uniform_int_distribution<int> dist(1, 100);

        for (int i = 0; i < totalCells; ++i) {
            const std::uint8_t value = dist(rng) <= game.initialAlivePercent ? 1 : 0;
            game.cells[static_cast<std::size_t>(i)] = value;
            game.aliveCells += value != 0 ? 1 : 0;
        }
    }

    int GetOpenMpThreadCount(const BenchmarkConfig& config) {
#ifdef _OPENMP
        if (config.threads > 0) {
            return config.threads;
        }
        return omp_get_max_threads();
#else
        (void)config;
        return 1;
#endif
    }

    void ConfigureOpenMp(const BenchmarkConfig& config) {
#ifdef _OPENMP
        if (config.threads > 0) {
            omp_set_num_threads(config.threads);

            std::printf("OpenMP enabled\n");
            std::printf("Requested threads: %d\n", config.threads);
            std::printf("Max OpenMP threads: %d\n", omp_get_max_threads());
        }
        else {
            std::printf("OpenMP enabled\n");
            std::printf("Using default OpenMP threads: %d\n", omp_get_max_threads());
        }
#else
        std::printf("OpenMP is NOT enabled. Build target was compiled without /openmp.\n");
        (void)config;
#endif
    }

    BenchmarkResult RunBenchmark(
        const BenchmarkConfig& config,
        const char* modeName,
        StepFunction stepFunction,
        int threadCount
    ) {
        GameState game;
        InitializeBenchmarkGame(game, config);

        BenchmarkResult result;
        result.modeName = modeName;
        result.rows = config.rows;
        result.cols = config.cols;
        result.totalCells = config.rows * config.cols;
        result.warmupSteps = config.warmupSteps;
        result.measuredSteps = config.measuredSteps;
        result.threads = threadCount;
        result.initialAliveCells = static_cast<unsigned int>(game.aliveCells);

        for (int i = 0; i < config.warmupSteps; ++i) {
            stepFunction(game);
        }

        std::vector<double> stepTimes;
        stepTimes.reserve(static_cast<std::size_t>(config.measuredSteps));

        const auto totalStart = Clock::now();

        for (int i = 0; i < config.measuredSteps; ++i) {
            const auto stepStart = Clock::now();
            stepFunction(game);
            const auto stepEnd = Clock::now();

            stepTimes.push_back(ToMilliseconds(stepEnd - stepStart));
        }

        const auto totalEnd = Clock::now();

        result.finalAliveCells = static_cast<unsigned int>(game.aliveCells);
        result.totalMs = ToMilliseconds(totalEnd - totalStart);

        if (config.measuredSteps > 0 && !stepTimes.empty()) {
            result.avgStepMs = result.totalMs / static_cast<double>(config.measuredSteps);
            result.minStepMs = *std::min_element(stepTimes.begin(), stepTimes.end());
            result.maxStepMs = *std::max_element(stepTimes.begin(), stepTimes.end());
            result.stepsPerSecond = 1000.0 / result.avgStepMs;
            result.cellsPerSecond = static_cast<double>(result.totalCells) * result.stepsPerSecond;
        }

        return result;
    }
}

BenchmarkResult RunSequentialBenchmark(const BenchmarkConfig& config) {
    return RunBenchmark(config, "seq", StepSimulation, 1);
}

BenchmarkResult RunOpenMpBenchmark(const BenchmarkConfig& config) {
    ConfigureOpenMp(config);
    return RunBenchmark(config, "omp", StepSimulationOpenMP, GetOpenMpThreadCount(config));
}

void PrintBenchmarkResult(const BenchmarkResult& result) {
    std::printf("=== Game of Life benchmark: %s ===\n", result.modeName.data());
    std::printf("------------- Simulation params ------------\n");
    std::printf("Field:          %d x %d\n", result.rows, result.cols);
    std::printf("Total cells:    %d\n", result.totalCells);
    std::printf("Initial alive:  %u\n", result.initialAliveCells);
    std::printf("Final alive:    %u\n", result.finalAliveCells);
    std::printf("Warmup steps:   %d\n", result.warmupSteps);
    std::printf("Measured steps: %d\n", result.measuredSteps);
    std::printf("Threads:        %d\n", result.threads);
    std::printf("---------------- Time results ---------------\n");
    std::printf("Total time:     %.3f ms\n", result.totalMs);
    std::printf("Avg step time:  %.6f ms\n", result.avgStepMs);
    std::printf("Min step time:  %.6f ms\n", result.minStepMs);
    std::printf("Max step time:  %.6f ms\n", result.maxStepMs);
    std::printf("Steps/sec:      %.3f\n", result.stepsPerSecond);
    std::printf("Cells/sec:      %.3f\n", result.cellsPerSecond);
}

namespace {
    // fopen / fopen_s wrapper — fopen_s is required on MSVC to avoid C4996.
    std::FILE* OpenFile(const char* path, const char* mode) {
#ifdef _MSC_VER
        std::FILE* f = nullptr;
        fopen_s(&f, path, mode);
        return f;
#else
        return std::fopen(path, mode);
#endif
    }
}

std::string BuildCsvPath(const BenchmarkResult& result) {
    // Format: benchmark_seq_500x500.csv  or  benchmark_omp_500x500.csv
    // Placed in cfg::BENCHMARK_OUTPUT_DIR ("." by default).
    std::string path = cfg::BENCHMARK_OUTPUT_DIR;

    if (!path.empty() && path.back() != '/' && path.back() != '\\') {
        path += '/';
    }

    path += "benchmark_";
    path += result.modeName.data();   // "seq" / "omp"
    path += '_';
    path += std::to_string(result.rows);
    path += 'x';
    path += std::to_string(result.cols);
    path += ".csv";

    return path;
}

void AppendBenchmarkCsv(const BenchmarkResult& result) {
    const std::string path = BuildCsvPath(result);

    // Always create a fresh file — mode "w" truncates if it already exists.
    std::FILE* f = OpenFile(path.c_str(), "w");
    if (!f) {
        std::printf("AppendBenchmarkCsv: cannot open '%s' for writing\n", path.c_str());
        return;
    }

    // Header row
    std::fprintf(f,
        "mode,rows,cols,total_cells,threads,"
        "warmup_steps,measured_steps,"
        "initial_alive,final_alive,"
        "total_ms,avg_step_ms,min_step_ms,max_step_ms,"
        "steps_per_sec,cells_per_sec\n"
    );

    // Data row
    std::fprintf(f,
        "%s,%d,%d,%d,%d,"
        "%d,%d,"
        "%u,%u,"
        "%.6f,%.6f,%.6f,%.6f,"
        "%.3f,%.3f\n",
        result.modeName.data(),
        result.rows, result.cols, result.totalCells, result.threads,
        result.warmupSteps, result.measuredSteps,
        result.initialAliveCells, result.finalAliveCells,
        result.totalMs, result.avgStepMs, result.minStepMs, result.maxStepMs,
        result.stepsPerSecond, result.cellsPerSecond
    );

    std::fclose(f);
    std::printf("Benchmark results saved to: %s\n", path.c_str());
}