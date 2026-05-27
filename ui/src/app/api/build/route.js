import { exec } from "child_process";
import { NextResponse } from "next/server";
import path from "path";

export async function POST() {
  return new Promise((resolve) => {
    // Run 'make all' using bash to ensure consistency
    exec("bash -c 'make all'", { cwd: path.resolve("..") }, (error, stdout, stderr) => {
      if (error) {
        return resolve(NextResponse.json({ success: false, error: error.message, stderr }));
      }
      return resolve(NextResponse.json({ success: true, stdout }));
    });
  });
}
