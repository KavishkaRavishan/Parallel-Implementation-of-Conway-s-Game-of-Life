import { spawn } from "child_process";
import path from "path";

// Helper: run a command and return { stdout, stderr, code }
function runCmd(bashCmd, cwd) {
  return new Promise((resolve) => {
    let stdout = "";
    let stderr = "";
    const child = spawn("bash", ["-c", bashCmd], { cwd });

    child.stdout.on("data", (d) => (stdout += d.toString()));
    child.stderr.on("data", (d) => (stderr += d.toString()));
    child.on("close", (code) => resolve({ stdout, stderr, code }));
    child.on("error", (err) => resolve({ stdout, stderr: err.message, code: 1 }));
  });
}

export async function GET(req) {
  const { searchParams } = new URL(req.url);
  const impl = searchParams.get("impl") || "serial";
  const rows = searchParams.get("rows") || "100";
  const cols = searchParams.get("cols") || "100";
  const gens = searchParams.get("gens") || "50";
  const threads = searchParams.get("threads") || "4";
  const processes = searchParams.get("processes") || "4";

  const projectRoot = path.resolve(process.cwd(), "..");

  // Build the command for the selected implementation
  let bashCmd = "";
  if (impl === "serial") {
    bashCmd = `./serial/gol_serial ${rows} ${cols} ${gens}`;
  } else if (impl === "openmp") {
    bashCmd = `./openmp/gol_openmp ${rows} ${cols} ${gens} ${threads}`;
  } else if (impl === "mpi") {
    bashCmd = `mpiexec --allow-run-as-root --oversubscribe -n ${processes} ./mpi/gol_mpi ${rows} ${cols} ${gens}`;
  } else if (impl === "cuda") {
    bashCmd = `./cuda/gol_cuda ${rows} ${cols} ${gens}`;
  } else if (impl === "hybrid") {
    bashCmd = `mpiexec --allow-run-as-root --oversubscribe -n ${processes} ./hybrid/gol_hybrid ${rows} ${cols} ${gens} ${threads}`;
  }

  console.log("Running command:", bashCmd);

  // Run the selected implementation and serial baseline in parallel
  const implPromise = runCmd(bashCmd, projectRoot);
  const serialPromise =
    impl !== "serial"
      ? runCmd(`./serial/gol_serial ${rows} ${cols} ${gens}`, projectRoot)
      : null;

  const [implResult, serialResult] = await Promise.all([
    implPromise,
    serialPromise,
  ]);

  const result = { impl, rows, cols, gens, threads, processes };

  // --- Parse implementation output ---
  const stdout = implResult.stdout;

  // Extract execution time from various output formats
  const timeMatch = stdout.match(
    /(?:Total Execution Time|OpenMP Execution Time).*?([\d.]+)\s*seconds/
  );
  if (timeMatch) {
    result.time = parseFloat(timeMatch[1]);
  }

  // OpenMP prints both serial and OMP times
  const ompTimeMatch = stdout.match(/OpenMP Execution Time:\s*([\d.]+)\s*seconds/);
  if (ompTimeMatch) result.time = parseFloat(ompTimeMatch[1]);

  // CUDA prints GPU time
  const gpuTimeMatch = stdout.match(/Total Execution Time \(GPU\):\s*([\d.]+)\s*seconds/);
  if (gpuTimeMatch) result.time = parseFloat(gpuTimeMatch[1]);

  // --- Compute speedup from serial baseline ---
  if (impl === "serial") {
    result.speedup = 1.0;
  } else if (serialResult) {
    const serialTimeMatch = serialResult.stdout.match(
      /Total Execution Time:\s*([\d.]+)\s*seconds/
    );
    if (serialTimeMatch && result.time) {
      const serialTime = parseFloat(serialTimeMatch[1]);
      result.serialTime = serialTime;
      result.speedup = serialTime / result.time;
    }
  }

  result.exitCode = implResult.code;
  result.rawOutput = stdout;
  if (implResult.stderr) result.stderr = implResult.stderr;

  return new Response(JSON.stringify(result), {
    headers: { "Content-Type": "application/json" },
  });
}
