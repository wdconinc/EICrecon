# WebAssembly Interface (MVP)

This page provides a lightweight interface to the EICrecon WebAssembly (WASM) build.
Currently, this is a minimal viable product (MVP) showcasing the plumbing required to run C++ code natively in the browser via Emscripten.

<div id="wasm-app" style="margin-top: 2em; border: 1px solid #333; border-radius: 5px; background: #1e1e1e; color: #d4d4d4; font-family: 'Courier New', Courier, monospace; height: 500px; display: flex; flex-direction: column; overflow: hidden; box-shadow: 0 4px 6px rgba(0,0,0,0.3);">
  <div style="background: #333; padding: 5px 10px; font-size: 12px; font-weight: bold; border-bottom: 1px solid #111;">
    eicrecon-wasm-runner
  </div>
  <div id="terminal-output" style="flex-grow: 1; overflow-y: auto; padding: 10px; white-space: pre-wrap; word-wrap: break-word;">
    <div>Loading WebAssembly module...</div>
  </div>
  <div style="display: flex; padding: 10px; border-top: 1px solid #333; background: #252526;">
    <button id="run-btn" style="background: #007acc; color: white; border: none; padding: 5px 15px; border-radius: 3px; cursor: pointer; font-family: monospace;" disabled>Run EICrecon</button>
  </div>
</div>

<script>
  const output = document.getElementById('terminal-output');
  const runBtn = document.getElementById('run-btn');

  var Module = {
    noInitialRun: true,
    print: function(text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      const resultLine = document.createElement('div');
      resultLine.innerText = text;
      resultLine.style.color = '#9cdcfe';
      output.appendChild(resultLine);
      output.scrollTop = output.scrollHeight;
    },
    printErr: function(text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      const resultLine = document.createElement('div');
      resultLine.innerText = text;
      resultLine.style.color = '#f48771';
      output.appendChild(resultLine);
      output.scrollTop = output.scrollHeight;
    }
  };

  const interval = setInterval(() => {
    if (!window.wasmLoaded) {
      window.wasmLoaded = true;
      clearInterval(interval);

      const script = document.createElement('script');
      script.src = 'wasm/eicrecon.js';
      script.onload = () => {
        Module.onRuntimeInitialized = () => {
          output.innerHTML += '<div>WebAssembly module loaded. Click "Run" to start.</div>';
          runBtn.disabled = false;

          runBtn.addEventListener('click', () => {
            output.innerHTML += '<div><span style="color: #4af626;">physicist@wasm:~$</span> eicrecon</div>';
            runBtn.disabled = true;
            try {
              Module.callMain(['-Pjana:nthreads=1']);
            } catch (e) {
              if (e !== 'unwind') {
                Module.printErr(e);
              }
            }
          });
        };
      };
      document.head.appendChild(script);
    }
  }, 200);
</script>
