import { spawn } from "child_process";
import path from "path";

export async function GET(req) {
  const { searchParams } = new URL(req.url);
  const rows = searchParams.get("rows") || "2000";
  const cols = searchParams.get("cols") || "2000";
  const gens = searchParams.get("gens") || "500";

  const bashCmd = `./benchmark.sh ${rows} ${cols} ${gens}`;
  console.log("Running benchmark:", bashCmd);

  const child = spawn("bash", ["-c", bashCmd], { cwd: path.resolve("..") });

  const stream = new ReadableStream({
    start(controller) {
      child.stdout.on("data", (data) => {
        controller.enqueue(data);
      });
      child.stderr.on("data", (data) => {
        console.error(`[benchmark stderr]: ${data}`);
      });
      child.on("close", (code) => {
        controller.close();
      });
      child.on("error", (err) => {
        console.error(err);
        controller.enqueue(new TextEncoder().encode(`Error: ${err.message}`));
        controller.close();
      });
    },
    cancel() {
      child.kill();
    }
  });

  return new Response(stream, {
    headers: {
      "Content-Type": "text/event-stream",
      "Cache-Control": "no-cache",
      "Connection": "keep-alive"
    }
  });
}
