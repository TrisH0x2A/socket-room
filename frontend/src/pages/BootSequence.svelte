<script>
  import { onMount, onDestroy } from 'svelte';
  export let onDone = () => {};
  const BOOT_LINES = [
    { text: 'BIOS v2.31 initialized',             delay: 80  },
    { text: 'CPU: x86_64 @ 3.6GHz ........... OK', delay: 60  },
    { text: 'Memory check: 16384MB .......... OK',  delay: 60  },
    { text: 'Loading kernel modules ......... OK',  delay: 80  },
    { text: 'Starting network services ...',        delay: 100 },
    { text: '  → eth0: link up 1000Mbps',           delay: 60  },
    { text: '  → TCP/IP stack initialized',         delay: 60  },
    { text: 'Mounting filesystems ........... OK',  delay: 80  },
    { text: 'Starting SQLite backend ........',     delay: 120 },
    { text: '  → DB path: data/socketroom.db',      delay: 60  },
    { text: '  → Schema OK, 4 rooms loaded',        delay: 60  },
    { text: 'Starting WebSocket server ......',     delay: 100 },
    { text: '  → Listening on :8080',               delay: 60  },
    { text: '  → Auth: session-based',              delay: 60  },
    { text: 'Loading frontend assets ........',     delay: 80  },
    { text: '  → Svelte 5 + Tailwind CSS',          delay: 60  },
    { text: '  → JetBrains Mono loaded',            delay: 40  },
    { text: '',                                     delay: 40  },
    { text: '████████████████████████ 100%',        delay: 200 },
    { text: '',                                     delay: 60  },
    { text: 'SOCKETROOM READY',                     delay: 0   },
  ];
  let lines    = [];
  let progress = 0;
  let done     = false;
  let alive    = true;   
  function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }
  function scrollAction(node, _deps) {
    const update = () => { node.scrollTop = node.scrollHeight; };
    update();
    return { update };
  }
  onMount(async () => {
    for (let i = 0; i < BOOT_LINES.length; i++) {
      if (!alive) return;
      await sleep(BOOT_LINES[i].delay + Math.random() * 40);
      if (!alive) return;
      lines    = [...lines, BOOT_LINES[i].text];
      progress = Math.round((i / (BOOT_LINES.length - 1)) * 100);
    }
    if (!alive) return;
    await sleep(500);
    if (!alive) return;
    done = true;
    await sleep(350);
    if (!alive) return;
    console.log("[BootSequence] Calling onDone()"); onDone();
  });
  onDestroy(() => { alive = false; });
</script>
<div class="boot-screen">
  <div class="boot-inner">
    <div class="boot-header">
      <span class="sys-name glow">SOCKETROOM</span>
      <span class="sys-ver">v1.0.0-release</span>
    </div>
    <div class="boot-log" id="boot-log" use:scrollAction={lines}>
      {#each lines as line, i}
        <div
          class="boot-line"
          class:dim={line === ''}
          style="animation-delay: {i * 5}ms"
        >
          {#if line.includes('SOCKETROOM READY')}
            <span class="ready-line glow">{line}</span>
          {:else if line.includes('100%')}
            <span class="progress-line">{line}</span>
          {:else}
            <span class="prompt-char">{line ? '>' : ' '}</span>
            {line}
          {/if}
        </div>
      {/each}
      {#if !done}
        <div class="boot-cursor cursor"></div>
      {/if}
    </div>
    <div class="progress-bar-wrap">
      <div class="progress-bar" style="width: {progress}%"></div>
    </div>
    <div class="progress-label">{progress}% — INITIALIZING SYSTEM</div>
  </div>
</div>
<style>
  .boot-screen {
    position: fixed;
    inset: 0;
    background: var(--bg);
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 40px;
  }
  .boot-inner {
    width: 100%;
    max-width: 680px;
    display: flex;
    flex-direction: column;
    gap: 16px;
  }
  .boot-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    border-bottom: 1px solid var(--border);
    padding-bottom: 10px;
  }
  .sys-name {
    font-size: 18px;
    font-weight: 700;
    color: var(--green);
    letter-spacing: 0.12em;
  }
  .sys-ver {
    font-size: 11px;
    color: var(--text-muted);
    letter-spacing: 0.08em;
  }
  .boot-log {
    display: flex;
    flex-direction: column;
    gap: 2px;
    max-height: 480px;
    overflow-y: auto;
    font-size: 12px;
    color: var(--text-primary);
    font-family: inherit;
  }
  .boot-line {
    animation: fadeIn 0.15s ease forwards;
    line-height: 1.6;
    display: flex;
    gap: 8px;
  }
  .boot-line.dim { height: 8px; }
  .prompt-char { color: var(--green-muted); user-select: none; }
  .ready-line {
    color: var(--green);
    font-size: 18px;
    font-weight: 700;
    letter-spacing: 0.2em;
    margin-top: 8px;
  }
  .progress-line { color: var(--green); letter-spacing: 0.05em; }
  .boot-cursor { height: 14px; }
  .progress-bar-wrap {
    height: 3px;
    background: var(--bg-hover);
    border-radius: 1px;
    overflow: hidden;
    margin-top: 8px;
  }
  .progress-bar {
    height: 100%;
    background: var(--green);
    box-shadow: 0 0 8px var(--green);
    transition: width 0.2s ease;
  }
  .progress-label {
    font-size: 10px;
    color: var(--text-muted);
    letter-spacing: 0.1em;
    text-align: center;
  }
  @keyframes fadeIn {
    from { opacity: 0; }
    to   { opacity: 1; }
  }
  .glow { text-shadow: 0 0 8px var(--green), 0 0 20px rgba(0,255,65,0.4); }
</style>
