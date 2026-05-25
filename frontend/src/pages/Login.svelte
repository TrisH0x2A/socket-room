<script>
  import { createEventDispatcher } from 'svelte';
  import { api } from '../services/api.js';
  const dispatch = createEventDispatcher();
  let mode = 'login';  
  let username = '';
  let password = '';
  let error = '';
  let loading = false;
  async function submit() {
    error = '';
    if (!username.trim() || !password.trim()) {
      error = 'USERNAME AND PASSWORD REQUIRED';
      return;
    }
    loading = true;
    try {
      if (mode === 'register') {
        await api.register(username.trim(), password);
        mode = 'login';
        error = 'ACCOUNT CREATED — PLEASE LOGIN';
        password = '';
      } else {
        const data = await api.login(username.trim(), password);
        const userObj = { id: data.user_id, username: data.username };
        dispatch('login', { user: userObj, token: data.token || '' });
      }
    } catch (e) {
      error = e.message.toUpperCase();
    } finally {
      loading = false;
    }
  }
  function onKeydown(e) {
    if (e.key === 'Enter') submit();
  }
</script>
<div class="login-backdrop">
  <div class="noise"></div>
  <div class="login-wrap slide-up">
    <div class="ascii-header">
      <pre class="logo glow">
  ███████╗ ██████╗  ██████╗██╗  ██╗███████╗████████╗██████╗  ██████╗  ██████╗ ███╗   ███╗
  ██╔════╝██╔═══██╗██╔════╝██║ ██╔╝██╔════╝╚══██╔══╝██╔══██╗██╔═══██╗██╔═══██╗████╗ ████║
  ███████╗██║   ██║██║     █████╔╝ █████╗     ██║   ██████╔╝██║   ██║██║   ██║██╔████╔██║
  ╚════██║██║   ██║██║     ██╔═██╗ ██╔══╝     ██║   ██╔══██╗██║   ██║██║   ██║██║╚██╔╝██║
  ███████║╚██████╔╝╚██████╗██║  ██╗███████╗   ██║   ██║  ██║╚██████╔╝╚██████╔╝██║ ╚═╝ ██║
  ╚══════╝ ╚═════╝  ╚═════╝╚═╝  ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝     ╚═╝</pre>
      <div class="subtitle">
        <span style="color:var(--text-muted)">[ </span>
        REALTIME TERMINAL CHAT
        <span style="color:var(--text-muted)"> ]</span>
      </div>
    </div>
    <div class="login-box glow-border">
      <div class="mode-tabs">
        <button
          class="tab"
          class:active={mode === 'login'}
          on:click={() => { mode = 'login'; error = ''; }}
        >LOGIN</button>
        <span class="tab-sep">|</span>
        <button
          class="tab"
          class:active={mode === 'register'}
          on:click={() => { mode = 'register'; error = ''; }}
        >CREATE ACCOUNT</button>
      </div>
      <div class="fields">
        <div class="field-group">
          <label class="field-label" for="login-username">USERNAME</label>
          <div class="field-wrap">
            <span class="field-prefix">$</span>
            <input
              id="login-username"
              bind:value={username}
              on:keydown={onKeydown}
              class="field-input"
              type="text"
              placeholder="your_username"
              maxlength="32"
              autocomplete="username"
              spellcheck="false"
              disabled={loading}
            />
          </div>
        </div>
        <div class="field-group">
          <label class="field-label" for="login-password">PASSWORD</label>
          <div class="field-wrap">
            <span class="field-prefix">$</span>
            <input
              id="login-password"
              bind:value={password}
              on:keydown={onKeydown}
              class="field-input"
              type="password"
              placeholder="••••••••"
              maxlength="128"
              autocomplete="current-password"
              disabled={loading}
            />
          </div>
        </div>
      </div>
      {#if error}
        <div class="error-msg" class:success={error.includes('CREATED')}>
          <span class="err-prefix">{error.includes('CREATED') ? '[+]' : '[!]'}</span>
          {error}
        </div>
      {/if}
      <button
        id="login-submit"
        class="submit-btn"
        on:click={submit}
        disabled={loading}
      >
        {#if loading}
          <span class="loading-dots">CONNECTING<span class="dots"></span></span>
        {:else}
          {mode === 'login' ? '[ CONNECT → ]' : '[ CREATE ACCOUNT → ]'}
        {/if}
      </button>
      <div class="login-footer">
        <span style="color:var(--text-dim)">v1.0.0</span>
      </div>
    </div>
  </div>
</div>
<style>
  .login-backdrop {
    position: fixed;
    inset: 0;
    background: var(--bg);
    display: flex;
    align-items: center;
    justify-content: center;
    overflow: hidden;
  }
  .noise {
    position: absolute;
    inset: 0;
    background-image:
      radial-gradient(circle at 20% 20%, rgba(0,255,65,0.03) 0%, transparent 50%),
      radial-gradient(circle at 80% 80%, rgba(0,255,65,0.02) 0%, transparent 50%);
    pointer-events: none;
  }
  .login-wrap {
    position: relative;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 24px;
    width: 100%;
    max-width: 620px;
    padding: 24px;
  }
  .ascii-header {
    text-align: center;
  }
  .logo {
    font-size: 10px;
    line-height: 1.2;
    letter-spacing: 0;
    color: var(--green);
    display: block;
    white-space: pre;
    overflow-x: auto;
  }
  @media (max-width: 768px) {
    .logo { font-size: 7px; }
  }
  @media (max-width: 540px) {
    .logo { font-size: 5px; }
  }
  @media (max-width: 380px) {
    .logo { font-size: 4px; }
  }
  @media (max-width: 300px) {
    .logo { font-size: 3.5px; }
  }
  .subtitle {
    font-size: 11px;
    color: var(--green-dim);
    letter-spacing: 0.2em;
    margin-top: 8px;
  }
  .login-box {
    width: 100%;
    max-width: 440px;
    background: var(--bg-panel);
    border: 1px solid var(--border);
    padding: 28px;
    display: flex;
    flex-direction: column;
    gap: 16px;
  }
  .mode-tabs {
    display: flex;
    align-items: center;
    gap: 10px;
    margin-bottom: 4px;
  }
  .tab {
    background: none;
    border: none;
    font-family: inherit;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0.1em;
    color: var(--text-muted);
    cursor: pointer;
    padding: 0;
    transition: color 0.1s;
  }
  .tab.active { color: var(--green); text-shadow: 0 0 8px rgba(0,255,65,0.5); }
  .tab:hover:not(.active) { color: var(--text-primary); }
  .tab-sep { color: var(--border); }
  .fields { display: flex; flex-direction: column; gap: 14px; }
  .field-group { display: flex; flex-direction: column; gap: 5px; }
  .field-label {
    font-size: 10px;
    letter-spacing: 0.12em;
    color: var(--text-muted);
    font-weight: 500;
  }
  .field-wrap {
    display: flex;
    align-items: center;
    border: 1px solid var(--border);
    background: var(--bg-input, #0d0d0d);
    transition: border-color 0.15s, box-shadow 0.15s;
  }
  .field-wrap:focus-within {
    border-color: var(--green-dim);
    box-shadow: 0 0 0 1px var(--green-dim), 0 0 8px rgba(0,255,65,0.15);
  }
  .field-prefix {
    padding: 10px 10px;
    color: var(--green-muted);
    font-size: 14px;
    border-right: 1px solid var(--border);
    user-select: none;
  }
  .field-input {
    flex: 1;
    background: transparent;
    border: none;
    padding: 10px 12px;
    font-family: inherit;
    font-size: 13px;
    color: var(--text-primary);
    outline: none;
    caret-color: var(--green);
  }
  .field-input::placeholder { color: var(--text-dim); }
  .field-input:disabled { opacity: 0.5; }
  .error-msg {
    font-size: 11px;
    color: #ff4444;
    letter-spacing: 0.05em;
    padding: 6px 10px;
    border: 1px solid rgba(255,68,68,0.3);
    background: rgba(255,68,68,0.05);
    display: flex;
    gap: 8px;
    align-items: center;
  }
  .error-msg.success {
    color: var(--green-dim);
    border-color: var(--green-muted);
    background: rgba(0,255,65,0.05);
  }
  .err-prefix { font-weight: 700; }
  .submit-btn {
    width: 100%;
    padding: 12px;
    background: var(--green);
    border: none;
    color: #000;
    font-family: inherit;
    font-size: 13px;
    font-weight: 700;
    letter-spacing: 0.1em;
    cursor: pointer;
    transition: all 0.2s;
    margin-top: 4px;
  }
  .submit-btn:hover:not(:disabled) {
    background: #00ff5a;
    box-shadow: 0 0 20px rgba(0,255,65,0.5), 0 0 40px rgba(0,255,65,0.2);
  }
  .submit-btn:disabled { opacity: 0.5; cursor: not-allowed; }
  .loading-dots .dots::after {
    content: '';
    animation: dotdot 1.2s steps(4,end) infinite;
  }
  @keyframes dotdot {
    0%   { content: ''; }
    25%  { content: '.'; }
    50%  { content: '..'; }
    75%  { content: '...'; }
    100% { content: ''; }
  }
  .login-footer {
    text-align: center;
    font-size: 10px;
    letter-spacing: 0.08em;
  }
  @keyframes slideUp {
    from { opacity: 0; transform: translateY(20px); }
    to   { opacity: 1; transform: translateY(0); }
  }
  .slide-up { animation: slideUp 0.5s cubic-bezier(.22,.61,.36,1) forwards; }
</style>
