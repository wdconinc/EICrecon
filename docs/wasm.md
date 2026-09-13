# WebAssembly Interface (MVP)

This page demonstrates the WebAssembly (WASM) integration into the EICrecon documentation.
Currently, this is a minimal viable product (MVP) showcasing the plumbing required to run C++ code natively in the browser via Emscripten, bypassing the need for remote server computation.

<div id="wasm-app" style="margin-top: 2em; padding: 1em; border: 1px solid #ccc; border-radius: 5px; background: #f9f9f9;">
  <h3>C++ WASM Module</h3>
  <input type="text" id="name-input" placeholder="Enter your name" value="Physicist" style="padding: 5px; font-size: 16px;"/>
  <button id="greet-btn" style="padding: 5px 10px; font-size: 16px; background-color: #007bff; color: white; border: none; border-radius: 3px; cursor: pointer;">Run WASM</button>
  <p id="wasm-output" style="margin-top: 15px; font-weight: bold; min-height: 1.5em;"></p>
</div>

<script>
  // Setup a simple loop to check when the DOM for our app is loaded
  // since Docsify loads content dynamically via Ajax.
  const interval = setInterval(() => {
    const btn = document.getElementById('greet-btn');
    if (btn && !window.wasmLoaded) {
      window.wasmLoaded = true;
      clearInterval(interval);

      // Load the Emscripten JS glue code dynamically
      const script = document.createElement('script');
      script.src = 'wasm/hello.js';
      script.onload = () => {
        // Module is defined by hello.js
        Module.onRuntimeInitialized = () => {
          btn.addEventListener('click', () => {
            const name = document.getElementById('name-input').value;
            const result = Module.greet(name);
            document.getElementById('wasm-output').innerText = result;
            document.getElementById('wasm-output').style.color = "green";
          });
          document.getElementById('wasm-output').innerText = "WASM Module Loaded. Ready to run.";
        };
      };
      document.head.appendChild(script);
    }
  }, 200);
</script>
