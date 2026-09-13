# WebAssembly Interface (MVP)

This page provides a lightweight shell interface to the EICrecon WebAssembly (WASM) build.
Currently, this is a minimal viable product (MVP) showcasing the plumbing required to run C++ code natively in the browser via Emscripten, simulating a CLI environment.

<div id="wasm-app" style="margin-top: 2em; border: 1px solid #333; border-radius: 5px; background: #1e1e1e; color: #d4d4d4; font-family: 'Courier New', Courier, monospace; height: 400px; display: flex; flex-direction: column; overflow: hidden; box-shadow: 0 4px 6px rgba(0,0,0,0.3);">
  <div style="background: #333; padding: 5px 10px; font-size: 12px; font-weight: bold; border-bottom: 1px solid #111;">
    eicrecon-wasm-shell
  </div>
  <div id="terminal-output" style="flex-grow: 1; overflow-y: auto; padding: 10px; white-space: pre-wrap; word-wrap: break-word;">
    <div>Loading WebAssembly module...</div>
  </div>
  <div style="display: flex; padding: 10px; border-top: 1px solid #333; background: #252526;">
    <span style="color: #4af626; margin-right: 8px;">physicist@wasm:~$</span>
    <input type="text" id="cmd-input" autocomplete="off" spellcheck="false" style="background: transparent; color: #d4d4d4; border: none; outline: none; flex-grow: 1; font-family: 'Courier New', Courier, monospace; font-size: 14px;" disabled />
  </div>
</div>

<script>
  const interval = setInterval(() => {
    const input = document.getElementById('cmd-input');
    const output = document.getElementById('terminal-output');

    if (input && output && !window.wasmLoaded) {
      window.wasmLoaded = true;
      clearInterval(interval);

      const script = document.createElement('script');
      script.src = 'wasm/hello.js';
      script.onload = () => {
        Module.onRuntimeInitialized = () => {
          output.innerHTML = '<div>WebAssembly module loaded. Type "help" for available commands.</div>';
          input.disabled = false;
          input.focus();

          input.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
              const cmd = input.value.trim();
              if (cmd) {
                // Echo command
                const promptLine = document.createElement('div');
                promptLine.innerHTML = `<span style="color: #4af626;">physicist@wasm:~$</span> ${cmd}`;
                output.appendChild(promptLine);

                if (cmd === 'clear') {
                  output.innerHTML = '';
                } else {
                  // Execute C++ command
                  const result = Module.execute_command(cmd);
                  if (result) {
                    const resultLine = document.createElement('div');
                    resultLine.innerText = result;
                    resultLine.style.color = '#9cdcfe';
                    output.appendChild(resultLine);
                  }
                }
              }
              input.value = '';
              output.scrollTop = output.scrollHeight;
            }
          });
        };
      };
      document.head.appendChild(script);
    }
  }, 200);
</script>
