// Renders .excalidraw files to SVG with Excalidraw's own exporter. It runs in
// headless Chromium so fonts and text come out the same as in the editor.
//
// Usage: node export.mjs <diagram.excalidraw>...
// Writes <diagram>.svg next to each input file.

import { createServer } from "node:http";
import { readFile, writeFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { chromium } from "playwright";

const inputs = process.argv.slice(2);
if (inputs.length === 0) {
  console.error("Usage: node export.mjs <diagram.excalidraw>...");
  process.exit(1);
}

// Serve the @excalidraw/utils bundle locally so the page can import it
const libDir = path.dirname(fileURLToPath(import.meta.resolve("@excalidraw/utils")));
const contentTypes = { ".js": "text/javascript", ".wasm": "application/wasm" };

const server = createServer(async (req, res) => {
  const { pathname } = new URL(req.url, "http://localhost");
  if (pathname === "/") {
    res.writeHead(200, { "Content-Type": "text/html" }).end("<!doctype html><html><body></body></html>");
    return;
  }
  const file = path.join(libDir, path.normalize(decodeURIComponent(pathname)));
  if (!file.startsWith(libDir + path.sep)) {
    res.writeHead(403).end();
    return;
  }
  try {
    const body = await readFile(file);
    res.writeHead(200, { "Content-Type": contentTypes[path.extname(file)] ?? "application/octet-stream" }).end(body);
  } catch {
    res.writeHead(404).end();
  }
});
await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve));
const origin = `http://127.0.0.1:${server.address().port}`;

const browser = await chromium.launch();
let failed = false;
try {
  const page = await browser.newPage();
  page.on("pageerror", (err) => console.error(err));
  await page.goto(origin);

  for (const input of inputs) {
    const output = input.replace(/\.excalidraw$/, "") + ".svg";
    try {
      const scene = JSON.parse(await readFile(input, "utf8"));
      const svg = await page.evaluate(async (scene) => {
        window.EXCALIDRAW_ASSET_PATH = `${location.origin}/`;
        const { exportScene } = await import("/index.js");
        const el = await exportScene({
          type: "svg",
          data: {
            elements: scene.elements.filter((e) => !e.isDeleted),
            appState: scene.appState,
            files: scene.files,
          },
        });
        return new XMLSerializer().serializeToString(el);
      }, scene);
      await writeFile(output, svg + "\n");
      console.log(`${input} -> ${output}`);
    } catch (err) {
      failed = true;
      console.error(`Failed to render ${input}:`, err);
    }
  }
} finally {
  await browser.close();
  server.close();
}
process.exit(failed ? 1 : 0);
