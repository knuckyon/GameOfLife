#include "raylib.h"

#include "AppConfig.h"
#include "Benchmark.h"
#include "CameraController.h"
#include "GameLogic.h"
#include "InputController.h"
#include "Renderer.h"
#include "UI.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {
    enum class RunMode {
        Demo,
        BenchmarkSequential,
        BenchmarkOpenMP
    };

    struct AppLaunchConfig {
        RunMode mode = RunMode::Demo;
        BenchmarkConfig benchmark;
    };

    bool IsArg(const char* value, const char* expected) {
        return std::strcmp(value, expected) == 0;
    }

    int ParseIntArg(const char* value, int fallback) {
        char* end = nullptr;
        const long parsed = std::strtol(value, &end, 10);

        if (end == value || *end != '\0') {
            return fallback;
        }

        return static_cast<int>(parsed);
    }

    unsigned int ParseUIntArg(const char* value, unsigned int fallback) {
        char* end = nullptr;
        const unsigned long parsed = std::strtoul(value, &end, 10);

        if (end == value || *end != '\0') {
            return fallback;
        }

        return static_cast<unsigned int>(parsed);
    }

    void PrintUsage(const char* exeName) {
        std::printf("Usage:\n");
        std::printf("  %s\n", exeName);
        std::printf("  %s --benchmark seq [options]\n", exeName);
        std::printf("  %s --benchmark omp [options]\n", exeName);
        std::printf("\nOptions:\n");
        std::printf("  --rows N       Field rows. Default: 500\n");
        std::printf("  --cols N       Field columns. Default: 500\n");
        std::printf("  --warmup N     Warmup steps. Default: 100\n");
        std::printf("  --steps N      Measured steps. Default: 1000\n");
        std::printf("  --alive N      Initial alive percent [0..100]. Default: 50\n");
        std::printf("  --seed N       Random seed. Default: 12345\n");
        std::printf("  --threads N    OpenMP threads. 0 = runtime default. Default: 0\n");
    }

    AppLaunchConfig ParseCommandLine(int argc, char** argv) {
        AppLaunchConfig config;

        for (int i = 1; i < argc; ++i) {
            if (IsArg(argv[i], "--help") || IsArg(argv[i], "-h")) {
                PrintUsage(argv[0]);
                std::exit(0);
            }

            if (IsArg(argv[i], "--benchmark") && i + 1 < argc) {
                const char* mode = argv[++i];

                if (IsArg(mode, "seq") || IsArg(mode, "sequential")) {
                    config.mode = RunMode::BenchmarkSequential;
                }
                else if (IsArg(mode, "omp") || IsArg(mode, "openmp")) {
                    config.mode = RunMode::BenchmarkOpenMP;
                }
                else {
                    std::printf("Unknown benchmark mode: %s\n", mode);
                    PrintUsage(argv[0]);
                    std::exit(1);
                }
            }
            else if (IsArg(argv[i], "--rows") && i + 1 < argc) {
                config.benchmark.rows = ParseIntArg(argv[++i], config.benchmark.rows);
            }
            else if (IsArg(argv[i], "--cols") && i + 1 < argc) {
                config.benchmark.cols = ParseIntArg(argv[++i], config.benchmark.cols);
            }
            else if (IsArg(argv[i], "--warmup") && i + 1 < argc) {
                config.benchmark.warmupSteps = ParseIntArg(argv[++i], config.benchmark.warmupSteps);
            }
            else if (IsArg(argv[i], "--steps") && i + 1 < argc) {
                config.benchmark.measuredSteps = ParseIntArg(argv[++i], config.benchmark.measuredSteps);
            }
            else if (IsArg(argv[i], "--alive") && i + 1 < argc) {
                config.benchmark.initialAlivePercent = ParseIntArg(argv[++i], config.benchmark.initialAlivePercent);
            }
            else if (IsArg(argv[i], "--seed") && i + 1 < argc) {
                config.benchmark.seed = ParseUIntArg(argv[++i], config.benchmark.seed);
            }
            else if (IsArg(argv[i], "--threads") && i + 1 < argc) {
                config.benchmark.threads = ParseIntArg(argv[++i], config.benchmark.threads);
            }
            else {
                std::printf("Unknown or incomplete argument: %s\n", argv[i]);
                PrintUsage(argv[0]);
                std::exit(1);
            }
        }

        return config;
    }

    int RunBenchmarkMode(const AppLaunchConfig& launchConfig) {
        BenchmarkResult result;

        if (launchConfig.mode == RunMode::BenchmarkSequential) {
            std::printf("Starting seq\n");
            result = RunSequentialBenchmark(launchConfig.benchmark);
        }
        else {
            std::printf("Starting omp\n");
            result = RunOpenMpBenchmark(launchConfig.benchmark);
        }

        PrintBenchmarkResult(result);
        AppendBenchmarkCsv(result);
        return 0;
    }

    void LogFrameStats(double simMs, double drawMs, double frameMs,
        int stepsThisFrame, const GameState& game)
    {
        std::printf(
            "sim: %.3f ms | draw: %.3f ms | frame: %.3f ms | steps: %d | gen: %d | alive: %d\n",
            simMs, drawMs, frameMs,
            stepsThisFrame, game.generation, game.aliveCells
        );
    }

    int RunDemoMode() {
        InitWindow(cfg::WINDOW_WIDTH, cfg::WINDOW_HEIGHT, "Game of Life");
        SetTargetFPS(60);

#ifdef _OPENMP
        const int maxThreads = omp_get_num_procs();

        omp_set_dynamic(0);              // Disable dynamic thread adjustment by the OpenMP runtime.
        omp_set_num_threads(maxThreads); // Use all available logical processors.

        std::printf("OpenMP enabled\n");
        std::printf("Available processors: %d\n", omp_get_num_procs());
        std::printf("Max OpenMP threads:   %d\n", omp_get_max_threads());
#else
        std::printf("OpenMP disabled. Demo uses sequential simulation.\n");
#endif

        GameState game;
        InitializeGame(game);

        float simulationAccumulator = 0.0f;

        double logTimer = 0.0;

        while (!WindowShouldClose()) {
            const float dt = GetFrameTime();

            const double frameStart = GetTime();

            // ------------------------------
            // Input / UI / camera update
            // ------------------------------
            UpdateCameraControls(game);
            HandleKeyboardInput(game);
            HandleCellDrawing(game);
            UpdateSidePanelControls(game);

            const double simStart = GetTime();

            // ------------------------------
            // Simulation update
            // ------------------------------
            int stepsThisFrame = 0;

            if (!game.isPaused) {
                const float simulationStep =
                    1.0f / static_cast<float>(cfg::BASE_GENERATION_TPS * game.speedMultiplier);

                simulationAccumulator += dt;

                while (simulationAccumulator >= simulationStep &&
                    stepsThisFrame < cfg::MAX_SIMULATION_STEPS_PER_FRAME) {
#ifdef _OPENMP
                    StepSimulationOpenMP(game);
#else
                    StepSimulation(game);
#endif
                    simulationAccumulator -= simulationStep;
                    ++stepsThisFrame;
                }

                if (stepsThisFrame == cfg::MAX_SIMULATION_STEPS_PER_FRAME) {
                    simulationAccumulator = 0.0f;
                }
            }
            else {
                simulationAccumulator = 0.0f;
            }

            const double simEnd = GetTime();

            // ------------------------------
            // Rendering
            // ------------------------------
            DrawGame(game);

            const double frameEnd = GetTime();

            // ------------------------------
            // Timing statistics
            // ------------------------------
            const double simMs = (simEnd - simStart) * 1000.0;
            const double drawMs = (frameEnd - simEnd) * 1000.0;
            const double frameMs = (frameEnd - frameStart) * 1000.0;

            logTimer += dt;

            if (logTimer >= 0.5) {
                LogFrameStats(simMs, drawMs, frameMs, stepsThisFrame, game);
                logTimer = 0.0;
            }
        }

        CloseWindow();
        return 0;
    }
}

int main(int argc, char** argv) {
    const AppLaunchConfig launchConfig = ParseCommandLine(argc, argv);

    if (launchConfig.mode == RunMode::BenchmarkSequential ||
        launchConfig.mode == RunMode::BenchmarkOpenMP) {
        return RunBenchmarkMode(launchConfig);
    }

    return RunDemoMode();
}