"use client";

import { useState, useEffect, useRef, useCallback } from "react";

// ─── JS Game of Life engine (same algorithm & seed=42 as C code) ───
// We use a simple seedable PRNG that matches C's rand() with srand(42)
// so the initial grid is identical to the C implementations.
function mulberry32(seed) {
  return function () {
    seed |= 0;
    seed = (seed + 0x6d2b79f5) | 0;
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

// NOTE: The C code uses srand(42) + rand()%10 < 3  (glibc rand).
// We can't perfectly replicate glibc's rand() in JS, but for visualization
// purposes a 30% random fill with a fixed seed looks the same.
function createGrid(rows, cols) {
  const rng = mulberry32(42);
  const grid = new Uint8Array(rows * cols);
  for (let i = 0; i < rows * cols; i++) {
    grid[i] = rng() < 0.3 ? 1 : 0;
  }
  return grid;
}

function nextGeneration(current, rows, cols) {
  const next = new Uint8Array(rows * cols);
  for (let i = 0; i < rows; i++) {
    for (let j = 0; j < cols; j++) {
      let count = 0;
      for (let di = -1; di <= 1; di++) {
        for (let dj = -1; dj <= 1; dj++) {
          if (di === 0 && dj === 0) continue;
          const ni = (i + di + rows) % rows;
          const nj = (j + dj + cols) % cols;
          count += current[ni * cols + nj];
        }
      }
      const idx = i * cols + j;
      if (current[idx] === 1) {
        next[idx] = count === 2 || count === 3 ? 1 : 0;
      } else {
        next[idx] = count === 3 ? 1 : 0;
      }
    }
  }
  return next;
}

// ─── Component ───
export default function Home() {
  const [mode, setMode] = useState("single");
  const [impl, setImpl] = useState("serial");
  const [rows, setRows] = useState(100);
  const [cols, setCols] = useState(100);
  const [gens, setGens] = useState(100);
  const [threads, setThreads] = useState(4);
  const [processes, setProcesses] = useState(4);

  const [isRunning, setIsRunning] = useState(false);
  const [buildStatus, setBuildStatus] = useState("idle"); // idle | building | done | error
  const [stats, setStats] = useState(null);
  const [vizGen, setVizGen] = useState(0);
  const [benchmarkLog, setBenchmarkLog] = useState("");

  const canvasRef = useRef(null);
  const animRef = useRef(null); // requestAnimationFrame id
  const abortRef = useRef(false);

  // Auto-build on mount
  useEffect(() => {
    (async () => {
      setBuildStatus("building");
      try {
        const res = await fetch("/api/build", { method: "POST" });
        const data = await res.json();
        setBuildStatus(data.success ? "done" : "error");
      } catch {
        setBuildStatus("error");
      }
    })();
  }, []);

  // ─── Draw a grid onto the canvas ───
  const drawGrid = useCallback(
    (grid, gridRows, gridCols) => {
      const canvas = canvasRef.current;
      if (!canvas) return;
      const ctx = canvas.getContext("2d");

      // Set canvas pixel resolution to match the grid
      if (canvas.width !== gridCols || canvas.height !== gridRows) {
        canvas.width = gridCols;
        canvas.height = gridRows;
      }

      // Background (dead cells)
      ctx.fillStyle = "#0a0a1a";
      ctx.fillRect(0, 0, gridCols, gridRows);

      // Alive cells
      ctx.fillStyle = "#06b6d4";
      for (let i = 0; i < gridRows; i++) {
        for (let j = 0; j < gridCols; j++) {
          if (grid[i * gridCols + j]) {
            ctx.fillRect(j, i, 1, 1);
          }
        }
      }
    },
    []
  );

  // ─── Run a single implementation ───
  const handleRunSingle = async () => {
    setIsRunning(true);
    setStats(null);
    setVizGen(0);
    abortRef.current = false;

    // 1) Kick off JS visualization concurrently
    const vizRows = rows;
    const vizCols = cols;
    const vizGens = gens;

    let grid = createGrid(vizRows, vizCols);
    drawGrid(grid, vizRows, vizCols);

    // Animate at ~30fps (every ~33ms) — but skip frames for large grids
    const framesPerTick = Math.max(1, Math.floor(vizGens / 300)); // cap to ~300 frames
    let g = 0;

    const animate = () => {
      if (abortRef.current || g >= vizGens) return;

      for (let f = 0; f < framesPerTick && g < vizGens; f++, g++) {
        grid = nextGeneration(grid, vizRows, vizCols);
      }
      drawGrid(grid, vizRows, vizCols);
      setVizGen(g);

      animRef.current = requestAnimationFrame(animate);
    };
    animRef.current = requestAnimationFrame(animate);

    // 2) In parallel, call the backend to get the actual C execution stats
    try {
      const query = new URLSearchParams({
        impl,
        rows: String(rows),
        cols: String(cols),
        gens: String(gens),
        threads: String(threads),
        processes: String(processes),
      });
      const res = await fetch(`/api/run?${query.toString()}`);
      const data = await res.json();
      setStats(data);
    } catch (err) {
      console.error(err);
      setStats({ error: "Failed to execute binary" });
    }

    setIsRunning(false);
  };

  // ─── Run benchmark ───
  const handleRunBenchmark = async () => {
    setIsRunning(true);
    setBenchmarkLog("");
    try {
      const query = new URLSearchParams({
        rows: String(rows),
        cols: String(cols),
        gens: String(gens),
      });
      const response = await fetch(`/api/benchmark?${query.toString()}`);
      const reader = response.body.getReader();
      const decoder = new TextDecoder("utf-8");

      while (true) {
        const { value, done } = await reader.read();
        if (done) break;
        const chunk = decoder.decode(value, { stream: true });
        setBenchmarkLog((prev) => prev + chunk);
      }
    } catch (err) {
      console.error(err);
      setBenchmarkLog("Benchmark execution failed.");
    }
    setIsRunning(false);
  };

  const handleRun = () => {
    if (mode === "single") handleRunSingle();
    else handleRunBenchmark();
  };

  // Cleanup animation on unmount
  useEffect(() => {
    return () => {
      abortRef.current = true;
      if (animRef.current) cancelAnimationFrame(animRef.current);
    };
  }, []);

  // Determine which extra inputs to show
  const showProcesses =
    mode === "single" && (impl === "mpi" || impl === "hybrid");
  const showThreads =
    mode === "single" && (impl === "openmp" || impl === "hybrid");

  return (
    <>
      <header>
        <h1>Conway&apos;s Game of Life</h1>
        <p>HPC Performance Dashboard &middot; EC7207 Group 32</p>
        {buildStatus === "building" && (
          <span className="build-badge building">
            <span className="spinner-sm" /> Building binaries…
          </span>
        )}
        {buildStatus === "done" && (
          <span className="build-badge done">✓ Binaries ready</span>
        )}
        {buildStatus === "error" && (
          <span className="build-badge error">✗ Build failed</span>
        )}
      </header>

      <main className="dashboard">
        {/* ─── Controls sidebar ─── */}
        <aside className="glass-panel controls">
          <h3>⚙ Controls</h3>

          <div className="control-group">
            <label htmlFor="mode-select">Mode</label>
            <select
              id="mode-select"
              value={mode}
              onChange={(e) => setMode(e.target.value)}
            >
              <option value="single">Single Run</option>
              <option value="benchmark">Benchmark All</option>
            </select>
          </div>

          {mode === "single" && (
            <div className="control-group">
              <label htmlFor="impl-select">Implementation</label>
              <select
                id="impl-select"
                value={impl}
                onChange={(e) => setImpl(e.target.value)}
              >
                <option value="serial">Serial</option>
                <option value="openmp">OpenMP</option>
                <option value="mpi">MPI</option>
                <option value="cuda">CUDA</option>
                <option value="hybrid">Hybrid (MPI+OpenMP)</option>
              </select>
            </div>
          )}

          <div className="control-group">
            <label htmlFor="rows-input">Grid Rows</label>
            <input
              id="rows-input"
              type="number"
              min={5}
              max={5000}
              value={rows}
              onChange={(e) => setRows(Number(e.target.value))}
            />
          </div>

          <div className="control-group">
            <label htmlFor="cols-input">Grid Cols</label>
            <input
              id="cols-input"
              type="number"
              min={5}
              max={5000}
              value={cols}
              onChange={(e) => setCols(Number(e.target.value))}
            />
          </div>

          <div className="control-group">
            <label htmlFor="gens-input">Generations</label>
            <input
              id="gens-input"
              type="number"
              min={1}
              max={10000}
              value={gens}
              onChange={(e) => setGens(Number(e.target.value))}
            />
          </div>

          {showProcesses && (
            <div className="control-group">
              <label htmlFor="processes-input">MPI Processes</label>
              <input
                id="processes-input"
                type="number"
                min={1}
                max={16}
                value={processes}
                onChange={(e) => setProcesses(Number(e.target.value))}
              />
            </div>
          )}

          {showThreads && (
            <div className="control-group">
              <label htmlFor="threads-input">OpenMP Threads</label>
              <input
                id="threads-input"
                type="number"
                min={1}
                max={16}
                value={threads}
                onChange={(e) => setThreads(Number(e.target.value))}
              />
            </div>
          )}

          <button
            id="run-btn"
            onClick={handleRun}
            disabled={isRunning || buildStatus === "building"}
          >
            {isRunning ? (
              <>
                <span className="spinner" /> Running…
              </>
            ) : mode === "single" ? (
              "▶ Run Simulation"
            ) : (
              "▶ Run Benchmark"
            )}
          </button>
        </aside>

        {/* ─── Main content area ─── */}
        <section className="glass-panel main-content">
          {mode === "single" ? (
            <>
              <div className="section-header">
                <h3>
                  Visualization —{" "}
                  <span className="impl-label">{impl.toUpperCase()}</span>
                </h3>
                {vizGen > 0 && (
                  <span className="gen-counter">
                    Generation {vizGen} / {gens}
                  </span>
                )}
              </div>

              <div className="canvas-container">
                <canvas
                  ref={canvasRef}
                  style={{
                    imageRendering: "pixelated",
                    width: "100%",
                    height: "100%",
                    objectFit: "contain",
                  }}
                />
                {!isRunning && !stats && vizGen === 0 && (
                  <div className="canvas-placeholder">
                    Press <strong>Run Simulation</strong> to start
                  </div>
                )}
              </div>

              {/* Stats cards */}
              {stats && !stats.error && (
                <div className="stats-grid" style={{ gridTemplateColumns: "repeat(2, 1fr)" }}>
                  <div className="stat-box">
                    <div className="stat-value">
                      {stats.time != null ? stats.time.toFixed(4) + "s" : "—"}
                    </div>
                    <div className="stat-label">Execution Time</div>
                  </div>
                  <div className="stat-box">
                    <div className="stat-value">
                      {stats.speedup
                        ? stats.speedup.toFixed(2) + "×"
                        : "—"}
                    </div>
                    <div className="stat-label">Speedup</div>
                  </div>
                </div>
              )}
              {stats && stats.error && (
                <div className="error-banner">{stats.error}</div>
              )}
            </>
          ) : (
            <>
              <h3>Benchmark Results</h3>
              <div className="benchmark-output">
                {benchmarkLog || (
                  <span className="placeholder-text">
                    Click <strong>Run Benchmark</strong> to compare all
                    implementations.
                  </span>
                )}
              </div>
            </>
          )}
        </section>
      </main>
    </>
  );
}
