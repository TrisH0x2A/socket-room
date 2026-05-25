<script>
  function goHome() {
    window.location.href = '/';
  }
</script>
<div class="screen-container">
  <div class="terminal-box error-box">
    <h1>404</h1>
    <h2>SYSTEM ROUTE NOT FOUND</h2>
    <p>The requested directory is empty or inaccessible.</p>
    <div class="actions">
      <button on:click={goHome} class="btn" style="width: 100%; padding: 12px; font-size: 1rem;">[ RETURN TO ROOT ]</button>
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
  .error-box {
    border-color: #ff4444;
  }
  .error-box h1 {
    font-size: 5rem;
    margin: 0;
    color: #ff4444;
  }
  .error-box h2 {
    color: var(--text-primary);
    margin-top: 10px;
  }
  .error-box p {
    color: var(--text-secondary);
    margin: 20px 0 30px;
  }
</style>
