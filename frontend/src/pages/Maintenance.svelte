<script>
  function retry() {
    window.location.reload();
  }
</script>
<div class="screen-container">
  <div class="terminal-box warning-box">
    <h1>503</h1>
    <h2>SYSTEM OFFLINE</h2>
    <p>Connection to the mainframe could not be established.<br/>The server may be down for maintenance.</p>
    <div class="actions">
      <button on:click={retry} class="btn" style="width: 100%; padding: 12px; font-size: 1rem;">[ RETRY CONNECTION ]</button>
    </div>
  </div>
</div>
<style>
  .screen-container {
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
    padding: 20px;
    background-color: var(--bg-primary);
  }
  .terminal-box {
    border: 1px solid var(--border-color);
    padding: 40px;
    max-width: 500px;
    width: 100%;
    text-align: center;
    background: var(--bg-secondary);
  }
  .warning-box {
    border-color: #ffaa00;
  }
  .warning-box h1 {
    font-size: 5rem;
    margin: 0;
    color: #ffaa00;
  }
  .warning-box h2 {
    color: var(--text-primary);
    margin-top: 10px;
    animation: blink 2s infinite;
  }
  .warning-box p {
    color: var(--text-secondary);
    margin: 20px 0 30px;
    line-height: 1.5;
  }
  @keyframes blink {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
  }
</style>
