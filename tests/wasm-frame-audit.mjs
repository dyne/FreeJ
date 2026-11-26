import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { readFileSync } from 'node:fs';

const [, , wasmFile, wasmTrace, nativeTrace, contextTrace] = process.argv;
const module = new WebAssembly.Module(readFileSync(wasmFile));
const allowed = new Set([
  'env.__cxa_throw',
  'env._abort_js',
  'env.emscripten_resize_heap',
  'wasi_snapshot_preview1.fd_close',
  'wasi_snapshot_preview1.fd_seek',
  'wasi_snapshot_preview1.fd_write',
]);

const imports = WebAssembly.Module.imports(module);
assert(imports.length > 0);
for (const { module, name, kind } of imports) {
  const qualified = `${module}.${name}`;
  assert(kind === 'function' && allowed.has(qualified), `unexpected ${kind} import ${qualified}`);
  console.log(`import ${qualified} (${kind})`);
}
assert(imports.some(({ name }) => name === 'fd_write'), 'stdio output import missing');

const normal = [
  '7856341278563412785634127856341278563412',
  '78563412efbf8f90eebe8e91edbd8d92ecbc8c93',
  '78563412e7b78798e6b68699e5b5859ae4b4849b',
].join('');
const cropped = [
  'e6b68699e5b5859ae4b4849be3b3839ce2b2829d',
  'deae7ea1ddad7da2dcac7ca3dbab7ba4daaa7aa5',
  'd6a676a9d5a575aad4a474abd3a373acd2a272ad',
].join('');
const times = [40000, 80000, 205000, 245000];
const frames = [normal, normal, cropped, cropped];
const expected = frames.flatMap((frame, tick) => [
  `frame-probe input ${tick} ${tick < 2 ? 1 : -1}`,
  `frame-probe present ${tick} ${frame}`,
  `frame-probe time ${tick} ${times[tick]}`,
]).join('\n') + '\n';

for (const file of [nativeTrace, wasmTrace, ...(contextTrace ? [contextTrace] : [])]) {
  assert.equal(readFileSync(file, 'utf8'), expected, `frame count, trace or pixels differ in ${file}`);
}
const digest = createHash('sha256').update(Buffer.from(frames.join(''), 'hex')).digest('hex');
console.log(`PASS 4 frames, 12 events, 240 exact BGRA bytes, sha256=${digest}`);
console.log(contextTrace ? 'PASS native probe, WASM probe and native Context match' : 'PASS native probe and WASM probe match');
