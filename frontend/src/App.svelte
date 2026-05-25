<script>
  import { onMount } from 'svelte';
  import { get } from 'svelte/store';
  import { auth } from './stores/auth.js';
  import { api }  from './services/api.js';
  import Login        from './pages/Login.svelte';
  import BootSequence from './pages/BootSequence.svelte';
  import Chat         from './pages/Chat.svelte';
  import NotFound     from './pages/NotFound.svelte';
  import Maintenance  from './pages/Maintenance.svelte';
  let state = 'loading';
  let user  = null;
  let token = '';
  onMount(async () => {
    if (window.location.pathname !== '/') {
      state = '404';
      return;
    }
    try {
      const data = await api.me();          
      user  = data;
      token = data.token || '';
      auth.login(data, token);              
      state = 'boot';
    } catch (err) {
      if (err.isOffline) {
        state = 'maintenance';
        return;
      }
      const stored = get(auth);
      if (stored.user && stored.token) {
        user  = stored.user;
        token = stored.token;
        state = 'boot';
      } else {
        state = 'login';
      }
    }
  });
  function onLogin(event) {
    const { user: u, token: t } = event.detail;
    user  = u;
    token = t;
    auth.login(u, t);
    state = 'boot';
  }
  function onBootDone() {
    console.log("[App] changing state to chat"); state = 'chat';
  }
  function onLogout() {
    user  = null;
    token = '';
    auth.logout();
    state = 'login';
  }
</script>
{#if state === 'loading'}
  <div class="splash">
    <span class="glow">INITIALIZING...</span>
  </div>
{:else if state === 'login'}
  <Login on:login={onLogin} />
{:else if state === 'boot'}
  <BootSequence onDone={onBootDone} />
{:else if state === 'chat'}
  <Chat {user} {token} onLogout={onLogout} />
{:else if state === '404'}
  <NotFound />
{:else if state === 'maintenance'}
  <Maintenance />
{/if}
<style>
  .splash {
    position: fixed;
    inset: 0;
    display: flex;
    align-items: center;
    justify-content: center;
    background: var(--bg);
    font-size: 14px;
    color: var(--green);
    letter-spacing: 0.2em;
  }
  .glow {
    text-shadow: 0 0 8px var(--green), 0 0 20px rgba(0,255,65,0.4);
    animation: blink 1s step-end infinite;
  }
  @keyframes blink {
    0%, 100% { opacity: 1; }
    50%       { opacity: 0.3; }
  }
</style>
